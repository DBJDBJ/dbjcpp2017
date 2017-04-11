#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

typedef enum {
 TEXT    =    0x4000,  // file mode is text (translated)
 BINARY  =    0x8000,  // file mode is binary (untranslated)
 WTEXT   =    0x10000, // file mode is UTF16 (translated)
 U16TEXT =    0x20000, // file mode is UTF16 no BOM (translated)
 U8TEXT  =    0x40000 // file mode is UTF8  no BOM (translated)
} FileMode ;

/*
For x64 (MSV)C++ issues and solutions see:
https://msdn.microsoft.com/en-us/library/aa384267.aspx
*/
DBJINLINE void RedirectIOToConsole(const FileMode & fmode_ = FileMode::TEXT )
{
	// maximum mumber of lines the output console should have
	static const WORD MAX_CONSOLE_LINES = BUFSIZ ;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	// allocate a console for this app
	assert( TRUE == AllocConsole() );
	// set the screen buffer to be big enough to let us scroll text
	assert( 0 != GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo) );
	
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	coninfo.dwMaximumWindowSize.Y = MAX_CONSOLE_LINES; // DBJ added

	assert(0 != SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize));

	auto redirector = [] (FILE * standard_stream, const FileMode & fmode_, DWORD handle_marker_ ) {
		long lStdHandle = HandleToLong(GetStdHandle(handle_marker_));
		int   hConHandle = _open_osfhandle(lStdHandle, fmode_);
		assert(-1 != hConHandle);
		FILE * fp = _fdopen(hConHandle, "w");
		assert(NULL != fp);
		*standard_stream = *fp;
		assert(0 == setvbuf(standard_stream, NULL, _IONBF, 0));
	};

	redirector(stdout, fmode_, STD_OUTPUT_HANDLE);
	redirector(stdin,  fmode_, STD_INPUT_HANDLE );
	redirector(stderr, fmode_, STD_ERROR_HANDLE );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
	std::ios::sync_with_stdio();
}
