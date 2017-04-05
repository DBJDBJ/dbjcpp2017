#pragma once

#include <string>
#include <vector>
#include <cassert>
#include <functional>
#include <guiddef.h>
#include <Unknwnbase.h>

#ifndef _UNICODE
#error DBJ modernprint requires UNICODE builds
#endif
#define DBJ_PRINT_TEST 1
#ifndef DBJINLINE
#define DBJINLINE static __forceinline
#endif

namespace dbj {

	namespace print {

		static const char PLACEHOLDER = '%';

		DBJINLINE void Append(std::string & target,
			char const * const value, size_t const size)
		{
			target.append(value, size);
		}
		template <typename P>
		DBJINLINE void Append(P target, char const * const value, size_t const size)
		{
			target("%.*s", size, value);
		}

#pragma region dbj overloads
#pragma endregion dbj overloads
		DBJINLINE void Append(FILE * target,
			char const * const value, size_t const size)
		{
			/*
			Kenny has declared size to be size_t type
			fprintf requires int for a size argument bellow
			this cast is ok in this case as it defines string
			precision value only
			*/
#pragma warning ( push )
#pragma warning ( disable: 4267 )
			fprintf(target, "%.*s",  static_cast<int>(size), value);
#pragma warning ( pop )
		}

		/*
		DBJ: in formated output to string use-case we do not need to allow for 
		sizes bigger than maxint for the platform.
		if really huge string target is used we leave it to the stdlib to throw 
		the exception on target.resize(back+size);
		*/
		template <typename ... Args>
		DBJINLINE void AppendFormat(std::string & target,
			char const * const format, Args ... args)
		{
#pragma warning ( push )
#pragma warning ( disable: 4267 )
			int const back = static_cast<int>(target.size());
#pragma warning ( pop )
			int const size = snprintf(nullptr, 0, format, args ...);
			target.resize(back + size);
			snprintf(&target[back], size + 1, format, args ...);
		}

		template <typename P, typename ... Args>
		DBJINLINE void AppendFormat(P target, char const * const format, Args ... args)
		{
			target(format, args ...);
		}

		template <typename ... Args>
		DBJINLINE void AppendFormat(FILE * target,
			char const * const format, Args ... args)
		{
			fprintf(target, format, args ...);
		}

		/*Let’s assume the Write driver function will pass each argument to a WriteArgument function. Here’s one for strings:*/
		template <typename Target>
		DBJINLINE void WriteArgument(Target & target, std::string const & value)
		{
			Append(target, value.c_str(), value.size());
		}

		/*Here’s another WriteArgument function for integer arguments:*/
		template <typename Target>
		DBJINLINE void WriteArgument(Target & target, int const value)
		{
			AppendFormat(target, "%d", value);
		}
		/*complex types can be used as arguments and temporary storage can even be relied upon to format their text representation. Here’s a WriteArgument overload for GUIDs:*/
		template <typename Target>
		DBJINLINE void WriteArgument(Target & target, GUID const & value)
		{
			wchar_t buffer[39];
			StringFromGUID2(value, buffer, _countof(buffer));
			AppendFormat(target, "%.*ls", 36, buffer + 1);
		}

		// Visualizing a Vector

			template <typename Target, typename Value>
			DBJINLINE void WriteArgument(Target & target, std::vector<Value> const & values)
		{
			for (size_t i = 0; i != values.size(); ++i)
			{
				if (i != 0)
				{
					WriteArgument(target, ", ");
				}
				WriteArgument(target, values[i]);
			}
		}

		// wide and normal characters
		template <typename Target, unsigned Count>
		DBJINLINE void WriteArgument(Target & target, char const (&value)[Count])
		{
			Append(target, value, Count - 1);
		}
		template <typename Target, unsigned Count>
		DBJINLINE void WriteArgument(Target & target, wchar_t const (&value)[Count])
		{
			/*
			DBJ
			https://msdn.microsoft.com/en-us/library/tcxf1dw6.aspx
			replaced ls with S
			*/
			AppendFormat(target, "%.*S", Count - 1, value);
		}
		
		/*
		using the above one can easily and safely write output using different character sets:

		Write(printf, "% %", "Hello", L"World");
		
		*/

		/*What if you don’t specifically or initially need to write output, but instead just 
		need to calculate how much space would be required ? 
		No problem, I can simply create a new target that sums it up :*/

		DBJINLINE void Append(size_t & target, char const *, size_t const size)
		{
			target += size;
		}
		template <typename ... Args>
		DBJINLINE void AppendFormat(size_t & target,
			char const * const format, Args ... args)
		{
			target += snprintf(nullptr, 0, format, args ...);
		}
		/*
		I can now calculate the required size quite simply :

		size_t count = 0;
		Write(count, "Hello %", 2015);
		assert(count == strlen("Hello 2015"));
		*/

		/*
		Anything that implements IFormatable can be used as the source of the WriteArgument
		This is used to avoid direct coupling with this Write() mechanism

		DBJ: removed usage of the __inteface keyword
		The __interface modifier is a Visual C++ extension to help implementing COM interfaces.
		And because COM is a C compatible definition, one cannot have 
		operators, Ctor or Dtors.
		*/
		struct IFormatable
		{
			typedef wchar_t *  StringType;
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
			DBJINLINE IFormatable::StringType 
			  cast ( const std::wstring & t) {
				return const_cast<IFormatable::StringType>(t.data());
			}
			DBJINLINE IFormatable::StringType
				cast(const IFormatable::StringType t ) {
				return const_cast<IFormatable::StringType>(t);
			}
		};

		/* usage of IFormatable */
		template <typename Target>
		DBJINLINE void WriteArgument(Target & target, IFormatable const & source_)
		{
#ifdef _DEBUG
			auto content_ = source_.content();
			AppendFormat(target, "%S", content_);
#else
			AppendFormat(target, "%S", source_.content());
#endif
		}

		// So what does the Write driver function look like ? 
		// For starters, there’s only one Write function :

		template <typename Target, unsigned Count, typename ... Args>
		DBJINLINE void Write(Target & target,
			char const (&format)[Count], Args const & ... args)
		{
			DBJ_VERIFY(
					Internal::CountPlaceholders(format) == sizeof ... (args)
			) ;
				Internal::Write(target, format, Count - 1, args ...);
		}

		namespace Internal {
			DBJINLINE constexpr unsigned CountPlaceholders(char const * const format)
			{
				return (*format == dbj::print::PLACEHOLDER ) +
					(*format == '\0' ? 0 : CountPlaceholders(format + 1));
			}

			template <typename Target, typename First, typename ... Rest>
			DBJINLINE void Write(Target & target, char const * const value,
				size_t const size, First const & first, Rest const & ... rest)
			{
				// Magic goes here
				size_t placeholder = 0;
				while (value[placeholder] != dbj::print::PLACEHOLDER )
				{
					++placeholder;
				}
				assert(value[placeholder] == dbj::print::PLACEHOLDER );
				Append(target, value, placeholder);

				WriteArgument(target, first);
				Write(target, value + placeholder + 1, size - placeholder - 1, rest ...);
			}

			// Ultimately, the compiler will run out of arguments and a non - variadic overload will be required to complete the operation :

			template <typename Target>
			DBJINLINE void Write(Target & target, char const * const value, size_t const size)
			{
				Append(target, value, size);
			}

		}// Internal

		/*
		dbj added 
		*/
		template <unsigned Count, typename ... Args>
		DBJINLINE void Print(char const (&format)[Count],
			Args const & ... args) noexcept
		{
			dbj::print::Write(printf, format, args ...);
		}

	} // print
	
} // dbj
/*
TESTING
*/
namespace dbj {
	namespace print {
		namespace test {
			namespace {
				using std::string;
				typedef std::function<string()> TestUnit;
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
						std::vector<int> const numbers{ 1, 2, 3, 4, 5, 6 };
						std::string text;
						Print("\n{ % }", numbers);
						Write(text, "{ % }", numbers);
						assert(text == "{ 1, 2, 3, 4, 5, 6 }");
						return "OK: Vector visualisation";
					},
				[] {
					std::vector<std::string> const names{ "Jim", "Jane", "June" };
					std::string text;
					Write(text, "{ % }", names);
					assert(text == "{ Jim, Jane, June }");
					return "OK: std::string printing";
					},
				[] {
					//we can now calculate the required size quite simply :
					size_t count = 0;
					Write(count, "Hello %", 2015);
					assert(count == strlen("Hello 2015"));
					return "OK: required size counting";
					},
				[] {
					std::string text;
					Write(text, "{%}", __uuidof(IUnknown));
					assert(text == "{00000000-0000-0000-C000-000000000046}");
					Print("\nIUnknown GUID:[%]",text);
					return "OK: GUID printing";
					},
				[]{
					class Test : public IFormatable
					{
						// remember: unicode only
						std::wstring name_;
					public:
						Test() : name_(L"class Test : public IFormatable {};") {
						}

						Test::~Test() {	name_.clear(); }

						IFormatable::StringType content() const noexcept {
							return IFormatable::cast(name_);
						}
					};
					Print("\nTest object content is: [%]", Test());
					return "OK: IFormatable Object printing";
					}
				};

				/*
				see this: http://www.cprogramming.com/c++11/c++11-lambda-closures.html
				*/
			} // anon ns
        DBJINLINE void do_the_tests()
				{
					for ( auto  tunit : test_units) {
						Print("\n%", tunit().data());
					}
				}
		} // test
	} // print
} // dbj

