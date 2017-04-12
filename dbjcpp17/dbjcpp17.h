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

	using std::string;
	typedef std::function<string()> TestUnit;
	typedef std::vector<TestUnit> AllUnits;

	/* DBJ added 
	   find first s2 in s1
	   return the position relative to the s1 begining
	   return -1 if s2 not found in s1
	*/
	template< typename S1, typename S2>
	auto find_first_of(const S1 & s1, const S2 & s2) {
		auto pos_ = std::find_first_of(
			std::begin(s1), std::end(s1),
			std::begin(s2), std::end(s2)
		);

		return ( pos_ == std::end(s1) ? -1 : std::distance( std::begin(s1), pos_ ) );
	}

	static AllUnits test_units = {
		[] {
		  const char format[] = {"abra % ka % dabra"};
		  const char placeholder[] = {"%"};

		  auto dbj = find_first_of(format, placeholder);

		  if (dbj < 0 )
			  dbj::print::Print("Placeholder % not found in %", placeholder, format);
		  else
			  dbj::print::Print("Found placeholder \"%\" in \"%\", at position: %", placeholder, format, 
				  static_cast<int>(dbj)
			  );

		return "OK: T1 -from->" __FILE__ ;
		} 
	};

	DBJINLINE void do_the_tests()
	{
		for (auto tunit : test_units) {
			 dbj::print::Print("\n%", tunit().data());
		}
	}
} } // eof dbj::test
#define DBJVERSION __DATE__ __TIME__
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION )
