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
// This file contains simple inlines to aid simple ad-hoc testing app's
//190997 JovanovD
//
// Complete change, if interested in previous version look into the VSS DB
//14NOV20000 DBJ
#pragma once
#ifndef DBJINLINE
#define DBJINLINE static __forceinline
#endif // !DBJINLINE
/*
IDS_STRING1             "TESTING DBJSYSTEMS FM"
IDS_TEST_BEGIN          "Executing Test "
IDS_TEST_END            "Finished Test "
IDS_PROMPT              ":"
IDS_UNDERSCORE          "_"
IDS_SPACE               " "
IDS_ERR_MSG_1           "Unknown Exception Caught"
*/
//---------------------------------------------------------------------------------------
namespace dbjsys {
	namespace fm {
		//---------------------------------------------------------------------------------------
		namespace  prompt_imp
		{
			typedef SREF<CriticalSection> CRIT_REF;

			DBJINLINE CRIT_REF critical_section()
			{
				static CRIT_REF cs_;
				if (cs_.isNull())
					cs_ = new NOTCriticalSection();
				return cs_;
			}

			DBJINLINE  std::wostream & the_stream()
			{
				static std::wostream & the_stream_ = std::wcout;
				return the_stream_;
			}

		}
		//----------------------------------------------------------------------------------------------
		//                static std::wostream & the_stream_ = std::wcout ;
		//--------------------------------------------------------------------------------------------
		DBJINLINE std::wostream & prompt_stream()
		{
			AutoLocker auto_lock(*prompt_imp::critical_section());
			// default is 'wcout'
			return prompt_imp::the_stream();
		}
		//----------------------------------------------------------------------------------------------
		// switch to arbitrary std::wostream. But how?
		DBJINLINE void set_prompt_stream(std::wostream & new_stream)
		{
			AutoLocker auto_lock(*prompt_imp::critical_section());
			std::wostream & wos = prompt_imp::the_stream();
#if PIGS_ARE_FLYING
			wos = new_stream;
#endif
		}
		//----------------------------------------------------------------------------------------------
		DBJINLINE void reset_prompt_stream()
		{
			AutoLocker auto_lock(*prompt_imp::critical_section());
#if PIGS_ARE_FLYING
			prompt_imp::the_stream() = std::wcout;
#endif
		}
		//----------------------------------------------------------------------------------------------
		DBJINLINE unsigned int prompth_length(unsigned int change_to = 0)
		{
			AutoLocker auto_lock(*prompt_imp::critical_section());
			static unsigned int current_length = 0; // default

			if ( change_to > 0)
				current_length = change_to;
			return current_length;
		}
		//----------------------------------------------------------------------------------------------
		DBJINLINE std::wostream & prompt(
			const wchar_t *  p, /* prompt message */
			const wchar_t * promptChar = 0,
			const wchar_t * fillChar = 0
		)
		{
			AutoLocker auto_lock(*prompt_imp::critical_section());
			const int PROMPT_LENGTH = prompth_length();
			prompt_imp::the_stream()
				<< std::endl
				<< std::setw(PROMPT_LENGTH)
				<< std::setfill(0 != fillChar ? fillChar[0] : L' ')
				<< p
				<< std::setw(0);

			if (0 != promptChar) prompt_imp::the_stream() << promptChar;

			prompt_imp::the_stream().flush();
			return prompt_imp::the_stream();
		}

		//---------------------------------------------------------------------------------------------------------
		DBJINLINE std::wostream & prompt(const wchar_t * c_, const unsigned int N_)
		{
			AutoLocker auto_lock(*prompt_imp::critical_section());
#ifdef _DEBUG
			if (c_ == 0) return prompt_imp::the_stream();
			if (N_ < 2) return prompt_imp::the_stream(); // for jokers using this
			if (!iswprint(c_[0])) return prompt_imp::the_stream(); // more anti-joker measures
#endif
			return   prompt_imp::the_stream()
				<< std::endl
				<< std::setw(N_ - 1)
				<< std::setfill(c_[0])
				<< c_ /* provoke output */
				<< std::setw(0)
				<< std::setfill(L' ');
		}
		//--------------------------------------------------------------------------------
		// this template and instantiations bellow can be used as functors
		// when outputing values to prompt()

		template< class T>
		DBJINLINE void val2prompt(const T & arg)
		{
			prompt((const wchar_t *)arg);
		}

		DBJINLINE void val2prompt(const _bstr_t & arg)
		{
			prompt((const wchar_t *)arg);
		}

		DBJINLINE void val2prompt(const wchar_t* arg)
		{
			// prompt( (const wchar_t*)_bstr_t(arg)) ;
			prompt(arg);
		}
		DBJINLINE void val2prompt(const std::wstring & arg)
		{
			prompt(arg.c_str());
		}

		

	}  // fm
} // dbjsys
//-----------------------------------------------------------------------------
