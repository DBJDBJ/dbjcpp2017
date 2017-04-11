#pragma once
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

/*
#define _O_TEXT        0x4000  // file mode is text (translated)
#define _O_BINARY      0x8000  // file mode is binary (untranslated)
#define _O_WTEXT       0x10000 // file mode is UTF16 (translated)
#define _O_U16TEXT     0x20000 // file mode is UTF16 no BOM (translated)
#define _O_U8TEXT      0x40000 // file mode is UTF8  no BOM (translated)
*/

typedef enum {
 TEXT    =    0x4000,  // file mode is text (translated)
 BINARY  =    0x8000,  // file mode is binary (untranslated)
 WTEXT   =    0x10000, // file mode is UTF16 (translated)
 U16TEXT =    0x20000, // file mode is UTF16 no BOM (translated)
 U8TEXT  =    0x40000 // file mode is UTF8  no BOM (translated)
} FileMode ;


DBJINLINE void RedirectIOToConsole(const FileMode & fmode_ = FileMode::TEXT )
{
	using namespace std;
	// maximum mumber of lines the output console should have
	static const WORD MAX_CONSOLE_LINES = 500;
	int		hConHandle;
	long	lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;
	// allocate a console for this app
	assert( TRUE == AllocConsole() );
	// set the screen buffer to be big enough to let us scroll text
	assert( 0 != GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo) );
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	assert(0 != SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize));

	// redirect unbuffered STDOUT to the console
	// https://msdn.microsoft.com/en-us/library/aa384267.aspx
	lStdHandle = HandleToLong(GetStdHandle(STD_OUTPUT_HANDLE));
	hConHandle = _open_osfhandle(lStdHandle, fmode_);
	assert(-1 != hConHandle);
	fp = _fdopen(hConHandle, "w");
	assert(NULL != fp);
	*stdout = *fp;
	assert(0 == setvbuf(stdout, NULL, _IONBF, 0));
	// redirect unbuffered STDIN to the console
	lStdHandle = HandleToLong(GetStdHandle(STD_INPUT_HANDLE));
	hConHandle = _open_osfhandle(lStdHandle, fmode_);
	assert(-1 != hConHandle);
	fp = _fdopen(hConHandle, "r");
	assert(NULL != fp);
	*stdin = *fp;
	assert(0 == setvbuf(stdin, NULL, _IONBF, 0));
	// redirect unbuffered STDERR to the console
	lStdHandle = HandleToLong(GetStdHandle(STD_ERROR_HANDLE));
	hConHandle = _open_osfhandle(lStdHandle, fmode_);
	assert(-1 != hConHandle);
	fp = _fdopen(hConHandle, "w");
	assert(NULL != fp);
	*stderr = *fp;
	assert(0 == setvbuf(stderr, NULL, _IONBF, 0));
	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
	ios::sync_with_stdio();
}

//End of File

/* End of File */