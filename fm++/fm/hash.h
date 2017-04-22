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
//  $Author: Dusan $
//  $Date: $
//  $Revision: $
//*****************************************************************************/

#pragma once
#include <functional>
#include <iterator>

//--------------------------------------------------------------------------------
//
// Hashing algorithms as function object[s]
// NOTE: See Stroustrup, C++, 3rd ed., 17.6.2.3 - Hashing, p.502
//
//--------------------------------------------------------------------------------
namespace dbjsys {
    namespace fm {
        namespace algo {
//--------------------------------------------------------------------------------
//
    template < typename T > struct Hash // : std::unary_function<T, size_t>
    {
		typedef T value_type;
        // typedef Error<Hash> Err ;
        // generic case depends on the feature of language:
        // every pointer adress can be cast into char pointer
        size_t operator () ( const T & key ) const
        {
            size_t      res = 0 ;
            size_t      len = sizeof(T) ;
            const wchar_t * p  = reinterpret_cast<const wchar_t*>(&key) ;

            while ( len -- )
                res = ( res << 1 ) ^*p++ ; // use bytes of key's presentation

			return res ;
        }
    } ;

    // specialization for wchar_t *, needs no casting
    template <> struct Hash<wchar_t *> // : std::unary_function< wchar_t *, size_t>
    {
        size_t operator () ( const wchar_t * key ) const
        {
                size_t   res = 0 ;
                while ( *key )
                    res = ( res << 1 ) ^*key++ ; // use int value of characters
                return res ;
        }
    } ;

    // specialization for _bstr_t, needs no casting
     template <> struct Hash<_bstr_t> // : std::unary_function< _bstr_t , size_t>
    {
        size_t operator () ( const _bstr_t & key ) const
        {
            return Hash<wchar_t *>()((wchar_t *)key) ;
       }
    } ;

    // specialization for any kind of a basic_string<> needs no casting either
     template <> struct Hash<std::string> // : std::function< size_t (std::string )>
    {
       size_t operator () ( const std::string & key ) const
        {
            size_t  res =   0;
            typedef std::string::const_iterator CI;
            CI  p   = key.begin() ;
            CI  end = key.end() ;

            while ( p != end ) res = (res<<1)^*p++ ; // use int value of characters
            return res ;
        }
    } ;
//--------------------------------------------------------------------------------
}  // eof namespace algo
//--------------------------------------------------------------------------------
    } //    namespace fm 
} // namespace dbjsys 
//--------------------------------------------------------------------------------

