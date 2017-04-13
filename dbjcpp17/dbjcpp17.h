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
#include <string>
#include <vector>
#include "../dbjfm/dbjfm.h"
//#include "../dbjtree/dbjtreeprint.h"
//#include "../dbjtree/dbjtree.h"
#include "../dbjmodernprint/modernprint.h"
#include "../dbjlibload/dbjlibloadtest.h"



namespace dbj { namespace test {

	namespace {
		using std::wstring;
		typedef std::function<wstring()> TestUnit;
		typedef std::vector<TestUnit> AllUnits;

		static AllUnits test_units = {
			[] {
			  const wchar_t format[] = {L"abra % ka % dabra"};
			  const wchar_t placeholder[] = {L"%"};

			  auto dbj = dbj::find_first_of(format, placeholder);

			  if (dbj < 0)
				  print::F(L"Placeholder % not found in %", placeholder, format);
			  else
				  print::F(L"Found placeholder \"%\" in \"%\", at position: %", placeholder, format,
					  static_cast<int>(dbj)
				  );

			return L"OK: T1 -from->" __FILE__ "\tFunction: " TEXT(__FUNCTION__);
			}
		};
	} // anon ns
	DBJINLINE void do_the_tests()
	{
		for (auto tunit : test_units) {
			 print::F(L"\n%", tunit().data());
		}
	}
} } // eof dbj::test
#define DBJVERSION __DATE__ __TIME__
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION )
#undef DBJVERSION
