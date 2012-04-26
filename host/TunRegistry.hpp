#ifndef HOST_TUN_H
#define HOST_TUN_H

#include <map>
#include <string>
#include <memory>

#include <exception.hpp>
#include <host/TunDevice.hpp>

namespace host {

	/**
	 * Registry and factory for tunnel devices.
	 *
	 * This class wraps the tunnel clone device of linux and keeps track of all
	 * tunnel devices it has created.
	 *
	 * This class is not copyable, but movable.
	 */
	class TunRegistry {
		private:
			std::map<std::string, std::unique_ptr<TunDevice>> devices;
			std::string tunCtl_;

		public:
			/**
			 * Creates a new TunRegistry.
			 *
			 * \param tunCtl Name of the tunnel clone device
			 */
			TunRegistry(const std::string& tunCtl)
				: tunCtl_(tunCtl)
			{
			}

			//! Default move constructor.
			TunRegistry(TunRegistry&& other) = default;
			//! Default move assignment operator.
			TunRegistry& operator=(TunRegistry&& rhs) = default;

			/**
			 * Creates a new TunDevice. The name for the new device will be taken from
			 * \a nameTemplate. If \a nameTemplate contains \c %d, the first occurence
			 * will be replaced by a number. If \a nameTemplate does not contain \c %d,
			 * the name for the device will be taken as given.
			 *
			 * \throws FileNotFound \c tunCtl could not be opened
			 * \throws InvalidOperation The name of the device could not be set
			 *
			 * \post
			 * \code
			 * result.fd() >= 0
			 * result.ifIndex() >= 0
			 * result.name() matches nameTemplate
			 * \endcode
			 */
			const TunDevice& createDevice(const std::string& nameTemplate);

			/**
			 * Attempts to locate a device named \a name in this registry. If no such
			 * device exists, \c nullptr will be returned.
			 */
			const TunDevice* findDevice(const std::string& name) const;

			/**
			 * Attempts to close the devices names \a name in this registry.
			 *
			 * \return \c true, iff the device \a name belonged to this registry.
			 */
			bool closeDevice(const std::string& name);
	};

}

#endif
