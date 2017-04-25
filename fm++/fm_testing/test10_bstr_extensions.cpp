#include "precomp.h"

/*
------------------------------------------------------------------------------------
*/
DBJINLINE
bstr_t & operator + ( bstr_t & b1, const bstr_t & b2 )
{
		b1.operator + ( b2 ) ;
		return b1 ;
}
/*
------------------------------------------------------------------------------------
*/
extern "C" void test10_bstr_extensions()
{
	using namespace dbjsys::fm::bstrex ;

	bstr bstr ;
	const wchar_t * const wsP = L" + WIDE STRING POINTER" ;
	wchar_t ws [] = L" + WIDE STRING" ;

	
	bstr = wsP ;
	bstr = ws  ;

	bstr = std::wstring(L" + STD::WSTRING") ;

	// 
	bstr = bstr ;
}

//-----------------------------------------------------------------

static void fun ( long a, long b) 
{
	long c = a == b ;
}


//-----------------------------------------------------------------------
// eofile
//-----------------------------------------------------------------------

