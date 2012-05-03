#ifndef INCLUDE_IPC_SERIALIZER_H
#define INCLUDE_IPC_SERIALIZER_H

#include <type_traits>
#include <cstring>
#include <memory>

#include <ipc/Archive.hpp>
#include <ipc/is_serializable.hpp>

namespace ipc {

	/**
	 * Provides methods to write instances of serializable types to an Archive.
	 *
	 * No information not explicitly provided to the serializer will be written
	 * to the archive. As a result, not type or length checking will be
	 * possible during deserialization.
	 */
	class Serializer {
		template<class T>
		friend Archive serialize(const T&);

		private:
			std::unique_ptr<uint8_t[]> buffer_;
			size_t offset_;
			std::vector<int> fds_;

			void push_raw(const void* data, size_t length)
			{
				if (buffer_) {
					memcpy(buffer_.get() + offset_, data, length);
				}
				offset_ += length;
			}

			template<class T>
			auto push(const T& item)
				-> typename std::enable_if<std::is_pod<T>::value>::type
			{
				push_raw(&item, sizeof(T));
			}

			template<class T>
			auto push(const T* array, size_t count)
				-> typename std::enable_if<std::is_pod<T>::value>::type
			{
				push_raw(array, count * sizeof(T));
			}

			template<class T>
			auto push(const T& item)
				-> typename std::enable_if<!std::is_pod<T>::value>::type
			{
				serialize(item, *this);
			}

			template<class T>
			auto push(const T* array, size_t count)
				-> typename std::enable_if<!std::is_pod<T>::value>::type
			{
				size_t idx = 0;
				while (count > 0) {
					this->write(array[idx]);

					idx++;
					count--;
				}
			}

			template<class T>
			Archive run(const T& item)
			{
				buffer_ = nullptr;
				offset_ = 0;

				write(item);

				buffer_ = std::unique_ptr<uint8_t[]>(new uint8_t[offset_]);
				offset_ = 0;

				write(item);

				return Archive(std::move(buffer_), offset_, std::move(fds_));
			}

			Serializer()
			{
			}

		public:
			/**
			 * Writes a single instance of a serializable type to the archive.
			 *
			 * \tparam T Type to read, must be serializable.
			 * \sa is_serializable
			 */
			template<class T>
			auto write(const T& item)
				-> typename std::enable_if<is_serializable<T>::value>::type
			{
				push<T>(item);
			}

			/**
			 * Write an array containing \a count elements of serializable type to the archive.
			 * No length checks are performed.
			 *
			 * \tparam T Type to read, must be serializable.
			 * \sa is_serializable
			 */
			template<class T>
			auto write(const T* array, size_t count)
				-> typename std::enable_if<is_serializable<T>::value>::type
			{
				push<T>(array, count);
			}

			//! Writes one file descriptor to the adjoint file descriptor list.
			void writeFd(int fd)
			{
				fds_.push_back(fd);
			}
	};

	/**
	 * Serializes an object tree into it's representation as an <tt>Archive</tt>.
	 *
	 * \copydetails Serializer
	 * \tparam T Type to write, must be serializable.
	 * \sa is_serializable
	 */
	template<class T>
	Archive serialize(const T& item)
	{
		static_assert(is_serializable<T>::value, "Attempted to serialize non-serializable type");
		return Serializer().run(item);
	}

}

#endif
