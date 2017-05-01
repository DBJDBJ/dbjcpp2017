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
#pragma once
// disable warning C4786
#pragma warning( disable: 4786 )
//--------------------------------------------------------------------
namespace dbjsys {
    namespace fm {
//--------------------------------------------------------------------
// 
        struct Comparable {
            bool operator == ( const long &  ) { return false; }
            bool operator != ( const long &  ) { return true ; }
        } ;
//--------------------------------------------------------------------
// 
        struct Pref : public Comparable { 
			Pref() {}
			virtual ~Pref() {}
            virtual void operator()() const = 0 ;
        };
//--------------------------------------------------------------------
// 
        struct Suf : public Comparable { 
			Suf  () {}
			virtual ~Suf() {}
            virtual void operator()() const = 0 ;
        };
//--------------------------------------------------------------------
        struct NullPref : public Pref { 
            virtual void operator()() const {}
        };
//--------------------------------------------------------------------
        struct NullSuf : public Suf { 
            virtual void operator()() const {}
        };
//--------------------------------------------------------------------
// robust wrapper:
template<class T, class Pref, class Suf> class Wrap;
// for MS C++, remove the friend declaration and make all members of
// Call_proxy public (MS C++ 6.0 doesn't support templates as friends)
template<class T, class Suf>
class Call_proxy {
	T* p;
	mutable bool own;
	Suf suffix;
public:
	explicit Call_proxy(T* pp, Suf su) :p(pp), own(true), suffix(su) 
    { 
        DBJ_VERIFY(p) ;
    }		// restrict creation 

	Call_proxy& operator=(const Call_proxy&);	// prevent assignment

#if ! defined( _MSC_VER )
	template<class U, class P, class S> friend class Wrap;
#endif

	Call_proxy(const Call_proxy& a)
		: p(a.p), own(true), suffix(a.suffix) { a.own=false; }

	~Call_proxy() { if (own) suffix(); }

	T* operator->() const  { return p; }

};
//-------------------------------------------------------------------------------
//
// 
template<class T, class Pref, class Suf>
class Wrap 
{
protected :
	// 
	T* p;
	// 
	int* owned;
	// 
	void incr_owned() { if (owned) ++*owned; }
	// 
	void decr_owned() { 
		if (owned && --*owned == 0) { 
				try {
					delete p; p = 0 ;
				}
				catch ( ... ) {
					DBJ_VERIFY(0);
				}
			delete owned; owned = 0 ;
		} 
	}

	Pref prefix;
	Suf suffix;

public:
    typedef T    wrapped_type;
    typedef Pref prefix_type ;
    typedef Suf  suffix_type ;
    // this is the ONLY allowed type for functions acting as prefixes or suffixes
    typedef void (* ps_function_type) ( ) ;

	//-------------------------------------------------
	// 
	constexpr bool valid() const noexcept
	{
		return ((this != 0) && (p != 0) && (&prefix != 0) && (&suffix != 0));
	}
    //////////////////////////////////////////////////////////
    // this works as long as all types involved have default
    // constructor available and are 'constructable'
	// 
    static Wrap make(
                wrapped_type    & wrapped , 
        const   prefix_type     & prefix  , 
        const   suffix_type     & suffix  )
    {
        return Wrap( wrapped, prefix, suffix );
    }
    // if using function pointers call this method 
	// 
    static Wrap make( ps_function_type p_, ps_function_type s_ )
    {
        return Wrap( new wrapped_type(), p_, s_ );
    }
    //////////////////////////
	// 
	Wrap( T& x, const Pref & pr, const Suf & su)
		:p(&x), owned(0), prefix(pr), suffix(su) 
    { 
        if ( ! valid() ) throw std::runtime_error(__FILE__) ;
    } 

	// 
	Wrap( T* pp, const Pref & pr, const Suf & su)
		:p(pp), owned(new int(1)), prefix(pr), suffix(su) 
    { 
        if ( ! valid() ) throw std::runtime_error(__FILE__) ;
    } 

	// 
	Wrap(const Wrap& a)
		:p(a.p), owned(a.owned), prefix(a.prefix), suffix(a.suffix)
	{ 
        if ( ! valid() ) throw std::runtime_error(__FILE__) ;
            incr_owned(); 
    }

	// 
	Wrap& operator=(const Wrap& a)
	{
        if ( this != &a )
        {
		    a.incr_owned();
		    decr_owned();
		    p = a.p;
		    owned = a.owned;
		    prefix = a.prefix;;
		    suffix = a.suffix;
        }
        if ( ! valid() ) throw std::runtime_error(__FILE__) ;
        return *this ;
	}

	// 
	virtual ~Wrap() { decr_owned(); }



    //-------------------------------------------------
	// 
	Call_proxy<T,Suf> operator->() const
		{ 
            prefix(); 
            return Call_proxy<T,Suf>(p,suffix); 
        }

    //-------------------------------------------------
    // DBJ added the const-nes 18MAR2001
	// DBJ added constexpr and noexcept 01MAY2017
	// 
	constexpr T * const direct() const noexcept // extract pointer to wrapped object
        { 
            return this->p; 
        } 

};

//--------------------------------------------------------------------
	namespace locking_wrap_implementation {
//--------------------------------------------------------------------

// 
	class SimpleCriticalSection : fm::Win32CriticalSection
    {
        public :
	// 
        virtual void enter () const	{   take_();   }
	// 
	    virtual void leave () const	{   release_();   }
    } ;

// 
    struct lock_type : public dbjsys::fm::Pref 
        { 
	// 
            SimpleCriticalSection * cs_ ;
	// 
            lock_type( SimpleCriticalSection * cs ) : cs_(cs) {}
	// 
			lock_type( const lock_type & other ) : cs_(other.cs_) {}
	// 
			lock_type & operator = ( const lock_type & other ) ;
			//
	// 
            void operator()() const {   DBJ_VERIFY(cs_); cs_->enter();  }
        };
	    //-------------------------------------------------
// 
    struct unlock_type : public dbjsys::fm::Suf 
        { 
	// 
            SimpleCriticalSection * cs_ ;
	// 
            unlock_type( SimpleCriticalSection * cs ) : cs_(cs) {}
	// 
			unlock_type( const unlock_type & other ) : cs_(other.cs_) {}
	// 
			unlock_type & operator = ( const unlock_type & other );
	// 
            void operator()() const { DBJ_VERIFY(cs_);  cs_->leave();  }
        };
//--------------------------------------------------------------------
	} //	namespace locking_wrap_implementation 
//--------------------------------------------------------------------
using namespace locking_wrap_implementation ;
// this wrap automagically locks anything  wrapped
// it also adds for_each() and find_if() as atomic operations
// on the wrap itself.
template< 
	typename T, 
	typename PREFIX = locking_wrap_implementation::lock_type, 
	typename SUFFIX = locking_wrap_implementation::unlock_type,
	typename CRTSCT = locking_wrap_implementation::SimpleCriticalSection >
		class WWrap
		: public fm::Wrap<T, PREFIX, SUFFIX >
{
	typedef fm::Wrap<T, PREFIX, SUFFIX > Parent ;
	CRTSCT * cs_ ;
	PREFIX * pp_ ;
	SUFFIX * sp_ ;
public :
	using PARENT = fm::Wrap<T, PREFIX, SUFFIX >;

	explicit WWrap () 
	: Parent( new T()
				, *(pp_ = new PREFIX(cs_))
				, *(sp_ = new SUFFIX(cs_ = new CRTSCT() ) )
			)
	{
		// above
		// make cs_ first
		// make sp_ second
		// make pp_ third
		// make p    fourth -- in the base class
	}

	~WWrap ()	{
		try {
			if ( pp_ ) delete pp_ ;
			if ( sp_ ) delete sp_ ;
			if ( cs_ ) delete cs_ ;
			pp_ = 0 ; sp_ = 0 ; cs_ = 0 ;

		} catch (...) 
		{
			DBJ_VERIFY(0) ;
		}
	}
    //-------------------------------------------------
    // Helper for 'Visitor Method' pattern as bellow
    // DBJ created 18MAR2001
    class AutoWrap
    {
        const Pref &  prefix_;  const Suf  &   suffix_ ;
    public :
        explicit
        AutoWrap( const Pref &  p_,  const Suf  &   s_) : prefix_(p_), suffix_(s_) { prefix_() ; }
        ~AutoWrap( ) { suffix_() ; }
    } ;
    //-------------------------------------------------
    // 'Visitor Method' -- the 'Implementation Pattern'?
    // execute for_each() on wrapped up container with
    // predicate given.
    // Wrap up the whole operation in the current prefix/suffix pair
    // If T is not an std container this will not compile
    // DBJ created 18MAR2001
    template<class P_>
        void for_each( P_ predicate) const
    {
		DBJ_VERIFY( this->valid() ) ; 
        AutoWrap ps(this->prefix, this->suffix) ;
		// T * p = this->direct();
        std::for_each( this->p->begin(), this->p->end(), predicate ) ; 
    }
    //-------------------------------------------------
    // 'Visitor Method' -- the 'Implementation Pattern'?
    // execute find_if() on wrapped up container with predicate given.
    // Return true if find was succesfull, false otherwise.
    // Wrap up the whole operation in the current prefix/suffix pair
    // If T is not an std container this will not compile
    // DBJ created 18MAR2001
    template<class I, class P_>
        const bool find_if( I & iter_ , P_ predicate) const
    {
			DBJ_VERIFY(this->valid());
			AutoWrap ps(this->prefix, this->suffix) ;
        iter_ = std::find_if(this->p->begin(), this->p->end(), predicate ) ;

        if ( iter_ == this->p->end() )
            return false ;
        else
            return true  ;
    }
//--------------------------------------------------------------------
} ; // eof WWrap
//--------------------------------------------------------------------
    } //    namespace fm 
} // namespace dbjsys 



