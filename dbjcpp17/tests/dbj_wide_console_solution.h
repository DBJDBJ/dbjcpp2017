#pragma once
// https://gist.githubusercontent.com/DBJDBJ/9d02f8dbfb0dceef372607af93eb53f2/raw/0f43a6ae18f48c9be1c5e6714c362c854e201e94/dbj_win_unicode_console_config.h
/*
----------------------------------------------------------------------
ACTUALLY:
http://archives.miloush.net/michkap/archive/2008/03/18/8306597.html
It seems this is very wizard like method?
Try this simeple test and see if it crashes your console:
<pre>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

int main(void) {
_setmode(_fileno(stdout), _O_U16TEXT);
wprintf(L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");
// following line is very likely to crash the UCRT
printf(L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd\n");
return 0;
}
</pre>

WARNING FROM BEFORE:

The following is a "hack" and shouild not be applied to the global app env.
It should be used inside functions in a local manner.
Neither "C" API or C++ stdlib, can work normaly if this is executed.
Probably no ANSI output will work at all.
----------------------------------------------------------------------
*/
/*
Copyright 2017 dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/* set to 1 if using UTF16 console output */
#define DBJU16 1

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#if DBJU16
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#endif

namespace dbj {
	namespace win {
		namespace console {
#if DBJU16

			/*
			WIN console is not unicode U16 by default
			thus console output of unicode string will not produce what is expected
			For example:
			const static std::wstring doubles = L"║═╚";
			const static std::wstring singles = L"│─└";

			std::wcout << doubles ;

			will not work is the following low level intialisatoon of console has not been
			performed

			CAUTION! Be sure to use wide char output functions only. Narow chard output will crash the 
			UCRT (https://support.microsoft.com/en-us/help/2999226/update-for-universal-c-runtime-in-windows)
			*/
			namespace {
				template<int OUTMODE = _O_U16TEXT >
				struct __declspec(novtable)
					CONSOLEINIT
				{
					unsigned int & counter()
					{
						static unsigned int counter_ = 0;
						return counter_;
					}
					/*
					*/
					CONSOLEINIT()
					{
						const unsigned & ctr = (counter())++;
						if (0 == ctr) {
							/*
							To print unicode into the Windows console one has to set it to UNICODE first.
							And then set it back so that UCRT does not throw. Solution is described here
							https://msdn.microsoft.com/en-us/library/tw4k6df8.aspx
							*/
							if (EOF == fflush(stdout)) {
								/* TODO: If fflush returns EOF, data may have been lost due to a write failure. */
							}
							_setmode(_fileno(stdout), OUTMODE);
							/*TODO: log the result here*/
						}
					}
					~CONSOLEINIT()
					{
						const unsigned ctr = --(counter());
						if (ctr < 1) {
							/*
							apparently must do this
							also make sure NOT to mix cout and wcout in the same executable!
							*/
							if (EOF == fflush(stdout)) {
								/* TODO: If fflush returns EOF, data may have been lost due to a write failure. */
							}
							_setmode(_fileno(stdout), _O_TEXT);
							/*TODO: log the result here*/
						}
					}
				};
				/*
				WARNING! No UCRT ANSI console output will work after this.
				*/
				static const CONSOLEINIT<> consoleinit__{};
			}
#endif // DBJU16
		} // console
	} // win
} // dbj

  /*
  If problems with them, one can undef the macros defined in here

  #undef WIN32_LEAN_AND_MEAN
  #undef VC_EXTRALEAN
  */
