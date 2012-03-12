#include "crypto.hpp"

#include "exception.hpp"
#include <stdexcept>
#include <string.h>

using namespace std;

CryptoSession::CryptoSession(gnutls_session_t session, CryptoContext& context)
	: context(context), _state(State::Invalid), session(session)
{
	if (!session) {
		throw std::invalid_argument("session");
	}

	gnutls_transport_set_ptr(session, this);

	gnutls_transport_set_pull_function(session, gnutls_pull);
	gnutls_transport_set_pull_timeout_function(session, gnutls_pull_timeout);
	gnutls_transport_set_push_function(session, gnutls_push);
}

CryptoSession::~CryptoSession()
{
	gnutls_deinit(session);
}



bs2::connection CryptoSession::connectStateChanged(OnStateChanged::slot_function_type fn)
{
	return stateChanged.connect(fn);
}

bs2::connection CryptoSession::connectPacketDecrypted(OnPacketDecrypted::slot_function_type fn)
{
	return packetDecrypted.connect(fn);
}

bs2::connection CryptoSession::connectPacketEncrypted(OnPacketEncrypted::slot_function_type fn)
{
	return packetEncrypted.connect(fn);
}



void CryptoSession::setState(State state)
{
	State oldState = _state;
	_state = state;
	stateChanged(*this, oldState);
}

void CryptoSession::fail(int code)
{
	_state = State::Aborted;
	throw CryptoException(code, gnutls_alert_get(session), gnutls_strerror(code));
}

CryptoSession::State CryptoSession::state() const
{
	return _state;
}



void CryptoSession::handleHandshake()
{
	int ret = gnutls_handshake(session);

	switch (ret) {
		case GNUTLS_E_GOT_APPLICATION_DATA:
			handleData();
			break;

		case GNUTLS_E_AGAIN:
			break;

		case GNUTLS_E_SUCCESS:
			setState(State::Open);
			break;

		default:
			fail(ret);
			break;
	}
}

void CryptoSession::handleData()
{
	static const size_t bufferSize = 1 << 14;
	uint8_t* buffer = new uint8_t[bufferSize];

	try {
		ssize_t ret = gnutls_record_recv(session, buffer, bufferSize);

		if (ret < 0) {
			if (ret == GNUTLS_E_REHANDSHAKE) {
				setState(State::Renegotiating);
				handleHandshake();
			} else {
				fail(ret);
			}
		} else if (ret == 0) {
			if (_state != State::Closed) {
				gnutls_bye(session, GNUTLS_SHUT_WR);
			}
			setState(State::Closed);
		} else {
			uint8_t* packetBuffer = new uint8_t[ret];
			memcpy(packetBuffer, buffer, ret);
			packetDecrypted(*this, Packet(packetBuffer, 0, ret));
		}
	} catch (...) {
		delete buffer;
		throw;
	}
}

void CryptoSession::handlePacket()
{
	if (_state == State::Opening || _state == State::Renegotiating) {
		handleHandshake();
	} else if (_state == State::Open) {
		handleData();
	} else {
		throw InvalidOperation("Session not ready");
	}
}



void CryptoSession::open()
{
	if (_state != State::Invalid) {
		throw InvalidOperation("Session not ready");
	}

	setState(State::Opening);
	handleHandshake();
}

void CryptoSession::renegotiate()
{
	if (_state != State::Open) {
		throw InvalidOperation("Session not ready");
	}

	gnutls_rehandshake(session);
	setState(State::Renegotiating);
}

void CryptoSession::close()
{
	if (_state != State::Open) {
		throw InvalidOperation("Session not ready");
	}

	gnutls_bye(session, GNUTLS_SHUT_WR);
	setState(State::Closed);
}

void CryptoSession::readPacket(const Packet& packet)
{
	currentPacket = unique_ptr<Packet>(new Packet(packet));
	handlePacket();
}

bool CryptoSession::writePacket(const Packet& packet)
{
	if (_state != State::Open) {
		throw InvalidOperation("Session not ready");
	}

	int result = gnutls_record_send(session, packet.data(), packet.length());
	if (result == GNUTLS_E_AGAIN) {
		return false;
	} else if (result < 0) {
		throw CryptoException(result, 0, gnutls_strerror(result));
	} else {
		return true;
	}
}



ssize_t CryptoSession::gnutls_pull(gnutls_transport_ptr_t ptr, void* data, size_t size)
{
	return reinterpret_cast<CryptoSession*>(ptr)->pull(data, size);
}

int CryptoSession::gnutls_pull_timeout(gnutls_transport_ptr_t ptr, unsigned int ms)
{
	return reinterpret_cast<CryptoSession*>(ptr)->pull_timeout(ms);
}

ssize_t CryptoSession::gnutls_push(gnutls_transport_ptr_t ptr, const void* data, size_t size)
{
	return reinterpret_cast<CryptoSession*>(ptr)->push(data, size);
}

ssize_t CryptoSession::pull(void* data, size_t size)
{
	if (currentPacket) {
		int len = size < currentPacket->length() ? size : currentPacket->length();
		memcpy(data, currentPacket->data(), len);
		currentPacket.reset();
		return len;
	} else {
		errno = EAGAIN;
		return -1;
	}
}

int CryptoSession::pull_timeout(unsigned int ms)
{
	if (currentPacket) {
		return currentPacket->length();
	} else {
		return 0;
	}
}

ssize_t CryptoSession::push(const void* data, size_t size)
{
	uint8_t* buffer = new uint8_t[size];
	memcpy(buffer, data, size);
	return packetEncrypted(*this, Packet(buffer, 0, size)).get();
}

