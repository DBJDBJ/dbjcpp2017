
#include "guicon.h"



DBJINLINE void test_std_io()
{
	FILE *		pFile;
	int			n;
	wchar_t		name[100] = {};

	assert( 0 == fopen_s( &pFile, __FILE__".txt", "w"));
	for (n = 0; n<3; n++)
	{
		puts("please, enter a name: ");
		assert( NULL !=  std::fgetws(name, 100, stdin));
		fwprintf(pFile, L"Name %d [%-10.10s]\n", n + 1, name);
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
	

	// this works but uses no standard stdio handles
	const static wstring doubles = L"\n║═╚" ;
	const static wstring singles = L"\n│─└" ;
	std::wprintf(L"%s\n",doubles.data());
	std::wprintf(L"%s\n",singles.data());

	// test stdio
	test_std_io();

	assert(0 >= fwprintf(stdout, L"Test wide output to stdout\n") );
	assert(0 >= fwprintf(stderr, L"Test wide output to stderr\n") );

	assert(0 >= fwprintf(stdout, L"Enter an integer to test stdin: "));
#pragma warning ( push )
#pragma warning ( disable: 4996 )
	scanf("\n%d", &iVar);
#pragma warning ( pop )
	std::wprintf(L"You entered %d\n", iVar);

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
	std::wprintf(L"\n\nDone! Exiting in %3d seconds", ( mili_secs_/1000) );
		Sleep(mili_secs_);
	return 0;
}

//End of File