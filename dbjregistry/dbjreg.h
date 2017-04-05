#pragma once
#define DBJVERSION __DATE__ __TIME__
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
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

#include <windows.h>
#include <assert.h>     // see https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/assert-macro-assert-wassert
#include <string>       // std::wstring
#include <utility>      // std::swap()
#include <vector>       // std::vector
#include <map>	        // std::map
#include <memory>       // std::unique_ptr

#ifndef UNICODE
#error Me require UNICODE. Me strict.
#endif
#ifndef _WINDOWS_
#error  Me require WINDOWS. Me strict.
#endif

namespace Reg {
	using std::wstring;
	using std::vector;
	using std::map;

	typedef std::wstring				StringT;
	typedef std::vector<std::wstring>	Stringvec;
	typedef std::vector<BYTE>			Bytevec;

	enum struct TypeId : DWORD {
		NONE = REG_NONE,
		SZ = REG_SZ,
		EXPAND_SZ = REG_EXPAND_SZ,
		BINARY = REG_BINARY,
		DWORD = REG_DWORD,
		MULTI_SZ = REG_MULTI_SZ
	};

	/*
	instead of crazy macro programing and a such, we will simply create
	an map of names for the enum above.
	*/
	const wstring & typeid2name(const TypeId & tid_) noexcept
	{
		static std::map<TypeId, StringT> tid_names_map_{
			{ TypeId::NONE , L"REG_NONE" },
			{ TypeId::SZ , L"REG_SZ" },
			{ TypeId::EXPAND_SZ , L"REG_EXPAND_SZ" },
			{ TypeId::BINARY , L"REG_BINARY" },
			{ TypeId::DWORD , L"REG_DWORD" },
			{ TypeId::MULTI_SZ , L"REG_MULTI_SZ" }
		};

		return tid_names_map_[tid_];
	}

	// internal things
	namespace {
		using namespace std;

		/*
		return  name, reg id name, value string
		*/
		wstring nrv_string(wstring n, wstring r, wstring v) {
			static wchar_t buf[BUFSIZ]{};
			wmemset(buf, 0x0, BUFSIZ);
			assert(
				swprintf_s(buf, L"Name:[%s]\t\tREG type id:[%s]\t\tValue:[%s]", n.data(), r.data(), v.data())
			> 0);
			return buf; // copy on return
		}

		wstring to_string(const Reg::Stringvec & vec) {
			wstring rval{};
			for (wstring x : vec)
			{
				rval += x + L":";
			}
			rval.resize(rval.size() - 1); // remove the trailing ':'
			return rval;
		}

		wstring to_hex_string(BYTE b)
		{
			wchar_t buf[10];
			assert(swprintf_s<10>(buf, L"0x%02X", b) > 0);
			return wstring(buf);
		}

		wstring to_hex_string(const Reg::Bytevec & bv)
		{
			wstring rval{};
			for (BYTE x : bv)
			{
				rval += to_hex_string(x) + L":";
			}
			rval.resize(rval.size() - 1); // remove the trailing ':'
			return rval;
		}

		wstring to_hex_string(DWORD dw)
		{
			wchar_t buf[20];
			assert(swprintf_s<20>(buf, L"0x%08X", dw) > 0);
			return wstring(buf);
		}
	} // anonymous namespace

	namespace registry {
		struct TypeSize {
			DWORD valueType = 0;
			DWORD dataSize = 0;
		};

		registry::TypeSize query_value_id_and_data_size(HKEY hKey, const std::wstring& valueName)
		{
			assert(hKey != nullptr);
			registry::TypeSize retval_{};

			// Query the value type and the data size for that value
			LONG result = ::RegQueryValueEx(
				hKey,
				valueName.data(),
				nullptr,        // reserved
				&retval_.valueType,
				nullptr,        // not ready to pass buffer to write data into yet
				&retval_.dataSize       // ask this API the total bytes for the data
			);
			if (result != ERROR_SUCCESS)
			{
				throw L"query_value_id_and_data_size() failed in returning value info.";
			}
			return retval_;
		}
		// MSVC emits a warning in 64-bit builds when assigning size_t to DWORD.
		// So, only in 64-bit builds, check proper size limits before conversion
		// and throw std::overflow_error if the size_t value is too big.
		__forceinline DWORD SafeSizeToDwordCast(size_t size)
		{
#ifdef _WIN64
			static size_t max_size_t = static_cast<size_t>((std::numeric_limits<DWORD>::max)());
			if (size > max_size_t)
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

		//
		// Helpers to read actual data from the registry.
		//
		// NOTE: The "valueSize" parameter contains the size of the value to be read in *BYTES*.
		// This is important for example to helper functions reading strings (REG_SZ, etc.),
		// as usually std::wstring methods consider sizes in wchar_ts.
		//

		// Reads a REG_DWORD value.
		DWORD ReadValueDword(HKEY hKey, const std::wstring& valueName)
		{
			assert(hKey != nullptr);

			DWORD valueData = 0;
			DWORD valueSize = sizeof(valueData);

			LONG result = ::RegQueryValueEx(
				hKey,
				valueName.data(),
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

			return valueData;
		}

		// Reads a REG_SZ value.
		std::wstring ReadValueString(HKEY hKey, const std::wstring& valueName, DWORD valueSize)
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
				valueName.data(),
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
			return str;
		}

		// Reads a REG_EXPAND_SZ value.
		std::wstring ReadValueExpandString(HKEY hKey, const std::wstring& valueName, DWORD valueSize)
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
				valueName.data(),
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
			return str;
		}

		// Reads a REG_BINARY value.
		std::vector<BYTE> ReadValueBinary(HKEY hKey, const std::wstring& valueName, DWORD valueSize)
		{
			assert(hKey != nullptr);

			// Data to be read from the registry
			std::vector<BYTE> binaryData(valueSize);
			DWORD sizeInBytes = valueSize;

			LONG result = ::RegQueryValueEx(
				hKey,
				valueName.data(),
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
		std::vector<std::wstring> ReadValueMultiString(HKEY hKey, const std::wstring& valueName, DWORD valueSize)
		{
			assert(hKey != nullptr);

			// Multi-string parsed into a vector of strings
			std::vector<std::wstring> multiStrings;

			// Buffer containing the multi-string
			std::vector<wchar_t> buffer(valueSize);

			DWORD sizeInBytes = valueSize;
			LONG result = ::RegQueryValueEx(
				hKey,
				valueName.data(),
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
			return multiStrings;
		}
		//
		// Helpers for SetValue()
		//

		void WriteValueBinary(HKEY hKey, const std::wstring& valueName, const Reg::Bytevec & data)
		{
			assert(hKey != nullptr);

			const DWORD dataSize = SafeSizeToDwordCast(data.size());
			LONG result = ::RegSetValueEx(
				hKey,
				valueName.data(),
				0, // reserved
				REG_BINARY,
				&data[0],
				dataSize);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegSetValueEx() failed in writing REG_BINARY value.", result);
			}
		}

		void WriteValueDword(HKEY hKey, const std::wstring& valueName, const DWORD data)
		{
			assert(hKey != nullptr);
			const DWORD dataSize = sizeof(data);
			LONG result = ::RegSetValueEx(
				hKey,
				valueName.data(),
				0, // reserved
				REG_DWORD,
				reinterpret_cast<const BYTE*>(&data),
				dataSize);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegSetValueEx() failed in writing REG_DWORD value.", result);
			}
		}

		void WriteValueString(HKEY hKey, const std::wstring& valueName, const std::wstring & value)
		{
			assert(hKey != nullptr);

			const std::wstring& str = value;

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

		void WriteValueExpandString(HKEY hKey, const std::wstring& valueName, const std::wstring & str)
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

		void WriteValueMultiString(
			HKEY hKey,
			const std::wstring& valueName,
			const std::vector<std::wstring> & multiString
		)
		{
			assert(hKey != nullptr);

#if 0
			// We need to build a whole array containing the multi-strings, with double-NUL termination
			std::vector<wchar_t> buffer;
			// Deep copy the single strings in the buffer
			if (!multiString.empty())
			{
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
#endif
			wstring buffer{};
			static wchar_t wendl[] = { L'\n' };
			for (const std::wstring& s : multiString)
			{
				buffer.append(s);
				buffer.append(wendl);
			}
				buffer.append(wendl); // end of string requires two \n

			// Size is in *BYTES*
			const DWORD dataSize = SafeSizeToDwordCast(buffer.size() * sizeof(wchar_t));

			LONG result = ::RegSetValueEx(
				hKey,
				valueName.data(),
				0, // reserved
				REG_MULTI_SZ,
				reinterpret_cast<const BYTE*>(buffer.data()),
				dataSize);
			if (result != ERROR_SUCCESS)
			{
				throw winreg::RegException(L"RegSetValueEx() failed in writing REG_MULTI_SZ value.", result);
			}
		}

		/*
		Delete the value in the registry.
		Currently no checks if value by the given key exists.
		*/
		static void delete_value(HKEY hKey, const wstring & value_name)
		{
			assert(hKey != nullptr);
			LONG result = ::RegDeleteValue(hKey, value_name.data());
			assert(result == ERROR_SUCCESS);
		}
	} // namespace

		/* Registry Value Interface */
	class IRegVal {
	public:
		/* pointer to base */
		typedef std::unique_ptr<IRegVal>   PointerT;

		virtual const TypeId  & regtypeid() const noexcept = 0;
		virtual const StringT & name()		const noexcept = 0;
		virtual const StringT & reg_type_name()	const noexcept = 0;

		/* use given key to read from registry */
		virtual const IRegVal & Read(HKEY hKey) = 0;
		/* use given key to write to registry */
		virtual const IRegVal & Write(HKEY hKey) const = 0;
		virtual const IRegVal & Swap(const IRegVal & rv_) = 0;
		// remove value from registry
		// do nothing with its object presentation
		virtual const IRegVal & Remove(HKEY hKey) const = 0;
		// prety print the instance
		virtual std::wstring pretty() const noexcept = 0;
	};

	/*
	Registry value is actually an value/name pair also bound in its behaviour
	with the Registry Type ID
	Thus we certainly could use std::pair here but that will push the whole
	mechanism a bit to far out of its original design. Perhaps.
	*/
	template<TypeId TID, typename T> class RegVal
		: public IRegVal
	{
	public:
		typedef RegVal<TID, typename T> ThisType;
		typedef T			ValueType;

		// interface methods
		virtual const TypeId  & regtypeid() const noexcept { return reg_type_id_; }

		RegVal(const Reg::StringT new_name_, const T & new_val_) noexcept
			: name_(new_name_), value_(new_val_)
		{
			/*
			since TID to T coupling is properly defined by template args
			we need no runtime checks. T has to match TID as the template type defines
			example:

			  RegVal<TypeId::DWORD, REG_DWORD> dword_reg_val_ { 0x13 } ;

			  will compile only if initor arg is of a DWORD type
			*/
		}
		/*
		need to be sure if copy constructor generate is sufficient
		*/
		const RegVal & operator=(const RegVal tmp) noexcept {
			return Swap(tmp)
				// not necessary since we can not assing diferent reg types
				// this->TypeID = tmp.TypeID;
				// return *this;
		}

		const T & value() const noexcept {
			return this->value_;
		}

		const StringT & name() const noexcept {
			return this->name_;
		}

		const StringT & reg_type_name() const noexcept {
			return typeid2name(this->reg_type_id_);
		}

		const IRegVal & Swap(const IRegVal & ir) {
			RegVal & rv_ = (RegVal &)ir;
			std::swap((T&)this->value_, (T&)rv_.value());
			std::swap((Reg::StringT&)this->name_, (Reg::StringT&)rv_.name());
			return *this;
		}

		/*
		actually set the new value member
		*/
		const IRegVal & set_val(T new_val_) {
			std::swap(this->value_, new_val_);
			return *this;
		}

		/*
		remove the value this object represents but only from the registry
		*/
		virtual const IRegVal & Remove(HKEY hKey) const
		{
			registry::delete_value(hKey, this->name());
			return *this;
		}

	private:
		T value_{};
		Reg::StringT name_{};
		const	TypeId	reg_type_id_{ TID };
	};

	/*
	REG_NONE specialization represents kind-of-a null RegVal
	We use this as a default RegVal type
	*/
	class Reg_NONE : public RegVal<TypeId::NONE, nullptr_t>
	{
	public:

		Reg_NONE() noexcept
			: RegVal(L"RG_NONE", nullptr)
		{
		}

		Reg_NONE(const Reg::StringT &, const nullptr_t &) noexcept
			: RegVal(L"RG_NONE", nullptr)
		{
		}
		/* use given key to read from registry */
		const RegVal & Read(HKEY) {
			return *this;
		}

		/* use given key to write to registry */
		const RegVal & Write(HKEY) const {
			return *this;
		}

		std::wstring pretty() const noexcept
		{
			return nrv_string(name(), reg_type_name(), L"null_ptr");
		}
	};

	/*
	 And here are all the types representing underlyng REG values
	*/
	class Reg_DWORD : public RegVal<TypeId::DWORD, DWORD>
	{
	public:
		typedef RegVal<TypeId::DWORD, DWORD> BaseType;
		Reg_DWORD(const StringT & n_, const ValueType & v_) : BaseType(n_, v_) { }

		/* use given key to read from registry */
		const Reg_DWORD & Read(HKEY hKey) {
			set_val(registry::ReadValueDword(hKey, this->name()));
			return *this;
		}

		/* use given key to write to registry */
		const Reg_DWORD & Write(HKEY hKey) const {
			registry::WriteValueDword(hKey, this->name(), this->value());
			return *this;
		}
		std::wstring pretty() const noexcept
		{
			return nrv_string(name(), reg_type_name(), to_hex_string(value()));
		}
	};

	class Reg_SZ : public RegVal<TypeId::SZ, Reg::StringT>
	{
	public:
		typedef RegVal<TypeId::SZ, StringT> BaseType;
		Reg_SZ(const StringT & n_, const BaseType::ValueType & v_) : BaseType(n_, v_) { }
		/* use given key to read from registry */
		const Reg_SZ & Read(HKEY hKey) {
			registry::TypeSize ts_ = registry::query_value_id_and_data_size(hKey, this->name());

			set_val(
				registry::ReadValueString(hKey, this->name(), ts_.dataSize)
			);

			return *this;
		}

		/* use given key to write to registry */
		const Reg_SZ & Write(HKEY hKey) const {
			registry::WriteValueString(hKey, this->name(), this->value());
			return *this;
		}

		std::wstring pretty() const noexcept
		{
			return nrv_string(name(), reg_type_name(), value());
		}
	};

	class Reg_EXPAND_SZ : public RegVal<TypeId::EXPAND_SZ, Reg::StringT>
	{
	public:
		typedef RegVal<TypeId::EXPAND_SZ, StringT> BaseType;
		Reg_EXPAND_SZ(const StringT & n_, const ValueType & v_) : BaseType(n_, v_) { }
		/* use given key to read from registry */
		const Reg_EXPAND_SZ & Read(HKEY hKey) {
			registry::TypeSize ts_ = registry::query_value_id_and_data_size(hKey, this->name());
			set_val(
				registry::ReadValueExpandString(hKey, this->name(), ts_.dataSize)
			);
			return *this;
		}

		/* use given key to write to registry */
		const Reg_EXPAND_SZ & Write(HKEY hKey) const {
			registry::WriteValueExpandString(hKey, this->name(), this->value());
			return *this;
		}

		std::wstring pretty() const noexcept
		{
			return nrv_string(name(), reg_type_name(), value());
		}
	};

	class Reg_MULTI_SZ : public RegVal<TypeId::MULTI_SZ, Reg::Stringvec>
	{
	public:
		typedef RegVal< TypeId::MULTI_SZ, ValueType> BaseType;
		Reg_MULTI_SZ(const StringT & n_, const ValueType & v_) : BaseType(n_, v_) { }
		/* use given key to read from registry */
		const Reg_MULTI_SZ & Read(HKEY hKey) {
			registry::TypeSize ts_ = registry::query_value_id_and_data_size(hKey, this->name());
			set_val(
				registry::ReadValueMultiString(hKey, this->name(), ts_.dataSize)
			);
			return *this;
		}

		/* use given key to write to registry */
		const Reg_MULTI_SZ & Write(HKEY hKey) const {
			registry::WriteValueMultiString(hKey, this->name(), this->value());
			return *this;
		}

		std::wstring pretty() const noexcept
		{
			return nrv_string(name(), reg_type_name(), to_string(value()));
		}
	};

	class Reg_BINARY : public RegVal<TypeId::BINARY, Reg::Bytevec>
	{
	public:
		typedef RegVal< TypeId::BINARY, ValueType> BaseType;
		Reg_BINARY(const StringT & n_, const ValueType & v_) : BaseType(n_, v_) { }
		/* use given key to read from registry */
		const Reg_BINARY & Read(HKEY hKey) {
			registry::TypeSize ts_ = registry::query_value_id_and_data_size(hKey, this->name());
			set_val(
				registry::ReadValueBinary(hKey, this->name(), ts_.dataSize)
			);
			return *this;
		}

		/* use given key to write to registry */
		const Reg_BINARY & Write(HKEY hKey) const {
			registry::WriteValueBinary(hKey, this->name(), this->value());
			return *this;
		}

		std::wstring pretty() const noexcept
		{
			return nrv_string(name(), reg_type_name(), to_hex_string(value()));
		}
	};

	/*
	Factory method
	*/
	static Reg::IRegVal::PointerT
		MakeValue(HKEY hKey, const std::wstring& valueName)
	{
		assert(hKey != nullptr);

		registry::TypeSize ts_ = registry::query_value_id_and_data_size(hKey, valueName);

		// According to this MSDN web page:
		//
		// "Registry Element Size Limits"
		//  https://msdn.microsoft.com/en-us/library/windows/desktop/ms724872(v=vs.85).aspx
		//
		// "Long values (more than 2,048 bytes) should be stored in a file,
		// and the location of the file should be stored in the registry.
		// This helps the registry perform efficiently."
		//

		// Dispatch to internal helper function based on the value's type
		switch (ts_.valueType)
		{
		case REG_BINARY:    return
			Reg::IRegVal::PointerT{
			 new Reg_BINARY(valueName,
				registry::ReadValueBinary(hKey, valueName, ts_.dataSize)
			)
		};
							break;
		case REG_DWORD:   return
			Reg::IRegVal::PointerT{
			 new Reg_DWORD(valueName,
				registry::ReadValueDword(hKey, valueName)
				)
		};
						  break;
		case REG_SZ: return
			Reg::IRegVal::PointerT{
				new Reg_SZ(valueName,
				registry::ReadValueString(hKey, valueName, ts_.dataSize)
				)
		};
					 break;
		case REG_EXPAND_SZ: return
			Reg::IRegVal::PointerT{
				new Reg_EXPAND_SZ(valueName,
				registry::ReadValueExpandString(hKey, valueName, ts_.dataSize)
				)
		};
							break;
		case REG_MULTI_SZ:  return
			Reg::IRegVal::PointerT{
				new Reg_MULTI_SZ(valueName,
				registry::ReadValueMultiString(hKey, valueName, ts_.dataSize)
				)
		};
							break;
		default:
			throw std::invalid_argument("Unsupported Windows Registry value type.");
		}
	}

	/*
	following streaming operators works, that is it is found and used
	Excersize for you: how?
	*/
	std::wostream & operator << (std::wostream & wos, const IRegVal & reg_val_) {
		return wos << reg_val_.pretty();
	}

	namespace test {
		void t1(std::wostream & wos)
		{
			Reg_NONE        rg_nn{};
			Reg_DWORD		rg_dw{ L"RG_DWORD",		0x13 };
			Reg_SZ			rg_sz{ L"RG_SZ",		L"Huee!" };
			Reg_EXPAND_SZ	rg_ez{ L"RG_EXPAND_SZ", L"Wuah!" };
			Reg_MULTI_SZ    rg_mz{ L"RG_MULTI_SZ",	Reg_MULTI_SZ::ValueType{L"Array",L"of", L"wstrings"} };
			Reg_BINARY		rg_by{ L"RG_BINARY",	Reg_BINARY::ValueType{ 0x1,0x2, 0x3 } };

			wos << rg_nn << std::endl
				<< rg_dw << std::endl
				<< rg_sz << std::endl
				<< rg_ez << std::endl
				<< rg_mz << std::endl
				<< rg_by << std::endl
				;
		}
	}
} // Reg

void swap(Reg::IRegVal & rv1, Reg::IRegVal & rv2)
{
	rv1.Swap(rv2);
}
