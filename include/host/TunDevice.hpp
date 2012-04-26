#ifndef INCLUDE_HOST_TUNDEVICE_H
#define INCLUDE_HOST_TUNDEVICE_H

#include <string>

namespace host {

	/**
	 * Represents a single tunnel device. When an instance of this class is destroyed,
	 * it's handle to the tunnel device will be destroyed. When all handles have been
	 * destroyed, the device is automatically deleted.
	 */
	class TunDevice {
		friend class TunRegistry;

		private:
			int fd_, ifIndex_;
			std::string name_;

			//! Closes the file descriptor held by this instance
			void close();

			//! Primitive constructor
			TunDevice(int fd, int ifIndex, const std::string& name)
				: fd_(fd), ifIndex_(ifIndex), name_(name)
			{
			}

		public:
			/**
			 * Constructs a new TunDevice from \a other using move semantics.
			 *
			 * \post
			 * \code
			 * other.fd() == -1
			 * other.ifIndex() == -1
			 * other.name() == ""
			 * \endcode
			 */
			TunDevice(TunDevice&& other);

			/**
			 * Releases the contents of the current instance and replaces them
			 * by the contents of \a rhs.
			 *
			 * \post
			 * \code
			 * rhs.fd() == -1
			 * rhs.ifIndex() == -1
			 * rhs.name() == ""
			 * \endcode
			 */
			TunDevice& operator=(TunDevice&& rhs);

			//! Releases the tunnel descriptor handle held by this instance
			~TunDevice();

			/**
			 * Creates a duplicate of the current instance with a unique file descriptor.
			 *
			 * \throws InvalidOperation The file descriptor could not be duplicated
			 */
			TunDevice dup() const;

			//! Returns the file descriptor number of the tunnel device
			int fd() const
			{
				return fd_;
			}

			//! Returns the interface index of the tunnel device
			int ifIndex() const
			{
				return ifIndex_;
			}

			//! Returns the interface name of the tunnel device
			const std::string& name() const
			{
				return name_;
			}
	};

}

#endif
