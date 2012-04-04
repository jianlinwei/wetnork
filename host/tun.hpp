#ifndef HOST_TUN_H
#define HOST_TUN_H

#include <map>
#include <string>

#include <exception.hpp>

class TunRegistry {
	private:
		std::map<std::string, int> devices;

	public:
		TunRegistry() = default;

		TunRegistry(const TunRegistry&) = delete;
		TunRegistry& operator=(const TunRegistry&) = delete;

		~TunRegistry();

		int findDevice(const std::string& name) const;

		std::string createDevice(const std::string& nameTemplate);

		void closeDevice(const std::string& name);
};

#endif
