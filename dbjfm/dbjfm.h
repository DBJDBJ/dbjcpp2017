﻿#pragma once
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
#define STR2(x) L#x
#define STR(x) STR2(x)
#endif
/* set to 1 if using com */
#define DBJCOM 0
/* seto ti 1 to test Policy-es */
#define DBJPOLICY 0

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include <combaseapi.h>
#include <crtdbg.h>

#include <io.h>
#include <fcntl.h>

#include <string>
#include <iostream>
#include <clocale>

#define DBJINLINE static __forceinline 

// Taken from MODERN v1.26 - http://moderncpp.com
// Copyright (c) 2015 Kenny Kerr
#pragma region
#ifdef _DEBUG
#define DBJ_ASSERT _ASSERTE
#define DBJ_VERIFY DBJ_ASSERT
#define DBJ_VERIFY_(result, expression) DBJ_ASSERT(result == expression)
template <typename ... Args>
DBJINLINE void DBJ_TRACE(wchar_t const * const message, Args ... args) noexcept
{
	wchar_t buffer[512] = {};
	swprintf_s(buffer, message, args ...);
	OutputDebugString(buffer);
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

namespace dbj {
#if 0
	struct __declspec(novtable)
		COUNTER
	{
		unsigned int & counter()
		{
		static unsigned int counter_ = 0;
		return counter_;
		}

		COUNTER() {
			const UINT & ctr = (counter())++;
			if (0 == ctr) {
				// make something once
			}
		}
		~COUNTER() {
			const UINT & ctr = --(counter());
			if (0 == ctr) {
				// destroy something once
			}
		}
	};
	// static COUNTER the_counter__;
#endif
#if DBJPOLICY
	namespace policy {
		using std::wstring;

		struct DefaultDeletingPolicy {
			template <typename O> 
			  void del (O * op_) const {
				  op_ ? std::default_delete<O>(op_):(void); op_ = nullptr;
			}

			  template <typename O>
			  void del(O * op_ [] ) const {
				  op_ ? std::default_delete<O[]>(op_) : (void); op_ = nullptr;
			  }

			  template <typename O>
			  void del(O & op_) const {
				  if (!std::is_empty<O>::value)
					  op_.~O();
			  }
		};
		/*
		optimization of
		https://en.wikipedia.org/wiki/Policy-based_design
		*/
		/*
		OPT 1: use default template arguments
		LOGIC: make API simpler. simpler code leads to less bugs
		*/
class WideWriterPolicy;
class LanguagePolicyEnglish;
class LanguagePolicyGerman;

template <typename LanguagePolicy = LanguagePolicyEnglish , typename OutputPolicy = WideWriterPolicy >
class HelloWorld 
{

public:
	// Behaviour method
	void run( ) const
	{
		// not made before this point; if ever.
		static LanguagePolicy language{};
		static OutputPolicy printer{};
		// Two policy methods
		printer.print(language.message());
	}
};

		struct WideWriterPolicy
		{
			template<typename MessageType>
			void print(MessageType const &message) const
			{
				std::wcout << message << std::endl;
			}
		};

		struct LanguagePolicyEnglish
		{
			wstring message() const
			{
				return L"Hello, World!";
			}
		};

		struct LanguagePolicyGerman
		{
			wstring message() const
			{
				return L"Hallo Welt!";
			}
		};

	} // policy
#endif

	/* string length size_t to DWORD to pacify the MSVC */
	DBJINLINE DWORD len2dword (const wchar_t * ws_) {
		return static_cast<DWORD>(std::wcslen(ws_));
	}

	DBJINLINE DWORD len2dword(const char * st_) {
		return static_cast<DWORD>(std::strlen(st_));
	}

	namespace win {
		namespace console {

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
			/*
			TODO: GetLastError()
			*/
			}

		~WideOut()
		{
			DBJ_VERIFY(0 != ::SetConsoleOutputCP(previous_code_page_));
			// TODO: should we "relase" this->output_handle_ ?
			/*
			TODO: GetLastError()
  		*/
		}
		void operator () (const wchar_t * const wp_) {
			DBJ_VERIFY( 0 != ::WriteConsoleW(this->output_handle_, wp_, len2dword(wp_), NULL, NULL));
		}
	};

	using std::wstring;
	using std::wcout;

	DBJINLINE void the_std_way() {

		static const wchar_t* str = L"爆ぜろリアル！弾けろシナプス！パニッシュメントディス、ワールド！";
		std::string original_locale(std::setlocale(LC_ALL, NULL), 255);
		std::string user_locale(std::setlocale(LC_ALL, ""), 255);

		wcout << "\nUpon entering this test, locale was found to be: " << original_locale.data() ;
		wcout << "\nLocale will be now set to what the user of this machine has desired: " << user_locale.data() ;
		wcout.flush();
		wcout.imbue(std::locale());
		wcout << "\nThe length of [" << str << "] is " << std::wcslen(str) ;
		wcout << "\nIf output is [], then the new locale is not enough to display the desired string \n";
		wcout << "\nOk, let's try once more with the original locale" << std::setlocale(LC_ALL, original_locale.data());
		wcout.flush();
		wcout.imbue(std::locale(original_locale));
		wcout << "\nThe length of [" << str << "] is " << std::wcslen(str);
	}
	/*
	This crashes the app with no way to catch the exception 
	ucrtbased.dll is the problem in this case
	*/
	DBJINLINE void test_crash_console_output() {
					
	try {
					
		fflush(stdout);
		_setmode(_fileno(stdout), _O_U16TEXT);

		printf("CRASH BANG!");

	}
	catch (...) {

		fflush(stdout);
		_setmode(_fileno(stdout), _O_TEXT);

		printf("NEVER REACHED :( ucrtbased.dll stops the show...");

	}
}

	DBJINLINE void test_wide_output() {
					WideOut helper_;
				/*
				if locale is  English_United Kingdom.1252 both strings bellow outpout as "|-+" 
				*/
				const static wstring doubles = L"║═╚";
				const static wstring singles = L"│─└";
				static const wchar_t wendl{L'\n'};
				
				wcout << L"\nDoubles: " << doubles;
				wcout << L"\nSingles: " << singles;
			}
				
	DBJINLINE void writeAnsiChars(HANDLE outhand_ , char * ansi__ = 0 )
				{
					::SetConsoleOutputCP(1252);

					char *ansi_pound = "\nANSI: \xA3\r\n"; //A3 == pound character in Windows-1252
					ansi_pound = ansi__ ? ansi__  : ansi_pound;
					WriteConsoleA(outhand_ , ansi_pound, len2dword(ansi_pound), NULL, NULL);
				}

	DBJINLINE void writeUnicodeChars(HANDLE outhand_,  const wchar_t * widestr__ = 0 )
				{
					if (widestr__) {
						WriteConsoleW(outhand_, widestr__, len2dword(widestr__), NULL, NULL);
						return;
					}
					wchar_t *arr[] =
					{
						L"\nUnicode:",
						L"\u00A3", //00A3 == pound character in UTF-16
						L"\u044F", //044F == Cyrillic Ya in UTF-16
						L"\n",   //CRLF
						0
					};

					for (int i = 0; arr[i] != 0; i++)
					{
						WriteConsoleW(outhand_, arr[i], len2dword(arr[i]), NULL, NULL);
					}
				}
	/*
	http://illegalargumentexception.blogspot.com/2009/04/i18n-unicode-at-windows-command-prompt.html#charsets_unicodeconsole
	*/
	DBJINLINE void basic_test()
				{
					const static wstring doubles = L"\nUnicode: ║═╚";
					const static wstring singles = L"\nUnicode: │─└";
					auto oh_ = ::GetStdHandle(STD_OUTPUT_HANDLE);
					DBJ_VERIFY(INVALID_HANDLE_VALUE != oh_);

					writeAnsiChars(oh_);
					writeUnicodeChars(oh_);
					writeUnicodeChars(oh_,doubles.data());
					writeUnicodeChars(oh_,singles.data());
				}
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

#if DBJPOLICY
	namespace test {
		void dbj_policy_testing () {
			using namespace dbj::policy;

			/*OPT1 WideWriterPolicy is default*/
			typedef HelloWorld<> HelloWorldEnglish;

			/* english is default */
			HelloWorld<> hello_world;
			hello_world.run(); // prints "Hello, World!"

			/* Use another language policy */
			/*OPT1 WideWriterPolicy is default*/
			typedef HelloWorld<LanguagePolicyGerman> HelloWorldGerman;

			HelloWorldGerman hello_world2;
			hello_world2.run(); // prints "Hallo Welt!"
		}
	}// test
#endif
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