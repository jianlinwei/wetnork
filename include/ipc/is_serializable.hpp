#ifndef INCLUDE_IPC_IS__SERIALIZABLE_H
#define INCLUDE_IPC_IS__SERIALIZABLE_H

#include <type_traits>

namespace ipc {

	class Serializer;
	class Deserializer;

	namespace detail {
		template<class T>
		constexpr bool IsComplexSerializable(typename std::conditional<false,
				decltype((
					serialize(std::declval<const T&>(), std::declval<Serializer&>()),
					deserialize(std::declval<T*>(), std::declval<Deserializer&>()))),
				int>::type)
		{
			return true;
		}

		template<class T>
		constexpr bool IsComplexSerializable(double)
		{
			return false;
		}
	}

	/**
	 * Type trait for the Serializable concept.
	 *
	 * A serializable type is either an arithmetic type, an enumeration type,
	 * or a type for which <tt>void serialize(const T&, Serializer&)</tt> and
	 * <tt>void deserialize(T*, Deserializer&)</tt> can be found through ADL.
	 * Additionally, all constant-size arrays of serializable types are also
	 * serializable.
	 *
	 * In principle, all POD types are serializable. All compound PODs are by
	 * default not serializable, since pointers in PODs require speciel treatment.
	 * PODs that are known to be trivially serializable (that is, by bitcopy),
	 * can be marked with a specialization of this template to communicate that
	 * to the serializer.
	 */
	template<class T>
	struct is_serializable : std::integral_constant<bool,
		std::is_arithmetic<T>::value
			|| std::is_enum<T>::value
			|| detail::IsComplexSerializable<T>(0)> {
	};

	//! \copydoc is_serializable
	template<class T, size_t N>
	struct is_serializable<T[N]> : is_serializable<T> {
	};

}

#endif
