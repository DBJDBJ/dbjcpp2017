#pragma once
/*
Copyright 2017 dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#if ! defined(STR)
#define STR2(x) L##x
#define STR(x) STR2(x)
#endif
/* set to 1 if using com */
#define DBJCOM 0

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <combaseapi.h>
#include <stdio.h>
#include <wchar.h>
#include <io.h>
#include <fcntl.h>
#include <assert.h>

#include <cstdio>
#include <string>
#include <iostream>
#include <strstream>
#include <clocale>
#include <locale>
#include <codecvt>
#include <functional>
#include <algorithm>

#ifdef UNICODE
;
#else
#error UNICODE is mandatory for __FILE__ to compile
#endif

#define DBJINLINE static __forceinline 

// Taken from MODERN v1.26 - http://moderncpp.com
// Copyright (c) 2015 Kenny Kerr
#pragma region Independent debug things
#ifdef _DEBUG
#define DBJ_ASSERT assert
#define DBJ_VERIFY DBJ_ASSERT
#define DBJ_VERIFY_(result, expression) DBJ_ASSERT(result == expression)
template <typename ... Args>
DBJINLINE void DBJ_TRACE(wchar_t const * const message, Args ... args) noexcept
{
	wchar_t buffer[512] = {};
	DBJ_VERIFY( -1 != _snwprintf_s(buffer, 512, 512, message, (args) ...));
    OutputDebugStringW(buffer);
	
	// DBJ_VERIFY( wprintf(message, (args) ...) > 0 );
}
#else
// code dissapears
#define DBJ_ASSERT __noop
// code stays
#define DBJ_VERIFY(expression) (expression)
// code stays
#define DBJ_VERIFY_(result, expression) (expression)
// code dissapears
#define DBJ_TRACE __noop
#endif
#pragma endregion

#pragma region dbj_simple

namespace dbj {

	/* 
	DBJ created 2017-04-13
	DBJ: "I do not know for sure if this is my invention" 

	s1 and s2 are "any" ranges as defined in C++ standard
	find first s2 in s1
	return the position relative to the s1 begining
	return -1 if s2 not found in s1

	NOTE1: move semantics implementation not advised here
	<ode>template< typename S1, typename S2>	auto find_first_of( S1 && s1, S2 && s2) ;</code>
	Because in that case s1 and s2 are "Universal" references which always would require
	std::forward<S1>(s1) and std::forward<S2>(s2) 
	which implies copying or moving. And we do not need any of them in here. Just iteration.

	NOTE2: I could have applied assert_static() here in order to provide meaningfull error messages 
	if users do pass s1 or s2 which are not the ranges. But, I have decided not to. So in particular be 
	wary of trying to pass pointers to string literals; they are not C++ ranges:

	const wchar_t * format = L"abra % ka % dabra" ;
	const wchar_t * placeholder = L"%" ;

	auto dbj = dbj::find_first_of( format, placeholder );
	*/
	template< typename S1, typename S2>
	DBJINLINE auto find_first_of(const S1 & s1, const S2 & s2) {
		auto pos_ = std::find_first_of(
			std::begin(s1), std::end(s1),
			std::begin(s2), std::end(s2)
		);

		return (pos_ == std::end(s1) ? -1 : std::distance(std::begin(s1), pos_));
	}

	/*
	http://en.cppreference.com/w/cpp/algorithm/iter_swap
	DBJ: I will show here one very speculative optimization
	*/
	template<class ForwardIt> DBJINLINE
	void selection_sort(ForwardIt begin, ForwardIt end)
	{
		for (ForwardIt i = begin; i != end; ++i)
			std::iter_swap(i, std::min_element(i, end));
	}

	template<typename R> DBJINLINE void selection_sort(R & range_)
	{
		selection_sort(std::begin(range_), std::end(range_));
	}
    /* 
	quick and dirty "any" range to wstring 
	with no delimiters between elements
	*/
	template<typename R>
	DBJINLINE std::wstring range_to_string(const R & range_) {
		std::wstring ws(std::begin(range_), std::end(range_));
		return ws;
	}

	template<typename T> struct Rez
	{
	private:
		std::wstringstream ss;
		wchar_t delim;
	public:
		Rez(const wchar_t & d) : delim(d), ss() { }
		void operator( )(const T & e) { ss << e << delim ; }
		/* cut-off the last delimiter and return the result string */
		std::wstring str() const {
			std::wstring str = ss.str(); 
			str.resize( str.size() - 1);
			return str;
		}
	};
	/*
	"any" range to wstring,	with user defined  delimiter between elements
	default delim is ":"
	*/
	template<typename R>
	DBJINLINE std::wstring range_to_string(const R & range_, const wchar_t delim ) {

		typedef Rez< R::value_type > Result;
		Result retval = std::for_each(std::begin(range_), std::end(range_), Result(delim));
		return retval.str() ;
	}
	/*
	*/
	namespace str {
		
		// http://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string
		DBJINLINE std::wstring to_wide(const std::string& str)
		{
			using convert_typeX = std::codecvt_utf8<wchar_t>;
			static std::wstring_convert<convert_typeX, wchar_t> converterX;
			return converterX.from_bytes(str);
		}
		/*
		this is here for "symetry" ... dbj.org stuff is UNICODE only
		*/
		DBJINLINE std::string to_str (const std::wstring& wstr)
		{
			using convert_typeX = std::codecvt_utf8<wchar_t>;
			static std::wstring_convert<convert_typeX, wchar_t> converterX;
			return converterX.to_bytes(wstr);
		}

		/*
		"H" , "ello" yields "Hello"
		using this is actually faster than auto s = L"H" + L"ello"
		*/
		DBJINLINE std::wstring & prepend(const std::wstring s1, std::wstring & s2)
		{
			return s2.insert(0, s1);
		}

	} // eof namespace str

	/*
	DBJ Exception
	*/
	class exception
	{
		std::wstring dbj_exception_data_;
	public:

		exception() throw()
			: dbj_exception_data_()
		{
		}

		explicit exception(wchar_t const* const _Message) throw()
			: dbj_exception_data_(_Message)
		{
		}

		/* copy from std::exception */
		exception(const std::exception & ws) throw() {
			dbj_exception_data_ = str::to_wide(ws.what());
		}

		exception(const std::wstring & _Message ) throw()
			: dbj_exception_data_(_Message)
		{
		}

		exception(exception const& _Other) throw()
			: dbj_exception_data_()
		{
			dbj_exception_data_ = _Other.dbj_exception_data_;
		}

		exception& operator=(exception const& _Other) throw()
		{
			if (this == &_Other) return *this;

			dbj_exception_data_.clear() ;
			dbj_exception_data_ = _Other.dbj_exception_data_;
			return *this;
		}

		virtual ~exception() throw()
		{
			dbj_exception_data_.clear();
		}

		virtual wchar_t const* what() const
		{
			return dbj_exception_data_.data() ? dbj_exception_data_.data() : L"Unknown exception";
		}
	};

} // eof dbj
  
  /*
  Stuff bellow is C. This may look like a kludge. But it is beautifully simple and
  working solutions when compared to the same C code wrapped into C++11.
  */

enum { dbj_simple_BUFFER_SIZE = 512 }; // windows BUFSIZ = 512

DBJINLINE const wchar_t * const dbj_simple_lastError(const wchar_t* msg)
{
	DWORD eNum;
	std::wstring sysMsg; sysMsg.resize(dbj_simple_BUFFER_SIZE);

	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		&(sysMsg[0]), 256, NULL);
#if 0
	wchar_t* p;
	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));
#endif
	static unsigned const bufsiz_ = dbj_simple_BUFFER_SIZE * 2;
	static wchar_t buf[bufsiz_] = L"";
	// clean the previous message if any
	wmemset(buf, L'\0', sizeof(buf) / sizeof(wchar_t));
	// Make the message
	assert(std::swprintf(buf, bufsiz_, TEXT("\n\t%s failed with error\n %d (%s)\n\n"), msg, eNum, sysMsg.data()) > 0);
	// Display the message to stdout 
	// if inside WIN32 app this will "do nothing"
	wprintf(buf);
	// Return the message
	return buf;
}

// Get the current working directory
extern "C" DBJINLINE const wchar_t * const dbj_simple_current_dir(LPCWSTR lpPathName) {
	static TCHAR infoBuf[dbj_simple_BUFFER_SIZE];
	assert(GetCurrentDirectory(dbj_simple_BUFFER_SIZE, infoBuf));
	return infoBuf;
}

// Set to current working directory
extern "C" DBJINLINE void dbj_simple_set_current_dir(LPCWSTR lpPathName) {
	assert(SetCurrentDirectory(lpPathName));
}

// Get and display the Windows directory.
extern "C" DBJINLINE const wchar_t * const dbj_simple_windows_dir() {
	static TCHAR infoBuf[dbj_simple_BUFFER_SIZE];
	assert(GetWindowsDirectory(infoBuf, dbj_simple_BUFFER_SIZE));
	return infoBuf;
}

// Get and display the system directory.
extern "C" DBJINLINE const wchar_t * const dbj_simple_system_dir() {
	static TCHAR infoBuf[dbj_simple_BUFFER_SIZE];
	assert(GetSystemDirectory(infoBuf, dbj_simple_BUFFER_SIZE));
	return infoBuf;
}
#pragma endregion dbj_simple

namespace dbj {
#pragma region single counter
	typedef void(*voidvoidfun) ();
	template< voidvoidfun ATSTART, voidvoidfun ATEND>
	struct __declspec(novtable)
		GLOBAL_BEGIN_END
	{
		unsigned int & counter()
		{
		static unsigned int counter_ = 0;
		return counter_;
		}

		GLOBAL_BEGIN_END() {
			const UINT & ctr = (counter())++;
			if (0 == ctr) {
				// do something once and upon construction
				ATSTART()(); 
			}
		}
		~GLOBAL_BEGIN_END() {
			const UINT & ctr = --(counter());
			if (0 == ctr) {
				// before destruction do something once
				ATEND()();
			}
		}
	};
	/* 
	Following creates unique type as long as it is not repeated
	somewehere elase in the same app
	-------------------------------------------------------------
	void f1 () { printf("Start once!"); } ;
	void f2 () { printf("End   once!"); } ;
	static GLOBAL_BEGIN_END<f1,f2> the_counter__;
	*/
#pragma endregion single counter

	/* string length size_t to DWORD to pacify the MSVC */
	DBJINLINE DWORD len2dword (const wchar_t * ws_) {
		return static_cast<DWORD>(std::wcslen(ws_));
	}

	DBJINLINE DWORD len2dword(const char * st_) {
		return static_cast<DWORD>(std::strlen(st_));
	}

	DBJINLINE auto chr2str(const char & c_) {
		static char str[2] = {0x00, 0x00};
		str[0] = c_;
		return str;
	};


	namespace win {
		namespace console {
/*
Not FILE * but HANDLE based output.
It also uses Windows.1252 Code Page.
This two are perhaps why this almost always works.

https://msdn.microsoft.com/en-us/library/windows/desktop/dd374122(v=vs.85).aspx

Even if you get your program to write UTF16 correctly to the console, 
Note that the Windows console isn't UTF16 friendly and may just show garbage.
*/
struct __declspec(novtable)	WideOut
	{
	HANDLE output_handle_;
	UINT   previous_code_page_;
	public:
		WideOut()
		{
			this->output_handle_ = ::GetStdHandle(STD_OUTPUT_HANDLE);
			DBJ_VERIFY(INVALID_HANDLE_VALUE != this->output_handle_);
				previous_code_page_  =::GetConsoleOutputCP();
				DBJ_VERIFY( 0 != ::SetConsoleOutputCP(1252) );
			/*			TODO: GetLastError()			*/
			}

		~WideOut()
		{
			DBJ_VERIFY(0 != ::SetConsoleOutputCP(previous_code_page_));
			// TODO: should we "relase" this->output_handle_ ?
			/*			TODO: GetLastError()  		*/
		}

		template<typename N>
		void operator () (const N & number_) {
					// static_assert( std::is_arithmetic<N>::value);
			std::wstring sn_ = std::to_wstring(number_);
			DBJ_VERIFY(0 != ::WriteConsoleW(this->output_handle_, sn_.data(), len2dword(sn_.data()), NULL, NULL));
		}

		void operator () (const wchar_t * const wp_) {
// last 2 args: no of chars, to in write (DWORD) and in-out written (LPDWORD)
#if _DEBUG
			DWORD to_write = len2dword(wp_);
			DWORD written;
			WriteConsoleW(this->output_handle_, wp_, to_write, &written, NULL);

			DBJ_VERIFY(to_write == written);
#else
			DBJ_VERIFY(0 != ::WriteConsoleW(this->output_handle_, wp_, len2dword(wp_), NULL, NULL));
#endif
		}

		void operator () (const std::wstring & wp_) {
#if _DEBUG
			DWORD to_write = len2dword(wp_.data());
			DWORD written;
			WriteConsoleW(this->output_handle_, wp_.data(), to_write, &written, NULL);

			DBJ_VERIFY(to_write == written);
#else
			DBJ_VERIFY(0 != ::WriteConsoleW(this->output_handle_, wp_.data(), len2dword(wp_), NULL, NULL));
#endif		
		}

		void operator () (const wchar_t & wp_) {
			wchar_t str[] = { wp_, L'\0' };
			DBJ_VERIFY(0 != ::WriteConsoleW(this->output_handle_, str, len2dword(str), NULL, NULL));
		}

		void operator () (const char & wp_) {
			char str[] = { wp_, '\0' };
			DBJ_VERIFY(0 != ::WriteConsoleA(this->output_handle_, str, len2dword(str), NULL, NULL));
		}

		void operator () (const char * const wp_) {
			DBJ_VERIFY(0 != ::WriteConsoleA(this->output_handle_, wp_, len2dword(wp_), NULL, NULL));
		}

		void operator () (const std::string & wp_) {
			DBJ_VERIFY(0 != ::WriteConsoleA(this->output_handle_, wp_.data(),
				(DWORD)wp_.size(),
				NULL, NULL));
		}
		/*
		http://en.cppreference.com/w/cpp/language/parameter_pack
		*/
		void print(const char * format) // base function
		{
			(*this)(format);
		}
		/*
			Primitive print(). Tries to handle "words" and "numbers".
			'%' is a replacement token
			No type designators
			No field width or precision values
		*/
		template<typename T, typename... Targs>
		void print(const char* format, T value, Targs... Fargs) // recursive variadic function
		{
			for (; *format != '\0'; format++) {
				if (*format == '%') {
					this->operator()(value);
					print(format + 1, Fargs...); // recursive call
					return;
				}
				this->operator()( *format ); // this calls with 'const char'
			}
        }

};


} // console
#if DBJCOM		
		namespace com {
			namespace {
				/*
				In anonymous namespace we hide the auto-initializer
				This ensures that COM is initialized “as soon as possible”
				This mechanism really works. Convince yourself once through the
				debugger, and then just forget about COM init/uninit.
				*/
				struct __declspec(novtable)
					COMAUTOINIT
				{
					unsigned int & counter()
					{
						static unsigned int counter_ = 0;
						return counter_;
					}
					/*
					If you call ::CoInitialize(NULL), after this method is used
					most likely the HRESULT will be :
					hRes = 0×80010106 — Cannot change thread mode after it is set.
					*/
					COMAUTOINIT()
					{
						const UINT & ctr = (counter())++;
						if (0 == ctr)
#if ( defined(_WIN32_DCOM)  || defined(_ATL_FREE_THREADED))
							HRESULT result = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
							HRESULT result = ::CoInitialize(NULL);
#endif
						/*TODO: log the result here*/
					}

					~COMAUTOINIT()
					{
						const UINT ctr = --(counter());
						if (ctr < 1)
							::CoUninitialize();
					}

				};
				static const COMAUTOINIT wtlcomautoinit__{};
			} // anonspace
		} // com
#endif // DBJCOM
	} // win
} // dbj

  /*
  If problems with them, one can undef the macros defined in here

  #undef DBJINL // static __forceinline
  #undef WIN32_LEAN_AND_MEAN
  #undef VC_EXTRALEAN
  */

#if 0
WIN console is not unicode by default
thus console output of unicode string will not produce what is expected
For example :
const static std::wstring doubles = L"║═╚";
const static std::wstring singles = L"│─└";

std::wcout << doubles;

will not work is the following low level intialisatoon of console has not been
performed

The following is a "hack" and shouild not be applied to the global app env
It should be used inside functions in a local manner.
Neither "C" API or C++ stdlib locales, can work normaly if this is executed.
Actualy no ANSI output will work at all.
For example the following will throw runtime exceptions

printf("C API says, locale setting is: %s\n", setlocale(LC_ALL, NULL));
std::wcout << "C++ std lib says, user-preferred locale setting is " << std::locale("").name().c_str() << '\n';
// on startup, the global locale is the "C" locale, if not set otherwise
std::wcout << L"\nNumber formating under 'C' locale: " << 1000.01 << wendl;
std::wcout << L"\nreplace the C++ global locale as well as the C locale with the user-preferred locale\n";
std::locale::global(std::locale(""));
printf("C API says, locale setting is now: %s\n", setlocale(LC_ALL, NULL));
std::wcout << L"\nuse the new global locale for future wide character output";
std::wcout.imbue(std::locale());
#endif
#define DBJVERSION __DATE__ __TIME__
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION )