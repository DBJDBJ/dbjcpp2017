
#pragma once

#include <windows.h>
/*
---------------------------------------------------------------------------
*/
#include <iostream>
#include <ios>
#include <fstream>
#include <conio.h>
#include <ctype.h>
#include <assert.h>

namespace dbj {
	namespace mutex_r_and_d {

		const int constantRun = 1;

		inline int stop_prompt(const char * msg_, const int requestedChar)
		{
			int keyChar_ = 0;
			int extChar_ = 0;

			// do {
			std::cout << std::endl << msg_ << std::flush;
			keyChar_ = _getch();

			if (keyChar_ == 0)
				extChar_ = _getch();

			if (toupper(keyChar_) == requestedChar)
				return toupper(keyChar_);

			if (keyChar_ == requestedChar)
				return keyChar_;
			// } while ( 1 ) ;

			return keyChar_;
		}
		/*
		---------------------------------------------------------------------------
			Test Interprocess Synchronization with Mutexes

			return 0 means stop this being called
		*/
		static int test_dbjMutex(void * arg_ = 0)
		{
			LPSECURITY_ATTRIBUTES	lpMutexAttributes = NULL;	// pointer to security attributes 
			BOOL					bInitialOwner = FALSE;	// flag for initial ownership 
			LPCTSTR					lpName = L"dbjmutex";	// pointer to mutex-object name 
			int						keyChar = 0;   // key pressed
			int						result_ = constantRun;

			HANDLE mutex_ = CreateMutex(lpMutexAttributes, bInitialOwner, lpName);

			assert(mutex_ != NULL);

			std::ofstream of("e:\\shared.txt", std::ios::app);

			std::cout << std::endl << "Waiting for 'dbjmutex' to get to the file ... " << std::endl << std::flush;

			int waitResult = ::WaitForSingleObject(mutex_, INFINITE);

			std::cout << std::endl << "Access granted ... " << std::endl << std::flush;

			switch (waitResult)
			{
				// If the function fails, the return value is WAIT_FAILED. 
				// To get extended error information, call GetLastError. 
			case WAIT_FAILED:
				break;

			case WAIT_ABANDONED:
				// The specified object is a mutex object that was not released 
				// by the thread that owned the mutex object before 
				// the owning thread terminated. 
				// Ownership of the mutex object is granted to the calling thread, 
				// and the mutex is set to nonsignaled.
				break;
			case WAIT_OBJECT_0:
				// The state of the specified object is signaled. 
				of << std::endl << "This line written by process::thread ["
					<< ::GetCurrentProcessId()
					<< "::"
					<< ::GetCurrentThreadId()
					<< "], time since boot-up was : "
					<< ::GetTickCount() / 1000 << " seconds."
					<< std::endl;

				assert(of.fail() == 0);

				::Sleep(1000);

				if (!constantRun)
					stop_prompt("Press 'R' to release mutex ", 'R');

				::ReleaseMutex(mutex_); // as soon as possible

				std::cout << std::endl << "Mutex released" << std::flush;
				break;
			case WAIT_TIMEOUT:
				// The time-out interval elapsed, and the object’s state is nonsignaled. 
				break;
			default:
				break;
			}; // 

			::CloseHandle(mutex_);

			if (!constantRun)
			{
				if ('R' == stop_prompt("Press 'R' to repeat test,any other key to finish ", 'R'))
					result_ = 1;
				else
					result_ = 0;
			}

			return result_;
		} // test_dbjMutex

		/*
		*/

		inline int test_mutex_dbj(int argc, char ** argv)
		{
			try {
				int c_ = 1;
				do {
					if (!test_dbjMutex())
						break;
				} while (c_); // any extended key get's us out
			}
			/*
			catch ( exception & x )
			{
				cerr << "\nException : " << x.what() ;
			}
			*/
			catch (...)
			{
				std::cerr << "\nUnknown Exception";
			}

			std::cerr << std::endl << argv[0] << " Done " << std::endl << std::flush;

			return 0;
		}
	} 	// eof namespace mutex_r_and_d 
} // eof namespace dbj