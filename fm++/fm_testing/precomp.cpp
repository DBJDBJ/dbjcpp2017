
#include "precomp.h"
// #pragma comment( lib, "kernel32.lib" )
//
//--------------------------------------------------------------------------------
namespace {
	// actually not required ...
	struct  _INITOR_ 
	{
		static UINT & counter() {
			static UINT counter_;
			return counter_;
		}

	_INITOR_ () 
	{
		// use and increment
		if ( counter() ++ == 0  )
		{
			// void * vp = ::malloc( sizeof( DBJtest ) ) ;
			// testing_ = new(vp) DBJtest() ;

#if _DEBUG
			const size_t nort = testing_.number_of_registered_tests() ;
#endif
		}
	}
	~_INITOR_ () 
	{
		// decrement and use
		if (  -- counter() == 0 )
		{
		}
	}
	} ;

	static  _INITOR_	__dbjsys_test_initor__ ;
} // namespace
//--------------------------------------------------------------------------------
// 
// #pragma init_seg( "__DBJSYSTEMS_TESTING_INITIALIZATION__" )
// #pragma warning( disable : 4073 ) 
// #pragma init_seg( lib )
// static dbj::test::implementation::sequence_type * dbj::test::implementation::sequence = 0;
// static dbj::test::DBJtest testing_;

