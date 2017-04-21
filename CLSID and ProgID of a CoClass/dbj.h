#pragma once
/*
https://support.microsoft.com/en-us/kb/286340
*/
// #include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <comdef.h>
#include <string>
#include <memory>
#include <functional>
#include <vector>

#define DBJINLINE static __forceinline

/*
----------------------------------------------------
*/
namespace dbj {
	
	DBJINLINE std::wstring windir() {
		TCHAR windir[MAX_PATH];
		GetWindowsDirectory(windir, MAX_PATH);
		return windir;
	}

	DBJINLINE wchar_t * LPWSTR( const std::wstring & wstr ) {
		return const_cast<wchar_t *>(wstr.c_str());
	}

	/*
	----------------------------------------------------
	*/
	DBJINLINE void DisplayError(HRESULT hr)
	{
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, hr,
			// Default language.
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			// Process any inserts into the lpMsgBuf.
			(LPTSTR)&lpMsgBuf, 0, NULL
		);
		// LPWSTR szTemp = WCHAR[100];
		static std::wstring tempstr; tempstr = L""; tempstr.reserve(1024);
		wsprintf(
			dbj::LPWSTR(tempstr.c_str()), L"%s\n", lpMsgBuf
		);
		::MessageBox(NULL, dbj::LPWSTR(tempstr), dbj::LPWSTR(tempstr), MB_OK);
	}

	DBJINLINE  void TError(HRESULT hr) {
		DisplayError(hr);
		::ExitProcess(hr);
	}

	DBJINLINE void CHECK( HRESULT hr_) {
		if (FAILED(hr_))
			TError(hr_);
	}

}


