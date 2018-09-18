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
//#include <strstream>
#include <sstream>
#include <clocale>
#include <locale>
#include <codecvt>
#include <functional>
#include <algorithm>

#ifndef UNICODE
#error UNICODE is mandatory for __FILE__ to compile
#endif

#define DBJINLINE __forceinline 

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
	HRESULT ui{};
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
	DBJINLINE std::wstring implode_to_string(const R & range_) {
		std::wstring ws(std::begin(range_), std::end(range_));
		return ws;
	}
	/*
	functor accumulator ... see the range_to_string() 
	*/
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
	DBJINLINE std::wstring range_to_string(const R & range_, const wchar_t delim = ':' ) {

		typedef Rez< R::value_type > Result;
		Result retval = std::for_each(std::begin(range_), std::end(range_), Result(delim));
		return retval.str() ;
	}
	// reverse and copy the result but for "any" range that compiler will "take" when calling this
	template< typename RANGE>
	DBJINLINE void reverse_copy(RANGE & reversed_copy, const RANGE & original)
	{
		using namespace std;
		reverse_copy(begin(original), end(original), begin(reversed_copy));
		// no copy result version
	}
	template< typename RANGE>
	DBJINLINE RANGE reverse_copy(const RANGE & original)
	{
		using namespace std;
		RANGE reversed_copy;
		reverse_copy(reversed_copy, original);
		return reversed_copy;
		// result copy, version
	}
	/*
	string utilities
	*/
	namespace str {
		namespace {
			// case insensitive string compare
			// (much) faster than std way
			class NocaseStrEqual {
			public:
				const bool operator()(const std::string & x, const std::string & y) const
				{
					const char * lpString1 = x.data();
					const char * lpString2 = y.data();

					int result = CompareStringA(
						LOCALE_SYSTEM_DEFAULT,// locale identifier
						NORM_IGNORECASE,      // comparison-style options
						lpString1,            // first string
						(long)x.size(),             // size of first string
						lpString2,            // second string
						(long)y.size()              // size of second string
					);
					DBJ_VERIFY(result);
					return result == CSTR_EQUAL;
				}

				const bool operator()(const std::wstring & x, const std::wstring & y) const
				{
					const wchar_t * lpString1 = x.data();
					const wchar_t * lpString2 = y.data();

					size_t result = CompareStringW(
						LOCALE_SYSTEM_DEFAULT,// locale identifier
						NORM_IGNORECASE,      // comparison-style options
						lpString1,            // first string
						(long)wcslen(lpString1),             // size of first string
						lpString2,            // second string
						(long)wcslen(lpString2)              // size of second string
					);
					DBJ_VERIFY(result);
					return result == CSTR_EQUAL;
				}
			};
		}
		/*
		public func calls the hidden implementation 
		NOTE: not safe in presence of multiple threads
		*/
		template< typename T >
		DBJINLINE
			bool compareNoCase(const T & s1, const T & s2)
		{
			static NocaseStrEqual  comparator_instance;
			return comparator_instance(s1, s2);
		}

		//----------------------------------------------------------------------------
		/*
		http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
		*/
		DBJINLINE std::string trim(const std::string &s)
		{
			using namespace std;

			auto  wsfront = find_if_not(s.begin(), s.end(), [](int c) {return isspace(c); });
			return string(
				wsfront,
				find_if_not(s.rbegin(),
					string::const_reverse_iterator(wsfront), [](int c) {return isspace(c); }
				).base()
			);
		}

		DBJINLINE std::wstring trim(const std::wstring &s)
		{
			using namespace std;

			auto  wsfront = find_if_not(s.begin(), s.end(), [](int c) {return iswspace(c); });
			return wstring(
				wsfront,
				find_if_not(s.rbegin(),
					wstring::const_reverse_iterator(wsfront), [](int c) {return iswspace(c); }
				).base()
			);
		}
		
		DBJINLINE std::wstring to_wide(const std::string& str)
		{
			return std::wstring(str.begin(), str.end());
/*
// http://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string
using convert_typeX = std::codecvt_utf8<wchar_t>;
			static std::wstring_convert<convert_typeX, wchar_t> converterX;
			return converterX.from_bytes(str);
*/
		}
		/*
		this is here for "symetry" ... dbj.org stuff is UNICODE only
		*/
		DBJINLINE std::string to_str (const std::wstring& wstr)
		{
			return std::string(wstr.begin(), wstr.end());
/*
			using convert_typeX = std::codecvt_utf8<wchar_t>;
			static std::wstring_convert<convert_typeX, wchar_t> converterX;
			return converterX.to_bytes(wstr);
			*/
		}

		/*
		"H" , "ello" yields "Hello"
		using this is actually faster than auto s = L"H" + L"ello"
		*/
		DBJINLINE std::wstring & prepend(const std::wstring s1, std::wstring & s2)
		{
			return s2.insert(0, s1);
		}
#if 0
		/* string length size_t to DWORD to pacify the MSVC */
		DBJINLINE DWORD len2dword(const wchar_t * const ws_) {
			return static_cast<DWORD>(std::wcslen(ws_));
		}

		DBJINLINE DWORD len2dword(const char * const st_) {
			return static_cast<DWORD>(std::strlen(st_));
		}
#else
		template<size_t N>
		DBJINLINE DWORD len2dword(const wchar_t (&ws_)[N]) {
			return static_cast<DWORD>(N);
		}

		template<size_t N>
		DBJINLINE DWORD len2dword(const char (&st_)[N]) {
			return static_cast<DWORD>(N);
		}
		DBJINLINE DWORD len2dword(const std::wstring & ws ) {
			return static_cast<DWORD>(ws.size());
		}

		DBJINLINE DWORD len2dword(const std::string & ws) {
			return static_cast<DWORD>(ws.size());
		}
#endif

		template <typename C>
		DBJINLINE auto chr2str(const std::char_traits<C> & c_) {
			static C str[2] = { C(), C() };
			str[0] = c_;
			return str;
		};

	} // eof namespace str

	/*
	DBJ Exception
	*/
	class exception
	{
		mutable std::wstring dbj_exception_data_{ L"Unknown Exception" };

		exception();
		/* copy from std::exception */
		exception(const std::exception & ws) throw() {
			dbj_exception_data_ = str::to_wide(ws.what());
		}

		exception(exception const& _Other) throw()
			: dbj_exception_data_()
		{
			dbj_exception_data_ = _Other.dbj_exception_data_;
		}

		exception& operator=(exception const& _Other) throw()
		{
			if (this == &_Other) return *this;

			dbj_exception_data_.clear();
			dbj_exception_data_ = _Other.dbj_exception_data_;
			return *this;
		}

	public:

		explicit exception(wchar_t const* const _Message) throw()
			: dbj_exception_data_(_Message)
		{
		}

		exception(const std::wstring & _Message ) throw()
			: dbj_exception_data_(_Message)
		{
		}

		virtual ~exception() throw()
		{
			dbj_exception_data_.clear();
		}

		virtual wchar_t const* what() const
		{
			return (!dbj_exception_data_.empty() ? dbj_exception_data_.data() : L"Unknown exception");
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
	TCHAR infoBuf[dbj_simple_BUFFER_SIZE];
	assert(GetCurrentDirectory(dbj_simple_BUFFER_SIZE, infoBuf));
	return infoBuf;
}

// Set to current working directory
extern "C" DBJINLINE void dbj_simple_set_current_dir(LPCWSTR lpPathName) {
	assert(SetCurrentDirectory(lpPathName));
}

// Get and display the Windows directory.
extern "C" DBJINLINE const wchar_t * const dbj_simple_windows_dir() {
	TCHAR infoBuf[dbj_simple_BUFFER_SIZE];
	assert(GetWindowsDirectory(infoBuf, dbj_simple_BUFFER_SIZE));
	return infoBuf;
}

// Get and display the system directory.
extern "C" DBJINLINE const wchar_t * const dbj_simple_system_dir() {
	TCHAR infoBuf[dbj_simple_BUFFER_SIZE];
	assert(GetSystemDirectory(infoBuf, dbj_simple_BUFFER_SIZE));
	return infoBuf;
}
#pragma endregion dbj_simple




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
#define DBJVERSION __FILE__ __DATE__ __TIME__
#pragma message( "Compiling Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION )
#undef  DBJVERSION
