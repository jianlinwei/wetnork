#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <array>
#include <cstring>

#include <signal.hpp>
#include <IPAddress.hpp>
#include <SocketAddress.hpp>
#include <Packet.hpp>

class Stream {
	public:
		typedef Signal<void (Stream& sender, const Packet& packet)> OnRead;

	private:
		OnRead read;

	protected:
		virtual void propagate(const Packet& packet);

		Stream() = default;

	public:
		Stream(Stream&&) = default;
		Stream& operator=(Stream&&) = default;

		virtual ~Stream();

		virtual bool write(const Packet& packet) = 0;

		template<class Iterator>
		auto write(const Iterator& begin, const Iterator& end)
			-> typename std::enable_if<std::is_same<typename std::decay<decltype(*begin)>::type, Packet>::value, bool>::type
		{
			size_t size = 0;
			for (auto it = begin; it != end; ++it) {
				size += it->length();
			}

			std::unique_ptr<uint8_t[]> buffer(new uint8_t[size]);
			size_t offset = 0;
			for (auto it = begin; it != end; ++it) {
				std::memcpy(buffer.get() + offset, it->data(), it->length());
				offset += it->length();
			}

			return write(Packet(buffer.release(), 0, size));
		}

		template<class... Args>
		auto write(const Args&... args)
			-> typename std::pair<decltype(std::make_tuple(static_cast<const Packet&>(args)...)), bool>::second_type
		{
			std::array<Packet, sizeof...(Args)> packets {{ args... }};

			return write(packets.begin(), packets.end());
		}

		virtual ms::connection connectRead(OnRead::slot_function_type fn);
};

#endif
