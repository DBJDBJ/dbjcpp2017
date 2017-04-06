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
// Portions of registry API usage inspired by WinReg(c) of Giovanni Dicanio <giovanni.dicanio@gmail.com>


#include "dbjcpp17.h"

/*
*/
int main()
{
	using std::wcout;
	using std::wcerr;
	using std::wstring;
	    wcout << "\n\nDBJCPP17 -- Modern C++ 2017 by DBJDBJ <dbj@dbj.org>  \n\n";
	try {
		dbj::test::do_the_tests();
		dbj::print::test::do_the_tests();
#if 0		
		dbj::win::console::the_std_way();
		dbj::win::console::test_crash_console_output();
		dbj::win::console::test_wide_output();
		dbj::win::console::basic_test();
#if DBJPOLICY
		dbj::test::dbj_policy_testing();
#endif
		dbj::tree::binary_node_test(wcout);
#endif
	}
	catch (std::exception & ex_) {
		wcerr << "std::exception: " << ex_.what() << "\n" ;
	}
	catch (const char * const ex_) {
		wcerr << "exception: " << ex_ << "\n";
	}
	catch (... ) {
		wcerr << L"Unknown exception: \n" ;
	}
	// eof main
	return 0;
}
