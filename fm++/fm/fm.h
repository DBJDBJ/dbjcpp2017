//*****************************************************************************/
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
//	Library of Foundation Mechanisms
#pragma once
//----------------------------------------------------------------
/*
Warning	413	warning C4996: '_ftime64': This function or variable may be unsafe. 
Consider using _ftime64_s instead. To disable deprecation, use 
_CRT_SECURE_NO_WARNINGS. See online help for details.	

Warning	412	warning C4996: 'swprintf': swprintf has been changed to conform with the ISO C standard, 
adding an extra character count parameter. To use traditional Microsoft swprintf, 
set _CRT_NON_CONFORMING_SWPRINTFS.	

Warning	2	warning C4996: 'std::copy': Function call with parameters that may be unsafe - 
this call relies on the caller to check that the passed values are correct. 
To disable this warning, use -D_SCL_SECURE_NO_WARNINGS. 
See documentation on how to use Visual C++ 'Checked Iterators'

*/
//////////////////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_NON_CONFORMING_SWPRINTFS 1
#define _SCL_SECURE_NO_WARNINGS 1
//////////////////////////////////////////////////////////////////////////////////
#include "configuration.h" 
//////////////////////////////////////////////////////////////////////////////////
#if ! defined( WIN32_LEAN_AND_MEAN )
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif
#if ! defined( STRICT )
#define STRICT // better type checking for WIN32 API
#endif
#include <windows.h> 
#include <ctype.h>
#include <time.h>
#include <comdef.h>

#if ! defined ( DBJ_VERIFY )
#include <assert.h>
#define DBJ_VERIFY assert
#endif
/*
this makes crtdbg.h complain even it is not directly included
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC 
#endif
*/
//////////////////////////////////////////////////////////////////////////////////
// #pragma warning ( disable : 4786 )
// #pragma warning ( disable : 4231 )
//////////////////////////////////////////////////////////////////////////////////
#include <stdexcept>	// includes all the standard exception classes.
#include <iterator>		// (<iterator.h>)
#include <vector>		// (<vector.h>)
#include <map>			// (<map.h>
#include <set>			// (<set.h>)
#include <list>			// (<list.h>)
#include <deque>		// (<deque.h>)
#include <queue>		// (<queue.h>)
#include <stack>		// (<stack.h>)
#include <string>		// (<string.h>, <ostring.h>)
#include <algorithm>	// (<algo.h>,<advalgo.h>, <algorith.h>)
#include <functional>	// (<function.h>)
//----------------------------------------------------------------
#include <iosfwd>		// only IO streams forward declarations are here
                        // when and if need the definitions include <fm/dbjio.h>
                        // DBJ 02.04.2001
// we MUST declare io globals because they are not declared in <iosfwd>
// we use only 'WIDE' io OBJECTS
#include "dbjio.h"
//----------------------------------------------------------------

#include <utility>		// includes pair, relational operators, make_pair
#include <memory>		// includes allocator,raw_storage_iterator, temporary buffer
						// functions, uninitialized_copy,uninitialized_fill, auto_ptr.

#include <cstdio>
#include <cstring>
#include <cstdlib>

#ifndef  DBJINLINE
#define DBJINLINE static __forceinline
#endif // ! DBJINLINE

						/* 
for ADO utils 
#define FMPP_ADO_SERVICES
This requires (in adotools.h)
#import FMPP_ADO_DLL_FULL_PATH rename ( "EOF", "adoEOF" )
so
#ifdef FMPP_ADO_SERVICES
#define FMPP_ADO_DLL_FULL_PATH "C:\Program Files\Common Files\SYSTEM\ADO\msado15.dll"
#endif // FMPP_ADO_SERVICES

*/

//----------------------------------------------------------------
#define dbjNOVTABLE __declspec( novtable )


//----------------------------------------------------------------
#include "com/dbjguid.h" // GUID macros, predicates , etc...
#include "identifiable.h" // provides GUID get_guid()
#include "glob.h"
// use ATL for this DBJ 18NOV2000 
// #include "strcore.h" // ansi unicode conversion inlines
// use ATL for this DBJ 18NOV2000 
#include "errortplt.h"
#include "pairparser.h"
#include "uid.h"
//#include "factorybaseimplementation.h"
//#include "FACTORY.H"
//#include "inifile.h"
#include "rciptr.h"
#include "rcobject.h"
//#include "rcowner.h"
#include "rcptr.h"
//#include "rctotal.h"
#include "rcsptr.h"
//#include "resource.h"
#include "statefullobject.h"
#include "doctor.h"
#include "locks.h"
#include "prompt.h"

#include "token.h"
//----------------------------------------------
// COM related FM's
#include "com/comutils.h"
#include "com/wscuser.h"
// #include "com/dbjcomstarter.h"
#include "com/dump_interfaces_services.h"
#include "com/combool.h"

#include "win/threadlocalstorage.h"
#include "algo.h" // uses TLS

#include "mutex.h"
#include "fact/factory.h"
#include "threadMap.h"
#include "keyvaluepair.h"
#include "timestamp.h"
#include "timer.h"
//----------------------------------------------
// WIN32 platform extensions
//
#include "variant_extensions.h"
#include "win/bstr.h"
#include "win/scmquery.h"
#include "win/checkversion.h"
#include "win/stringr.h"
#include "win/event.h"
#include "win/ievlog.h"
#include "win/fileversion.h"

#include "wrap.h"
#include "lockedcontainer.h"
#include "hash.h"
#include "cmdline.h"

#include "com/DispCollectionHandler.h"
#include "com/dbjsyscomcategory.h"
#include "com/filestream.h"

//----------------------------------------------------------------
//----------------------------------------------------------------
// USE THIS TO PUT TEMPORARY REMINDER COMMENTS
// The following silliness is explained in just a few lines... Read on.
#define Stringize( L )			#L
#define MakeString( M, L )		M(L)
#define $Line					\
	MakeString( Stringize, __LINE__ )
#define TODO				\
	__FILE__ "(" $Line ") : [*TODO*]: "
// The preceeding silliness allows statements like:
//		#pragma message(Reminder "Fix this problem!")
// Which will cause messages like:
//		C:\Source\Tsr\TSRP3\TSRP3.h(47): Reminder: Fix this problem!
// to show up during compiles.  Note that you can NOT use the
// words "error" or "warning" in your reminders, since it will make
// the compiler think it should abort execution.  You can double click
// on these fine messages and jump to the line in question.
#if 0
//--------------------------------------------------------------------------------
namespace dbjsys {
	namespace fm {
		//--------------------------------------------------------------------------------
		// void switchErrLog(const wchar_t * name, const int appendLog);
		static const unsigned volatile int APPEND_TO_LOG = 1;
		// 
		struct  __DBJSYS_FM_LIB_INITOR__
		{
			// 
			static long counter_;
			// 
			__DBJSYS_FM_LIB_INITOR__();
			// 
			~__DBJSYS_FM_LIB_INITOR__();
		};

		static  __DBJSYS_FM_LIB_INITOR__	__dbjsys_fm_lib_initor__;

		//--------------------------------------------------------------------------------
		__DBJSYS_FM_LIB_INITOR__::__DBJSYS_FM_LIB_INITOR__()
		{
			// use and increment
			if (counter_++ == 0)
			{
				switchErrLog(::dbjsys::glob::DFLT_LOG_FILE(), APPEND_TO_LOG);

			}
		}
		__DBJSYS_FM_LIB_INITOR__::~__DBJSYS_FM_LIB_INITOR__()
		{
			// decrement annd use
			if (--counter_ == 0)
			{
				using namespace std;
				// IOSTREAM OBJECTS
				cout.flush();
				cerr.flush();
				clog.flush();
				// WIDE IOSTREAM OBJECTS
				wcout.flush();
				wcerr.flush();
				wclog.flush();
			}
		}

//#pragma comment(exestr, "DBJ*FM++ (c) 1997 - 2015 by DBJ Ltd.")
//#pragma comment(exestr, "DBJ*FM++ (c) 2010 - 2015 by DBJ.ORG ")
//#pragma warning( disable : 4073 ) 
//#pragma init_seg( lib )
		static long __DBJSYS_FM_LIB_INITOR__::counter_ = 0;
		//--------------------------------------------------------------------------------
	}
}
#endif
//--------------------------------------------------------------------------------

