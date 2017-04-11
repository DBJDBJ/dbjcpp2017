#include "../dbjfm/dbjfm.h"
#include "guicon.h"
/*
This produces no output. Unless using: dbj::win::console::WideOut
Which uses no standard handlers (stdin, stdout, stderr)
*/
int APIENTRY wWinMain(
	_In_		HINSTANCE hInstance,
	_In_opt_	HINSTANCE hPrevInstance,
	_In_		LPWSTR    lpCmdLine,
	_In_		int       nCmdShow
)
{
	using namespace std;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	RedirectIOToConsole();
	using dbj::win::console::WideOut;
	WideOut dbj;
#if 0
	// this works but uses no standard handles
	const static wstring doubles = L"\n║═╚";
	const static wstring singles = L"\n│─└";
	dbj.print("%",doubles);
	dbj.print("%",singles);
#endif
	int iVar;
	// test stdio
	assert(0 >= fprintf(stdout, "Test output to stdout\n") );
	assert(0 >=  fprintf(stderr, "Test output to stderr\n") );
	assert(0 >=  fprintf(stdout, "Enter an integer to test stdin: ") );
#pragma warning ( push )
#pragma warning ( disable: 4996 )
	scanf("%d", &iVar);
#pragma warning ( pop )
	printf("You entered %d\n", iVar);
	//test iostreams
	cout << "Test output to cout" << endl;
	cerr << "Test output to cerr" << endl;
	clog << "Test output to clog" << endl;
	cout << "Enter an integer to test cin: ";
	cin >> iVar;
	cout << "You entered " << iVar << endl;

	// test wide iostreams
	wcout << L"Test output to wcout" << endl;
	wcerr << L"Test output to wcerr" << endl;
	wclog << L"Test output to wclog" << endl;
	wcout << L"Enter an integer to test wcin: ";
	wcin  >> iVar;
	wcout << L"You entered " << iVar << endl;

	// test CrtDbg output
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_RPT0(_CRT_WARN, "\nThis is testing _CRT_WARN output\n");
	_RPT0(_CRT_ERROR, "\nThis is testing _CRT_ERROR output\n");
	_ASSERT(0 && "\ntesting _ASSERT");
	_ASSERTE(0 && "\ntesting _ASSERTE");

	unsigned mili_secs_ = 5000;
		dbj.print("\n\nDone! Exiting in % seconds", std::to_string( mili_secs_/1000) );
		Sleep(mili_secs_);
	return 0;
}

//End of File