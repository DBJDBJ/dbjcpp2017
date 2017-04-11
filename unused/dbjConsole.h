// dbjConsole.h: interface for the dbjConsole class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include <assert.h>
#include <tchar.h>
#include <time.h>
#include <windows.h>
#include <process.h>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace dbj {

	class dbjConsoleImp;

	namespace cons {

	/*
	anonymous namespace for implementation
	*/

	namespace {
		class dbjConsoleImp
		{
			dbjConsoleImp() : mutex_(NULL)
			{
				hStdOut_ = ::GetStdHandle(STD_OUTPUT_HANDLE);
				/* Get display screen's text row and column information. */
				::GetConsoleScreenBufferInfo(hStdOut_, &csbi_);
			}

		public:
			void startMutex(const wchar_t * mutexName)
			{
				if (mutex_ != NULL)
					return;

				mutex_ = CreateMutex(
					(LPSECURITY_ATTRIBUTES)NULL, // pointer to security attributes 
					(BOOL)1, // flag for initial ownership 
					(LPCTSTR)mutexName  // pointer to mutex-object name 
				);
			}

			void releaseMutex()			{
				if (::ReleaseMutex(mutex_))
				{
					::CloseHandle(mutex_);
					mutex_ = NULL;
				}
			}
			wchar_t * scratchPad()		{
				return scratchPad_;
			}
			void setTitle(const wchar_t * title_)	{
				int result = ::SetConsoleTitle(title_);
				assert(result);
			}

			const wchar_t * getTitle()	{
				int result_ = ::GetConsoleTitle(scratchPad_, BUFSIZ);
				assert(result_);
				return scratchPad_;
			}

			~dbjConsoleImp() {
				::CloseHandle(hStdOut_);
			}
			
			static dbjConsoleImp & instance() {
				static dbjConsoleImp singleInstance_;
				return singleInstance_;
			}

			static void * instanceAsVoidPointer() {
				return (void *) & dbjConsoleImp::instance();
			}

		private:
			static wchar_t scratchPad_[BUFSIZ * sizeof(wchar_t)];
			CONSOLE_SCREEN_BUFFER_INFO csbi_;
			HANDLE hStdOut_;
		protected:
			HANDLE mutex_;
		};
		/* static member initialization */
		wchar_t dbjConsoleImp::scratchPad_[BUFSIZ * sizeof(wchar_t)] = L"";

	} /* eof anonymous namespace for implementation	*/

	class dbjConsole
	{
	public:
		static dbjConsole & instance() {
			static dbjConsole singleInstance_;
			return singleInstance_;
		}
		~dbjConsole()
		{
			::InterlockedDecrement(&done_);
		}
		static const long done() throw() { return done_; }

	protected:
		dbjConsole() : imp_(dbjConsoleImp::instance())
		{
			::InterlockedIncrement(&done_);
		}
	private:
		const dbjConsoleImp & imp_;
		static long done_;
	};

	long dbjConsole::done_ = 0;

	/* anonymous namespace for testing	*/
	namespace {

		static time_t startTime_;

		static void timeString(std::wstring & timeString_result_)
		{
			struct tm newtime;
			char * am_pm = "AM";
			time_t long_time;
			time(&long_time);                /* Get time as long integer. */

			double elapsed_time = difftime(long_time, startTime_);

			// newtime = localtime( &long_time ); /* Convert to local time. */
			if (0 != localtime_s(&newtime, &long_time)) { /* Convert to local time. */
				throw "timeString() could not call localtime_s()";
			}

			if (newtime.tm_hour > 12)        /* Set up extension. */
				am_pm = "PM";
			if (newtime.tm_hour > 12)        /* Convert from 24-hour */
				newtime.tm_hour -= 12;    /*   to 12-hour clock.  */
			if (newtime.tm_hour == 0)        /*Set hour to 12 if midnight. */
				newtime.tm_hour = 12;

			//if ( -1 == swprintf_s( result_, 26, L"%.19s %s", _wasctime_s( &newtime ), am_pm ))
			//	throw "timeString() could not call swprintf_s()";
			/*
			standard and best solution:
			--------------------------
			char buffer[256];
			strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", &your_tm);
			*/

			std::wstringstream  buffer;

			buffer << std::put_time(&newtime, L"%a %b %d %H:%M:%S %Y");

			// now the result is in `buffer.str()
			timeString_result_ = buffer.str();
		}

		/*
		beginthread() requires the following function pointer declaration
		void( __cdecl * thread_function )( void * ),

		titler runs on a separate thread repeatedly updating the console title
		with the current time stamp
		title depends on console implementation not on the public dbj console interface
		*/
		static void titler(void * arg)
		{
			time(&startTime_);

			try {

				dbjConsoleImp * dbjcon_ = (dbjConsoleImp*)arg;

				std::wstring buff_;

				while (0 != dbjConsole::done())
				{

					timeString(buff_ = dbjcon_->scratchPad());
					dbjcon_->setTitle(buff_.c_str());

					::Sleep(1000);
				} // while

			}
			catch (...)
			{
				/* Should never reach here */
				throw "titler() has caught an exception that it should not";
			}
		}
	} /* eof anonymous namespace for testing	*/

	inline void test()
	{
		/*
		start the console 
		*/
		dbjConsole::instance();
		/*
		and give its implementation instance to the titler thread function
		*/
		_beginthread(titler, 0, dbjConsoleImp::instanceAsVoidPointer() );

	}

} // eof namespace cons
} // eof namespace dbj