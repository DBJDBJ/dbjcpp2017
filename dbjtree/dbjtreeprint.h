﻿#pragma once
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

#include <string>
// #include <iostream>
// #include <iomanip>
// #include <sstream>
// #include <stdio.h>
// #include <stdlib.h>
// console unicode output helper
#include "../dbjfm/dbjfm.h"
#include "../dbjmodernprint/modernprint.h"

namespace dbj {
	namespace treeprint {
		namespace {
			const static std::wstring doubles = L"║═╚";
			const static std::wstring singles = L"│─└";
			const static wchar_t SS = L' '; // single space
			static enum :int { V = 0, H = 1, K = 2 } idx;

			/*
			Print the binary tree made of NODE's
			NODE must have three methods:
			NODE * left  ();
			NODE * right ();
			NODE::DataType * data  ();
			Where NODE::DataType requirement is to be unicode output streamable

			The actual alorithm inspired by: http://www.randygaul.net/2015/06/15/printing-pretty-ascii-trees/
			*/
			template<typename NODE>
			class BinaryTreePrinter {

				const static size_t BUFSIZ_ = 2056;
				wchar_t depth[BinaryTreePrinter::BUFSIZ_] = {};
				int di = int(0);

				void Push(wchar_t c)
				{
					// depth[di++] = L' ';
					depth[di++] = c;
					depth[di++] = L' ';
					depth[di++] = L' ';
					depth[di] = 0;
				}

				void Pop()
				{
					depth[di -= 3] = 0;
				}
					std::wstring os_;
			public:
				BinaryTreePrinter(){}

				~BinaryTreePrinter(){os_.clear();}

				/*
				print into the string, return its result in an wstring
				*/
				std::wstring operator ()(NODE * tree, const int outwid_ = 0)
				{
					dbj::win::console::WideOut helper_;

					print_(tree, outwid_);
					this->os_.append(L"\n");
					std::wstring retval(this->os_.data());
					// actually output the extended charset bits, on the console
					const wchar_t * widestr__ = retval.data();
							helper_(widestr__);

					return retval; // copy 
				}
			private:
				template<typename NODE>
				std::wstring  & print_(NODE* tree, const int outwid_)
				{
					using dbj::print::Write;

					// this->os_ << L"|" << std::setw(outwid_) << *(tree->data()) << L"|\n";

					Write(os_, "|%*S|\n", outwid_, *(tree->data()));

					if (tree->left()) {
						// this->os_ << depth << singles[V] << L"\n";
						// this->os_ << depth << singles[K] << singles[H] << singles[H];
						Write(os_, "%S%C\n", depth, singles[V]);
						Write(os_, "%S%C%C\n", depth, singles[K], singles[H], singles[H]);
						Push(singles[V]);
						print_(tree->left(), outwid_);
						Pop();
					}
					if (tree->right()) {
						// this->os_ << depth << singles[V] << L"\n";
						// this->os_ << depth << singles[K] << singles[H] << singles[H];
						Write(os_, "%S%C\n", depth, singles[V]);
						Write(os_, "%S%C%C%C\n", depth, singles[K], singles[H], singles[H]);
						Push(SS);
						print_(tree->right(), outwid_);
						Pop();
					}

					return this->os_;
				}

			}; // eof BinaryTreePrinter
		} // namespace

		template <typename T>
		static __forceinline
			void binary (std::wstring & os_, T * tree )
		{
			BinaryTreePrinter<T> tpf;
				os_.append(tpf(tree)) ;
		}
#if 0
		void test2()
		{
		/*

		 http://stackoverflow.com/questions/2492077/output-unicode-strings-in-windows-console-app

		 */
			static std::wstring doubles = L"║═╚";
			static std::wstring singles = L"│─└";
			_setmode(_fileno(stdout), _O_U16TEXT);
			std::wcout << L"Testing unicode -- English -- Ελληνικά -- Español." << std::endl
				<< doubles << std::endl
				<< singles << std::endl;

			/* 
			  apparently must do this
			  also make sure NOT to mix cout and wcout in the same executable
			*/
			std::wcout << std::flush; 
			fflush(stdout); 
			_setmode(_fileno(stdout), _O_TEXT); 
		}
#endif
	} //namespace treeprint 
} //namespace dbj 
#define DBJVERSION __DATE__ __TIME__
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION ) 