//*****************************************************************************/
//
//                  
//
//                 Copyright (c)  1997 - 2015 by Dusan B. Jovanovic (dbj@dbj.org) 
//                          All Rights Reserved
//
//        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF Dusan B. Jovanovic (dbj@dbj.org)
//
//        The copyright notice above does not evidence any
//        actual or intended publication of such source code.
//
//  $Author: DBJ  $
//  $Date: $
//  $Revision: $
//*****************************************************************************/
// This is based on the code found at http://www.codeguru.com
// FileVersion.h: interface for the file_version class. by Manuel Laflamme
//////////////////////////////////////////////////////////////////////

#pragma once

#pragma comment(lib, "version")

#ifdef _DEBUG
#undef THIS_FILE
const static bstr_t THIS_FILE(__FILE__);
#endif

//*****************************************************************************/
namespace dbjsys {
	namespace fm {

		// 2018SEP25 DBJ changed to wstring version from _bstr_t
		inline std::wstring format( const wchar_t * pszFormat, ...)
		{
			static auto BUFSIZE_ = 1024U;
			va_list arglist;
			va_start(arglist, pszFormat);
			std::wstring buff( BUFSIZE_ + 1 , wchar_t(0) );
			int rtn = wvsprintfW(&buff[0], pszFormat, arglist);
			DBJ_VERIFY(rtn < buff.size());
			va_end(arglist);
			return buff;
		}

		//------------------------------------------------------------------
		// use this to have meaningfull error message if module from which
		// version info is required can not be located
		// 
		template< typename E >
		struct check_module_loadability
		{
			// 
			HMODULE mhandle;
			// 
			check_module_loadability(LPCWSTR file_name)
				: mhandle(0)
			{
				dbjTHROWIF(0 == (mhandle = LoadLibrary(file_name)), E);
			}
			// 
			~check_module_loadability()
			{
				FreeLibrary(mhandle);
			}
		};

// 
class file_version final
{ 
public: 
	
	typedef Error<file_version> Err ;
	typedef Win32Error<file_version> WinErr ;

	explicit file_version( const wchar_t * modulename /* = L"" */ ) 
		: version_data_( NULL ) 
		, lang_charset_(0)
		, module_name_(modulename)
	{ 
		this->Open(module_name_);
	}

	~file_version() 
	{ 
		Close();
	} 

	bstr_t GetFileDescription() { return QueryValue(L"FileDescription"); };
	bstr_t GetFileVersion() { return QueryValue(L"FileVersion"); };
	bstr_t GetInternalName() { return QueryValue(L"InternalName"); };
	bstr_t GetCompanyName() { return QueryValue(L"CompanyName"); };
	bstr_t GetLegalCopyright() { return QueryValue(L"LegalCopyright"); };
	bstr_t GetOriginalFilename() { return QueryValue(L"OriginalFilename"); };
	bstr_t GetProductName() { return QueryValue(L"ProductName"); };
	bstr_t GetProductVersion() { return QueryValue(L"ProductVersion"); };

private:
	void Close()
	{
		if ( NULL == version_data_ ) delete[] version_data_; 
		version_data_ = NULL;
		lang_charset_ = 0;
	}

	//------------------------------------------------------------------
	bool Open(LPCWSTR lpszModuleName)
	{
		Close();

		_ASSERT(lpszModuleName != 0);
		_ASSERT(version_data_ == NULL);

		// Get the version information size for allocate the buffer
		DWORD dwHandle = NULL;
		DWORD dwDataSize = NULL;

		check_module_loadability<WinErr> cml(lpszModuleName);

		if (0 == (dwDataSize = ::GetFileVersionInfoSize((LPWSTR)lpszModuleName, &dwHandle))
			)
			dbjTHROWERR(L"no versioning resource found in the executable module");

		// Allocate buffer and retrieve version information
		version_data_ = new BYTE[dwDataSize];
		if (!::GetFileVersionInfo((LPWSTR)lpszModuleName, dwHandle, dwDataSize,
			(void**)version_data_))
		{
			Close();
			throw  WinErr(__FILE__, __LINE__);
		}

		// Retrieve the first language and character-set identifier
		UINT nQuerySize;
		DWORD* pTransTable;
		if (!::VerQueryValue(version_data_, L"\\VarFileInfo\\Translation",
			(void **)&pTransTable, &nQuerySize))
		{
			Close();
			throw  WinErr(__FILE__, __LINE__);
		}

		// Swap the words to have lang-charset in the correct format
		lang_charset_ = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));

		return TRUE;
	}
	//------------------------------------------------------------------
	bstr_t QueryValue(LPCWSTR lpszValueName,
		DWORD dwLangCharset = 0 )
	{
		// Must call Open() first
		if (this->version_data_ == NULL)
		{
			if (module_name_.length() > 1)
				this->Open(module_name_);
			else
				dbjTHROWERR(L"resource has to be opened, but module name was not given");
		}

		if (this->version_data_ == NULL)
			return L"";

		// If no lang-charset specified use default
		if (dwLangCharset == 0)
			dwLangCharset = this->lang_charset_;

		// Query version information value
		UINT nQuerySize = 0;
		LPVOID lpData = 0;
		bstr_t strValue;
		
		std::wstring strBlockName= format(L"\\StringFileInfo\\%08lx\\%s", dwLangCharset, lpszValueName);

		dbjTHROWIF(
			0 == ::VerQueryValue(
			(void **)version_data_,
				(wchar_t *)strBlockName.c_str(), &lpData, &nQuerySize)
			, WinErr);

		strValue = (LPCWSTR)lpData;

		return strValue;
	}


	//------------------------------------------------------------------
	bool GetFixedInfo(VS_FIXEDFILEINFO& vsffi)
	{
		// Must call Open() first
		_ASSERT(version_data_ != NULL);
		if (version_data_ == NULL)
			return FALSE;

		UINT nQuerySize;
		VS_FIXEDFILEINFO* pVsffi;
		if (::VerQueryValue((void **)version_data_, L"\\",
			(void**)&pVsffi, &nQuerySize))
		{
			vsffi = *pVsffi;
			return TRUE;
		}

		return FALSE;
	}
	//------------------------------------------------------------------
	std::wstring GetFixedFileVersion()
	{
		std::wstring strVersion;
		VS_FIXEDFILEINFO vsffi;

		if (GetFixedInfo(vsffi))
		{
			strVersion = format(L"%u,%u,%u,%u", HIWORD(vsffi.dwFileVersionMS),
				LOWORD(vsffi.dwFileVersionMS),
				HIWORD(vsffi.dwFileVersionLS),
				LOWORD(vsffi.dwFileVersionLS));
		}
		return strVersion;
	}
	//------------------------------------------------------------------
	std::wstring GetFixedProductVersion()
	{
		std::wstring strVersion;
		VS_FIXEDFILEINFO vsffi;

		if (GetFixedInfo(vsffi))
		{
			strVersion = format(L"%u,%u,%u,%u", HIWORD(vsffi.dwProductVersionMS),
				LOWORD(vsffi.dwProductVersionMS),
				HIWORD(vsffi.dwProductVersionLS),
				LOWORD(vsffi.dwProductVersionLS));
		}
		return strVersion;
	}
	//------------------------------------------------------------------

// Attributes
private :
    LPBYTE  version_data_; 
    DWORD   lang_charset_; 
	bstr_t  module_name_   ;
}; // file_version

//*****************************************************************************/
	} //	namespace fm 
} //namespace dbjsys 
//*****************************************************************************/


