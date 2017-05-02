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
#if 0
/*
std::vector based implementation

vector<string> args(argv + 1, argv + argc);



*/
#endif
//--------------------------------------------------------------------------------------
namespace dbjsys { namespace fm  {
//--------------------------------------------------------------------------------------
	/* Command Line Interface */
	class CLI {
	public:
#ifdef _UNICODE
		typedef typename std::wstring string;
		using bstr = dbjsys::fm::bstr::wbstr ;
#else
		typedef typename std::string string;
		using bstr = dbjsys::fm::bstr::nbstr;
#endif

		typedef typename std::vector<string> CLIVector;
		typedef typename CLIVector::iterator Iterator_type;
		using value_type = CLIVector::value_type ;

		class not_found : protected std::runtime_error {
			not_found();
		public:
			// notorious std::exception suite dos work only with narrow strings
			not_found(const std::string & m_) : runtime_error(m_) {
			}
			const char * what() const noexcept {
				auto msg = std::string("dbj::fm::CLI::Error") + " not found: " + this->runtime_error::what();
				return msg.c_str();
			}
		};
	private:
		CLIVector args_vec;
	public:
		// argument 0 from the command line
		const string & exe_name() const noexcept
		{
#ifdef _UNICODE
			static string executable = __wargv[0]; 
#else
			static string executable = __argv[0];
#endif
			return executable;
		} 

		// moving not allowed
		CLI(CLI &&) {}
		const CLI & operator = (CLI &&) {}
	protected :
		CLI()
//			: args_vec( __wargv, __wargv + __argc)
//			: args_vec( __argv, __argv + __argc)
		{
#ifdef _DEBUG
#ifdef _UNICODE
			assert(__wargv);
			assert(__argc);
#else
			assert(__argv);
			assert(__argc);
#endif
#endif
			args_vec = std::vector<string>(__wargv + 1, __wargv + __argc);
		}
	public:
		static CLI & singleton() {
			static CLI singleton_;
			return singleton_;
		}

		// copying allowed
		CLI(const CLI & right ) : args_vec(right.args_vec) { }
		const CLI & operator = (const CLI & right ) {
			if (this != &right) {
				this->args_vec.clear();
				this->args_vec = right.args_vec;
			}
			return *this;
		}

		auto operator [] (size_t idx_ )
		{
			assert(idx_ < args_vec.size());
			return args_vec[idx_];
		}

		// return -1 if not found or index to the element found in the vector of arguments
		// tag to be found is asci string but actual parama are CLI::string
		// that is string or wstring
		auto find(const char * const tag_) const {
			CLI::string tag = fm::bstr::cast<CLI::string>(tag_);
				auto iter = std::find(args_vec.begin(), args_vec.end(), tag);
					return (iter == args_vec.end() ? -1 : std::distance(args_vec.begin(), iter));
		}

		/*
		main extractor : gets by tag and casts the return value into the type desired
		uses variant as convertor
		*/
		template<typename T>
		T operator () (const char * cl_symbol, const T & default_value ) const
		{
				try {
					auto vt = (*this)[cl_symbol];
					return (T)vt;
				}catch (CLI::not_found &) {
					return default_value;
				}
				catch (...) {
					// can not be converted to T
					std::string msg; msg.resize( BUFSIZ, 0x00 );
					sprintf(
						&msg[0], "Can not convert return value to type %s, CLI argument is: %s",
						typeid(default_value).name(), cl_symbol);
					dbjFMERR(msg.data());
				}
		}


		_variant_t operator [] (const char * cl_symbol) const
		{
			auto pos = find(cl_symbol);
			if (pos == -1)
				throw CLI::not_found(cl_symbol);
			return _variant_t(args_vec[pos].data());
		}
#ifdef _UNICODE
		const wchar_t key_prefix = L'-';
#else
		const char key_prefix = '-';
#endif
		bool is_key(const size_t & pos ) const noexcept {
				return args_vec[pos][0] == key_prefix;
		}

		using kvpair = std::pair<string, std::vector<string> >;
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
		 kvpair kv( const dbjsys::fm::bstr::nbstr & key) const {
				 auto kpos = find(key);

			 if (kpos == -1)
				     throw not_found(key);

			 size_t kpos2 = ++ kpos  ;
			 kvpair::second_type vec;

			 while (kpos2 < args_vec.size()) {
				 if (is_key(kpos2)) {
					 break;
				 } 
				 vec.push_back(args_vec[kpos2]);
				 ++ kpos2;
			  }
			 return std::make_pair((string)key, vec);
		}

	}; // eof CLI 
#if 0
	// use this as an universal cli argument that returns strings or wstrings 
	DBJINLINE 
		const CLI::value_type clargument(
		const char  * cl_tag, 
		const CLI::value_type & def_val
	)
	{
		try {
#ifdef _DEBUG
			auto cli = CLI::singleton() ;
			auto vr = cli[cl_tag];
			auto rv = (CLI::value_type)_bstr_t(vr);
			return rv;
#else
			return (CLI::value_type)_bstr_t(CLI::singleton()[cl_tag]);
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
#if 0
/*
Implementaion from cpp moved bellow
 */
namespace dbjsys {
	namespace fm {
		//--------------------------------------------------------------------------------------
		namespace {

#if defined( _UNICODE )
			typedef wchar_t value_type;
#else
#error DBJ*FM++ __FILE__ has to be compiled as UNICODE 
#endif

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
			class CmdLineArguments {

				// 
				ScopeLocker critical_section__;

				//--------------------------------------------------------
				// return true it str begin's with prefix
				// 
				static const bool begins_with(const value_type * prefix, const value_type * str)
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
				static const value_type * right_of(const value_type * prefix, const value_type * str)
				{
					int j = 0;
					while ((prefix[j]) && (prefix[j] == str[j])) j++;
					return str + j;
				}

				//--------------------------------------------------------
				// 
				value_type ** the_cli_arguments;
			public:
				// 
				int      number_of_arguments;
				//
				// UNICODE version only!
				CmdLineArguments()
					: the_cli_arguments(__wargv), number_of_arguments(__argc)
				{
					DBJ_VERIFY(the_cli_arguments);
				}

				// 
				~CmdLineArguments()
				{
					the_cli_arguments = 0;
				}

				// return true if symbol exist on the current command line
				const bool symbol_exists(const value_type * prefix)
				{
					Lock auto_lock(critical_section__); // lock the whole instance

					const bool result_ = false; // default

					if ((!prefix) && (!*prefix))
						return result_; // anti jokers measure

					register int j = 0;

					const value_type * candidate_ = NULL;

					while (NULL != (candidate_ = operator [] (j++)))
					{
						if (!begins_with(prefix, candidate_))
							continue;
						return true; // FOUND!
					}
					return result_;
				}
				// 
				const value_type * operator [] (const int index) const
				{
					Lock auto_lock(critical_section__); // lock the whole instance
					if (index < 0) return NULL;
					if (index >(number_of_arguments - 1)) return NULL;
					return the_cli_arguments[index];
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
				const value_type * operator [] (const value_type * prefix) const
				{
					Lock auto_lock(critical_section__); // lock the whole instance

					const value_type * result_ = (value_type*)0;

					if ((!prefix) && (!*prefix))
						return result_; // anti jokers measure

					register int j = 0;

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
				_variant_t operator () (const value_type * arg_name_) const
				{
					const value_type * cl_str = this->operator [] (arg_name_);
					if ((cl_str != 0) && (cl_str[0] != 0))
						return _variant_t(cl_str);
					else
						return _variant_t(); // VT_EMPTY
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

			CmdLineArguments & cline_ = CmdLineArguments::instance();
		}

		//--------------------------------------------------------------------------------------


		// 
		// CAUTION:
		// cl_argument<bstr_t>, covers wchar_t uses perfectly well ...
		//--------------------------------------------------------------------------------------
	} // namespace fm 
} // namespace dbj 
#endif

#if 0
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
			T defval_; // default value
			T reqval_; // requested value
					   // no default c-tor allowed
			cl_argument() {}
		public:
			// hide the following typedef  do not make it "library wide"
#if defined( _UNICODE )
			typedef wchar_t value_type;
			typedef _bstr_t string_type;
#else
			//		#warning DBJ*FM++ __FILE__ has to be compiled as UNICODE 
			typedef wchar_t value_type;
			typedef _bstr_t string_type;
#endif

			// type of the cli value for the cli tag given
			typedef T Type;
			// type of the string abstraction used
			typedef string_type String_type;
			// copy constructor must receive a default value for the cli arguments
			explicit
				cl_argument(const T & defval) : defval_(defval), reqval_(defval)
			{
			}

			const operator [] (const unsigned int & index) const
			{
				return string_type(cline_[index]);
			}
			//--------------------------------------------------------------------------------------
			//
			//    cl_argument encapsulates single comand line argument. 
			//--------------------------------------------------------------------------------------
			const bool exists(const value_type  * const cl_symbol)
			{
				return cline_.symbol_exists(cl_symbol);
			}
			//--------------------------------------------------------------------------------------
			//
			// resolve the actual value and its type by c.l. symbol used 
			//
			//--------------------------------------------------------------------------------------
			const T & operator () (const value_type  * const cl_symbol)
			{
				try {
					_variant_t vart = cline_(cl_symbol);

					if (VT_EMPTY != ((VARIANT)vart).vt)
					{
						this->reqval_ = (T)vart;
					}
					else {
						this->reqval_ = this->defval_;
					}
				}
				catch (const ::_com_error & cerr_) {
					assert(cerr_.ErrorMessage()); /* conversion failed */
				}
				return this->reqval_;
			}

			// was the cmd line query made or/and was the last query OK ?
			const bool defval() { return defval_ == reqval_; }
		};
		//----------------------------------------------------------------------------------------------
		// common CLI arg types are just these two

		typedef dbjsys::fm::cl_argument<string_type>   cli_argument_string;
		typedef dbjsys::fm::cl_argument<long>          cli_argument_long;

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
		namespace test {
			//--------------------------------------------------------------------------------------
			// usage example
			inline long saberi(long a, long b)
			{
				return a + b;
			}
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
				cl_argument<bstr_t> kukulele(L"uh");


				wchar_t * clarg = kukulele(L"-name");
				long r = saberi(no_of_elements(L"-n"), str_len(L"-s"));

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
