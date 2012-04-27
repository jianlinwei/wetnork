#ifndef HOST_TUNADMINCONTEXT_H
#define HOST_TUNADMINCONTEXT_H

#include <memory>
#include <netlink/netlink.h>

#include <host/TunDevice.hpp>

namespace host {

	/**
	 * Defines administrative operations on TunDevice instances.
	 */
	class TunAdminContext {
		private:
			nl_sock* nl_;

			//! Closes the netlink socket, if it is open.
			void close();

		public:
			/**
			 * Creates a new TunAdminContext.
			 *
			 * \throws std::runtime_error Interface to the kernel could not be established
			 */
			TunAdminContext();

			//! Releases the TunAdminContext and all kernel resources associated with it
			~TunAdminContext();

			//! Constructs a new TunAdminContext using move semantics.
			TunAdminContext(TunAdminContext&& other);
			//! Assigns the resources of \a rhs to \c this using move semantics.
			TunAdminContext& operator=(TunAdminContext&& rhs);


			/**
			 * Changes the link state of \a tun.
			 *
			 * \throws std::runtime_error Link state could not be changed
			 * \post
			 * \a tun is in "up" state iff <tt>up == true</tt>
			 */
			void setLinkState(const TunDevice& tun, bool up);

			/**
			 * Changes the link MTU of \a tun.
			 *
			 * \throws std::runtime_error Link MTU could not be changed
			 * \post
			 * MTU of \a tun equals \a mtu
			 */
			void setLinkMtu(const TunDevice& tun, size_t mtu);
	};

}

#endif
