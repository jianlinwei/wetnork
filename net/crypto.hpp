#ifndef NET_CRYPTO_H
#define NET_CRYPTO_H

#include <gnutls/gnutls.h>
#include <gnutls/openpgp.h>
#include <memory>
#include <set>
#include <string>

#include <exception.hpp>
#include <crypto.hpp>
#include <network.hpp>
#include <signal.hpp>

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

class CryptoContext {
	private:
		gnutls_certificate_credentials_t credentials;

		std::set<KeyFingerprint> _permissiblePeers;

		static int gnutls_certificate_verify(gnutls_session_t session);

	public:
		CryptoContext(const CryptoContext&) = delete;
		CryptoContext& operator=(const CryptoContext&) = delete;

		CryptoContext(const std::string& pubKey, const std::string& privKey, const std::string& subkey = "auto");
		~CryptoContext();

		const std::set<KeyFingerprint>& permissiblePeers() const;
		void permissiblePeers(const std::set<KeyFingerprint>& peers);

		CryptoSession openSession(Stream& next, bool server);
};



class CryptoSession : public Stream {
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

		typedef Signal<void (CryptoSession& self, State oldState)> OnStateChanged;

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
		CryptoContext& context;
		Stream& next;
		ms::scoped_connection nextOnRead;

		OnStateChanged stateChanged;
		State _state;

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
		static ssize_t gnutls_vec_push(gnutls_transport_ptr_t ptr, const giovec_t* iov, int count);

		ssize_t pull(void* data, size_t size);
		int pull_timeout(unsigned int ms);
		ssize_t push(const void* data, size_t size);
		ssize_t vec_push(const giovec_t* iov, int count);

		void readPacket(Stream& sender, const Packet& packet);

		CryptoSession(gnutls_session_t sesssion, Stream& next, CryptoContext& context);

	public:
		CryptoSession(const CryptoSession&) = delete;
		CryptoSession& operator=(const CryptoSession&) = delete;

		CryptoSession(CryptoSession&&) = default;
		CryptoSession& operator=(CryptoSession&&) = default;

		~CryptoSession();

		State state() const;

		ms::connection connectStateChanged(OnStateChanged::slot_function_type fn);

		bool write(const Packet& packet) override;

		void open();
		void renegotiate();
		void close();
};

#endif
