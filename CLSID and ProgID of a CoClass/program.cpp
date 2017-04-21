/*
----------------------------------------------------
And now for some modern times :)
*/
#include "dbj.h"

class Guardian {
public:	

	typedef ITypeLib  * PTYPELIB;
	typedef ITypeInfo * PTYPEINFO;
	typedef TYPEATTR  * PTYPEATTR;

protected:

	PTYPELIB  pTypeLib = NULL;
	PTYPEINFO pTypeInfo = NULL;
	PTYPEATTR pTypeAttr = NULL;

	void cleanup()
	{
		if (pTypeAttr) pTypeLib->Release();		pTypeLib = NULL;
		if (pTypeAttr)	pTypeInfo->ReleaseTypeAttr(pTypeAttr);			pTypeAttr = NULL;
		if (pTypeInfo)	pTypeInfo->Release(); 	pTypeInfo = NULL;
	}

public:

	operator PTYPELIB  () { return pTypeLib;  }
	operator PTYPELIB *() { return &pTypeLib; }
	operator PTYPEINFO () { return pTypeInfo; }
	operator PTYPEATTR () { return pTypeAttr; }

	~ Guardian() {
		cleanup();
	}
};



/*
----------------------------------------------------
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// _bstr_t str;
	// HRESULT hr;
	std::wstring string_buff;  string_buff.resize( BUFSIZ, L'\0' );
	std::wstring typelib_path; typelib_path.resize(BUFSIZ, L'\0');
	Guardian guard;

	Guardian::PTYPELIB	pTypeLib = guard; // calls appropriate casting operators
	Guardian::PTYPEINFO pTypeInfo = guard;
	Guardian::PTYPEATTR pTypeAttr = guard;
	//

	typelib_path = L"c:\\dbj\\comdlg32.ocx" ;

	// This assumes that the .ocx file contains the type library information.
	// If it does not, we can replace the .ocx file with the .tlb file.
	dbj::CHECK( LoadTypeLib( const_cast<LPCOLESTR>(typelib_path.c_str()), & pTypeLib) );

	LPOLESTR prog_id = (LPOLESTR)CoTaskMemAlloc(BUFSIZ);

	UINT iCount = pTypeLib->GetTypeInfoCount();
	// There may be multiple types that are defined in a type library. 
	// Let us iterate through them to find the CoClasses that are defined.
	for (UINT i = 0; i < iCount; i++)
	{
		// Obtain ITypeInfo interface.
		dbj::CHECK(pTypeLib->GetTypeInfo(i, &pTypeInfo));
		// Obtain TYPEATTR structure.
		dbj::CHECK(pTypeInfo->GetTypeAttr(&pTypeAttr));

		// if this is a CoClass.
		if (TKIND_COCLASS == pTypeAttr->typekind) {
			memset(prog_id, BUFSIZ, L'\0');
			dbj::CHECK(ProgIDFromCLSID(pTypeAttr->guid, &prog_id));

			string_buff = L"Inside the component : \n\n";
			string_buff += typelib_path;
			string_buff += L"\n\nFound the PROGID:\n\n";
			string_buff += prog_id;
			// Display the ProgID.
			::MessageBoxW(NULL, string_buff.data(), L"clsid2progid (c) DBJ 2017", MB_OK);
		}
	}
	::CoTaskMemFree(prog_id);
		return 0;
}


/*
----------------------------------------------------
EOFILE
*/

