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
	using wbstr = dbjsys::fm::bstr::wbstr  ;

	const wchar_t * const wsP = L" + WIDE STRING POINTER" ;
	wchar_t ws [] = L" + WIDE STRING" ;

	
	wbstr b1(wsP) ;
	wbstr b2(ws)  ;

	// b1 = std::wstring(L" + STD::WSTRING") ;

	// 
	b1 = b2 ;
}

//-----------------------------------------------------------------

static void fun ( long a, long b) 
{
	long c = a == b ;
}


//-----------------------------------------------------------------------
// eofile
//-----------------------------------------------------------------------

