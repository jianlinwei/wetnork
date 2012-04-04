#ifndef TUN_H
#define TUN_H

#include <string>
#include <sys/types.h>
#include <ev++.h>
#include <boost/utility.hpp>
#include <boost/signals2.hpp>

#include <network.hpp>

class TunDevice : public Stream, boost::noncopyable {
	private:
		const int fd;
		const std::string _name;
		ev::io watcher;

		void watcherEvent(ev::io& io, int revents);

	public:
		TunDevice(const int fd, const std::string& name, const ev::loop_ref& loop);

		~TunDevice();

		const std::string name() const;

		bool write(const Packet& packet) override;
};

#endif
