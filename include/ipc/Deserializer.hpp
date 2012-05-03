#ifndef INCLUDE_IPC_DESERIALIZER_H
#define INCLUDE_IPC_DESERIALIZER_H

#include <type_traits>
#include <cstring>
#include <memory>

#include <ipc/Archive.hpp>
#include <ipc/is_serializable.hpp>

namespace ipc {

	/**
	 * Provides methods to read instances of serializable types from an Archive.
	 *
	 * During deserialization, no type or length checks are performed. As a
	 * result, it is perfectly valid to serialize an instance of type A, and
	 * then attempt to deserialize the resulting Archive into an instance of
	 * type B. If types A and B are not compatible in this regard, undefined
	 * behaviour will result.
	 */
	class Deserializer {
		template<class T>
		friend T deserialize(const Archive&);

		private:
			const Archive& archive_;
			size_t byteOffset_;
			size_t fdOffset_;

			void pop_raw(void* data, size_t length)
			{
				memcpy(data, archive_.data() + byteOffset_, length);
				byteOffset_ += length;
			}

			template<class T>
			auto pop()
				-> typename std::enable_if<std::is_pod<T>::value, T>::type
			{
				T result;
				pop_raw(&result, sizeof(T));
				return result;
			}

			template<class T>
			auto pop(T* result, size_t count)
				-> typename std::enable_if<std::is_pod<T>::value>::type
			{
				pop_raw(result, count * sizeof(T));
			}

			template<class T>
			auto pop()
				-> typename std::enable_if<!std::is_pod<T>::value, T>::type
			{
				union {
					int dummy;
					T value;
				} u;
				deserialize(&u.value, *this);
				return u.value;
			}

			template<class T>
			auto pop(T* result, size_t count)
				-> typename std::enable_if<!std::is_pod<T>::value>::type
			{
				size_t idx = 0;
				while (count > 0) {
					this->pop(result[idx]);

					idx++;
					count--;
				}
			}

			template<class T>
			T run()
			{
				return read<T>();
			}

			Deserializer(const Archive& archive)
				: archive_(archive)
			{
			}

		public:
			/**
			 * Reads a single instance of a serializable type from the archive.
			 *
			 * \tparam T Type to read, must be serializable.
			 * \sa is_serializable
			 */
			template<class T>
			auto read()
				-> typename std::enable_if<is_serializable<T>::value, T>::type
			{
				return pop<T>();
			}

			/**
			 * Reads an array containing \a count elements of serializable type from the archive.
			 * No length checks are performed.
			 *
			 * \tparam T Type to read, must be serializable.
			 * \sa is_serializable
			 */
			template<class T>
			auto read(T* array, size_t count)
				-> typename std::enable_if<is_serializable<T>::value>::type
			{
				pop<T>(array, count);
			}

			int readFd()
			{
				return archive_.fds().at(fdOffset_++);
			}
	};

	/**
	 * Deserializes an object tree from it's representation as an <tt>Archive</tt>.
	 *
	 * \copydetails Deserializer
	 * \tparam T Type to deserialize, must be serializable.
	 * \sa is_serializable
	 */
	template<class T>
	T deserialize(const Archive& archive)
	{
		static_assert(is_serializable<T>::value, "Attempted to deserialize non-serializable type");
		return Deserializer(archive).run<T>();
	}

}

#endif
