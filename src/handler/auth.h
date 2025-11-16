#ifndef CPET_140_FINAL_PROJECT_AUTH_H
#define CPET_140_FINAL_PROJECT_AUTH_H

#include <string>

class auth {
    public:
        static std::string authGateway(const std::string& username, const std::string& password, const std::string& deviceCode);
        static bool checkSodium();

        // Hash a password with libsodium's recommended password hashing API (argon2id via crypto_pwhash)
        // Returns an encoded hash string (including parameters and salt) on success, empty string on failure.
        static std::string hashPassword(const std::string &password);

        // Verify a password against the encoded hash produced by hashPassword.
        // Returns true if the password matches, false otherwise.
        static bool verifyPassword(const std::string &encodedHash, const std::string &password);
};


#endif //CPET_140_FINAL_PROJECT_AUTH_H