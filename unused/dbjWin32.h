#pragma once
#include <windows.h>
#include <wchar.h> /* removed tchar.h */
#include <stdio.h>
#include <iostream>
#include <unordered_map>

namespace dbj {


	namespace win32 {

		extern "C" const wchar_t * const printError(const wchar_t* msg);

		namespace get {

			namespace {
				typedef std::unordered_map<std::wstring, std::wstring> KV;
					static KV keyval = {};

					bool found(KV::key_type & k) {
#ifdef _DEBUG
						_ASSERTE(!k.empty());
						auto i1 = keyval.find(k);
						auto i2 = keyval.end();
						auto result = i1 == i2;
						return ! result;
#else
						return ! ( keyval.find(k) == keyval.end() );
#endif
					}

			} // eof implementation namespace

			enum { INFO_BUFFER_SIZE = 1024 };
/*
*/			
			typedef BOOL(* GetNameFP1)(LPWSTR, LPDWORD);

			template<GetNameFP1 fp>
			std::wstring get_sys_name(const wchar_t * f_name_) {
				static std::wstring the_name_;
				DWORD  bufCharCount = INFO_BUFFER_SIZE;
				if (the_name_.empty()) {
						the_name_.reserve(INFO_BUFFER_SIZE);
						if (!fp((LPWSTR)the_name_.c_str(), (LPDWORD)&bufCharCount)) {
							printError(f_name_);
							the_name_.clear();
						}
				}
				return the_name_;
			}
/*
*/
			typedef BOOL(*GetNameFP2)(LPWSTR, UINT);

			template<GetNameFP2 fp>
			  std::wstring get_sys_path( LPWSTR f_name_ ) {
				if (! found(std::wstring(f_name_)) ) {
					std::wstring the_name_;
					the_name_.reserve(INFO_BUFFER_SIZE);
					if (!fp((LPWSTR)the_name_.c_str(), (UINT)INFO_BUFFER_SIZE)) {
						throw dbj::utl::make_exception(printError(f_name_));
					}
#ifdef _DEBUG
					auto seemee =
#endif 
					keyval[f_name_] = the_name_;
				}
#ifdef _DEBUG
				auto seemee = keyval[f_name_];
#endif
				return  keyval[f_name_] ;
			}

			std::wstring computer_name() {
				return get_sys_name<GetComputerName>(L"GetComputerName");
			}

			std::wstring user_name() {
				return get_sys_name<GetUserName>(L"GetUserName");
			}

			const std::wstring system_directory() {
#ifdef _DEBUG
				KV::key_type key = L"GetSystemDirectory";
				auto sd = get_sys_path<(GetNameFP2)GetSystemDirectory>(L"GetSystemDirectory");
				auto val = keyval[key];
				return val;
#else
				return get_sys_path<(GetNameFP2)GetSystemDirectory>(L"GetSystemDirectory");
#endif
			}

			std::wstring windows_directory() {
				return get_sys_path<(GetNameFP2)GetWindowsDirectory>(L"GetWindowsDirectory");
			}
		}; //eof namespace get

		extern "C" const wchar_t * const printError(const wchar_t* msg)
		{
			DWORD eNum;
			wchar_t sysMsg[256];
			wchar_t* p;

			eNum = GetLastError();
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, eNum,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				sysMsg, 256, NULL);

			// Trim the end of the line and terminate it with a null
			p = sysMsg;
			while ((*p > 31) || (*p == 9))
				++p;
			do { *p-- = 0; } while ((p >= sysMsg) &&
				((*p == '.') || (*p < 33)));

			wchar_t buf[1024] = {};
			// Make the message
			_ASSERTE( swprintf_s(buf, 1024, TEXT("\n\t%s failed with error %d (%s)"), msg, eNum, sysMsg) > 0);
			// Display the message
			wprintf(buf);
			// Return the message
			return buf;
		}

	} // eof namespace win 32
} // eof namespace dbj