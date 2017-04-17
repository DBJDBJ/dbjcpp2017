// WideConsoleSolution.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
/*
The requested string is this: кошка 日本国
*/
static const wchar_t request[] = {L"кошка 日本国"};
static const wchar_t request_coded[] = { L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n" };

int main(void) {
	dbj::win::console::WideOut wo;
	// _setmode(_fileno(stdout), _O_U16TEXT);
	wprintf(L"\nwprintf() result: %s\n", request );
	// following crashes the UCRT (aka Universal CRT)
	// printf ("\nprintf() result: %S\n",   L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");
	wo.print("\nwo.print() result: %S\n", request);
	return 0;
}
