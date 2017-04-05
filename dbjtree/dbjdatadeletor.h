#pragma once
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
namespace dbj {
	namespace policy {
		/*
		(c) 2017 by dbj@dbj.org
		*/
		// 		__declspec(novtable)
		template <typename T>
		struct DataDeletor {
			/* return false if deletion has not been succesfull */
			virtual bool operator () (T & data) noexcept = 0;
		};
		//		__declspec(novtable)
		template <typename T>
		struct DefaultDataDeletor : DataDeletor<T> {

			bool operator () (T & data) noexcept {
				try {
					if (false == std::is_empty<T>::value) {
						/*
						another deletor might check if value equals default value
						data == T{}
						this has meaning for integral types
						*/
						data.~T();
					}
				}
				catch (...) {
					/* TODO: log the exception here first, return second */
					return false;
				}
				return true;
			}
		};
		namespace test {
			
			template<typename D, typename DDPolicy = DefaultDataDeletor<T>
			>
			struct Node {
				D data;
				Node(const D & data_) : data(data_) {}
			   ~Node() {
				   DDPolicy(data);
				   /*
				   note the template typename argument usage pattern above
				   this allows for not storing instance of DDPolicy but still
				   be able to use/call it
				   */
			   }
			};

				void f1 () {

					Node<std::wstring> sn(L"HELLO");
					sn.~Node<std::wstring>(); 

				}
		   } // test
	} // policy
} // dbj