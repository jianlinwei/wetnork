#ifndef INCLUDE_IPC_ARCHIVE
#define INCLUDE_IPC_ARCHIVE

#include <Packet.hpp>
#include <vector>
#include <memory>

namespace ipc {

	/**
	 * Represents a serialized object tree.
	 */
	class Archive {
		private:
			std::unique_ptr<uint8_t[]> data_;
			size_t length_;
			std::vector<int> fds_;

		public:
			//! Initializes a new Archive.
			Archive(std::unique_ptr<uint8_t[]>&& data, size_t length, std::vector<int>&& fds)
				: data_(std::move(data)), length_(length), fds_(std::move(fds))
			{
			}

			/**
			 * Returns an array of bytes containing binary data for the regular parts
			 * of the serialized object tree.
			 */
			const uint8_t* data() const
			{
				return data_.get();
			}

			//! Returns the length of the array in \c data().
			size_t length() const
			{
				return length_;
			}

			/**
			 * Returns a vector of file descriptors adjoint to the regular data
			 * of the object tree.
			 */
			const std::vector<int>& fds() const
			{
				return fds_;
			}
	};

}

#endif
