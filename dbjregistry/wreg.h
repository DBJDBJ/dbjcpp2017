#pragma once
#define DBJVERSION __DATE__ __TIME__
#pragma message("")
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma message("")
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION ) 
/* Copyright 2017 dbj@dbj.org

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
////////////////////////////////////////////////////////////////////////////////
//
// WinReg -- C++ Wrappers around Windows Registry APIs
//
// by Giovanni Dicanio <giovanni.dicanio@gmail.com>
//
//
////////////////////////////////////////////////////////////////////////////////


//==============================================================================
//
// *** NOTES ***
//
// Assume Unicode builds.
// TCHAR-model == OBSOLETE (sucks these days!)
//
//
// My Coding Style Rules for this module:
// --------------------------------------
// Max column limit = 100 columns
// Indent 4 spaces (more readable than 2)
// Aligned matching braces for better grouping and readability.
//
//==============================================================================
#include <windows.h>    // Windows Platform SDK
#include <assert.h>     // assert()
#include <stdexcept>    // std::invalid_argument, std::runtime_error
#include <string>       // std::wstring
#include <utility>      // std::swap()
#include <vector>       // std::vector
// C libraries
#include <string.h>     // wcslen()
#include <limits>       // numeric_limits

// #include "dbjregval.h"

namespace winreg
{
	//------------------------------------------------------------------------------
	// Exception indicating errors from Windows Registry APIs.
	//------------------------------------------------------------------------------
	/*
	DBJ removed inheritance from runtime_error
	*/
	class RegException
	{
		RegException() {}
	public:
		/*
		RegException(const wchar_t * msg, LONG errorCode)
			: errorMessage(msg)
			, m_errorCode(errorCode)
		{}
		*/

		RegException(const std::wstring& msg, LONG errorCode = 0 )
			: errorMessage(msg)
			, m_errorCode(errorCode)
		{}


		const LONG & code () const noexcept {
			return m_errorCode;
		}

		const std::wstring message () const noexcept {
			return errorMessage + win32_errstring(m_errorCode) ;
		}

		/*
		If err code is 0, or not given ::GetLastError() is used
		*/
		static std::wstring win32_errstring(DWORD lastErrorCode = 0)
		{
			std::wstring result;
			LPWSTR lpMsgBuf = NULL;

			if (lastErrorCode == 0) lastErrorCode = ::GetLastError();

			if (lastErrorCode != 0)
			{
				assert(FormatMessageW(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					lastErrorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPWSTR)&lpMsgBuf, 0, NULL)
				);
			}
			else {
				lpMsgBuf = L"";
			}
			result.assign(lpMsgBuf);
			::LocalFree(lpMsgBuf);
			return L"\nWIN32 Last Error:" + result;
		}

	private:
		LONG m_errorCode;
		std::wstring errorMessage;
	};

	std::wostream & operator << (std::wostream & wos, const RegException & rx) {
		return wos	<< std::endl		
					<< L"RegException :" << std::endl 
					<< L"Code:\t"		<< rx.code()	<< std::endl 
					<< L"Message:\t"	<< rx.message() << std::endl;
	}

	//------------------------------------------------------------------------------
	//
	// "Variant-style" Registry value.
	//
	// Currently supports the following value types:
	//
	//  Windows Registry Type       C++ higher-level type
	// -----------------------------------------------------------
	// REG_DWORD                    DWORD
	// REG_SZ                       std::wstring
	// REG_EXPAND_SZ                std::wstring
	// REG_MULTI_SZ                 std::vector<std::wstring>
	// REG_BINARY                   std::vector<BYTE>
	//
	//------------------------------------------------------------------------------
	class RegKey;
	// class RegValue;
		/*
	DBJ: this is obviously a template but no time right now ...
	*/
#if 0
	class RegValue
	{
		// Initialize empty (type is REG_NONE)
		RegValue() = default;

	public:

		enum struct TypeId : DWORD { 
			NONE		= REG_NONE, 
			DWORD		= REG_DWORD, 
			SZ			= REG_SZ, 
			EXPAND_SZ	= REG_EXPAND_SZ,
			MULTI_SZ	= REG_MULTI_SZ, 
			BINARY		= REG_BINARY 
		} ;

		// typedef DWORD TypeId; // REG_SZ, REG_DWORD, etc.

		RegValue( const std::wstring & name, TypeId typeId)	
			: name_(name), m_typeId(typeId)
		{
		}

		const std::wstring & name() const noexcept { return this->name_; }


		const TypeId & GetType() const	noexcept {
			return m_typeId;
		}


		bool IsEmpty() const	noexcept {
			return m_typeId == TypeId::NONE;
		}


		void Reset( TypeId type, const std::wstring & newname_ )	noexcept {
			ResetValues();
			m_typeId = type;
			this->name_ = newname_;
		}


		DWORD Dword() const	{
			if (m_typeId != TypeId::DWORD)
				throw std::invalid_argument("Dword() called on a non-DWORD registry value.");

			return m_dword;
		}

		/* DBJ: so that users are not left guessing the exact type declaration */
		typedef std::wstring String;
		/* DBJ: conversion operators may be dangerous */
		operator const String () const {
			if (m_typeId != TypeId::SZ)
				throw std::invalid_argument("String() called on a non-REG_SZ registry value.");

			return m_string;
		}
		/*
		const std::wstring & String() const	{

			if (m_typeId != TypeId::SZ)
				throw std::invalid_argument("String() called on a non-REG_SZ registry value.");

			return m_string;
		}
		*/


		const std::wstring & ExpandString() const		{

			if (m_typeId != TypeId::EXPAND_SZ)
			{
				throw std::invalid_argument(
					"ExpandString() called on a non-REG_EXPAND_SZ registry value.");
			}

			return m_expandString;
		}


		const std::vector<std::wstring> & MultiString() const	{
			if (m_typeId != TypeId::MULTI_SZ)
			{
				throw std::invalid_argument(
					"MultiString() called on a non-REG_MULTI_SZ registry value.");
			}

			return m_multiString;
		}


		const std::vector<BYTE> & Binary() const {
			if (m_typeId != TypeId::BINARY)
			{
				throw std::invalid_argument(
					"Binary() called on a non-REG_BINARY registry value.");
			}

			return m_binary;
		}

		/* DBJ: operator = (),	are used for setting's 
		   DBJ: note how argument is not a reference
		*/
		RegValue & operator = ( const DWORD new_val_ )	{
			if (m_typeId != TypeId::DWORD)
				throw RegException(L"REG value Type ID mismatch: DWORD required");
			m_dword = new_val_;
			return *this;
		}


		RegValue & operator = ( const std::wstring new_val_ )	{
			if (m_typeId != TypeId::SZ)
				throw RegException(L"REG value Type ID mismatch: REG_SZ required");
			m_string = new_val_;
			return *this;
		}

		/* DBJ: kludge -- spec operator '+' with different meaning */
		RegValue & operator + ( const RegValue::String new_val_) {
			if (m_typeId != TypeId::EXPAND_SZ)
				throw RegException(L"REG value Type ID mismatch: REG_EXPAND_SZ required");
					m_expandString = new_val_ ;
			return *this;
		}

		/* DBJ: so that users are not left guessing the exact type declaration */
		typedef std::vector<std::wstring> Stringvec;

		/* DBJ: note how std:swap<>() requires both args to be non const */
		RegValue & operator = ( Stringvec new_val_ )	{
			if (m_typeId != TypeId::MULTI_SZ)
				throw RegException(L"REG value Type ID mismatch: REG_MULTI_SZ required");
			std::swap( m_multiString , new_val_ );
			return *this;
		}

		/* DBJ: so that users are not left guessing the exact type declaration */
		typedef std::vector<BYTE> Bytevec;

		/* DBJ: not how std:swap<>() requires both args to be non const */
		RegValue & operator = (Bytevec  new_val_ )	{

			if (m_typeId != TypeId::BINARY)
				throw RegException(L"REG value Type ID mismatch: REG_BINARY required");
			std::swap( m_binary , new_val_ );
			return *this;
		}

		// *** IMPLEMENTATION ***
	private:
		// Win32 Registry value type
		TypeId m_typeId{ TypeId::NONE };

		DWORD m_dword{ 0 };                         // REG_DWORD
		std::wstring m_string;                      // REG_SZ
		std::wstring m_expandString;                // REG_EXPAND_SZ
		std::vector<std::wstring> m_multiString;    // REG_MULTI_SZ
		std::vector<BYTE> m_binary;                 // REG_BINARY

#pragma region DBJ
		/* DBJ added */
		std::wstring name_;
#pragma endregion

		// Clear all the data members representing various values (m_dword, m_string, etc.)
		void ResetValues()
		{
			m_dword = 0;
			m_string.clear();
			m_expandString.clear();
			m_multiString.clear();
			m_binary.clear();

			/* DBJ added */
			name_.clear() ;
		}

	};
#endif
//------------------------------------------------------------------------------
//                      Anonumous namespace Private Helper Functions
//------------------------------------------------------------------------------
	namespace {


		// MSVC emits a warning in 64-bit builds when assigning size_t to DWORD.
		// So, only in 64-bit builds, check proper size limits before conversion
		// and throw std::overflow_error if the size_t value is too big.
		DWORD SafeSizeToDwordCast(size_t size)
		{
#ifdef _WIN64
			if (size > static_cast<size_t>((std::numeric_limits<DWORD>::max)()))
			{
				throw std::overflow_error(
					"SafeSizeToDwordCast(): Input size_t too long, it doesn't fit in a DWORD.");
			}

			// This cast is now safe
			return static_cast<DWORD>(size);
#else
			// Just fine in 32-bit builds
			return size;
#endif
		}
	}
#if 0
	namespace {
		//
		// Helpers called by QueryValue() to read actual data from the registry.
		//
		// NOTE: The "valueSize" parameter contains the size of the value to be read in *BYTES*.
		// This is important for example to helper functions reading strings (REG_SZ, etc.), 
		// as usually std::wstring methods consider sizes in wchar_ts.
		//

		// Reads a REG_DWORD value.
		Reg::Reg_DWORD ReadValueDwordInternal(HKEY hKey, const std::wstring& valueName)
		{
			assert(hKey != nullptr);

			DWORD valueData = 0;
			DWORD valueSize = sizeof(valueData);

			LONG result = ::RegQueryValueEx(
				hKey,
				valueName.c_str(),
				nullptr, // reserved
				nullptr, // type not required in this helper: we're called as dispatching by QueryValue()
				reinterpret_cast<BYTE*>(&valueData),   // where data will be read
				&valueSize
			);
			assert(valueSize == sizeof(DWORD)); // we read a DWORD

			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegQueryValueEx() failed in returning REG_DWORD value.", result);
			}
			Reg::Reg_DWORD value{ valueName, valueData };
			return value;
		}


		// Reads a REG_SZ value.
		Reg::Reg_SZ ReadValueStringInternal(HKEY hKey, const std::wstring& valueName, DWORD valueSize)
		{
			assert(hKey != nullptr);

			// valueSize is in bytes, we need string length in wchar_ts
			const DWORD stringBufferLenInWchars = valueSize / sizeof(wchar_t);

			// Make room for result string
			std::wstring str;
			str.resize(stringBufferLenInWchars);

			DWORD sizeInBytes = valueSize;

			LONG result = ::RegQueryValueEx(
				hKey,
				valueName.c_str(),
				nullptr, // reserved
				nullptr, // not interested in type (we know it's REG_SZ)
				reinterpret_cast<BYTE*>(&str[0]),   // where data will be read
				&sizeInBytes
			);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegQueryValueEx() failed in returning REG_SZ value.", result);
			}

			//
			// In the remarks section of RegQueryValueEx()
			//
			// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724911(v=vs.85).aspx
			//
			// they specify that we should check if the string is NUL-terminated, and if it isn't,
			// we must add a NUL-terminator.
			//
			if (str[stringBufferLenInWchars - 1] == L'\0')
			{
				// Strip off the NUL-terminator written by the API
				str.resize(stringBufferLenInWchars - 1);
			}
			// The API didn't write a NUL terminator, at the end of the string, which is just fine,
			// as wstrings are automatically NUL-terminated.

			Reg::Reg_SZ value{ valueName, str };
			return value;
		}


		// Reads a REG_EXPAND_SZ value.
		Reg::Reg_EXPAND_SZ ReadValueExpandStringInternal(HKEY hKey, const std::wstring& valueName, DWORD valueSize)
		{
			// Almost copy-and-paste from ReadValueStringInternal()
			assert(hKey != nullptr);

			// valueSize is in bytes, we need string length in wchar_ts
			const DWORD stringBufferLenInWchars = valueSize / sizeof(wchar_t);

			// Make room for result string
			std::wstring str;
			str.resize(stringBufferLenInWchars);
			DWORD sizeInBytes = valueSize;

			LONG result = ::RegQueryValueEx(
				hKey,
				valueName.c_str(),
				nullptr, // reserved
				nullptr, // not interested in type (we know it's REG_EXPAND_SZ)
				reinterpret_cast<BYTE*>(&str[0]),   // where data will be read
				&sizeInBytes
			);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegQueryValueEx() failed in returning REG_EXPAND_SZ value.",
					result);
			}

			//
			// In the remarks section of RegQueryValueEx()
			//
			// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724911(v=vs.85).aspx
			//
			// they specify that we should check if the string is NUL-terminated, and if it isn't,
			// we must add a NUL-terminator.
			//
			if (str[stringBufferLenInWchars - 1] == L'\0')
			{
				// Strip off the NUL-terminator written by the API
				str.resize(stringBufferLenInWchars - 1);
			}
			// The API didn't write a NUL terminator, at the end of the string, which is just fine,
			// as wstrings are automatically NUL-terminated.

			Reg::Reg_EXPAND_SZ value{ valueName, str };
			return value;
		}


		// Reads a REG_BINARY value.
		std::vector<BYTE> ReadValueBinaryInternal(HKEY hKey, const std::wstring& valueName, DWORD valueSize)
		{
			assert(hKey != nullptr);

			// Data to be read from the registry
			std::vector<BYTE> binaryData(valueSize);
			DWORD sizeInBytes = valueSize;

			LONG result = ::RegQueryValueEx(
				hKey,
				valueName.c_str(),
				nullptr, // reserved
				nullptr, // not interested in type (we know it's REG_BINARY)
				binaryData.data(),   // where data will be read
				&sizeInBytes
			);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegQueryValueEx() failed in returning REG_BINARY value.", result);
			}

			return binaryData;
		}


		// Reads a REG_MULTI_SZ value.
		Reg::Reg_MULTI_SZ ReadValueMultiStringInternal(HKEY hKey, const std::wstring& valueName, DWORD valueSize)
		{
			assert(hKey != nullptr);

			// Multi-string parsed into a vector of strings
			std::vector<std::wstring> multiStrings;

			// Buffer containing the multi-string
			std::vector<wchar_t> buffer(valueSize);

			DWORD sizeInBytes = valueSize;
			LONG result = ::RegQueryValueEx(
				hKey,
				valueName.c_str(),
				nullptr, // reserved
				nullptr, // not interested in type (we know it's REG_MULTI_SZ)
				reinterpret_cast<BYTE*>(buffer.data()),   // where data will be read
				&sizeInBytes
			);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegQueryValueEx() failed in returning REG_MULTI_SZ value.",
					result);
			}

			// Scan the read multi-string buffer, and parse the single various strings,
			// adding them to the result vector<wstring>.
			const wchar_t* pszz = buffer.data();
			while (*pszz != L'\0')
			{
				// Get current string length
				const size_t len = wcslen(pszz);

				// Add this string to the resulting vector
				multiStrings.push_back(std::wstring(pszz, len));

				// Point to next string (or end: \0)
				pszz += len + 1;
			}

			Reg::Reg_MULTI_SZ value{ valueName, multiStrings };
			return value;
		}



		//
		// Helpers for SetValue()
		//

		void WriteValueBinaryInternal(HKEY hKey, const std::wstring& valueName, const Reg::Bytevec & data)
		{
			assert(hKey != nullptr);

			const DWORD dataSize = SafeSizeToDwordCast(data.size());
			LONG result = ::RegSetValueEx(
				hKey,
				valueName.c_str(),
				0, // reserved
				REG_BINARY,
				&data[0],
				dataSize);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegSetValueEx() failed in writing REG_BINARY value.", result);
			}
		}


		void WriteValueDwordInternal(HKEY hKey, const std::wstring& valueName, const DWORD data)
		{
			assert(hKey != nullptr);
			const DWORD dataSize = sizeof(data);
			LONG result = ::RegSetValueEx(
				hKey,
				valueName.c_str(),
				0, // reserved
				REG_DWORD,
				reinterpret_cast<const BYTE*>(&data),
				dataSize);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegSetValueEx() failed in writing REG_DWORD value.", result);
			}
		}


		void WriteValueStringInternal(HKEY hKey, const std::wstring& valueName, const std::wstring & value)
		{
			assert(hKey != nullptr);

			const std::wstring& str = value ;

			// According to MSDN doc, this size must include the terminating NUL
			// Note that size is in *BYTES*, so we must scale by wchar_t.
			const DWORD dataSize = SafeSizeToDwordCast((str.size() + 1) * sizeof(wchar_t));

			LONG result = ::RegSetValueEx(
				hKey,
				valueName.c_str(),
				0, // reserved
				REG_SZ,
				reinterpret_cast<const BYTE*>(str.c_str()),
				dataSize);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegSetValueEx() failed in writing REG_SZ value.", result);
			}
		}


		void WriteValueExpandStringInternal(HKEY hKey, const std::wstring& valueName, const std::wstring & str )
		{
			assert(hKey != nullptr);
			/* 
			assert(value.GetType() == RegValue::TypeId::EXPAND_SZ);
			const std::wstring & str = value.ExpandString();
			*/

			// According to MSDN doc, this size must include the terminating NUL.
			// Note that size is in *BYTES*, so we must scale by wchar_t.
			const DWORD dataSize = SafeSizeToDwordCast((str.size() + 1) * sizeof(wchar_t));

			LONG result = ::RegSetValueEx(
				hKey,
				valueName.c_str(),
				0, // reserved
				REG_EXPAND_SZ,
				reinterpret_cast<const BYTE*>(str.c_str()),
				dataSize);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegSetValueEx() failed in writing REG_EXPAND_SZ value.", result);
			}
		}


		void WriteValueMultiStringInternal(
			HKEY hKey, 
			const std::wstring& valueName, 
			const std::vector<std::wstring> & multiString
			/*const Reg::Reg_XX& value*/)
		{
			assert(hKey != nullptr);

			// We need to build a whole array containing the multi-strings, with double-NUL termination
			std::vector<wchar_t> buffer;

			// Get total buffer size, in wchar_ts
			size_t totalLen = 0;
			for (const std::wstring& s : multiString)
			{
				// +1 to include the terminating NUL for current string
				totalLen += (s.size() + 1);
			}
			// Consider another terminating NUL (double-NUL-termination)
			totalLen++;

			// Optimization: reserve room in the multi-string buffer
			buffer.resize(totalLen);

			// Deep copy the single strings in the buffer
			if (!multiString.empty())
			{
				wchar_t* dest = &buffer[0];
				for (const std::wstring& s : multiString)
				{
					// Copy the whole string to destination buffer, including the terminating NUL
					wmemcpy(dest, s.c_str(), s.size() + 1);

					// Skip to the next string slot
					dest += s.size() + 1;
				}

				// Add another NUL terminator
				*dest = L'\0';
			}
			else
			{
				// Just write two NULs in the buffer
				buffer.resize(2);
				buffer[0] = L'\0';
				buffer[1] = L'\0';
			}

			// Size is in *BYTES*
			const DWORD dataSize = SafeSizeToDwordCast(buffer.size() * sizeof(wchar_t));

			LONG result = ::RegSetValueEx(
				hKey,
				valueName.c_str(),
				0, // reserved
				REG_MULTI_SZ,
				reinterpret_cast<const BYTE*>(buffer.data()),
				dataSize);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegSetValueEx() failed in writing REG_MULTI_SZ value.", result);
			}
		}


#endif


		std::vector<std::wstring> EnumerateSubKeyNames(HKEY hKey)
		{
			assert(hKey != nullptr);

			// Get sub-keys count and max sub-key name length
			DWORD subkeyCount = 0;
			DWORD maxSubkeyNameLength = 0;
			LONG result = ::RegQueryInfoKey(
				hKey,
				nullptr, nullptr,           // not interested in user-defined class of the key 
				nullptr,                    // reserved
				&subkeyCount,               // how many sub-keys here? 
				&maxSubkeyNameLength,       // useful to preallocate a buffer for all keys
				nullptr, nullptr, nullptr,  // not interested in all this stuff
				nullptr, nullptr, nullptr   // (see MSDN doc)
			);
			if (result != ERROR_SUCCESS)
			{
				throw RegException(L"RegQueryInfoKey() failed while trying to get sub-keys info.", result);
			}

			// Result of the function
			std::vector<std::wstring> subkeyNames;

			// Temporary buffer to read sub-key names into
			std::vector<wchar_t> subkeyNameBuffer(maxSubkeyNameLength + 1); // +1 for terminating NUL

																			// For each sub-key:
			for (DWORD subkeyIndex = 0; subkeyIndex < subkeyCount; subkeyIndex++)
			{
				DWORD subkeyNameLength = SafeSizeToDwordCast(subkeyNameBuffer.size()); // including NUL

				result = ::RegEnumKeyEx(
					hKey,
					subkeyIndex,
					&subkeyNameBuffer[0],
					&subkeyNameLength,
					nullptr, nullptr, nullptr, nullptr);
				if (result != ERROR_SUCCESS)
				{
					throw RegException(L"RegEnumKeyEx() failed trying to get sub-key name.", result);
				}

				// When the RegEnumKeyEx() function returns, subkeyNameBufferSize
				// contains the number of characters read, *NOT* including the terminating NUL
				subkeyNames.push_back(std::wstring(subkeyNameBuffer.data(), subkeyNameLength));
			}

			return subkeyNames;
		}


		std::vector<std::wstring> EnumerateValueNames(HKEY hKey)
		{
			assert(hKey != nullptr);

			// Get values count and max value name length
			DWORD valueCount = 0;
			DWORD maxValueNameLength = 0;
			LONG result = ::RegQueryInfoKey(
				hKey,
				nullptr, nullptr,
				nullptr,
				nullptr, nullptr,
				nullptr,
				&valueCount, &maxValueNameLength,
				nullptr, nullptr, nullptr);
			if (result != ERROR_SUCCESS)
			{
				throw RegException(L"RegQueryInfoKey() failed while trying to get value info.", result);
			}

			std::vector<std::wstring> valueNames;

			// Temporary buffer to read value names into
			std::vector<wchar_t> valueNameBuffer(maxValueNameLength + 1); // +1 for including NUL

																		  // For each value in this key:
			for (DWORD valueIndex = 0; valueIndex < valueCount; valueIndex++)
			{
				DWORD valueNameLength = SafeSizeToDwordCast(valueNameBuffer.size()); // including NUL

																					 // We are just interested in the value's name
				result = ::RegEnumValue(
					hKey,
					valueIndex,
					&valueNameBuffer[0],
					&valueNameLength,
					nullptr,    // reserved
					nullptr,    // not interested in type
					nullptr,    // not interested in data
					nullptr     // not interested in data size
				);
				if (result != ERROR_SUCCESS)
				{
					throw RegException(L"RegEnumValue() failed to get value name.", result);
				}

				// When the RegEnumValue() function returns, valueNameLength
				// contains the number of characters read, not including the terminating NUL
				valueNames.push_back(std::wstring(valueNameBuffer.data(), valueNameLength));
			}

			return valueNames;
		}
#if 0

		Reg::IRegVal::PointerT 
		QueryValue(	HKEY hKey, 	const std::wstring& valueName )
		{
			assert(hKey != nullptr);

			TypeSize ts_ = query_value_id_and_data_size(hKey, valueName);

			// According to this MSDN web page:
			//
			// "Registry Element Size Limits"
			//  https://msdn.microsoft.com/en-us/library/windows/desktop/ms724872(v=vs.85).aspx
			//
			// "Long values (more than 2,048 bytes) should be stored in a file, 
			// and the location of the file should be stored in the registry. 
			// This helps the registry perform efficiently."
			//

			Reg::IRegVal::PointerT retval = nullptr;

			// Dispatch to internal helper function based on the value's type
			switch (ts_.valueType)
			{
			case REG_BINARY:    return retval = new  ReadValueBinaryInternal(hKey, valueName, ts_.dataSize);
				break;
			case REG_DWORD:     return retval = new  ReadValueDwordInternal(hKey, valueName);
				break;
			case REG_SZ:        return retval = new ReadValueStringInternal(hKey, valueName, ts_.dataSize);
				break;
			case REG_EXPAND_SZ: return retval = new ReadValueExpandStringInternal(hKey, valueName, ts_.dataSize);
				break;
			case REG_MULTI_SZ:  return retval = new ReadValueMultiStringInternal(hKey, valueName, ts_.dataSize);
				break;
			default:
				throw std::invalid_argument("Unsupported Windows Registry value type.");
			}
			return retval;
		}

		template< Reg::TypeId TID, typename T >
		void SetValueInternal(HKEY hKey, const std::wstring& valueName, const Reg::RegVal<TID, T> & reg_val_)
		{
			assert(hKey != nullptr);

			switch (reg_val_.GetType())
			{
				/*
				DBJ: six function bellow are now non dependant on RegVal
				*/
			case Reg::TypeId::BINARY:    return WriteValueBinaryInternal(hKey, valueName, reg_val_.Binary());
			case Reg::TypeId::DWORD:     return WriteValueDwordInternal(hKey, valueName, reg_val_.Dword());
			case Reg::TypeId::SZ:        return WriteValueStringInternal(hKey, valueName, (Reg::StringT)reg_val_);
			case Reg::TypeId::EXPAND_SZ: return WriteValueExpandStringInternal(hKey, valueName, reg_val_.ExpandString());
			case Reg::TypeId::MULTI_SZ:  return WriteValueMultiStringInternal(hKey, valueName, reg_val_.MultiString());

			default:
				throw std::invalid_argument("Unsupported Windows Registry value type.");
			}
		}


		void DeleteValue(HKEY hKey, const std::wstring& valueName)
		{
			assert(hKey != nullptr);

			LONG result = ::RegDeleteValue(hKey, valueName.c_str());
			if (result != ERROR_SUCCESS)
			{
				throw RegException(L"RegDeleteValue() failed.", result);
			}
		}


		std::wstring ExpandEnvironmentStrings(const std::wstring& source)
		{
			DWORD requiredLen = ::ExpandEnvironmentStrings(source.c_str(), nullptr, 0);
			if (requiredLen == 0)
			{
				return std::wstring(); // empty
			}

			std::wstring str;
			str.resize(requiredLen);
			DWORD len = ::ExpandEnvironmentStrings(source.c_str(), &str[0], requiredLen);
			if (len == 0)
			{
				// Probably error?
				// ...but just return an empty string if can't expand.
				return std::wstring();
			}

			// Size (len) returned by ExpandEnvironmentStrings() includes the terminating NUL,
			// so subtract - 1 to remove it.
			str.resize(len - 1);
			return str;
		}


		std::wstring ValueTypeIdToString(const Reg::TypeId & typeId)
		{
			switch (typeId)
			{
			case Reg::TypeId::BINARY:    return L"REG_BINARY";       break;
			case Reg::TypeId::DWORD:     return L"REG_DWORD";        break;
			case Reg::TypeId::EXPAND_SZ: return L"REG_EXPAND_SZ";    break;
			case Reg::TypeId::MULTI_SZ:  return L"REG_MULTI_SZ";     break;
			case Reg::TypeId::SZ:        return L"REG_SZ";           break;
			case Reg::TypeId::NONE:      return L"REG_NONE";         break;

			default:
				// Should I throw?
				return L"Unsupported/Unknown registry value type";
				break;
			}
		}


		void LoadKey(HKEY hKey, const std::wstring& subKey, const std::wstring& filename)
		{
			LONG result = ::RegLoadKey(hKey, subKey.c_str(), filename.c_str());
			if (result != ERROR_SUCCESS)
			{
				throw RegException(L"RegLoadKey failed.", result);
			}
		}


		void SaveKey(HKEY hKey, const std::wstring& filename, LPSECURITY_ATTRIBUTES security = nullptr)
		{
			LONG result = ::RegSaveKey(hKey, filename.c_str(), security);
			if (result != ERROR_SUCCESS)
			{
				throw RegException(L"RegSaveKey failed.", result);
			}
		}
	} // anon namespace
#endif
	  //------------------------------------------------------------------------------
	  // Convenient C++ wrapper on raw HKEY registry key handle.
	  //
	  // This class is movable but non-copyable. 
	  // Key is automatically closed by the destructor.
	  // *Don't* call ::RegCloseKey() on this key wrapper!
	  //------------------------------------------------------------------------------
	class RegKey
	{
		RegKey() noexcept
			: the_key_(nullptr)
		{}
		
		/* DBJ: mover is here so that compiler does not generate one 
		        and also it is not accidentaly used
         */
		RegKey& operator=(RegKey&& other) noexcept
		{
		if (&other != this)	{
	        	Close();
				the_key_ = other.the_key_; other.the_key_ = nullptr;
     		}
		return *this;
		}

	public:

		static HKEY ConnectRegistry(const std::wstring& machineName, HKEY hKey)
		{
			HKEY hKeyResult = nullptr;
			LONG result = ::RegConnectRegistry(machineName.data(), hKey, &hKeyResult);
			if (result != ERROR_SUCCESS)
			{
				throw RegException(L"RegConnectRegistry failed.", result);
			}

			return hKeyResult ;
		}

		/* DBJ: in essence a factory method */
		static RegKey OpenKey(HKEY hKey, const std::wstring& subKeyName, REGSAM accessRights = KEY_READ)
		{
			assert(hKey != nullptr);

			HKEY hKeyResult = nullptr;
			LONG result = ::RegOpenKeyEx(
				hKey,
				subKeyName.data(),
				0, // no special option of symbolic link
				accessRights,
				&hKeyResult
			);
			if (result != ERROR_SUCCESS)
			{
				throw RegException(
					L"RegOpenKeyEx() failed trying opening a key:{\n" + subKeyName + L"\n}\n",
					result
				);
			}
				return RegKey(hKey,hKeyResult, subKeyName);
		}

		/* 
		Return the handle to the subkey, relative to the key handle sent in
		DBJ: also a factory method 
		*/
		static RegKey CreateKey(
			HKEY hKey_parent, 
			const std::wstring& subKeyName,
			DWORD options = 0, 
			REGSAM accessRights = KEY_WRITE | KEY_READ,
			LPSECURITY_ATTRIBUTES securityAttributes = nullptr,
			LPDWORD disposition = nullptr)
		{
			assert(hKey_parent != nullptr);

			HKEY hKeyResult = nullptr;
			LONG result = ::RegCreateKeyEx(
				hKey_parent,
				subKeyName.data(),
				0,          // reserved
				nullptr,    // no user defined class
				options,
				accessRights,
				securityAttributes,
				&hKeyResult,
				disposition
			);
			if (result != ERROR_SUCCESS)
			{
				throw RegException(L"RegCreateKeyEx() failed.", result);
			}

			return RegKey(hKey_parent, hKeyResult, subKeyName );
		}


		RegKey(HKEY parent_, HKEY hKey, std::wstring path_) noexcept
			: parent_key_(parent_), the_key_(hKey), the_key_name_(path_)
		{}

		/* DBJ: mover ctor */
		RegKey(RegKey&& other) noexcept
		{
			this->Swap(other);
		}

		~RegKey() noexcept
		{
			Close();
		}


		HKEY Handle() const noexcept
		{
			return the_key_;
		}


		bool IsValid() const noexcept
		{
			return the_key_ != nullptr;
		}
		/*
		return true if this instance is a hndle to one of the predefined keys
		if arg key is sent use that one
		note: not all the keys are supported as desribed here 
		https://msdn.microsoft.com/en-us/library/ms724836%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
		*/
		static const bool isTop( const HKEY & the_key_) noexcept
		{

			if (the_key_ == HKEY_CLASSES_ROOT)	return true;
			if (the_key_ == HKEY_CURRENT_CONFIG) return true;
			if (the_key_ == HKEY_CURRENT_USER)	return true;
			if (the_key_ == HKEY_LOCAL_MACHINE) return true;
			if (the_key_ == HKEY_USERS)			return true;

			return false;
		}

		/*
		We can delete this key only if it has no subkeys!
		This is deliberate design decision.
		*/
		void Delete(REGSAM view = KEY_WOW64_64KEY)
		{
			assert(  this->the_key_ != nullptr);
			if (isTop(this->the_key_))
				throw RegException(L"Delete() refused on top level registry key");

			auto result = ::RegDeleteKey(this->parent_key_, this->key_name().data());

			if (result != ERROR_SUCCESS)
			{
				throw RegException(L"RegDeleteTree() failed.", result);
			}
		}

		void Swap(RegKey& other) noexcept;

		const std::wstring key_name() const noexcept { return this->the_key_name_; }

	private:
		// 
		const HKEY &	null_key = HKEY{};
		HKEY			parent_key_{};
		HKEY			the_key_{};
		std::wstring	the_key_name_{};

		void Close() noexcept
		{
			if (the_key_ != nullptr)
			{
				assert(::RegCloseKey(the_key_) == ERROR_SUCCESS);
			}
			the_key_ = nullptr;
		}
	};

	// we follow the std::swap() "requirement"
	void RegKey::Swap(RegKey& other) noexcept
	{
		std::swap(parent_key_, other.parent_key_);
		std::swap(the_key_, other.the_key_);
		std::swap(the_key_name_, other.the_key_name_);
	}

	void swap(RegKey& lhs, RegKey& rhs) noexcept
	{
		lhs.Swap(rhs);
	}

	/*--------------------------------------------------------------*/

} // namespace winreg


