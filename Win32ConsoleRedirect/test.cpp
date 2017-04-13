
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
int APIENTRY test_console_redirect(
)
{
	using namespace std;
	int iVar;
	RedirectIOToConsole( FileMode::U16TEXT);
	
	// this works but uses no standard stdio handles
	const static wstring doubles = L"\n║═╚" ;
	const static wstring singles = L"\n│─└" ;
	std::wprintf(L"%s\n",doubles.data());
	std::wprintf(L"%s\n",singles.data());

	// test stdio throws the exception
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

int APIENTRY wWinMain(
	_In_		HINSTANCE hInstance,
	_In_opt_	HINSTANCE hPrevInstance,
	_In_		LPWSTR    lpCmdLine,
	_In_		int       nCmdShow
)
{
	// Convert char* string to a wchar_t* string.  
	auto cv = [](const char * const orig) {
		auto newsize = ::strlen(orig) + 1;
		static std::wstring wcstring = {};
		wcstring.clear();
		wcstring.resize(newsize);
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, &wcstring[0], newsize, orig, _TRUNCATE);
		return wcstring.data();
	};

	auto mb = [](const wchar_t * const msg_) {
		int msgboxID = MessageBoxW(
			NULL,
			(LPCWSTR)msg_,
			(LPCWSTR)TEXT(__FILE__),
			MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
		);
	};
	try {
		UNREFERENCED_PARAMETER(hInstance);
		UNREFERENCED_PARAMETER(hPrevInstance);
		UNREFERENCED_PARAMETER(lpCmdLine);
		UNREFERENCED_PARAMETER(nCmdShow);

		return test_console_redirect();
	}
	catch (std::exception sex_) {
		mb(cv(sex_.what()));
	}
	catch (const wchar_t * sex_) {
		mb(sex_);
	}
	catch (const char * sex_) {
		mb(cv(sex_));
	}
	catch (...) {
		mb(L"Unknown Exception");
	}
	return 0;
}
	//End of File