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

//	All DBJINLINE  version of a simple but important UID .
//
//	Most important operations are relational operators
//	generated.
//	Library of Sub-System Mechanisms

#pragma once
//---------------------------------------------------------------------------------------
namespace dbjsys {
	namespace fm {
//---------------------------------------------------------------------------------------
//	The abstraction encapsulating the definition of an
//	Unique Identifier.
//
//	In this context it is an identifier unique on the whole
//	OQ system level.
//
//	This is an 'non recycleable' value, that is if discarded
//	it can not be reused again to identify any new entity.
//	The most common implementation  uses the Timestamp type.
class UID ;
// 
class UID 
{
public:
                    
	typedef long type;            
    typedef dbjsys::fm::Error< UID > Err;

	// 
    static const UID::type nullValue() { return(UID::type)-1 ; }
	
	// 
	static const UID & null () {  static const UID null_(nullValue()) ;
	    return null_ ;
    }
    /////////////////////////////////////////
	// 
    UID() {  val = UID::nullValue() ; }

	// 
    UID(const UID &right) {  val = right.val ;   }

	// 
    UID(const type  & newVal) { val = newVal ; }


	// 
     ~UID()    {
        //val = UID::NullValue ;
    }

	// 
	const UID & operator=(const UID &right);
	
	// 
	friend bool operator==(const UID &left,const UID &right);
	// 
	friend bool operator!=(const UID &left,const UID &right);
	// 
	friend bool operator<(const UID &left,const UID &right);
	// 
	friend bool operator>(const UID &left,const UID &right);
	// 
	friend bool operator<=(const UID &left,const UID &right);
	// 
	friend bool operator>=(const UID &left,const UID &right);

	// 
	friend std::ostream & operator<<(std::ostream &stream,const UID &right);
	// 
	friend std::istream & operator>>(std::istream &stream,UID &object);


	// 
    UID::type getVal () const;

    //	Helper function  returns UID::type value as a dbjsys::fm::tstring
	// 
    const _bstr_t asString () const;

	// 
	operator _bstr_t () const
	{
		return _bstr_t( this->asString() ) ;
	}

	// 
	UID &  operator << (const wchar_t * );

	// 
	UID &  operator << (const _bstr_t & str)
	{
		return *this << static_cast<const wchar_t*>(str) ;
    }

	// 
	const int isNull() const 
	{
		return val == nullValue()  ;
	}

private:	
	// 
	UID::type  val;
                 
};
///////////////////////////////////////////////////////////////////////////////////////////
// 
// Class UID 
// DBJINLINE  implementation
// 
///////////////////////////////////////////////////////////////////////////////////////////
__forceinline const UID & UID::operator=(const UID &right)
{
    if ( this != &right )
	    val = right.getVal();
	return *this;
}


DBJINLINE  bool operator==(const UID &left,const UID &right)
{
  	return (left.getVal() == right.getVal());
}

DBJINLINE  bool operator!=(const UID &left,const UID &right)
{
    return (left.getVal() != right.getVal());
}


DBJINLINE  bool operator<(const UID &left,const UID &right)
{
    return (left.getVal() < right.getVal());
}

DBJINLINE  bool operator>(const UID &left,const UID &right)
{
    return (left.getVal() > right.getVal());
}

DBJINLINE  bool operator<=(const UID &left,const UID &right)
{
  	return (left.getVal() <= right.getVal());
}

DBJINLINE  bool operator>=(const UID &left,const UID &right)
{
  	return (left.getVal() >= right.getVal());
}


DBJINLINE  std::ostream & operator<<(std::ostream &stream,const UID &right)
{
  	return stream << right.getVal() ;
}

DBJINLINE  std::istream & operator>>(std::istream &stream,UID &object)
{
  	return stream >> object.val ;
}

__forceinline  UID::type UID::getVal () const
{
  return val ;
}

__forceinline  const _bstr_t UID::asString () const
{
    return (_bstr_t)_variant_t(this->val); 
}

__forceinline  UID &  UID::operator << (const wchar_t * wstr)
{
    dbjVERIFY (( 0 != wstr ))  ;
    val = (UID::type)_wtol(wstr) ;
	return *this ;
}
//---------------------------------------------------------------------------------------
	} ; // fm
}; // dbjsys
//---------------------------------------------------------------------------------------
