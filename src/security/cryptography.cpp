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


#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>

#include "cryptography.h"

#include "sodium.h"


bool cryptography::checkSodium() {
    return sodium_init() >= 0;
}


// Generates a cryptographically secure random key.
// keyBits: size of a key in *bits* (32 to 8192)
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


std::string cryptography::hashKey(const std::string& key) {
    if (!checkSodium()) return {};

    constexpr size_t hashLen = crypto_generichash_BYTES;
    std::vector<unsigned char> hash(hashLen);

    if (const unsigned char* data = key.empty() ? nullptr : reinterpret_cast<const unsigned char*>(key.data()); crypto_generichash(hash.data(), hashLen, data, key.size(), nullptr, 0) != 0) {
        sodium_memzero(hash.data(), hash.size());
        return {};
    }

    std::string hex = toHex(hash);
    sodium_memzero(hash.data(), hash.size());
    return hex;
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


bool cryptography::encryptFile(const std::string &filePath, const std::vector<unsigned char> &key) {
    if (key.empty()) return false;

    constexpr size_t required = crypto_aead_xchacha20poly1305_ietf_KEYBYTES;
    constexpr size_t saltLen = crypto_pwhash_SALTBYTES;

    // Generate a random salt and derive a key using Argon2-like pwhash for resistance
    std::vector<unsigned char> salt(saltLen);
    randombytes_buf(salt.data(), saltLen);

    std::vector<unsigned char> derivedKey(required);
    if (crypto_pwhash(derivedKey.data(), required,
                      reinterpret_cast<const char*>(key.data()), key.size(),
                      salt.data(),
                      crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        return false; // out of memory or too slow
    }

    std::ifstream fin(filePath, std::ios::binary);
    if (!fin.is_open()) {
        sodium_memzero(derivedKey.data(), derivedKey.size());
        return false;
    }

    fin.seekg(0, std::ios::end);
    const auto fileSize = fin.tellg();
    fin.seekg(0, std::ios::beg);

    std::vector<unsigned char> plaintext(static_cast<size_t>(fileSize));
    fin.read(reinterpret_cast<char*>(plaintext.data()), static_cast<std::streamsize>(fileSize));
    fin.close();

    std::vector<unsigned char> ciphertext(plaintext.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    unsigned long long ciphertext_len;
    crypto_aead_xchacha20poly1305_ietf_encrypt(ciphertext.data(), &ciphertext_len,
        plaintext.data(), plaintext.size(),
        nullptr, 0, nullptr, nonce, derivedKey.data());
    ciphertext.resize(ciphertext_len);

    // Write: salt || nonce || ciphertext
    const std::string outPath = filePath + ".enc";
    std::ofstream ofstream(outPath, std::ios::binary | std::ios::trunc);
    if (!ofstream.is_open()) {
        sodium_memzero(derivedKey.data(), derivedKey.size());
        return false;
    }

    ofstream.write(reinterpret_cast<const char*>(salt.data()), static_cast<std::streamsize>(salt.size()));
    ofstream.write(reinterpret_cast<const char*>(nonce), static_cast<std::streamsize>(sizeof(nonce)));
    ofstream.write(reinterpret_cast<const char*>(ciphertext.data()), static_cast<std::streamsize>(ciphertext.size()));
    ofstream.close();

    sodium_memzero(derivedKey.data(), derivedKey.size());
    return true;
}


bool cryptography::decryptFile(const std::string& filePath, const std::vector<unsigned char>& key, std::string* errorMsg) {
    if (key.empty()) { if (errorMsg) *errorMsg = "Empty key provided"; return false; }

    constexpr size_t required = crypto_aead_xchacha20poly1305_ietf_KEYBYTES;
    constexpr size_t saltLen = crypto_pwhash_SALTBYTES;

    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open()) { if (errorMsg) *errorMsg = "Cannot open encrypted file: " + filePath; return false; }

    in.seekg(0, std::ios::end);
    const auto fileSize = in.tellg();
    in.seekg(0, std::ios::beg);

    // compute a minimum valid encrypted file size in the same type as fileSize
    constexpr std::streamoff minValid = static_cast<std::streamoff>(saltLen)
                                        + static_cast<std::streamoff>(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES)
                                        + static_cast<std::streamoff>(crypto_aead_xchacha20poly1305_ietf_ABYTES);
    if (fileSize < minValid) {
        if (errorMsg) *errorMsg = "File too small to be valid encrypted file";
        return false;
    }

    // Read salt
    std::vector<unsigned char> salt(saltLen);
    in.read(reinterpret_cast<char*>(salt.data()), static_cast<std::streamsize>(salt.size()));

    // Read nonce
    std::vector<unsigned char> nonce(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    in.read(reinterpret_cast<char*>(nonce.data()), static_cast<std::streamsize>(nonce.size()));

    // Read ciphertext
    const auto remaining = static_cast<size_t>(fileSize) - saltLen - nonce.size();
    std::vector<unsigned char> ciphertext(remaining);
    in.read(reinterpret_cast<char*>(ciphertext.data()), static_cast<std::streamsize>(remaining));
    in.close();

    // Derive key using salt and provided key material
    std::vector<unsigned char> derivedKey(required);
    if (crypto_pwhash(derivedKey.data(), required,
                      reinterpret_cast<const char*>(key.data()), key.size(),
                      salt.data(),
                      crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        if (errorMsg) *errorMsg = "Key derivation failed during decrypt";
        return false;
    }

    unsigned long long len = 0;
    std::vector<unsigned char> plaintext(ciphertext.size());

    if (crypto_aead_xchacha20poly1305_ietf_decrypt(plaintext.data(), &len,
                                                   nullptr,
                                                   ciphertext.data(), ciphertext.size(),
                                                   nullptr, 0,
                                                   nonce.data(), derivedKey.data()) != 0) {
        sodium_memzero(derivedKey.data(), derivedKey.size());
        if (errorMsg) *errorMsg = "Decryption failed: wrong key or corrupted file";
        return false;
    }

    // Overwrite the encrypted file with decrypted content
    std::ofstream out(filePath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        sodium_memzero(derivedKey.data(), derivedKey.size());
        if (errorMsg) *errorMsg = "Cannot write decrypted file";
        return false;
    }
    out.write(reinterpret_cast<const char*>(plaintext.data()), static_cast<std::streamsize>(len));
    out.close();

    sodium_memzero(derivedKey.data(), derivedKey.size());
    return true;
}
