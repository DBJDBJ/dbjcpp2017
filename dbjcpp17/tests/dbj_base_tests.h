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

		TEST(dbj_basic_tests, __wide_out__) {

			win::console::WideOut wout;
			const static std::wstring doubles = L"║═╚";
			const static std::wstring singles = L"│─└";

			wout.print("\nOnly words: [%], and numbers: [%][%]", L"WORD", 12.34, 0x13);
			wout.print("\nAnd now the unicode squigly bits: [%][%]\n", doubles, singles);

			wprintf(L"\nWPRINTF attempt: [%s][%s]\n", doubles.data(), singles.data());

			printf("\nPRINTF attempt: [%S][%S]\n", doubles.data(), singles.data());
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

		 template<typename R>
		 DBJINLINE std::wstring range_to_string(const R & range_) {
			 std::wstring ws(std::begin(range_),std::end(range_));
			 return ws;
		 }

		 TEST(dbj_basic_tests, __selection_sort__) 
		 {
			 win::console::WideOut wo;

			 std::vector<wchar_t> vec = { L'W', L'Z', L'Y', L'X', L'9', L'8', L'7' };
				wo.print("\n\nUnsorted %", range_to_string(vec));
				dbj::selection_sort(vec);
				wo.print("\nSorted %\n\n", range_to_string(vec));

				
		 }

} // eof dbj basic tests suite
#define DBJVERSION __DATE__ __TIME__
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION )
#undef DBJVERSION
