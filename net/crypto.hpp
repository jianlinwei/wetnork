#ifndef NET_CRYPTO_H
#define NET_CRYPTO_H

#include "signal.hpp"
#include "network-common.hpp"
#include "exception.hpp"
#include "../include/crypto.hpp"

#include <boost/utility.hpp>
#include <gnutls/gnutls.h>
#include <gnutls/openpgp.h>
#include <memory>
#include <set>

class CryptoException : public Exception {
	private:
		int _code;
		int _alert;

	public:
		CryptoException(int code, int alert, const std::string& what);

		int code() const;
		int alert() const;
};



class CryptoSession;

class CryptoContext : private boost::noncopyable {
	private:
		gnutls_certificate_credentials_t credentials;

		std::set<KeyFingerprint> _permissiblePeers;

		static int gnutls_certificate_verify(gnutls_session_t session);

	public:
		CryptoContext();
		~CryptoContext();

		const std::set<KeyFingerprint>& permissiblePeers() const;
		void permissiblePeers(const std::set<KeyFingerprint>& peers);

		CryptoSession* openSession(bool server);
};



class CryptoSession : private boost::noncopyable {
	friend class CryptoContext;
	public:
		enum class State {
			Invalid,
			Opening,
			Open,
			Renegotiating,
			Closed,
			Aborted
		};

	private:
		// non-fatal errors:
		// AGAIN, REHANDSHAKE, WARNING_ALERT_RECEIVED
		// possible warning alerts:
		//	bad_certificate, unsupported_certificate, certificate_revoked, certificate_expired,
		//	certificate_unknown, user_canceled, no_renegotiation
		// things to look out for:
		//	* close_notify packets in accept()
		//	* HelloVerifyRequest in active connections (endpoint ip changed?)
		//	* disallow all resumes
		//	* rehandshakes in intervals
		//	* alerts are generally a very bad thing to receive. see list above; everything is fatal
		//	* only secure renegotiation
		//	* send handshake_failure when appropriate
		//	* reject connections with cert changes
		typedef Signal<void (CryptoSession& self, State oldState)> OnStateChanged;
		typedef Signal<void (CryptoSession& self, const Packet& packet)> OnPacketDecrypted;
		typedef Signal<ssize_t (CryptoSession& self, const Packet& packet)> OnPacketEncrypted;

		CryptoContext& context;

		OnStateChanged stateChanged;
		State _state;

		OnPacketDecrypted packetDecrypted;
		OnPacketEncrypted packetEncrypted;

		gnutls_session_t session;

		std::unique_ptr<Packet> currentPacket;

		void setState(State state);
		void fail(int code);

		void handlePacket();
		void handleHandshake();
		void handleData();

		static ssize_t gnutls_pull(gnutls_transport_ptr_t ptr, void* data, size_t size);
		static int gnutls_pull_timeout(gnutls_transport_ptr_t ptr, unsigned int ms);
		static ssize_t gnutls_push(gnutls_transport_ptr_t ptr, const void* data, size_t size);

		ssize_t pull(void* data, size_t size);
		int pull_timeout(unsigned int ms);
		ssize_t push(const void* data, size_t size);

		CryptoSession(gnutls_session_t sesssion, CryptoContext& context);

	public:
		~CryptoSession();

		State state() const;

		bs2::connection connectStateChanged(OnStateChanged::slot_function_type fn);

		bs2::connection connectPacketDecrypted(OnPacketDecrypted::slot_function_type fn);
		void readPacket(const Packet& packet);

		bs2::connection connectPacketEncrypted(OnPacketEncrypted::slot_function_type fn);
		bool writePacket(const Packet& packet);

		void open();
		void renegotiate();
		void close();
};

#endif
