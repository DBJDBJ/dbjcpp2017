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
	dbj::test_deletors();
	// _bstr_t str;
	// HRESULT hr;
	WCHAR* prog_id = NULL;
	std::wstring string_buff { BUFSIZ };
	Guardian guard;

	Guardian::PTYPELIB	pTypeLib = guard; // calls appropriate casting operators
	Guardian::PTYPEINFO pTypeInfo = guard;
	Guardian::PTYPEATTR pTypeAttr = guard;
	//
	std::wstring typelib_path ;

	typelib_path = L"c:\\dbj\\comdlg32.ocx" ;

#ifdef _DEBUG
	const TCHAR * see_mee = typelib_path.c_str();
#endif
	// This assumes that the .ocx file contains the type library information.
	// If it does not, we can replace the .ocx file with the .tlb file.
	dbj::CHECK( LoadTypeLib( const_cast<LPCOLESTR>(typelib_path.c_str()), & pTypeLib) );

	UINT iCount = pTypeLib->GetTypeInfoCount();
	// There may be multiple types that are defined in a type library. 
	// Let us iterate through them to find the CoClasses that are defined.
	for (UINT i = 0; i < iCount; i++)
	{
		// Obtain ITypeInfo interface.
		dbj::CHECK(pTypeLib->GetTypeInfo(i, &pTypeInfo));
		// Obtain TYPEATTR structure.
		dbj::CHECK(pTypeInfo->GetTypeAttr(&pTypeAttr));

		prog_id = const_cast<wchar_t *>(string_buff.c_str());
		// if this is a CoClass.
		if (TKIND_COCLASS == pTypeAttr->typekind)
			dbj::CHECK(ProgIDFromCLSID(pTypeAttr->guid, &prog_id));
				// Display the ProgID.
				::MessageBox(NULL, prog_id, prog_id, MB_OK);

	}
		return 0;
}


/*
----------------------------------------------------
EOFILE
*/

