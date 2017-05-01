#include "precomp.h"

namespace wrapper_test {
	using namespace dbjsys;
	//----------------------------------------------------------------------
	// simplest prefix and suffix functions
	inline void prefix() { fm::prompt(L"Prefix"); };
	//----------------------------------------------------------------------
	inline void suffix() { fm::prompt(L"Suffix"); };
	//----------------------------------------------------------------------
	// user defined class
	typedef std::vector< std::wstring > string_vector;
	//--------------------------------------------------------------------
	typedef dbjsys::fm::Wrap< string_vector, void(*)(), void(*)() > SV_fwrapped;
	//----------------------------------------------------------------------
	inline void robust_wrap_1()
	{
		SV_fwrapped   xx = SV_fwrapped::make(&prefix, &suffix);
		xx->push_back(L"One");
		xx->push_back(L"Two");
		xx->push_back(L"Three");
	}
	//----------------------------------------------------------------------
	// And now not so simple but extremely usefull wrap-up!
	//-------------------------------------------------
	typedef dbjsys::fm::WWrap< std::list<bstr_t> > WVector;
	//-------------------------------------------------
	template< class T> inline void val2prompt(const T & t_)
	{
		dbjsys::fm::prompt(t_);
	}
	//
	template void val2prompt<bstr_t>(const bstr_t & t_);
	//
	// 
	template< class T> struct ValMatch // Predicate 
	{
		// 
		const T & to_find;
		// 
		ValMatch(const T & lf) : to_find(lf) {}
		// 
		bool operator () (const T & t_) const
		{
			return t_ == to_find;
		}
	};
	//----------------------------------------------------------------------
	inline void robust_wrap_2()
	{
		// Usage:
		WVector wrapped_vector;

		wrapped_vector->push_back(L"One");
		wrapped_vector->push_back(L"Two");
		wrapped_vector->push_back(L"Three");

		typedef WVector::wrapped_type::value_type VT;

		wrapped_vector.for_each(val2prompt<VT>);

		WVector::wrapped_type::iterator  result;

		if (
			wrapped_vector.find_if(
				result,
				ValMatch<WVector::wrapped_type::value_type>(L"Two")
			)
			)
		{
			fm::prompt(L"\nFound: ");
			fm::prompt(*result);
		}
		fm::prompt(L"\nDone\n");
	}
	//----------------------------------------------------------------------
} // namespace wrapper_test
  //----------------------------------------------------------------------

using namespace dbjsys::fm ;
using namespace dbjsys::fm::bstr ;
//
static volatile const bstr_t BSTRFILE (__FILE__) ;
//--------------------------------------------------------------------------------
extern "C" void wrap_test()
{
	typedef WWrap<std::list<_bstr_t> > llist_type ;
	
	llist_type llist ;

	llist->push_back( L"0" ) ;
	llist->push_back( L"1" ) ;
	llist->push_back( L"2" ) ;
	llist->push_back( L"3" ) ;
	llist->push_back( L"4" ) ;
	llist->push_back( L"5" ) ;
	llist->push_back( L"6" ) ;
	llist->push_back( L"7" ) ;
	llist->push_back( L"8" ) ;
	llist->push_back( L"9" ) ;

	std::for_each( llist->begin(), llist->end(), val2prompt<bstr_t> ) ;
	llist.for_each( val2prompt<bstr_t> ) ;

	wrapper_test::robust_wrap_2() ;
}
//----------------------------------------------------------------------------
//
typedef std::map<int, bstr_t > IntStringMap ;
//
extern "C" void wrapwrap_test ( void) 
{
	WWrap<IntStringMap> ism_ ;

	ism_->insert( IntStringMap::value_type( 1, L"One" ) ) ;

	ism_->find(1) ;
}
//-----------------------------------------------------------------------
// Register the test[s]
//-----------------------------------------------------------------------
static  const int    dummy1 = DBJTESTREG( wrap_test ) ;
static  const int    dummy2 = DBJTESTREG( wrapwrap_test ) ;
//-----------------------------------------------------------------------
// eofile
//-----------------------------------------------------------------------
