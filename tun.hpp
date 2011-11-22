#ifndef TUN_H
#define TUN_H

#include <string>
#include <sys/types.h>
#include <ev++.h>
#include <boost/utility.hpp>
#include <boost/signal.hpp>

class TunDevice : private boost::noncopyable {
	private:
		typedef boost::signal<void()> OnCanRead;

		const int fd;
		const std::string _name;
		ev::io watcher;

		OnCanRead onCanRead;

		void watcherEvent(ev::io& io, int revents);

		TunDevice(const int fd, const std::string name, ev_loop* loop);

	public:
		static TunDevice* create(const std::string name_template, ev_loop* loop);

		~TunDevice();

		const std::string name() const;

		ssize_t read(char *buffer, size_t len);
		ssize_t write(const char *buffer, size_t len);

		boost::signals::connection connect(OnCanRead::slot_function_type cb);
};

#endif
