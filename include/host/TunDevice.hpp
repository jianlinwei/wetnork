#ifndef INCLUDE_HOST_TUNDEVICE_H
#define INCLUDE_HOST_TUNDEVICE_H

#include <string>

namespace host {

	class TunDevice {
		private:
			int fd_, ifIndex_;
			std::string name_;

			void close();

		public:
			TunDevice(int fd, int ifIndex, const std::string& name);

			TunDevice(TunDevice&& other);

			TunDevice& operator=(TunDevice&& rhs);

			~TunDevice();

			int fd() const;

			int ifIndex() const;

			const std::string& name() const;
	};

}

#endif
