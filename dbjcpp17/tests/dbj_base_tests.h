#pragma once
/* Copyright 2017 dbj@dbj.org

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
#ifndef UNICODE
#error UNICODE not defined?
#endif
// #include <iostream>
#include <vector>
#include "../../dbjfm/dbjfm.h"
//#include "../dbjtree/dbjtreeprint.h"
//#include "../dbjtree/dbjtree.h"
// #include "../../dbjmodernprint/modernprint.h"
// #include "../../dbjlibload/dbjlibloadtest.h"

// #include <UnitTest++\UnitTest++.h>
#include "../../dbjgtest/gtest.h"

namespace {

	using namespace dbj ;
	using std::wstring;
	using std::wcout;

		TEST(dbj_basic_tests, the_std_way)
		{
			static const wchar_t* str = L"爆ぜろリアル！弾けろシナプス！パニッシュメントディス、ワールド！";
			std::string original_locale(std::setlocale(LC_ALL, NULL), 255);
			std::string user_locale(std::setlocale(LC_ALL, ""), 255);

			wcout << "\nUpon entering this test, locale was found to be: " << original_locale.data();
			wcout << "\nLocale will be now set to what the user of this machine has desired: " << user_locale.data();
			wcout.flush();
			wcout.imbue(std::locale());
			wcout << "\nThe length of [" << str << "] is " << std::wcslen(str);
			wcout << "\nIf output is [], then the new locale is not enough to display the desired string \n";
			wcout << "\nOk, let's try once more with the original locale" << std::setlocale(LC_ALL, original_locale.data());
			wcout.flush();
			wcout.imbue(std::locale(original_locale));
			wcout << "\nThe length of [" << str << "] is " << std::wcslen(str);
		}

		TEST(dbj_basic_tests, basic_test)
			{
			dbj::win::console::WideOut helper_;
			/*
			if locale is  "English_United Kingdom.1252" both strings bellow outpout as "|-+"
			*/
			const static wstring doubles = L"║═╚";
			const static wstring singles = L"│─└";
			static const wchar_t wendl{ L'\n' };

			wcout << L"\nDoubles: " << doubles;
			wcout << L"\nSingles: " << singles;
		}

		
		/*
		http://illegalargumentexception.blogspot.com/2009/04/i18n-unicode-at-windows-command-prompt.html#charsets_unicodeconsole
		*/
		TEST(dbj_basic_tests, basic_idea_test) 
		{
			auto writeAnsiChars = [] (HANDLE outhand_, char * ansi__ = 0)
			{
				char *ansi_pound = "\nANSI: \xA3\r\n"; //A3 == pound character in Windows-1252
				ansi_pound = ansi__ ? ansi__ : ansi_pound;
				WriteConsoleA(outhand_, ansi_pound, len2dword(ansi_pound), NULL, NULL);
			};

			auto writeUnicodeChars = [] (HANDLE outhand_, const wchar_t * widestr__ = 0)
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
			};
			const static wstring doubles = L"\nUnicode: ║═╚";
			const static wstring singles = L"\nUnicode: │─└";
			auto oh_ = ::GetStdHandle(STD_OUTPUT_HANDLE);
			DBJ_VERIFY(INVALID_HANDLE_VALUE != oh_);

			::SetConsoleOutputCP(1252);

			writeAnsiChars(oh_);
			writeUnicodeChars(oh_);
			writeUnicodeChars(oh_, doubles.data());
			writeUnicodeChars(oh_, singles.data());
		}

#if 0
		/*
		http://en.cppreference.com/w/cpp/locale/locale
		use no ios veriosn of this to check if we have 
		standard iso cpp behaviour on this platform

		MS-Windows character sets:

		Windows-1250 for Central European languages that use Latin script, 
		(Polish, Czech, Slovak, Hungarian, Slovene, Serbian, Croatian, Bosnian, Romanian and Albanian)
		Windows-1251 for Cyrillic alphabets
		Windows-1252 for Western languages
		Windows-1253 for Greek
		Windows-1254 for Turkish
		Windows-1255 for Hebrew
		Windows-1256 for Arabic
		Windows-1257 for Baltic languages
		Windows-1258 for Vietnamese
		*/
		TEST(dbj_basic_tests, __switch_to_user_locale__) {
			wprintf(  L"\n");
			printf(    "\nUser-preferred locale setting is %s" , std::locale().name().c_str() ) ;
			wprintf(  L"\non startup, the global locale is the \"C\" locale");
			wprintf(  L"\noutput the number 1000.01:  %.2f", 1000.01 ) ;
			auto enloc = std::locale("en-UK");
			printf("\nreplace the C++ global locale as well as the C locale with the user-preferred locale: %s", 
				enloc.name().data()
			);
			std::locale::global(enloc) ;
			wprintf(L"\nuse the new global locale for future wide character output");
			// std::wcout.imbue(std::locale())) ;
			wprintf( L"\noutput the same number again: %.2f", 1000.01 ) ;
			wprintf( L"\n");
		}
#endif
		TEST(dbj_basic_tests, __wide_out__) {

			win::console::WideOut wout;
			const static std::wstring doubles = L"║═╚";
			const static std::wstring singles = L"│─└";

			wout.print("\nOnly words: [%], and numbers: [%][%]", L"WORD", 12.34, 0x13);
			wout.print("\nAnd now the unicode squigly bits: [%][%]\n", doubles, singles);

			wprintf(L"\nWPRINTF attempt: [%s][%s]\n", doubles.data(), singles.data());

			// printf("\nPRINTF attempt: [%S][%S]\n", doubles.data(), singles.data());
		}

		 TEST(dbj_basic_tests, __find_first_of__) {

			 win::console::WideOut wout;

				const wchar_t format[] = { L"abra % ka % dabra" };
				const wchar_t placeholder[] = { L"%" };

				auto dbj = dbj::find_first_of(format, placeholder);

				EXPECT_NO_FATAL_FAILURE(&dbj);

				if (dbj < 0)
					DBJ_TRACE(L"\nPlaceholder %s not found in %s", placeholder, format);
				else
					DBJ_TRACE(L"\nFound placeholder \"%s\" in \"%s\", at position: %d", placeholder, format,
						static_cast<int>(dbj)
					);
		 }

		 /*
		 The requested string is this: кошка 日本国
		 */
		 TEST(dbj_basic_tests, __console_setmode_solution__) 
		 {
	     static const wchar_t request[] = { L"кошка 日本国" };
		 static const wchar_t request_coded[] = { L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n" };

			 dbj::win::console::WideOut wo;
			 // _setmode(_fileno(stdout), _O_U16TEXT);
			 wprintf(L"\nwprintf() result: %s\n", request);
			 // following crashes the UCRT (aka Universal CRT)
			 // printf ("\nprintf() result: %S\n",   L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");
			 wo.print("\nwo.print() result: %S\n", request);
		 }

		 TEST(dbj_basic_tests, __selection_sort__) 
		 {
			 win::console::WideOut wo;
			 std::vector<wchar_t> vec = { L'W', L'Z', L'Y', L'X', L'9', L'8', L'7' };
				wo.print("\n\nUnsorted %", range_to_string(vec, L':'));
				dbj::selection_sort(vec);
				wo.print("\nSorted %\n\n", range_to_string(vec, L':'));
		 }

} // eof dbj basic tests suite
#define DBJVERSION __DATE__ __TIME__
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION )
#undef DBJVERSION
