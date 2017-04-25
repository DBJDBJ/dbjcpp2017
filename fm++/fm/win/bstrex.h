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
//
// _bstr_t extensions
//

#pragma once


//*****************************************************************************/
namespace dbjsys {
	namespace fm {
		namespace bstrex {
			class bstr_conversion {
			public:
				typedef Error<bstr_conversion> Err;
			};

			using Err = bstr_conversion::Err ;
/**/
			class bstr : public _bstr_t {
			public:
				
				using parent = _bstr_t;

				explicit bstr() : parent() { }
				~bstr() { delete(this); }

				bstr( std::wstring ws) { parent(ws.data());  }
				const bstr & operator = (const std::wstring & ws) { 
					this->Assign(parent(ws.data())); return *this; 
				}
				operator const std::wstring() const noexcept { return std::wstring((wchar_t *)(*this)); }

				bstr( std::string st) { parent(st.data());  }
				const bstr & operator = (const std::string & st) { 
					this->Assign(parent(st.data())); return *this;
				}
				operator const std::string() const noexcept { return std::string((char *)(*this)); }

				bstr & operator << ( const _bstr_t & b2)
				{
					// CLI * that = reinterpret_cast<CLI*>(this);
					(*this) += b2; //  right;
					return *this;
				}

			};

#if 0
			//--------------------------------------------------------------
			// here are the explicit  operators to help compiler to resolve
			// what it thinks is ambigious
			DBJINLINE
				bstr & operator << (bstr & b1, const bstr & b2)
			{
				_bstr_t left((char *)b1);
				_bstr_t right((char *)b2);
				return b1 = bstr(left + right);
			}
#endif

#if 0
		// here we enforce our policies about bstr_t
		class dbjBSTR 
		{
			_bstr_t the_bstr_ ;
		public :
			explicit dbjBSTR( const bstr_t & other_ ) 
				: the_bstr_( other_ ) // COPYING
			{
			}
			// POLICY: do not make bstr_t with 0 pointer inside
			dbjBSTR( )
#ifdef _UNICODE
				: the_bstr_( L"") // avoid NULL bstr_t!
#else
				: the_bstr_("") // avoid NULL bstr_t!
#endif
			{
			}
			// POLICY: do not make bstr_t with 0 pointer inside
			explicit dbjBSTR( const  wchar_t * other_ )
				: the_bstr_( other_ == 0 ? L"" : other_ ) // avoid NULL bstr_t!
			{
			}
			operator bstr_t & () { return the_bstr_ ; }
			operator const bstr_t & () const { return the_bstr_; }
			// POLICY: do not let non const pointers out
			operator const wchar_t * () const 
			{ 
				return (const wchar_t *)the_bstr_; 
			}
			// POLICY: do not let char * out!
			operator const char * () const 
			{
				_ASSERT(0); return 0;
			}
		} ;
		//---------------------------------------------------------------------------------------
		// 
		// First  we define inline definitions to make compilations of the two templated 
		// operators for streaming to and from bstr_t.
		// These definitions are here because they  are for type which are used by FM
		// like std::wostream, std::wstring,etc.
		// The defintions for types which are specific will be defined in the modules
		// which are declaring those types. But in this same namespace.

		//---------------------------------------------------------------------------------------
		// assign std::string to _bstr_t
		DBJINLINE
			_bstr_t &
			assign_to_bstr_t_(_bstr_t & bstr, const std::string & ws_)
		{
			return bstr = static_cast<const char *>(ws_.c_str());
		}
		// assign _bstr_t to std::string
		DBJINLINE
			std::string &
			assign_from_bstr_t_(std::string & ws_, const _bstr_t & bstr_)
		{
			return ws_ = static_cast<const char *>(bstr_);
		}



		//---------------------------------------------------------------------------------------
		// assign wchar_t * to _bstr_t
		DBJINLINE
			_bstr_t &
		assign_to_bstr_t_ ( _bstr_t & bstr, const wchar_t * ws_ ) 
		{
			dbjVERIFY( ws_ ) ;
			return bstr = static_cast<const wchar_t *>(ws_) ;
		}
		// assign _bstr_t to  wchar_t * 
		DBJINLINE
			wchar_t * 
		assign_from_bstr_t_( wchar_t * ws_ , const _bstr_t & bstr_ )
		{
			dbjVERIFY( ws_ ) ;
			// as long as bstr_t exist ws_ will point to something valid
			// whatever ws_ was pointing to before is lost
			return ws_ = const_cast< wchar_t *>((const  wchar_t *)bstr_) ;
		}

		// assign std::wstring to _bstr_t
		DBJINLINE
			_bstr_t &
			assign_to_bstr_t_(_bstr_t & bstr, const std::wstring & ws_)
		{
			return bstr = ws_.data() ;
		}
		// assign _bstr_t to std::wstring
		DBJINLINE
			std::wstring &
			assign_from_bstr_t_(std::wstring & ws_, const _bstr_t & bstr_)
		{
			return ws_ = (wchar_t *)bstr_;
		}

		//---------------------------------------------------------------------------------------
		// 
		// These definitions are here because they  are for type which are used by FM
		// like std::wostream, std::wstring,etc.
		// The defintions for types which are specific should be defined in the modules
		// which are declaring those types. But in this same namespace.
		//---------------------------------------------------------------------------------------
		DBJINLINE
		std::wostream & assign_from_bstr_t_(std::wostream & t_, const bstr_t & bstr)
		{
			return t_ << ((const wchar_t*)bstr);
		}
		// wostream to bstr_t -- a tricky one !
		DBJINLINE
			bstr_t & assign_from_bstr_t_(bstr_t & bstr, const std::wostream & t_)
		{
			std::wostringstream wos;
			wos << t_.rdbuf();
			bstr += wos.str().c_str();
			return bstr;
		}
#endif
		//------------------------------------------------------------------------
		//
	    // By defining the two operatos bellow we provoke the compiler to 
		// use them and in turn to signal which versions of
		// assign_to_bstr_t_(), and assign_from_bstr_t_()
		// are yet to be defined. 
		//
		//
		// streaming INTO the bstr FROM the instance of type T
		template <typename T>
		DBJINLINE
			bstr & operator << (bstr & bstr, const  T & t )
		{
			return bstr = t ;
		}

		// streaming FROM the _bstr_t INTO the instance of type T
		template <typename T>
			DBJINLINE
		T & operator << ( T & t_, const bstr & bstr )
		{
			return  t = bstr ;
		}
			// try casting ARG to T using the _bstr_t
			// T has to be copyable
			template<typename T, typename ARG>
			DBJINLINE
				T bstr_cast( const ARG & arg, bool kick_the_bucket = true) {

				static_assert(std::is_copy_constructible<T>::value, __FUNCTION__ " error: type T is not copy constructible");

				try {
					_bstr_t bart(arg);
					return (T)bart;
				}
				catch (const ::_com_error & cerr_) {
					if (kick_the_bucket) {
						assert(0 && cerr_.ErrorMessage()); /* conversion failed */
					}
				}

			}
//*****************************************************************************/
		} // namespace bstrex 
	} // fm
}  // dbjsys
//*****************************************************************************/

