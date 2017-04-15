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

#include <UnitTest++\UnitTest++.h>

SUITE( dbj_basic_tests ) {

	using namespace dbj ;

		 TEST(find_first_of) {

			 win::console::WideOut wout;

				const wchar_t format[] = { L"abra % ka % dabra" };
				const wchar_t placeholder[] = { L"%" };

				auto dbj = dbj::find_first_of(format, placeholder);

				REQUIRE CHECK(&dbj);

				if (dbj < 0)
					DBJ_TRACE(L"\nPlaceholder %s not found in %s", placeholder, format);
				else
					DBJ_TRACE(L"\nFound placeholder \"%s\" in \"%s\", at position: %d", placeholder, format,
						static_cast<int>(dbj)
					);

				DBJ_TRACE(L"\nFinished: %s, file: %s", TEXT(__FUNCTION__), TEXT(__FILE__));
		 }

		 TEST(selection_sort) {
			 win::console::WideOut wout;
			 
			 std::vector<wchar_t> vec = { L'W', L'Z', L'Y', L'X', L'9', L'8', L'7' };

				DBJ_TRACE(L"\n\nUnsorted %s", vec);
				dbj::selection_sort(vec);
				DBJ_TRACE(L"\nSorted %s", vec);
				DBJ_TRACE(L"\nFinished: %s, file: %s", TEXT(__FUNCTION__), TEXT(__FILE__));
		 }

} // eof dbj basic tests suite
#define DBJVERSION __DATE__ __TIME__
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION )
#undef DBJVERSION
