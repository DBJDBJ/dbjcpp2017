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
#include <functional>
#include <type_traits>

#ifndef DBJINLINE
#define DBJINLINE static __forceinline
#endif

namespace dbj {
	template <typename T, std::size_t >
	void deletor(T * pointer_) {

		if (!std::is_reference<T>::value);
		throw std::invalid_argument("No pointers to references allowed in " __FUNCTION__);
		if (std::is_pointer<T>::value)
			throw std::invalid_argument("No pointers to pointers allowed in " __FUNCTION__);

		if (std::is_class<T>::value) {
			if (std::is_destructible<T>::value) {
				delete pointer_; pointer = nullptr;
			}
		}
		else {
			throw std::invalid_argument("Can not delete non-destructible pointers in " __FUNCTION__);
		}
	}

	template<typename T, std::size_t N>
	void deletor(T(&array)[N]) {

		if (N < 1)
			throw std::invalid_argument("Array with less then 1 element supplied to " __FUNCTION__);

		if (std::is_class<T>::value) {
			if (std::is_destructible<T>::value)
				for (T & element : array) {
					element.~T();
				}
		}
		else {
			for (T & element : array) { element = T{}; }

		}
	}

	/*
	all templated deletor types
	*/
	// template<typename T> struct ObjectDeletor;
	/*
	create and hold , deleting function for T []
	*/
	template<
		typename T, typename = typename std::enable_if<std::is_array<T>::value, T>::type
	>
		struct ObjectDeletor {

		using value_type = T;

		explicit ObjectDeletor()
		{
			// also we do only classes here
			// static_assert(std::is_class<T>::value);
			// does not have to be --> static_assert(std::is_nothrow_destructible<T>::value);
			// does not have to be --> static_assert(std::is_trivially_destructible<T>::value);
			// static_assert(std::is_destructible<T>::value);
		};
		/*
		// or just use it
		void operator ()( const T tp_ ) const
		{
		delete tp_;
		}
		*/
		template <class T, std::size_t N>
		constexpr std::size_t size(const T(&array)[N]) noexcept
		{
			return N;
		}

		template <class T, std::size_t N>
		void operator ()(T(&array)[N]) const
		{
#ifdef _DEBUG
			size_t see_me = N;
			T & first = array[see_me - 1];
#endif
			if (N) {
				for (T & element : array) {
					element.~T();
				}
			}
		}
	};

	DBJINLINE void test_deletors() {

		const wchar_t warr[] = { L"wide char array on the stack" };
		int ia[] = { 0,1,1,2,3 };


		// does not compile 
		/*
		ObjectDeletor<wchar_t> warr_deletor;
		warr_deletor(warr);

		ObjectDeletor<int> ia_deletor;
		ia_deletor(ia);

		deletor(ia);
		*/

		typedef std::function<void()> VFT;
		VFT vf = []() { bool x; return x = true; };
		VFT vfa[] = { vf,vf,vf };
		ObjectDeletor<VFT[]> vft_deletor;
		vft_deletor(vfa);

		// does compile
		ObjectDeletor<int[]> int_deletor;
		// does compile
		deletor(ia);

	}
} // dbj
#define DBJVERSION __FILE__ __DATE__ __TIME__
#pragma message( "Compiling Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION )
#undef  DBJVERSION
