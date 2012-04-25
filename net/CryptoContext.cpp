#include <stdexcept>
#include <string.h>
#include <limits>

#include <exception.hpp>

#include "crypto.hpp"

using namespace std;

CryptoContext::CryptoContext(const string& pubKey, const string& privKey, const std::string& subkey)
{
	if (pubKey.size() >= numeric_limits<int>::max()) {
		throw invalid_argument("pubKey");
	}
	if (privKey.size() >= numeric_limits<int>::max()) {
		throw invalid_argument("privKey");
	}

	int err = gnutls_certificate_allocate_credentials(&credentials);
	if (err != 0) {
		throw CryptoException(err, 0, gnutls_strerror(err));
	}

	gnutls_certificate_set_verify_function(credentials, &gnutls_certificate_verify);

	gnutls_datum_t pubDat = {
		reinterpret_cast<unsigned char*>(const_cast<char*>(pubKey.data())),
		static_cast<unsigned int>(pubKey.size()) };
	gnutls_datum_t privDat = {
		reinterpret_cast<unsigned char*>(const_cast<char*>(privKey.data())),
		static_cast<unsigned int>(privKey.size()) };

	err = gnutls_certificate_set_openpgp_key_mem2(credentials, &pubDat, &privDat, subkey.c_str(),
			GNUTLS_OPENPGP_FMT_BASE64);
	if (err != 0) {
		gnutls_certificate_free_credentials(credentials);
		throw CryptoException(err, 0, gnutls_strerror(err));
	}
}

CryptoContext::~CryptoContext()
{
	gnutls_certificate_free_credentials(credentials);
}

const set<KeyFingerprint>& CryptoContext::permissiblePeers() const
{
	return _permissiblePeers;
}

void CryptoContext::permissiblePeers(const set<KeyFingerprint>& peers)
{
	_permissiblePeers = peers;
}

CryptoSession CryptoContext::openSession(Stream& next, bool server)
{
	gnutls_session_t session;

	int err = gnutls_init(&session, (server ? GNUTLS_SERVER : GNUTLS_CLIENT) | GNUTLS_DATAGRAM | GNUTLS_NONBLOCK);
	if (err < 0) {
		throw CryptoException(err, 0, gnutls_strerror(err));
	}

	err = gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, credentials);
	if (err < 0) {
		throw CryptoException(err, 0, gnutls_strerror(err));
	}

	if (server) {
		gnutls_certificate_server_set_request(session, GNUTLS_CERT_REQUIRE);
	}

	return CryptoSession(session, next, *this);
}

int CryptoContext::gnutls_certificate_verify(gnutls_session_t session)
{
	CryptoContext& context = reinterpret_cast<CryptoSession*>(gnutls_session_get_ptr(session))->context;

	if (gnutls_certificate_type_get(session) != GNUTLS_CRT_OPENPGP) {
		return 1;
	}

	unsigned int size;
	const gnutls_datum_t* peerCert = gnutls_certificate_get_peers(session, &size);

	if (!peerCert) {
		return 1;
	}

	gnutls_openpgp_crt_t cert;
	int result = 0;

	if (gnutls_openpgp_crt_init(&cert)) {
		return 1;
	}
	if (gnutls_openpgp_crt_import(cert, peerCert, GNUTLS_OPENPGP_FMT_RAW)) {
		result = 1;
	}

	if (!result) {
		char fpr[128];
		size_t fprLen = sizeof(fpr);
		if (gnutls_openpgp_crt_get_fingerprint(cert, fpr, &fprLen)) {
			result = 1;
		} else {
			result = context._permissiblePeers.count(KeyFingerprint(fpr, fprLen)) ? 0 : 1;
		}
	}

	gnutls_openpgp_crt_deinit(cert);

	return result;
}
