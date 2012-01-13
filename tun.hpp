#ifndef TUN_H
#define TUN_H

#include <string>
#include <sys/types.h>
#include <ev++.h>
#include <boost/utility.hpp>
#include <boost/signals2.hpp>

#include "network.hpp"

class TunDevice : private boost::noncopyable {
	private:
		typedef boost::signals2::signal<void (TunDevice& sender, const Packet& packet)> OnCanRead;

		const int fd;
		const std::string _name;
		ev::io watcher;

		OnCanRead onCanRead;

		void watcherEvent(ev::io& io, int revents);

		TunDevice(const int fd, const std::string& name, const ev::loop_ref& loop);

	public:
		static TunDevice* create(const std::string& name_template, const ev::loop_ref& loop);

		~TunDevice();

		const std::string name() const;

		ssize_t write(const Packet& packet);

		boost::signals2::connection connect(OnCanRead::slot_function_type cb);
};

#endif
