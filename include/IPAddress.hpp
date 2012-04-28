#ifndef INCLUDE_IPADDRESS_H
#define INCLUDE_IPADDRESS_H

#include <netinet/in.h>
#include <string>
#include <cstring>
#include <boost/operators.hpp>

//! Enumeration of all valid and known address families
enum class AddressFamily {
	//! IPv4
	IPv4 = AF_INET,

	//! IPv6
	IPv6 = AF_INET6
};

//! This class represents an IP address, v4 or v6.
class IPAddress : boost::totally_ordered<IPAddress> {
	private:
		union {
			in_addr in;
			in6_addr in6;
		} addr_;
		AddressFamily family_;
		
	public:
		/**
		 * Creates a new IPv4 address from a given native address.
		 *
		 * \post
		 * \code
		 * family() == AddressFamily::IPv4
		 * *in() == addr
		 * \endcode
		 */
		IPAddress(const in_addr& addr)
			: family_(AddressFamily::IPv4)
		{
			addr_.in = addr;
		}

		/**
		 * Creates a new IPv6 address from a given native address.
		 *
		 * \post
		 * \code
		 * family() == AddressFamily::IPv6
		 * *in6() == addr
		 * \endcode
		 */
		IPAddress(const in6_addr& addr)
			: family_(AddressFamily::IPv6)
		{
			addr_.in6 = addr;
		}

		/**
		 * Creates a new IP address from a given string. The default address formats for
		 * IPv4 and IPv6 are supported.
		 */
		static IPAddress parse(const std::string& addr);

		//! Address family of the current instance.
		AddressFamily family() const
		{
			return family_;
		}

		//! Native value for the address family
		int native_family() const
		{
			return static_cast<int>(family_);
		}

		//! Size of the current address in native form.
		size_t native_size() const
		{
			switch (family_) {
				case AddressFamily::IPv4:
					return sizeof(addr_.in);

				case AddressFamily::IPv6:
					return sizeof(addr_.in6);
			}
		}

		/**
		 * Pointer to native IPv4 address.
		 *
		 * \pre
		 * \code
		 * family() == AddressFamily::IPv4
		 * \endcode
		 */
		const in_addr& in() const
		{
			return addr_.in;
		}

		/**
		 * Pointer to native IPv6 address.
		 *
		 * \pre
		 * \code
		 * family() == AddressFamily::IPv6
		 * \endcode
		 */
		const in6_addr& in6() const
		{
			return addr_.in6;
		}

		//! Returns a string represantation of the current instance.
		std::string str() const;

	public:
		//! Compares two instances of IPAddress for \e less-than.
		friend bool operator<(const IPAddress& lhs, const IPAddress& rhs)
		{
			return lhs.family() < rhs.family()
				|| std::memcmp(&lhs.in6(), &rhs.in6(), lhs.native_size());
		}

		//! Compares two instances of IPAddress for \e equals.
		friend bool operator==(const IPAddress& lhs, const IPAddress& rhs)
		{
			return lhs.family() == rhs.family()
				&& std::memcmp(&lhs.in6(), &rhs.in6(), lhs.native_size()) == 0;
		}
};

#endif
