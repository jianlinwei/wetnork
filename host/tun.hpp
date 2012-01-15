#ifndef HOST_TUN_H
#define HOST_TUN_H

#include <boost/utility.hpp>
#include <map>
#include <string>
#include "exception.hpp"

class TunRegistry : private boost::noncopyable {
	private:
		std::map<std::string, int> devices;

	public:
		int findDevice(const std::string& name) const;

		std::string createDevice(const std::string& nameTemplate);

		void closeDevice(const std::string& name);
};

#endif
