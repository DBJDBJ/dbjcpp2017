
#pragma once
//--------------------------------------------------------------------
namespace dbjsys {
    namespace fm {


template < typename T > 
interface LockedContainer
{    
	class SimpleCriticalSection : fm::Win32CriticalSection
    {
        public :
        virtual void enter () const	{   take_();   }
	    //-------------------------------------------------
	    virtual void leave () const	{   release_();   }
    } ;

    struct lock_type : public dbjsys::fm::Pref 
        { 
            SimpleCriticalSection & cs_ ;
            lock_type( SimpleCriticalSection & cs ) : cs_(cs) {}
            void operator()() const 
            {   cs_.enter();  }
        };
	    //-------------------------------------------------
    struct unlock_type : public dbjsys::fm::Suf 
        { 
            SimpleCriticalSection & cs_ ;
            unlock_type( SimpleCriticalSection & cs ) : cs_(cs) {}
            void operator()() const 
            {   cs_.leave();  }
        };

    typedef fm::Wrap< T, lock_type, unlock_type > WrappedType ;
	typedef fm::Call_proxy< T, unlock_type > ProxyType ;

	LockedContainer() : 
		lock_( cs_ ),
		unlock_( cs_ ), 
		wrapped_instance_( t_instance, lock_, unlock_ )
	{}

	ProxyType operator->() const 
	{ 
		return wrapped_instance_.operator->() ; 
	}

	private:
		// order is important
		SimpleCriticalSection cs_ ;
		lock_type lock_ ;
		unlock_type unlock_ ;
		T t_instance ; // the container being wrapped
		WrappedType wrapped_instance_ ;
} ;

	} // fm
} // dbjsys