#ifndef INCLUDE_PACKET_H
#define INCLUDE_PACKET_H

#include <memory>
#include <cstdint>
#include <cstddef>

/**
 * Represent a packet of data. Instances of this class are immutable.
 */
class Packet {
	private:
		static void NullDelete(const uint8_t*);
		static void ArrayDelete(const uint8_t*);

		std::shared_ptr<const uint8_t> data_;
		ptrdiff_t offset_;
		size_t length_;

		Packet(const std::shared_ptr<const uint8_t>& data, ptrdiff_t offset, size_t length)
			: data_(data), offset_(offset), length_(length)
		{
		}

	public:
		//! Tag type for noncapturing packet construction
		struct nocapture_t {};
		//! Tag value for noncapturing packet construction
		static constexpr nocapture_t NoCapture = {};

		/**
		 * Constructs a new packet. The new instances takes ownership of \a data. When all \c Packet
		 * instances referencing \a data are destroyed, \a data is <tt>delete[]</tt>ed.
		 *
		 * \post
		 * \code
		 * data() == data + offset
		 * length() == length
		 * \endcode
		 */
		Packet(const uint8_t* data, ptrdiff_t offset, size_t length)
			: data_(data, ArrayDelete), offset_(offset), length_(length)
		{
		}

		/**
		 * Constructs a new packet. The new instance does not take ownership of \a data. Thus,
		 * when \a data becomes invalid, so does any packet that references \a data.
		 *
		 * \post
		 * \code
		 * data() == data + offset
		 * length() == length
		 * \endcode
		 */
		Packet(const uint8_t* data, ptrdiff_t offset, size_t length, nocapture_t)
			: data_(data, NullDelete), offset_(offset), length_(length)
		{
		}

		//! The data block contained within the current instance.
		const uint8_t* data() const
		{
			return data_.get() + offset_;
		}

		//! Length of the contained data block.
		size_t length() const
		{
			return length_;
		}

		/**
		 * Creates a new packet instance starting at most \a bytes after \c data().
		 * Calling this method is equivalent to creating a packet with the same \c data(),
		 * but larger \c offset and smaller \c length. If \c bytes is larger than \c length(),
		 * the resulting \c Packet will reference \c data(), but be 0 bytes long.
		 */
		Packet skip(size_t bytes) const;
};

#endif
