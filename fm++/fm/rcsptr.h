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
//---------------------------------------------------------------------------------------
namespace dbjsys {
	namespace fm {
//---------------------------------------------------------------------------------------

// Reference Counting SHARED Reference.
// No copying is done, so you can have a Ref to a virtual base class
// 
template<class T> 
class SREF
{
public:
	SREF( T * body ) noexcept ;
	SREF( ) noexcept  ;
	SREF( const SREF<T> & other ) noexcept  ;
	SREF<T> & operator=( const SREF<T> & other ) ; //noexcept ;
	
	// 
    ~SREF()
    { 
        release(); 
    }
	// 
	T * operator->() const noexcept 
    { 
		_ASSERT( pointee_ ) ;
        return pointee_; 
    }
	// 
	const T & operator *() const noexcept 
    { 
		_ASSERT( pointee_ ) ;
        return *pointee_; 
    }
	// 
	T & operator *() noexcept 
    { 
		_ASSERT( pointee_ ) ;
        return *pointee_; 
    }

	// To easy to invoke this without realizing
    //operator T*(void) noexcept 
    //{ 
    //    return pointee_; 
    //}
    // This is more explicit
	// 
    T * ptr() noexcept
    {
        return pointee_ ;
    }

	/*
	cause problem in STL container
	T * operator &()
	{
		return pointee_ ;
	}*/
	
	// 
	bool isNull() const noexcept
    { 
        return pointee_ == 0; 
    }

    T * detach() noexcept ;
    bool operator<( const SREF<T> & rhs ) const noexcept ;
    bool operator>( const SREF<T> & rhs ) const noexcept ;
	// 
	bool operator==( const SREF<T> & rhs ) const noexcept ;

private:

	inline void release() // noexcept
	{
		if (!count_)
			return;    // ref. counting null !

		if (*count_ > 0)
			--(*count_);   // dec. count

		if (*count_ == 0)
		{
			if (pointee_)
			{
				delete pointee_;   // destructor may throw exception
				pointee_ = 0;
			}

			delete count_;
			count_ = 0;
		}
	}

	inline void addref() noexcept
	{
		if (!count_)
			return;    // ref. counting null !

		if (*count_ > 0)
			++(*count_);
	}
	// 
	int * count_;
	// 
	T * pointee_;
}; // SREF

template<class T>
inline SREF<T>::SREF( T * pointee ) noexcept
    : count_( new int(1) ), pointee_( pointee )
{
}

template<class T>
inline SREF<T>::SREF() noexcept
    : count_(0), pointee_(0)
{
}

template<class T>
inline SREF<T>::SREF( const SREF<T> & other ) noexcept
    : count_( other.count_ ), pointee_( other.pointee_ )
{
	addref();
}

template<class T>
inline SREF<T> & SREF<T>::operator=( const SREF<T> & other ) // noexcept
{
    if( this != &other )
	{  
		release() ; // may throw exception
		pointee_ = other.pointee_;
		count_ = other.count_;
		addref() ;
	}
	return *this ;
}

#if 0
template<class T>
inline void SREF<T>::release() // noexcept
{
    if( ! count_ )
        return ;    // ref. counting null !

    if( *count_ > 0 )
        --(*count_) ;   // dec. count

	if( *count_ == 0 )
	{
        if( pointee_ )
        {
            delete pointee_ ;   // destructor may throw exception
            pointee_ = 0;
        }

		delete count_;
		count_ = 0;
	}
}

template<class T>
inline void SREF<T>::addref() noexcept
{
    if( ! count_ )
        return ;    // ref. counting null !

	if( *count_ > 0 )
		++(*count_) ;
}
#endif
template<class T>
inline T * SREF<T>::detach() noexcept
{
	if( ! count_ )
        return 0 ;

    if( *count_ > 0 ) 
        *count_ = -1 ;  // pointee_ will not be deleted!

	return pointee_;
}

template<class T>
inline bool SREF<T>::operator<( const SREF<T> & rhs ) const noexcept
{
	return pointee_ < rhs.pointee_;
}

template<class T>
inline bool SREF<T>::operator>( const SREF<T> & rhs ) const noexcept
{
	return pointee_ > rhs.pointee_;
}

template<class T>
inline bool SREF<T>::operator==( const SREF<T> & rhs ) const noexcept
{
	return rhs.pointee_ == pointee_;
}

//---------------------------------------------------------------------------------------
	} ; // fm
}; // dbjsys
//---------------------------------------------------------------------------------------


