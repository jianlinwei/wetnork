#include "TunAdminContext.hpp"

#include <stdexcept>
#include <linux/netlink.h>
#include <netlink/route/link.h>

using namespace host;


namespace {

	struct RtnlLink {
		private:
			rtnl_link* link_;

			RtnlLink(rtnl_link* link)
				: link_(link)
			{
			}

			void close()
			{
				if (link_) {
					rtnl_link_put(link_);
				}
			}

		public:
			RtnlLink()
				: link_(rtnl_link_alloc())
			{
				if (!link_) {
					throw std::runtime_error("Could not allocate rtnl_link");
				}
			}

			static RtnlLink get(nl_sock* nl, int ifIndex)
			{
				rtnl_link* link;

				int err = rtnl_link_get_kernel(nl, ifIndex, nullptr, &link);
				if (err < 0) {
					throw std::runtime_error("Could not get rtnl_link");
				}

				return RtnlLink(link);
			}

			~RtnlLink()
			{
				close();
			}

			RtnlLink(RtnlLink&& other)
			{
				link_ = other.link_;
				other.link_ = nullptr;
			}

			RtnlLink& operator=(RtnlLink&& other)
			{
				close();
				link_ = other.link_;
				other.link_ = nullptr;
				return *this;
			}

			operator rtnl_link*()
			{
				return link_;
			}
	};

}


TunAdminContext::TunAdminContext()
	: nl_(nl_socket_alloc())
{
	if (!nl_) {
		throw std::runtime_error("Could not open netlink socket");
	}

	int err = nl_connect(nl_, NETLINK_ROUTE);
	if (err < 0) {
		throw std::runtime_error(nl_geterror(err));
	}
}

TunAdminContext::~TunAdminContext()
{
	close();
}

TunAdminContext::TunAdminContext(TunAdminContext&& other)
	: nl_(other.nl_)
{
	other.nl_ = nullptr;
}

TunAdminContext& TunAdminContext::operator=(TunAdminContext&& rhs)
{
	close();
	nl_ = rhs.nl_;
	rhs.nl_ = nullptr;
	return *this;
}

void TunAdminContext::close()
{
	if (nl_) {
		nl_close(nl_);
		nl_socket_free(nl_);
	}
}


void TunAdminContext::setLinkState(const TunDevice& tun, bool up)
{
	RtnlLink link = RtnlLink::get(nl_, tun.ifIndex()), change;

	if (up) {
		rtnl_link_set_flags(change, IFF_UP);
	} else {
		rtnl_link_unset_flags(change, IFF_UP);
	}

	int err = rtnl_link_change(nl_, link, change, 0);
	if (err < 0) {
		throw std::runtime_error(nl_geterror(err));
	}
}

void TunAdminContext::setLinkMtu(const TunDevice& tun, size_t mtu)
{
	RtnlLink link = RtnlLink::get(nl_, tun.ifIndex()), change;

	rtnl_link_set_mtu(change, mtu);

	int err = rtnl_link_change(nl_, link, change, 0);
	if (err < 0) {
		throw std::runtime_error(nl_geterror(err));
	}
}
