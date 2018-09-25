//-----------------------------------------------------------------------
//
// $Author: Muh $
//
// $Revision: 31 $
//
//

#include "precomp.h"

#include "test_declarations.h"

namespace {

void show_help_screen( const wchar_t * exe_name, std::wostream & os = std::wclog )
{
		using namespace dbj::test ; 
		
		dbjsys::fm::set_prompt_stream( os ) ;

		dbgout(L"%s",  L"------------------------------------------------" ) ;
		dbgout(L"%s",  exe_name ) ;
		dbgout(L"%s",  L"\nDBJ*FM++ 'micro-testing' application" ) ;
		dbgout(L"%s",  L"(c)2001-2006-2017 by DBJ@DBJ.ORG \n" ) ;
		dbgout(L"%s",  L"------------------------------------------------" ) ;
		dbgout(L"%s",  L"Usage:" ) ;
		dbgout(L"%s",  L"Please provide one argument tagged with '-t'" ) ;
		dbgout(L"%s",  L"to state which test should be  executed." ) ;
		dbgout(        L"For example:\n %s -t %s\n",  exe_name, L"test6" ) ;
		dbgout(L"%s",  L"If test name given is '~', then ALL of the tests\n will be executed." ) ;
		dbgout(L"%s",  L"If NO test name is given, then it is considered to be '~'\n" ) ;
		dbgout(L"%s",  L"Other arguments considered and their effects:" ) ;
		dbgout(L"%s",  L"-ALL \tALL of the tests will be executed" ) ;
		dbgout(L"%s",  L"-?   \tThe list of available tests" ) ;
		dbgout(L"%s",  L"\nNO arguments at all: this help screen\n" ) ;
		dbgout(L"%s",  L"NOTE: Command line tags are case sensitive." ) ;
		dbgout(L"%s",  L"------------------------------------------------" ) ;
}
 void show_test_names ( const dbjsys::char_type * exe_name, std::wostream & os = std::wclog )
{
		using namespace dbj::test ; 
		
		dbjsys::fm::set_prompt_stream( os ) ;

		dbgout(L"%s",  L"------------------------------------------------" ) ;
		dbgout(L"%s",  exe_name ) ;
		dbgout(L"%s",  L"------------------------------------------------" ) ;
		dbgout(L"%s",  L"\nTests currently available:\n" ) ;
			testing_.show_registered_tests(os) ;
		dbgout(L"%s",  L"------------------------------------------------" ) ;
}
//-----------------------------------------------------------------------
} // namespace


//-----------------------------------------------------------------------
//
// Output:
//
// All normal output goes to stdout
//
// Return codes:
//
// 0	all is  fine
// 1    no arguments or  wrong argument
// 2    exception cought
//
//-----------------------------------------------------------------------

int wmain ( int argc, wchar_t ** argv )
{
	using bart = dbjsys::fm::bstr::nbstr ;

	dbjsys::fm::cli_argument_string::Type 
		testname_; 

	try {
		// testing_.run("wrap_test", std::wcout);
		// testing_.run("wrapwrap_test", std::wcout);

			show_test_names(argv[0], std::wcout);
			// show_help_screen(argv[0], std::wcout);


		if (testing_.number_of_registered_tests() < 1)
		{
			dbj::test::dbgout(L"%s --  %s", argv[0], L"No tests registered?");
			return 2;
		}

		testname_ = L"~"; // do them ALL

		try {
			testing_.run( testname_.data(), std::wcout);
		}
		catch (wchar_t * x) {
			dbj::test::dbgout(L"%s ERROR --  %s", argv[0], x);
			return 2;
		}
	}
	dbjPOPERR
	catch (const std::exception & x) {
		dbjsys::fm::algo::errBox( TEXT(__FILE__) L"\n%S", x.what() );
	}
	catch (wchar_t * x) {
		dbj::test::dbgout(L"%s ERROR --  %s", argv[0], x);
		return 2;
	}
	return 0 ;
}
// EOF

