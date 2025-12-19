/*
 * CpET 140 Final Project — Cryptography module header
 * StructuraCost - Security - Cryptography module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Handle cryptographic operations such as encryption, decryption, hashing, and key management.
 * - Manage database encryption sessions.
 *
 * Boundaries
 * - Interacts with the security module for authentication and data protection.
 *
 * Notes
 * - This module is crucial for ensuring data integrity and confidentiality within the application.
 */

#ifndef CPET_140_FINAL_PROJECT_CRYPTOGRAPHY_H
#define CPET_140_FINAL_PROJECT_CRYPTOGRAPHY_H

#include <vector>
#include <string>

class Cryptography {
public:
    static bool checkSodium();
    static std::string hashKey(const std::string &key, int hashLen);
    static std::vector<unsigned char> generateKey(size_t keyBits);
    static std::string toHex(const std::vector<unsigned char>& key);
    static std::string saltKey(const std::string& key);
    static bool encryptFile(const std::string &filePath, const std::vector<unsigned char> &key);
    static bool decryptFile(const std::string &filePath, const std::vector<unsigned char> &key, std::string* errorMsg = nullptr);
};

// Backward compatibility
namespace cryptography {
    inline bool checkSodium() { return Cryptography::checkSodium(); }
    inline std::string hashKey(const std::string &key, int hashLen) { return Cryptography::hashKey(key, hashLen); }
    inline std::vector<unsigned char> generateKey(size_t keyBits) { return Cryptography::generateKey(keyBits); }
    inline std::string toHex(const std::vector<unsigned char>& key) { return Cryptography::toHex(key); }
    inline std::string saltKey(const std::string& key) { return Cryptography::saltKey(key); }
    inline bool encryptFile(const std::string &filePath, const std::vector<unsigned char> &key) {
        return Cryptography::encryptFile(filePath, key);
    }
    inline bool decryptFile(const std::string &filePath, const std::vector<unsigned char> &key, std::string* errorMsg = nullptr) {
        return Cryptography::decryptFile(filePath, key, errorMsg);
    }
}


// Merged: DBEncryptionSession moved from encryption_session.{h,cpp} into cryptography.{h,cpp}
namespace security {

// Manages the in-memory session key (derived from login password)
// and provides helpers to encrypt/decrypt all application databases.
class DBEncryptionSession {
public:
    // Set/replace session password (stores as raw bytes in-memory)
    static void setPassword(const std::string& password);
    // Wipe the in-memory key
    static void clear();
    // Whether a session key is currently set
    static bool hasKey();

    // Decrypt all encrypted databases ("*.db.enc" -> "*.db").
    // Non-existent files are skipped; failures are logged and cause false return.
    static bool decryptAllDbs();
    // Encrypt all plaintext databases ("*.db" -> "*.db.enc") and delete plaintext on success.
    // Non-existent files are skipped; failures are logged and cause false return.
    static bool encryptAllDbs();

private:
    static std::vector<unsigned char>& key();

    static bool decryptOne(const std::string& encPath);
    static bool encryptOne(const std::string& dbPath);
};

} // namespace security


#endif //CPET_140_FINAL_PROJECT_CRYPTOGRAPHY_H