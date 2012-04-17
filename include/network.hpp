#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <array>

#include <signal.hpp>

struct SocketAddress {
	private:
		union {
			sockaddr_in in;
			sockaddr_in6 in6;
		} addr;
		socklen_t len;

		SocketAddress() = delete;

	public:
		SocketAddress(const sockaddr* addr);						

		static SocketAddress parse(const std::string& addr, in_port_t port);

		int family() const;

		const sockaddr* native() const;

		socklen_t native_len() const;

		bool operator<(const SocketAddress& other) const;

		bool operator==(const SocketAddress& other) const;
};



class Packet {
	private:
		static void NullDelete(const uint8_t*);
		static void ArrayDelete(const uint8_t*);

		std::shared_ptr<const uint8_t> _data;
		ptrdiff_t _offset;
		size_t _length;

		Packet(const std::shared_ptr<const uint8_t>& data, ptrdiff_t offset, size_t length);

	public:
		struct nocapture_t {};
		static constexpr nocapture_t NoCapture = {};

		Packet(const uint8_t* data, ptrdiff_t offset, size_t length);

		Packet(const uint8_t* data, ptrdiff_t offset, size_t length, nocapture_t);

		const uint8_t* data() const;

		size_t length() const;

		Packet skip(size_t bytes) const;
};



class Stream {
	public:
		typedef Signal<void (Stream& sender, const Packet& packet)> OnRead;

	private:
		OnRead read;

	protected:
		virtual void propagate(const Packet& packet);

		Stream() = default;

	public:
		Stream(const Stream&) = delete;
		Stream& operator=(const Stream&) = delete;

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
				memcpy(buffer.get() + offset, it->data(), it->length());
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

		virtual bs2::connection connectRead(OnRead::slot_function_type fn);
};

#endif
