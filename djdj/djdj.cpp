// djdj.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <functional> // std::ref

/*
typedef struct _tagpropertykey
{
	GUID fmtid;
	DWORD pid;
} 	PROPERTYKEY;
*/
/*
LPOLESTR is wchar_t *
*/
#ifndef LPOLESTR
#define LPOLESTR wchar_t *
#endif
class guardian {
protected:
	LPOLESTR lpolestr;
public:
	guardian() : lpolestr(NULL) { 	}
	guardian(LPOLESTR olestr) : lpolestr(olestr) { 	}
	~guardian() { if (lpolestr) {	CoTaskMemFree(lpolestr); lpolestr = NULL;}	}
	operator LPOLESTR () const noexcept { return this->lpolestr; }
	auto operator & () const noexcept { return (LPOLESTR FAR *)&(this->lpolestr); }
};

wchar_t * guid2str ( _In_ const IID &  global_uid_ )
{
	static guardian lpolestr;
	auto doublepinter = &lpolestr;
	StringFromCLSID( global_uid_, doublepinter );
	return (LPOLESTR)lpolestr ;
}

typedef std::reference_wrapper<PROPERTYKEY> PROPERTYKEYREF;

void show_property_key (PROPERTYKEYREF kref) 
{
	auto prop = kref.get();
	printf("\n\n|%64s| PROPERTYKEY", " ");
	printf("\n|%64d| PID", prop.pid   ) ;
	printf("\n|%64S| FMTID\n", guid2str(prop.fmtid)) ;
}

int main()
{
	PROPERTYKEY pk = { __uuidof(0), 2 };
	PROPERTYKEYREF rf = pk;
	show_property_key(rf);
// using the operator PROPERTYKEY & ()
printf("\n\n\nPID|%32d|", ((PROPERTYKEYREF::type)PROPERTYKEYREF(rf)).pid);
return 0;
}

