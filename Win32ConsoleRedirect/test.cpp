#include "../dbjfm/dbjfm.h"
#include "guicon.h"

DBJINLINE void test_std_io()
{
	FILE * pFile;
	int n;
	char name[100] = {};

	assert( 0 == fopen_s( &pFile, __FILE__".txt", "w"));
	for (n = 0; n<3; n++)
	{
		puts("please, enter a name: ");
		assert( NULL != gets_s(name));
		fprintf(pFile, "Name %d [%-10.10s]\n", n + 1, name);
	}
	
	fclose(pFile);
}
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
	int iVar;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	RedirectIOToConsole();
	using dbj::win::console::WideOut;
	WideOut dbj;

	// this works but uses no standard stdio handles
	const static wstring doubles = L"\n║═╚" ;
	const static wstring singles = L"\n│─└" ;
	dbj.print("%\n",doubles);
	dbj.print("%\n",singles);

	// test stdio
	test_std_io();

	assert(0 >=  fprintf(stdout, "Test output to stdout\n") );
	assert(0 >=  fprintf(stderr, "Test output to stderr\n") );
	assert(0 >=  fprintf(stdout, "%S", L"Test wide output to stdout\n"));
	assert(0 >=  fprintf(stderr, "%S", L"Test wide output to stderr\n"));

	assert(0 >= fprintf(stdout, "Enter an integer to test stdin: "));
#pragma warning ( push )
#pragma warning ( disable: 4996 )
	scanf("%d", &iVar);
#pragma warning ( pop )
	printf("You entered %d\n", iVar);

	// test CrtDbg output
	_CrtSetReportMode(_CRT_ASSERT,	_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT,	_CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR,	_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR,	_CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_WARN,	_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN,	_CRTDBG_FILE_STDERR);
	_RPT0(_CRT_WARN, "\nThis is testing _CRT_WARN output\n");
	_RPT0(_CRT_ERROR, "\nThis is testing _CRT_ERROR output\n");
	_ASSERT(0 && "\ntesting _ASSERT");
	_ASSERTE(0 && "\ntesting _ASSERTE");

	unsigned mili_secs_ = 15000;
		dbj.print("\n\nDone! Exiting in % seconds", std::to_string( mili_secs_/1000) );
		Sleep(mili_secs_);
	return 0;
}

//End of File