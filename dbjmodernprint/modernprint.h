#pragma once
/* 
Copyright 2017 dbj@dbj.org

Inspired with: https://msdn.microsoft.com/magazine/dn973010

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
#include <vector>
#include <cassert>
#include <functional>
#include <guiddef.h>
#include <Unknwnbase.h>

#ifndef UNICODE
#error DBJ modernprint requires UNICODE builds
#error Also make sure not to use char, printf() or any other CRT ANSI function anywhere ...
#error And please try to achieve that without including tchar.h
#endif

#define DBJ_PRINT_TEST 1
#ifndef DBJINLINE
#define DBJINLINE static __forceinline
#endif


namespace dbj {

	using std::wstring ;

	namespace print {

		static const wchar_t PLACEHOLDER = L'%';
		/*
		Unicode applications should always cast zero to TCHAR when using null-terminated strings. 
		The code 0x0000 is the Unicode string terminator for a null-terminated string. 
		A single null byte is not sufficient for this code, because many Unicode characters contain null bytes 
		as either the high or the low byte. An example is the letter A, for which the character code is 0x0041.
		*/
		static const unsigned WTERM = 0x0000;

		/*
		DBJ: this is a performance hit, but delivers transformation to wide string
		*/
		DBJINLINE void Append(wstring & target,
			wchar_t const * const value, size_t const size)
		{
			target.append(value, size);
		}

		/* just print to wprintf() basically */
		template <typename P>
		DBJINLINE void Append(P target, wchar_t const * const value, size_t const size)
		{
			target(L"%.*s", size, value);
		}

		DBJINLINE void Append(FILE * target,
			wchar_t const * const value, size_t const size)
		{
			/*
			Kenny has declared size to be size_t type
			f(w)printf requires int for a size argument bellow
			this cast is ok in this case as it defines string
			precision value only
			*/
#pragma warning ( push )
#pragma warning ( disable: 4267 )
			fwprintf(target, L"%.*s",  static_cast<int>(size), value);
#pragma warning ( pop )
		}

		/*
		DBJ: in formated output to string use-case we do not need to allow for 
		sizes bigger than maxint for the platform.
		if really huge string target is used we leave it to the stdlib to throw 
		the exception on target.resize(back+size);
		*/
		template <typename ... Args>
		DBJINLINE void AppendFormat(wstring & target,
			wchar_t const * const format, Args ... args)
		{
#pragma warning ( push )
#pragma warning ( disable: 4267 )
			int const back = static_cast<int>(target.size());
#pragma warning ( pop )
			// buffer, buffercount, maxcount, format, ...
			/*
			http://en.cppreference.com/w/cpp/io/c/fwprintf
			no can do for wide strings
			int const size = _snwprintf_s(nullptr, 0, target.max_size(), format, args ...);
			*/
			// BUFSIZ == 512 in stdio.h
			wchar_t buf[BUFSIZ] = {};
			assert(std::swprintf(buf, BUFSIZ, format, args ...) > -1);
			target.append(buf);
		}

		template <typename P, typename ... Args>
		DBJINLINE void AppendFormat(P target, wchar_t const * const format, Args ... args)
		{
			target(format, args ...);
		}

		template <typename ... Args>
		DBJINLINE void AppendFormat(FILE * target,
			wchar_t const * const format, Args ... args)
		{
			fwprintf(target, format, args ...);
		}

		/* Internal::Write will pass each argument to a WriteArgument function. Here’s one for wstrings:*/
		template <typename Target>
		DBJINLINE void WriteArgument(Target & target, wstring const & value)
		{
			Append(target, value.c_str(), value.size());
		}


		/*Here’s another WriteArgument function for integer arguments:*/
		template <typename Target>
		DBJINLINE void WriteArgument(Target & target, int const value)
		{
			AppendFormat(target, L"%d", value);
		}
		// DBJ added for wchar_t
		template <typename Target>
		DBJINLINE void WriteArgument(Target & target, const wchar_t & value)
		{
			AppendFormat(target, L"%c", value);
		}
		/*complex types can be used as arguments and temporary storage can even be relied upon to format their text representation. Here’s a WriteArgument overload for GUIDs:*/
		template <typename Target>
		DBJINLINE void WriteArgument(Target & target, GUID const & value)
		{
			wchar_t buffer[39] = {};
			StringFromGUID2(value, buffer, _countof(buffer));
			AppendFormat(target, L"%.*ls", 36, buffer + 1);
		}

		// Visualizing a Vector

			template <typename Target, typename Value>
			DBJINLINE void WriteArgument(Target & target, std::vector<Value> const & values)
		{
			for (size_t i = 0; i != values.size(); ++i)
			{
				if (i != 0)
				{
					WriteArgument(target, L", ");
				}
				WriteArgument(target, values[i]);
			}
		}

		// wide characters

		template <typename Target, unsigned Count>
		DBJINLINE void WriteArgument(Target & target, wchar_t const (&value)[Count])
		{
			/*
			https://msdn.microsoft.com/en-us/library/tcxf1dw6.aspx
			*/
			AppendFormat(target, L"%.*s", Count - 1, value);
		}

		/*
		Anything that implements IPrintable can be used as the source of the WriteArgument
		This is used to avoid direct coupling with this Write() mechanism

		DBJ: removed usage of the __inteface keyword
		The __interface modifier is a Visual C++ extension to help implementing COM interfaces.
		And because COM is a C compatible definition, one cannot have 
		operators, Ctor or Dtors.
		*/
		struct IPrintable
		{
			typedef wstring  StringType;
			/* 
			concrete offspring returns its content through here in a single string 
			other than this we could simply impose a 'contract' on implementors and
			ask for casting function to be implemented, which will make it more 
			error prone since people will simply not know (or care) to implement
			casting operator to be able to Write() out their classes instances

			also we here dictate usage of wide char pointers which means unicode
			DBJ: I am avoiding std::wstring to make this more usable for solving
			ABI issues. If ever ...
			*/
			virtual StringType content() const noexcept = 0 ;

		protected:
			/*
			DBJ added helpers for inheritors 
			these functions are static so they do not appear in the vtable
			*/
			DBJINLINE wchar_t * 
			  cast ( const wstring & t) noexcept {
				return const_cast<wchar_t *>(t.data());
			}
		};

		/* usage of IPrintable */
		template <typename Target>
		DBJINLINE void WriteArgument(Target & target, IPrintable const & source_)
		{
#ifdef _DEBUG
			auto content_ = source_.content();
			AppendFormat(target, L"%s", content_);
#else
			AppendFormat(target, L"%s", source_.content());
#endif
		}

		// So what does the Write driver function look like ? 
		// For starters, there’s only one Write function :

		template <typename Target, unsigned Count, typename ... Args>
		DBJINLINE void Write(Target & target,
			wchar_t const (&format)[Count], Args const & ... args)
		{
#if _DEBUG
			auto place_holder_count = Internal::CountPlaceholders(format);
			auto num_of_args = sizeof ... (args);

			if (place_holder_count != num_of_args)
				throw L"Write() Exception: number of format arguments placeholders does not match a number of args.";
#else
			DBJ_VERIFY(
					Internal::CountPlaceholders(format) == sizeof ... (args)
			) ;
#endif
				Internal::Write(target, format, Count - 1, args ...);
		}

		namespace Internal 
		{
			DBJINLINE constexpr unsigned CountPlaceholders(wchar_t const * const format)
			{
				return (*format == dbj::print::PLACEHOLDER ) +
					(*format == L'\0' ? 0 : CountPlaceholders(format + 1));
			}

			template <typename Target, typename First, typename ... Rest>
			DBJINLINE void Write(
				Target & target, wchar_t const * const format,
				size_t const size, First const & first, Rest const & ... rest)
			{
				// find the position of the firs placeholder in the format string
				size_t placeholder = 0;
				while (format[placeholder] != dbj::print::PLACEHOLDER )
				{
					++placeholder;
				}
				DBJ_VERIFY(format[placeholder] == dbj::print::PLACEHOLDER );
				
				// first append to the target whatever is before the first placeholder
				Append(target, format, placeholder);
				// now use the templated function found for the type First
				WriteArgument(target, first);
				// recursively proceed with the rest or use the non variadic Write overload
				// to stop the recursion
				Write(target, format + placeholder + 1, size - placeholder - 1, rest ...);
			}

			// Ultimately, the compiler will run out of arguments and a non - variadic 
			//  overload will be required to complete the operation :

			template <typename Target>
			DBJINLINE void Write(Target & target, wchar_t const * const value, size_t const size)
			{
				Append(target, value, size);
			}

		}// Internal

		/*
		dbj added 
		*/
		template <unsigned Count, typename ... Args>
		DBJINLINE void F(wchar_t const (&format)[Count],
			Args const & ... args) 
		{
#if _DEBUG
			if ( -1 ==	dbj::find_first_of(format, L"%") )
				throw L"Can not start print::F() arguments, with a string which has no '%' (aka the placeholder) in it.";
#endif
				dbj::print::Write(wprintf, format, args ...);
		}

		/* print::F(1,"Joe", in(*)(), L"Q") */
		template <typename ... Args>
		DBJINLINE void F( Args const & ... args) 
		{
			const unsigned argsize = sizeof ... (args);
			wchar_t  format[argsize+1] = {} ;
				std::wmemset(format, L'%', argsize);
					format[argsize] = L'\0';
//				DBJ_VERIFY(argsize == strlen(format));
			dbj::print::Write(wprintf, format, args ...);
		}
#if 0
		template <unsigned Count, typename ... Args>
		DBJINLINE void F(char const (&format)[Count],
			Args const & ... args)
		{
#pragma message ( __FUNCTION__ )
#pragma message ("dbjmodernprint print::F() can not be used for ANSI char's. Please use L\"string literal\"")
		}
#endif
	} // print
} // dbj
/*
TESTING
*/
namespace dbj {
	namespace print {
		namespace test {
			namespace {
				using std::wstring;
				typedef std::function<wstring()> TestUnit;
				typedef std::vector<TestUnit> AllUnits;
#if 0
				DBJINLINE AllUnits & register_unit(TestUnit & tu_ ) {
					static AllUnits test_units;
					if ( tu_ )
						test_units.push_back(tu_);
					return test_units;
				}
#endif
				static AllUnits test_units = {
					[] {
					std::wstring wtext = {};

					Write(wtext, L"{ % % % % }", L"0", L"1 2", L"3 4 5", L"6 7 8 9");
					DBJ_VERIFY(wtext == L"{ 0 1 2 3 4 5 6 7 8 9 }");


					return L"OK: simple test 1";
				},
					[] {
							wstring wtext = {L"\n\n["};

							Write(wtext, L"{ % % % % }", L"0", L"1 2", L"3 4 5", L"6 7 8 9");

							print::F(wtext, L"]---[", wtext, L"]-- - [", __uuidof(IUnknown), L"]\n\n");

							return L"OK: simple test 2";
				       },
					[] {
						std::vector<int> const numbers{ 1, 2, 3, 4, 5, 6 };
						wstring text;
						print::F(L"\n{ % }",numbers);
						Write(text, L"{ % }", numbers);
						DBJ_VERIFY(text == L"{ 1, 2, 3, 4, 5, 6 }");
						return L"OK: Vector<int> visualisation";
					},
				[] {
					std::vector<wstring> const names{ L"Jim", L"Jane", L"June" };
					wstring text;
					Write(text, L"{ % }", names);
					DBJ_VERIFY(text == L"{ Jim, Jane, June }");
					return L"OK: Vector<std::string> printing to std::wstring";
					},
#if 0
				[] {
					//we can now calculate the required size quite simply :
					size_t count = 0;
					Write(count, L"Hello %", 2015);
					// wcslen()
					DBJ_VERIFY(count == wstring(L"Hello 2015").size());
					return L"OK: required size counting";
					},
#endif
				[] {
					wstring text;
					Write(text, L"{%}", __uuidof(IUnknown));
					DBJ_VERIFY(text == L"{00000000-0000-0000-C000-000000000046}");
					print::F(L"\nIUnknown GUID is:[%]", __uuidof(IUnknown));
					return L"OK: GUID printing";
					},
				[]{
					class Test : public IPrintable
					{
						// remember: unicode only
						typename IPrintable::StringType name_;
					public:
						Test() : name_(L"class Test : public IPrintable {}; file:" __FILE__ ) {
						}

						~Test() {	name_.clear(); }

						IPrintable::StringType content() const noexcept {
							return name_ ;
						}
					};
					print::F(L"\n\nTest IPrintable object content is: [%]\n", Test());
					return L"\nOK: IPrintable Object printing";
					}
				};

				/*
				see this: http://www.cprogramming.com/c++11/c++11-lambda-closures.html
				ps: but why?
				*/
			} // anon ns
        DBJINLINE void do_the_tests()
				{
					for ( auto  tunit : test_units) {
						print::F(L"\n%", tunit().data());
					}
				}
		} // test
	} // print
} // dbj
#define DBJVERSION __DATE__ __TIME__
#pragma message("")
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma message("")
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION ) 
#undef DBJVERSION


