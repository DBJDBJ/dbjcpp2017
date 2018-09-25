//-----------------------------------------------------------------------
//
// $Author: Dusan $
//
// $Revision: 8 $
//
//
#include "precomp.h"
//------------------------------------------------------------------------
using namespace dbjsys::fm ;
using namespace dbjsys::fm::bstr ;
//------------------------------------------------------------------------
//strcmp
struct NocaseStrEqual : public std::equal_to<std::string>
{
bool operator()( const std::string& x, const std::string& y ) const 
{
    const char * lpString1 = x.c_str() ;
    const char * lpString2 = y.c_str() ;

   size_t result = CompareStringA(
              LOCALE_SYSTEM_DEFAULT,// locale identifier
              NORM_IGNORECASE,      // comparison-style options
              lpString1,            // first string
              (long)x.size(),             // size of first string
              lpString2,            // second string
              (long)y.size()              // size of second string
    );

   return result == CSTR_EQUAL ;
}
} ;
//----------------------------------------------------------------------
// strcmp but ignore case
inline bool operator || ( 
            const std::string & s1,  
            const std::string & s2
         )
{
    static NocaseStrEqual comparator ;
    return comparator(s1, s2 ) ;
}
//-----------------------------------------------------------------------
inline char compareAndUpperizeChars ( const char & c1 , const char & c2 )
{
    return  toupper( c1 > c2 ? c1 : c2 ) ;
}
//-----------------------------------------------------------------------
extern "C" void test1( void )
{
    std::string                     s("AaBbCcDdeEfF") ;
    std::string                     r( "Hello" ) ;
    std::string::iterator sbegin =  s.begin() ;
    std::string::iterator send =    s.end() ;
    std::string::iterator rbegin =  r.begin() ;
    NocaseStrEqual                  nse ;

	prompt(L"Before transformation: ") << _bstr_t(s.c_str());
	
	std::transform( sbegin, send, sbegin, toupper ) ;
    std::transform( sbegin, send, sbegin, sbegin, compareAndUpperizeChars ) ;

    prompt( L"After transformation: " ) << _bstr_t(s.c_str()) ;

    prompt( _bstr_t(s.c_str()) ) << L" || " << _bstr_t(r.c_str()) << L" returned: " 
        << ( s || r  ? L"equal" : L"not equal" ) ;
}
//-----------------------------------------------------------------------
extern"C" void test2()
{
    _bstr_t bst = "ANSI" ;

    dbjsys::fm::prompt( bst ) ;

    bst = L"UNICODE" ;

    dbjsys::fm::prompt( bst ) ;
}
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
extern"C"  void test5_timestamp()
{
	dbjsys::fm::Timestamp ts_ ;

	dbjsys::fm::prompt( ts_() ) ;
}
//-----------------------------------------------------------------------
// tokenizers testing
//requires #define tokenizers_testing
extern"C"  void test6()
{
#if defined (tokenizers_testing)
 dbjsys::fm::token_test() ;
#endif
}
//
extern"C"  void test7_testing_thread_map()
{
#if defined ( testing_thread_map )
    dbjsys::fm::test_statefull_thread_map() ;
    dbjsys::fm::test_stateless_thread_map() ;
#endif
}

//
extern"C"  void test8()
{
#ifdef dbjsys_fm_tlsot_testing
    dbjsys::fm::tls_test() ;
#endif
}
namespace {
	
	using dbjsys::fm::algo::Hash;
	using std::string;

	DBJINLINE void hash_test()
	{
		auto show1 = [](const string & name) {std::printf("\nhash(%s) : %d", name.data(), (long)Hash<string>()(name)); };
		auto show2 = [](const _bstr_t & name) {std::printf("\nhash(%s) : %d", (char*)name, (long)Hash<_bstr_t>()(name)); };
		auto show3 = [](const wchar_t * name) {std::printf("\nhash(%S) : %d", name, (long)Hash<wchar_t *>()(name)); };

		auto applicator = [](auto first, auto last, auto function) {
			std::for_each(first, last, function);
		};

		_bstr_t names[] = { L"Pera", L"Zika", L"Mika", L"Laza" };
		applicator(names, names + 4, show2 );

		std::string tricky_names[] = { "PeraPeri", "ZikaZiki", "MikaMiki", "LazaLazi" };
		applicator(tricky_names, tricky_names + 4, show1 );

		const wchar_t * very_tricky_names[] = { L"Deepak the Proof Rider", L"Mahmudul the Big Equalizer", L"Ryan is gone", L"DBJ the Curator" };
		applicator(very_tricky_names, very_tricky_names + 4, show3);

	}

}
// Hash algo testing
// depends on Hash_algo_testing being defined!
extern"C"  void hash_test_()
{
	hash_test();
}

#if defined(__test_CmdLineArguments__)
extern "C" void test_CmdLineArguments()
{

	using namespace dbjsys::fm;

	cl_argument<long>     cla1(512);
	cl_argument<long>     cla2(256);

	fun(cla1(L"-n"), cla2(L"-s"));

	// in case command line was  
	// '-n 1024 -s 512' fun() above will receive 1024 and 512
	// '-n1024 -s512'   fun() above will receive 1024 and 512
	// '-n1024'         fun() above will receive 1024 and 256
	// empty            fun() above will receive  512 and 256

}
static const int dummy0 = DBJTESTREG(test_CmdLineArguments);
#endif

/**********************************************************************************/
DBJINLINE std::wostream & operator << (std::wostream & wos, const std::string & ss)
{
	return wos << (wchar_t*)_bstr_t(ss.c_str());
}
//
extern"C"  void token_test()
{
	btokenizer btoken(L"01234;5678;9", L";");
	prompt(L"tokenizerT<_bstr_t>");
	prompt(L"First word = ") << (wchar_t*)btoken[0];
	prompt(L"Second word = ") << (wchar_t*)btoken.getWord(1);
	prompt(L"Third word = ") << (wchar_t*)btoken.getWord(2);

	tokenizer token("01234;5678;9", ";");
	prompt(L"tokenizerT<std::string>");
	prompt(L"First word = ") << token[0];
	prompt(L"Second word = ") << token.getWord(1);
	prompt(L"Third word = ") << token.getWord(2);


	wtokenizer wtoken(L"01234;5678;9", L";");
	prompt(L"tokenizerT<std::wstring>");
	prompt(L"First word = ") << wtoken[0];
	prompt(L"Second word = ") << wtoken.getWord(1).c_str();
	prompt(L"Third word = ") << wtoken.getWord(2).c_str();

	return;
}


//-----------------------------------------------------------------------
// Register the test[s]
//-----------------------------------------------------------------------

DBJ_TREG( test1 ) ;
DBJ_TREG( test2 ) ;
DBJ_TREG(hash_test_) ;
DBJ_TREG( test5_timestamp ) ;
DBJ_TREG( test6 ) ;
DBJ_TREG( test7_testing_thread_map ) ;
DBJ_TREG( test8 ) ;
DBJ_TREG(token_test) ;
//-----------------------------------------------------------------------
// eofile
//-----------------------------------------------------------------------
