#ifndef CRYPTO_H
#define CRYPTO_H

#include <cstdint>
#include <vector>

class KeyFingerprint {
	private:
		std::vector<char> _data;

	public:
		KeyFingerprint(const char* data, unsigned int length);

		const char* get() const;
		const unsigned int length() const;

		bool operator<(const KeyFingerprint& other) const;
		bool operator==(const KeyFingerprint& other) const;
};

#endif
