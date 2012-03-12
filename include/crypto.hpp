#ifndef CRYPTO_H
#define CRYPTO_H

class KeyFingerprint {
	private:
		const char* _data;
		int _length;

		void set(const char* data, int length);

	public:
		KeyFingerprint(const char* data, int length);
		KeyFingerprint(const KeyFingerprint& fp);
		~KeyFingerprint();

		const char* get() const;
		const int length() const;

		bool operator<(const KeyFingerprint& other) const;
		bool operator==(const KeyFingerprint& other) const;

		KeyFingerprint& operator=(const KeyFingerprint& other);
};

#endif
