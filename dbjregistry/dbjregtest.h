#pragma once
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
#include <iostream>
#include <string>
#include <vector>

#include "wreg.h"
#include "dbjreg.h"

namespace dbj {
	namespace test {

		using std::wcout;
		using std::wstring;
		using std::vector;
		/*
		Key and Value asbtractions are decoupled
		Does this lead to 'unnatural' code? Prehaps. See bellow.
		*/
		winreg::RegKey return_key_with_some_values(HKEY parent_key, const std::wstring & testKeyName)
		{
			wcout << L"Creating test key [" + testKeyName + L"]\nand writing some values into it...\n";

			auto key = winreg::RegKey::CreateKey(parent_key, testKeyName);

			Reg::Reg_DWORD{ L"Test DWORD", 0x64 }.Write(key.Handle());
			Reg::Reg_SZ{ L"Test REG_SZ", L"Hello World" }.Write(key.Handle());
			Reg::Reg_EXPAND_SZ{ L"Test REG_EXPAND_SZ", L"%WinDir%" }.Write(key.Handle());
			Reg::Reg_MULTI_SZ{ L"Test REG_MULTI_SZ" , vector<wstring> { L"Ciao",L"Hi",L"Connie" } }.Write(key.Handle());
			Reg::Reg_BINARY{ L"Test REG_BINARY", Reg::Bytevec{ 0x22,0x33,0x44 } }.Write(key.Handle());

			return key;
		}

		//
		// Test Delete
		//
		void test_delete(
			const HKEY & parent_key,
			const std::wstring & testKeyName,
			const wstring & valueName = L"TestValue")
		{
			wcout << L"Using test key [" + testKeyName + L"]\nand writing/deleting values into it...\n"
				<< L"Then erasing the whole key\n";

			winreg::RegKey key = winreg::RegKey::OpenKey(parent_key, testKeyName, KEY_WRITE | KEY_READ);

			// Write value
			Reg::Reg_SZ   tv_{ valueName, L"secret text" };
			tv_.Write(key.Handle());
			// Delete a reg value, keep the object representing it in memory
			tv_.Remove(key.Handle());

			// Try accessing a non-existent value
			try
			{
				wcout << "Trying accessing value just deleted from registry ...\n";
				tv_.Read(key.Handle());
			}
			catch (const winreg::RegException& ex)
			{
				wcout << L"winreg::RegException correctly caught.\n";
				wcout << L"Error code: " << ex.code() << L'\n';
				// Error code should be 2: ERROR_FILE_NOT_FOUND
				if (ex.code() == ERROR_FILE_NOT_FOUND)
				{
					wcout << L"All right, I expected ERROR_FILE_NOT_FOUND (== 2).\n\n";
				}
			}

			// Delete the whole key --> from the REGISTRY that is!
			key.Delete();
		}

		//
		// Enum Values
		//
		void enum_values(HKEY key)
		{
			auto valueNames = winreg::EnumerateValueNames(key);

			for (const auto& valueName : valueNames)
			{
				/* Reg::IRegVal::PointerT */
				auto value =
					Reg::MakeValue(key, valueName);
				/*
				wcout << valueName
				<< L" of type: " << value->reg_type_name()
				<< L"\n";
				*/
				wcout << (*value) << L"\n";
			}
		}

		//
		// Enum Sub Keys
		//
		void enum_sub_keys(HKEY key) {
			wcout << "\nEnumerating sub keys:\n";

			auto subNames = winreg::EnumerateSubKeyNames(key);

			for (const auto& subName : subNames)
			{
				auto subKey = winreg::RegKey::OpenKey(key, subName);
				wcout << "For the key:" << subName << " Values are \n";
				enum_values(subKey.Handle());
				wcout << "-----------------------------------------------------------------\n";
			}
		}

		void dbj_registry_tests() {
			Reg::test::t1(wcout);

			wstring testKeyName = L"TEST_KEY_DBJ";

			auto test_key = return_key_with_some_values(HKEY_CURRENT_USER, testKeyName);

			enum_values(test_key.Handle());

			test_delete(HKEY_CURRENT_USER, test_key.key_name());

			// KEY_READ is default access req. 
			// Why is that hidden?
			testKeyName =
				L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FolderDescriptions\\";
			auto key = winreg::RegKey::OpenKey(HKEY_LOCAL_MACHINE, testKeyName);
			enum_sub_keys(key.Handle());

		}
	} // eof test ns
} // eof dbj
#define DBJVERSION __DATE__ __TIME__
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION )