/*
* CpET 140 Final Project — Cryptography module
* StructuraCost - Security - Cryptography module
*
* Contributors:
*  Joshua Literal
*
* Purpose
* - Handle cryptographic operations such as encryption, decryption, hashing, and key management.
*
* Boundaries
* - Interacts with the security module for authentication and data protection.
*
* Notes
* - This module is crucial for ensuring data integrity and confidentiality within the application.
*/


#include "cryptography.h"
#include <sodium.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>


bool cryptography::checkSodium() {
    return sodium_init() >= 0;
}


// Generates a cryptographically secure random key.
// keyBits: size of key in *bits* (32 to 8192)
// returns: vector of random bytes
std::vector<unsigned char> cryptography::generateKey(const size_t keyBits) {
    if (keyBits < 32 || keyBits > 8192 || (keyBits % 8) != 0) return {};

    const auto keyBytes = keyBits / 8;
    std::vector<unsigned char> key(keyBytes);
    randombytes_buf(key.data(), keyBytes);
    return key;
}


std::string cryptography::toHex(const std::vector<unsigned char>& key) {

    std::ostringstream ss;

    for (const unsigned char c : key)
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);

    return ss.str();
}


std::string cryptography::saltKey(const std::string& key) {

    constexpr size_t saltBytes = crypto_pwhash_SALTBYTES;

    std::vector<unsigned char> salt(saltBytes);
    randombytes_buf(salt.data(), saltBytes);
    const size_t derivedLen = key.size();
    std::vector<unsigned char> derived(derivedLen);

    if (crypto_pwhash(derived.data(), derivedLen,
                      key.data(), key.size(),
                      salt.data(),
                      crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {return "";}

    return toHex(derived);
}


bool cryptography::encryptFile(const std::string &filePath, const unsigned char *key) {
    std::ifstream fin(filePath, std::ios::binary);
    if (!fin.is_open()) return false;

    fin.seekg(0, std::ios::end);
    const auto fileSize = fin.tellg();
    fin.seekg(0, std::ios::beg);

    std::vector<unsigned char> plaintext(fileSize);
    fin.read(reinterpret_cast<char*>(plaintext.data()), fileSize);
    fin.close();

    std::vector<unsigned char> ciphertext(plaintext.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    unsigned long long ciphertext_len;
    crypto_aead_xchacha20poly1305_ietf_encrypt(ciphertext.data(), &ciphertext_len,
        plaintext.data(), plaintext.size(),
                                                nullptr, 0, nullptr, nonce, key);
    ciphertext.resize(ciphertext_len);

    std::ofstream fout(filePath, std::ios::binary | std::ios::trunc);
    if (!fout.is_open()) return false;

    fout.write(reinterpret_cast<const char *>(nonce), sizeof(nonce));
    fout.write(reinterpret_cast<const char *>(ciphertext.data()), ciphertext.size());
    fout.close();

    return true;
}


bool cryptography::decryptFile(const std::string& filePath, const std::vector<unsigned char>& key) {
    if (key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) return false;

    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open()) return false;

    in.seekg(0, std::ios::end);
    const auto fileSize = in.tellg();
    in.seekg(0, std::ios::beg);

    std::vector<unsigned char> fileData(fileSize);
    in.read(reinterpret_cast<char*>(fileData.data()), fileSize);
    in.close();

    constexpr size_t nonceLen = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
    if (fileData.size() < nonceLen + crypto_aead_xchacha20poly1305_ietf_ABYTES) return false;

    std::vector<unsigned char> nonce(fileData.begin(), fileData.begin() + nonceLen);
    std::vector<unsigned char> ciphertext(fileData.begin() + nonceLen, fileData.end());

    unsigned long long len = 0;
    std::vector<unsigned char> plaintext(ciphertext.size());

    if (crypto_aead_xchacha20poly1305_ietf_decrypt(plaintext.data(), &len,
                                                   nullptr,
                                                   ciphertext.data(), ciphertext.size(),
                                                   nullptr, 0,
                                                   nonce.data(), key.data()) != 0) {
        return false;
    }

    std::ofstream out(filePath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;
    out.write(reinterpret_cast<const char*>(plaintext.data()), len);
    out.close();
    return true;
}
