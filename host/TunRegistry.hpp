#ifndef HOST_TUN_H
#define HOST_TUN_H

#include <map>
#include <string>
#include <memory>

#include <exception.hpp>
#include <host/TunDevice.hpp>

namespace host {

	class TunRegistry {
		private:
			std::map<std::string, std::unique_ptr<TunDevice>> devices;
			std::string tunCtl_;

		public:
			TunRegistry(const std::string& tunCtl)
				: tunCtl_(tunCtl)
			{
			}

			TunRegistry(TunRegistry&& other) = default;
			TunRegistry& operator=(TunRegistry&& rhs) = default;

			TunDevice& createDevice(const std::string& nameTemplate);

			TunDevice* findDevice(const std::string& name) const;

			bool closeDevice(const std::string& name);
	};

}

#endif
