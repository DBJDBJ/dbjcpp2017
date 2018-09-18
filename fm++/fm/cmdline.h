//*****************************************************************************/
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
#include "fm.h"
#include <thread>
#include <chrono>

// NOTE: use dbjsys::string_type

/*
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <map>
*/

/// <summary>
/// The Command Line encapsulation aka TCL-ENC
/// by dbj.org
/// (c) statement is on the bottom of this file
/// <remarks>
/// Perhaps interesting in this design is the decision 
/// not to succumb to C like cli 
/// which is based on char ** (or wchar_t ** in case of Windows). 
/// Internaly to represent the command line we will  use 
/// <code>
///  std::vector< std::wstring >; 
/// </code>
/// when dealing with the actual cli we will transform asap to  this
/// thus internaly we do not have to deal with raw pointers.
/// </remarks>
/// </summary>
namespace dbj::app_env {

	/// <summary>
	/// we develop only unicode windows app's
	/// this dats type we use everyhwere
	/// to provide CLI interface implementation
	/// </summary>
	using data_type = std::vector< std::wstring >;

	using map_type =
		std::map<std::wstring, std::wstring>;

	namespace inner {

		inline auto app_env_initor() {

			/// <summary>
			/// transform argw to data_type
			/// base your cli proc code on data_type
			/// instead of raw pointers 
			/// data_type is standard c++ range
			/// </summary>
			/// <param name="args">__argw</param>
			/// <param name="ARGC">__argc</param>
			/// <returns>
			/// instance of data_type
			/// </returns>
			auto command_line_data = [](size_t ARGC, wchar_t **  args)
				-> data_type
			{
				_ASSERTE(*args != nullptr);
				return data_type{ args, args + ARGC };
			};

			/// <summary>
			/// pointers to the runtime environment
			/// from UCRT
			/// </summary>
#define _CRT_DECLARE_GLOBAL_VARIABLES_DIRECTLY
			wchar_t **  warg = (__wargv);
			const unsigned __int64	argc = (__argc);
			wchar_t **  wenv = (_wenviron);
#undef _CRT_DECLARE_GLOBAL_VARIABLES_DIRECTLY

			/// <summary>
			///  we are here *before* main so 
			/// __argv or __argw might be still empty
			/// thus we will wait 1 sec for each of them 
			/// if need to be
			///  TODO: this is perhaps naive implementation?
			/// </summary>
			if (*warg == nullptr) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			_ASSERTE(*warg != nullptr);

			if (*wenv == nullptr) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			_ASSERTE(*wenv != nullptr);

			// extract the unicode  command line
			data_type
				warg_data{ command_line_data(static_cast<std::size_t>(argc), warg) };

			// calculate the count of env vars 
			auto count_to_null = [](auto ** list_) constexpr->size_t {
				size_t rez{ 0 };
				_ASSERTE(*list_ != nullptr);
				for (; list_[rez] != NULL; ++rez) {};
				return rez;
			};

			size_t evc = count_to_null(wenv);

			// extract the environment pointer list
			data_type
				wenvp_data{ command_line_data(evc, wenv) };
			app_env::map_type
				wenvp_map{};

			//transform env vars to k/v map
			// each envar entry is 
			// L"key=value" format
			for (auto kv : wenvp_data) {
				auto delimpos = kv.find(L"=");
				auto key = kv.substr(0, delimpos);
				auto val = kv.substr(delimpos + 1);
				wenvp_map[key] = val;
			}

			return make_tuple(argc, warg_data, evc, wenvp_map);
		};
	}

	// client code gets the instance of this
	class structure final {
	public:
		using string_type	= typename data_type::value_type;
		using cli_type		= data_type;
		using env_var_type	= map_type;

		const size_t			cli_args_count{};
		const data_type			cli_data{};
		const size_t			env_vars_count{};
		const map_type			env_vars{};

		// get's cli data -- no error check
		string_type operator [] (size_t pos_) const noexcept 
		{ return cli_data[pos_];  }
		// get's env var data -- no error check
		env_var_type::mapped_type operator []
		(env_var_type::key_type key_) const noexcept = delete;
				// { return env_vars[key_]; }

	private:

		void* operator new(std::size_t sz) = delete ;
		void operator delete(void* ptr) = delete ;

		structure(
			unsigned __int64 argc,
			data_type   argv,
			unsigned __int64 env_count,
			map_type	env_vars_
		)
			: cli_args_count(argc),
			cli_data(argv),
			env_vars_count(env_count),
			env_vars(env_vars_)
		{}

	public:
		static structure & instance() {
			auto once = []() {
				auto[argc, warg, envc, env_map] = inner::app_env_initor();
				return structure{ argc, warg, envc, env_map };
			};
			static structure app_env_single_instance_ = once();
			return app_env_single_instance_;
		}
	};

} // dbj::app_env
//--------------------------------------------------------------------------------------

// std::vector based implementation
// vector<string> args(argv + 1, argv + argc);
//
namespace dbjsys { namespace fm  {
//--------------------------------------------------------------------------------------
	/* Command Line Interface */
	class CLI final {

		::dbj::app_env::structure & app_env =
			::dbj::app_env::structure::instance();

	public:

		class not_found : protected std::runtime_error {
			not_found();
		public:
			
			// notorious std::exception suite does work only with narrow strings
			not_found(const std::wstring & m_) 
				: runtime_error(
					std::string(m_.begin(), m_.end())
				) {		}
			
			not_found(const std::string & m_) : runtime_error(m_) {		}


			const char * what() const noexcept {
				auto msg = std::string("dbj::fm::CLI::Error") + " not found: " + this->runtime_error::what();
				return msg.c_str();
			}
		};

		// argument 0 from the command line
		const ::dbjsys::string_type & exe_name() const noexcept
		{
			return this->app_env[0] ;
		} 

		// moving not allowed
		CLI(CLI &&) = delete;
		const CLI & operator = (CLI &&) = delete;
	protected :
		CLI() {	}
	public:
		static CLI & singleton() {
			static CLI singleton_;
			return singleton_;
		}

		// copying allowed
		CLI(const CLI & ) = default;
		CLI & operator = (const CLI & ) = default;

		::dbjsys::string_type operator [] (size_t idx_ )
		{
			assert(idx_ <  this->app_env.cli_data.size() );
			return this->app_env[idx_] ;
		}

		// return -1 if not found or index to the element found in the vector of arguments
		// tag to be found is asci string but actual params are CLI::string
		// that is string or wstring
		size_t find(const ::dbjsys::string_type & tag_) const 
		{
			auto iter = std::find(
				this->app_env.cli_data.begin(), 
				this->app_env.cli_data.end(), 
				tag_
			);
			return (iter == this->app_env.cli_data.end() ? -1
				: std::distance(this->app_env.cli_data.begin(), iter));
		}

		/*
		main extractor : gets by tag and casts the return value into the type desired
		uses variant as convertor
		*/
		template<typename T>
		T operator () (
			const ::dbjsys::char_type * cl_symbol, 
			const T & default_value ) const
		{
				try {
					auto vt = (*this)[cl_symbol];
					return (T)vt;
				}catch (CLI::not_found &) {
					return default_value;
				}
				catch (...) {
					// can not be converted to T
					std::string msg( BUFSIZ, 0x00 );
					sprintf(
						&msg[0], "Can not convert return value to type %s, CLI argument is: %s",
						typeid(default_value).name(), cl_symbol);
					dbjFMERR(msg.data());
				}
		}

		_variant_t operator [] (const ::dbjsys::char_type * cl_symbol) const
		{
			size_t pos = find(cl_symbol);
				if (pos == -1)	throw CLI::not_found(cl_symbol);

			return _variant_t(this->app_env[pos].data());
		}

#ifdef _UNICODE
		const ::dbjsys::char_type key_prefix = L'-';
#else
		const const ::dbjsys::char_type  key_prefix = '-';
#endif
		bool is_key(const size_t & pos ) const noexcept {
				return this->app_env[pos][0] == key_prefix;
		}

		using kvpair = std::pair<
			typename ::dbjsys::string_type, 
			std::vector<typename ::dbjsys::string_type> 
		>;
		///<summary>
		///return key value pair found by key 
		///value is vector of strings, consider example of a (slightly weird but valid) CL:
		///<code>-? -t 13 -name Abra Ka Dabra -bool TRUE</code>
		///above makes all the kvpairs like these:
		///<code>
		///"-?", {}
		///"-t", { "13" }
		///"-name", { "Abra", "Ka", "Dabra" }
		///"-bool", { "TRUE" }
		///</code>
		///</summary>
		 kvpair kv( const ::dbjsys::string_type & key) const 
		 {
				 auto kpos = find(key);

			 if (kpos == -1)
				     throw not_found(key);

			 size_t kpos2 = ++ kpos  ;
			 kvpair::second_type vec;

			 while (kpos2 < this->app_env.cli_args_count ) {
				 if (is_key(kpos2)) {
					 break;
				 } 
				 vec.push_back(this->app_env[kpos2]);
				 ++ kpos2;
			  }
			 return std::make_pair((::dbjsys::string_type)key, vec);
		}

	}; // eof CLI 
#if 1
	// use this as an universal cli argument that returns strings or wstrings 
	DBJINLINE 
		const ::dbjsys::string_type  clargument(
		const ::dbjsys::char_type  * cl_tag,
		const ::dbjsys::string_type  & def_val
	)
	{
		try {
#ifdef _DEBUG
			auto cli = CLI::singleton() ;
			auto vr = cli[cl_tag];
			auto rv = (::dbjsys::string_type)_bstr_t(vr);
			return rv;
#else
			return (::dbjsys::string_type)_bstr_t(CLI::singleton()[cl_tag]);
#endif
		}
		catch (CLI::not_found &){
			return def_val;
		}
	}
#endif
//--------------------------------------------------------------------------------------
} // namespace fm 
} // namespace dbj 
#if 1
/*
Implementaion from cpp moved bellow
 */
namespace dbjsys {
	namespace fm {
		//--------------------------------------------------------------------------------------
		namespace inner {

			// oo wrap up of the command line. ASCII version; uses __argv and __argc.
			// _UNICODE version uses __wargv and __argc.
			// NOTE:    this works regardless of the VS project type: 
			//          console, windows, even DLL app.
			//
			// Created  DBJ     03122000
			//
			// This is singleton as implementation
			// It is also thread safe
			// str::string or std:wstring are not used here
			// this depends on comutil.h
			//
			// 
			class CmdLineArguments final {

				ScopeLocker critical_section__;

				::dbj::app_env::structure & the_cli =
					::dbj::app_env::structure::instance();

				//--------------------------------------------------------
				// return true it str begin's with prefix
				// 
				static const bool begins_with(
					const dbjsys::char_type  * prefix, 
					const dbjsys::char_type  * str
				)
				{
					bool result_ = true;
					for (int j = 0; prefix[j] != 0; j++)
					{
						result_ = result_ && (prefix[j] == str[j]);
					}

					return result_;
				}
				//--------------------------------------------------------
				// return string part on the right of prefix given
				// we can assume that both arguments begin with the prefix string
				// and that prefix is shorter than str
				// 
				static const dbjsys::char_type  * right_of(
					const dbjsys::char_type  * prefix, 
					const dbjsys::char_type  * str
				)
				{
					int j = 0;
					while ((prefix[j]) && (prefix[j] == str[j])) j++;
					return str + j;
				}

			public:
				CmdLineArguments()	{	}

				// 
				~CmdLineArguments()	{ 	}

				// return true if symbol exist on the current command line
				const bool symbol_exists(const dbjsys::char_type  * prefix)
				{
					Lock auto_lock(critical_section__); // lock the whole instance

					const bool result_ = false; // default

					if ((!prefix) && (!*prefix))
						return result_; // anti jokers measure

					int j = 0;

					const dbjsys::char_type  * candidate_ = NULL;

					while (NULL != (candidate_ = operator [] (j++)))
					{
						if (!begins_with(prefix, candidate_))
							continue;
						return true; // FOUND!
					}
					return result_;
				}
				// 
				const ::dbjsys::char_type  * operator [] (size_t index) const
				{
					Lock auto_lock(critical_section__); // lock the whole instance
					if (index > ( the_cli.cli_data.size() - 1) ) return nullptr ;
					return the_cli[index].c_str();
				}
				//
				// return argument value of the argument that begins with a prefix
				// return NULL if prefix not found
				// argument prefix and value may have a space in between them, eg.:
				// "-f output.txt"
				// or not, eg.: "-foutput.txt"
				// In either case the same value shall be returned
				// In case of illegal cli rubish will be returned, eg.:
				// "-f -eoutput.err"  will return "-eoutput.err" if "-f" was the prefix given.
				// "-f <eol>"  will return "" if "-f" was the prefix given.
				// 
				//
				// 
				const dbjsys::char_type * operator [] (const dbjsys::char_type  * prefix) const
				{
					Lock auto_lock(critical_section__); // lock the whole instance

					const dbjsys::char_type  * result_{};

					if ((!prefix) && (!*prefix))
						return result_; // anti jokers measure

					int j = 0;

					while (NULL != (result_ = operator [] (j++)))
					{
						if (!begins_with(prefix, result_))
							continue;
						result_ = right_of(prefix, result_);
						if ((result_) && (*result_))
							break;
						result_ = operator [] (j); // next argument
						break;
					}
					return result_;
				}
				//------------------------------------------------------------
				// return argument found by its name
				// ret type is variant
				const dbjsys::char_type *  operator () (const dbjsys::char_type  * arg_name_) const
				{
					return this->operator [] (arg_name_);
				}

				//------------------------------------------------------------
				// the singleton method delivers one instance for all threads
				// I think we need no locking here ? DBJ 08042001
				// 
				static CmdLineArguments & instance()
				{
					static CmdLineArguments instance__;
					return instance__;
				}

			};

		} // inner namespace 

		inline inner::CmdLineArguments & cline_ = inner::CmdLineArguments::instance();

		// 
		// CAUTION:
		// cl_argument<bstr_t>, covers wchar_t uses perfectly well ...
		// CAUTION 2018 :)
		// no it does not
		//--------------------------------------------------------------------------------------
	} // namespace fm 
} // namespace dbj 
#endif

#if 1
namespace dbjsys {
	namespace fm {

		/*
		--------------------------------------------------------------------------------------

		oo wrap up of the command line. ASCII version; uses __argv and __argc.
		_UNICODE version uses __wargv and __argc.
		NOTE:    this works regardless of the VS project type:
		console, windows, even DLL app.

		Created		DBJ     03122000
		Improved		DBJ     26122005

		--------------------------------------------------------------------------------------
		the concept is to make a single command line argument encapsulation
		whose value is also of an exact static type (not just a string)
		so that in one go user get's the vale from the cli that is also of a required type

		since CLI conceptualy proced only "words" and "numbers" there are two helper typedefs

		dbjsys::fm::cli_argument_string;    // for words
		dbjsys::fm::cli_argument_long  ;    // for numbers

		Use as this:

		in this example we are expecting cli to have two tags with long values both
		with tags "-n" and "-s", so that cli is for example:

		whatever.exe -age 128 -s 2.0

		fun ( long, long ) ; // to be processed by this function internally

		// def.val must be given as a single ctor argument
		cli_argument_long     age( 12 ) ;
		cli_argument_long     str_len( 256 ) ;

		// now the usage is as simple as this
		fun ( age("-age"), str_len("-s") ) ;

		Usage semantics variant 2:

		We do not give tag as a ctor argument so that we have greater flexibility
		to re-use cl_argument<> instance for 2 or more tags but of the same type.
		Example of this usage logic:

		cli_argument_long  long_arg(13);

		auto age = long_arg(L"-age") ;     // get the -age cli argument of type int
		auto sln = long_arg(L"-str_len") ; // get the -str_len cli argument of the type int

		The slight drawback is that 13 is the def.val. for both -age and -length

		Usage semantics variant 3:

		If this is all too confusing one can use a method provided to enforce yet another
		usage semantics

		auto age = cli_argument<long>(L"-age",0);

		NOTES:

		intantiation of cl_argument<> is not expensive because it shares a singleton
		implementation in the back.

		Said implementation is also thread resilient.

		*/
		template<typename T> class cl_argument
		{
			// 
			T defval_{}; // default value
			T reqval_{}; // requested value
			// no default c-tor allowed
			cl_argument() {}
		public:

			using string_type = typename ::dbjsys::string_type;

			// type of the cli value for the cli tag given
			typedef T Type;
			// copy constructor must receive a default value for the cli arguments
			explicit
				cl_argument(const T & defval) : defval_(defval), reqval_(defval)
			{
			}

			const string_type & operator [] (const unsigned int & index) const
			{
				return cline_[index] ;
			}
			//--------------------------------------------------------------------------------------
			//
			//    cl_argument encapsulates single comand line argument. 
			//--------------------------------------------------------------------------------------
			const bool exists(const dbjsys::char_type   * const cl_symbol)
			{
				return cline_.symbol_exists(cl_symbol);
			}
			//--------------------------------------------------------------------------------------
			//
			// resolve the actual value and its type by c.l. symbol used 
			//
			//--------------------------------------------------------------------------------------
			const T & operator () (dbjsys::char_type * cl_symbol) // const
			{
					const dbjsys::char_type * vart = cline_(cl_symbol);

					if ( vart )	{
						this->reqval_ =  vart ;
					}
					else {
						this->reqval_ = this->defval_;
					}
				return this->reqval_;
			}

			// was the cmd line query made or/and was the last query OK ?
			const bool defval() { return defval_ == reqval_; }
		};
		//----------------------------------------------------------------------------------------------
		// common CLI arg types are just these two

		typedef typename dbjsys::fm::cl_argument<::dbjsys::string_type>   
			cli_argument_string;
		typedef typename dbjsys::fm::cl_argument<long>          
			cli_argument_long;

		//----------------------------------------------------------------------------------------------
		// use this as an universal cli method that enforces slightly different semantics from above
		/*
		template<class T>
		inline const T cli_argument_val(const wchar_t  * const cl_tag, const T def_val )
		{
		cl_argument<T> cliarg_(def_val);
		return cliarg_(cl_tag);
		}
		*/
#if 0
		// for string_type 
		inline const string_type cli_argument_(const wchar_t  * const cl_tag, const string_type & def_val)
		{
			cli_argument_string cliarg_(def_val);
			return cliarg_(cl_tag);
		}
		// for long
		inline const long cli_argument_(const wchar_t  * const cl_tag, const long & def_val)
		{
			cli_argument_long cliarg_(def_val);
			return cliarg_(cl_tag);
		}
		// just a "normal" indexed access to the CLI
		inline const string_type cli_argument_(unsigned int idx_)
		{
			cli_argument_string cliarg_(L"");
			return cliarg_[idx_];
		}
#endif
		namespace test {
			//--------------------------------------------------------------------------------------
			// usage example
			inline auto saberi = [] (auto a, auto b) {
				return a + b;
			};

			inline const void test_CmdLineArguments()
			{
				cl_argument<long>     no_of_elements(512);
				cl_argument<long>     str_len(256);
				// kukulele is made to prove that we do not need 
				// wchar_t forced instantiation since it is very nicely
				// substituted by _bstr_t instance of cl_argument<T>
				// 
				// so instead of this
				// cl_argument<wchar_t *> kukulele(L"uh") ;
				// declare this
				/*
				cl_argument<wchar_t *> kukulele(L"uh");
				wchar_t * clarg = kukulele(L"-name" );
				long r = saberi(no_of_elements( L"-n" ), 
					str_len( L"-s" ));
                 */
				// if cl was '-n 1024 -s 512' fun() above will receive 1024 and 512
				// if cl was '-n1024 -s512'   fun() above will receive 1024 and 512
				// if cl was '-n1024'         fun() above will receive 1024 and 256
				// if cl was ''               fun() above will receive  512 and 256
			}
		} // test
		  //--------------------------------------------------------------------------------------
	} // namespace fm 
} // namespace dbj 
#endif
