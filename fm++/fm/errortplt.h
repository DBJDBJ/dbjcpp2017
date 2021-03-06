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
//	Library of Sub-System Mechanisms
#if ! defined ( type_info )
#include <typeinfo.h> 
#endif
/*
Compiler Warning (level 3) C4290 Visual Studio 2013Visual Studio 2013
C++ exception specification ignored except to indicate a function is not __declspec(nothrow) 
*/
#pragma warning( disable : 4290 )

// ----------------------------------------------------------------------
namespace dbjsys {
	namespace fm {

DBJINLINE _bstr_t & errstring_(DWORD lastErrorCode)
		{
			static _bstr_t result;
			LPWSTR lpMsgBuf = NULL;

			if (lastErrorCode != 0)
			{

				_ASSERT(FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					lastErrorCode, // result of GetLastError()
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPWSTR)&lpMsgBuf, 0, NULL)
				);
				result = lpMsgBuf;
				::LocalFree(lpMsgBuf);
			}
			else {
				result = L"Not a Win32 error";
			}
			return result;
		}
//
template< typename T > 
DBJINLINE _bstr_t dbjtypename( T * t_null /* = (T*)0 */ ) 
{
	#if defined( _CPPRTTI )
		return _bstr_t(typeid(T).name()) ;
	#else
		return _bstr_t(L"UNKNOWN -- non RTTI build") ;
	#endif
}
// give the typename as _bstr_t
#define DBJTYPEID(T) dbjtypename<T>((T*)0)
//---------------------------------------------------------------------------------------
//
// interface to each and every DBJ generated error fm class
// 
		class IError
		{
			const _bstr_t errmsg_ ;
			bool reported_ ;
			// no default ctor
			IError() ;

		public:
			IError( const _bstr_t & themsg, bool DO_NOT_BEEP_ON_EXIT = false )
				: errmsg_(themsg), reported_(DO_NOT_BEEP_ON_EXIT)
			{
			}		
			virtual ~IError()
			{
				errmsg_.~_bstr_t();
			}
			const _bstr_t & what () const noexcept
			{ 
				return errmsg_ ; 
			}
			const _bstr_t & report( const bool & pop = false ) const
			{
				if (false == reported_)
				{
					const_cast<IError*>(this)->reported_ = true;
					::Beep(100, 300);
					/*
					TODO: server side apps must not pop message boxes, but log the messages ...
					*/
					if (pop )
					err_msg_box(errmsg_);
				}
				return errmsg_;
			}

			// make 'standard' error message
	// 
			static _bstr_t  & makeErrMsg
			(const _bstr_t & typeName, const _bstr_t & msg, const _bstr_t & file, const long line)
			{
				static const wchar_t prompt1[] = L"\nException from : ";
				static const wchar_t prompt2[] = L"\n\nWhat : ";
				static const wchar_t prompt3[] = L"\n\nFile : ";
				static const wchar_t prompt4[] = L"\n\nLine : ";

				static _bstr_t NEM = L"Not enough memory! " + _bstr_t(__FILE__) + L":" + _bstr_t(_variant_t(__LINE__));
				static _bstr_t text;
				try
				{
					text = prompt1 + typeName;
					text += prompt2 + msg;
					text += prompt3 + file;
					text += prompt4 + _bstr_t(_variant_t(line));
				}
				catch (...)
				{
					return NEM;
				}
				return text;
			}			// make win32 error message
	// 
			static _bstr_t  & makeErrMsg(const _bstr_t & typeName, DWORD err_code, const _bstr_t & file, const long line)
			{
				return IError::makeErrMsg(
					typeName,
					errstring_(err_code),
					file,
					line);
			}

			static void err_msg_box(
				const _bstr_t & msg_, 
				const _bstr_t & title_ = glob::FMPP_TITLE , 
				bool   exit_  = false)
			{
				try {
					::MessageBox(NULL, 
						(LPCTSTR)(glob::ERR_INTRO + msg_),
						title_, MB_OK);
				}
				catch (...) {
					/* server side apps must not pop message boxes */
				}

				if ( exit_)
					ExitProcess(GetLastError());
			}
		} ;
//------------------------------------------------------------------------------------
// concrete class to be used as abase of all DBJ exceptions
// that can be made
// 
template <class T> class Error : public IError  
{
  
	typedef  IError	parent_type ;
  public:
        //	The class that has thrown this exception.
    typedef T thrower_type;

	// 
	Error(const DWORD & win32error_code, const  _bstr_t & file, const int line, bool DO_NOT_BEEP_ON_EXIT = false) noexcept
		  : parent_type(
            parent_type::makeErrMsg( DBJTYPEID(thrower_type), win32error_code,file,line),
			DO_NOT_BEEP_ON_EXIT )
      {
      }
  	//
	// 
      Error (const _bstr_t & msg, bool DO_NOT_BEEP_ON_EXIT = false ) noexcept
		  : parent_type(
            parent_type::makeErrMsg( DBJTYPEID(thrower_type), msg,L"~",0),
			DO_NOT_BEEP_ON_EXIT )
      {
      }

	//
	// 
      Error (const  _bstr_t & msg, const  _bstr_t & file, const int line, bool DO_NOT_BEEP_ON_EXIT = false ) noexcept
		  : parent_type( 
		    parent_type::makeErrMsg( DBJTYPEID(thrower_type) ,msg, file, line),
			DO_NOT_BEEP_ON_EXIT )
      {
      }

	  // 
	  const _bstr_t & what() const
	  {
		  return parent_type::what();
	  }

	  const _bstr_t & report(const bool & pop_ = false ) const
	  {
		  return parent_type::report(pop_);
	  }
};
//-------------------------------------------------------------------------------------
// WIn32 error message + who did it,but not where
// 
template<class T> class Win32Error : public Error<T>
{
	typedef Error<T> Parent ;
	
	Win32Error() {};
    public:
	// 
		Win32Error(const _bstr_t & file, const long & line_)
			: Parent( ::GetLastError(), file, line_ ) 
		{
		}

		Win32Error(DWORD winerr_, const _bstr_t & file, const long & line_)
			: Parent(winerr_, file, line_)
		{
		}

		~Win32Error()
		{
				::SetLastError(0);
		}

		//template<class T>
		static _bstr_t report_only(
			DWORD winerr_ = ::GetLastError(),
			const _bstr_t & file = L"", 
			const long & line_ = 0)
		{
			Win32Error<T> ERR_(winerr_, file,line_);
			ERR_.report(true);
			return ERR_.what();
		}
} ;

//	----------------------------------------------
#undef DBJTYPEID
//	----------------------------------------------
	/*
	Helper methods used by macros bellow

	TODO: as normal template + specializations suite of functions
	*/
// transform class _com_error to IError ;
template < class T > DBJINLINE
void __dbj_throw__ ( const _com_error & e, const _bstr_t & file, const int line , Error<T> * )
{
        throw Error<T>( dbjsys::fm::getErrMsg( e ),file,line) ;
}
//	----------------------------------------------
// construct desired IError inheritor
template < class T > DBJINLINE
void __dbj_throw__(const _bstr_t &  msg, const _bstr_t &  file, const int line, Error<T> *)
{
	throw Error<T>(msg, file, line);
}
//	----------------------------------------------
template < class T > 
DBJINLINE
void __dbj_throw__(const _bstr_t &  msg, const _bstr_t &  file, const int line, Win32Error<T> *)
{
	// effectively rectify the mistakes of ussing Win32Error when Error<> should be enough
	throw Error<T>(msg, file, line);
}
//	----------------------------------------------
/*
template < class T > inline
void __dbj_throw__ ( const _bstr_t & msg, const char * file, const int line, T * )
{
        throw T(msg, _bstr_t(file) ,line) ;
}
*/
//	----------------------------------------------
/* 
This in effect transforms std::exception to IError
*/
template < class T > DBJINLINE
void __dbj_throw__(const std::exception & msg, const char * file, const int line, Error<T> *)
{
	throw Error<T>(msg.what(), _bstr_t(file), line);
}
/* */
//	----------------------------------------------
	}; // fm
}; // dbjsys

//	-----------------------------------------------------
/*
following are not generic macrose
they require class Err to be declared inside a scope
in which it is used, to provide the typename

class X {
dbjMAKE_ERR(X);
dbjERR(ErrSpecific) ;
void throwerr () {  dbjVERIFY(false); }
}

Please note that above we have a mechanism that creates a micro hierarchy
with  dbjsys::fm::Error<X> as a base class and
ErrSpecific as its offspring

Always used by dbjVERIFY() macro bellow

*/
// macro that makes Err type to be in 
// the scope required by this mechanism
#define dbjMAKE_ERR(x) typedef dbjsys::fm::Error<x> Err
// 
#define dbjTHROWERR(m) dbjsys::fm::__dbj_throw__(m,__FILE__,__LINE__, (Err*)0)

#define dbjPOPERR catch ( const dbjsys::fm::IError & dbjfmerr ) { dbjfmerr.report(true); }

//
// this assertion always works , e.g. in a release mode
// string argument is always taken as UNICODE (prefixed with  'L')
#define dbjVERIFY(x) if (!(x)) dbjTHROWERR(L#x)
// make offspring of the internall Err class
#define dbj_ERR(x) struct dbjNOVTABLE x : public Err { x() : Err(L#x, true ) {} } ;
/*
following are a bit more generic macros
*/
//
#define dbjCOMVERIFY( x ) _com_util::CheckError( x )
// 
//
#define dbjTHROWIF(x,y) if ( x ) throw y(__FILE__,__LINE__) 

/**
<summary>
We want more info when we catch the exception
We want to know at least the class of the object
WHO has thrown the exception, besides the actual
message saying WHAT has happened. And because we
are pro-active we will want to know from WHERE this
exception is thrown.
This is the concept of what we want, and this is
the class built around that concept.
</summary>

<example>
<code>
class A {
public :
typedef Error<A> Err ; // this is all we have to do

A () {
// throw Err("A::A() failed") ; // re-active usage
throw Err("A::A() failed", __FILE__, __LINE__) ; //
pro-active usage
}
} ;

int main ( int argc , TCHAR ** argv )
{
try {
A aha ;
}
catch ( A::Err & x ) // when A is visible, we can use
it
{	                 // to access it's interface
x.display() ;
}
catch ( exception & x ) // or like this when A is not
visible
{                       // but still with extended info
cerr << endl << x.what() << endl ;
}
return 0 ;
}
</code>

The above test will print out the following :

Exception from    : class A
In File , On Line :
C:\architect\cpp\errtemplate\errtemplate.h , 81
Exception message : A::A() failed
</example>
*/
namespace dbjsys {
	namespace error {
		struct DBJ_FM_WIDE {  };
		/// <summary>
		/// Create FM wide dbjsys::error::Err
		/// </summary>
		typedef dbjsys::fm::Error<DBJ_FM_WIDE> Err;
		/// <summary>
		/// Use FM wide dbjsys::error::Err
		/// </summary>
#define dbjFMERR(m) dbjsys::fm::__dbj_throw__(m,__FILE__,__LINE__, (dbjsys::error::Err*)0)
	} 
}

