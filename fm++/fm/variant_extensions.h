#pragma once

namespace dbj {
	namespace fm {
		namespace var {
			// try casting using the _variant_t
			// try casting ARG to T using the _bstr_t
			// T has to be copyable
			template<typename T, typename ARG>
			DBJINLINE
				T variant_cast(const ARG & arg, bool kick_the_bucket = true) {

				static_assert(std::is_convertible<_variant_t, T>::value, __FUNCTION__ " can not convert from variant to T");

				static_assert(std::is_copy_constructible<T>::value, __FUNCTION__ " error: type T is not copy constructible");

				try {
					_variant_t vart(arg);
					assert(VT_EMPTY != ((VARIANT)vart).vt);
					return (T)vart;
				}
				catch (const ::_com_error & cerr_) {
					if (kick_the_bucket ) {
						assert(0 && cerr_.ErrorMessage()); /* conversion failed */
					}
				}
			}
		}
	}
}
