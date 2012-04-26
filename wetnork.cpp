#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <linux/if.h>
#include <fcntl.h>
#include <gnutls/gnutls.h>
#include <ev.h>

//#include "net/tun.hpp"
#include "host/TunRegistry.hpp"

using namespace std;

//TunDevice *tun1, *tun2;
//TunRegistry reg;

//void io_cb(TunDevice& sender, const Packet& packet)
//{
//	printf("read: %lu\n", packet.length());
//	int len = tun2->write(packet);
//	printf("written: %s (%i)\n", strerror(errno), len);
//}

int main(int argc, char **argv)
{
	std::string s = "/dev/net/tun";
	host::TunRegistry tr(s);

	{
		auto& td = tr.createDevice("tun%d-%%d");
		fprintf(stdout, "%s %i %i\n", td.name().c_str(), td.fd(), td.ifIndex());
	}
	{
		auto& td = tr.createDevice("tun%d-%%d");
		fprintf(stdout, "%s %i %i\n", td.name().c_str(), td.fd(), td.ifIndex());
	}

	getchar();
//	if (!ev_default_loop(0)) {
//		printf("No default loop!\n");
//		return 1;
//	}
//
//	string tun1name = reg.createDevice("tun-p-%d");
//	tun1 = new TunDevice(reg.findDevice(tun1name), tun1name, ev_default_loop(0));
//	perror(tun1->name().c_str());
//
//	string tun2name = reg.createDevice("tun-p-%d");
//	tun2 = new TunDevice(reg.findDevice(tun2name), tun2name, ev_default_loop(0));
//	perror(tun2->name().c_str());
//
//	tun1->connectCanRead(io_cb);
//
//	ev_run(ev_default_loop(0), 0);
//
//	delete tun1;
//	delete tun2;

	return 0;
}

