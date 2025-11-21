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


std::string cryptography::hashKey(const std::string& key) {
    if (!checkSodium()) return {};

    constexpr size_t hashLen = crypto_generichash_BYTES;
    std::vector<unsigned char> hash(hashLen);

    const unsigned char* data = key.empty() ? nullptr : reinterpret_cast<const unsigned char*>(key.data());
    if (crypto_generichash(hash.data(), hashLen, data, key.size(), nullptr, 0) != 0) {
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
    std::ofstream fout(outPath, std::ios::binary | std::ios::trunc);
    if (!fout.is_open()) {
        sodium_memzero(derivedKey.data(), derivedKey.size());
        return false;
    }

    fout.write(reinterpret_cast<const char*>(salt.data()), static_cast<std::streamsize>(salt.size()));
    fout.write(reinterpret_cast<const char*>(nonce), static_cast<std::streamsize>(sizeof(nonce)));
    fout.write(reinterpret_cast<const char*>(ciphertext.data()), static_cast<std::streamsize>(ciphertext.size()));
    fout.close();

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

    if (fileSize < static_cast<std::streamoff>(saltLen + crypto_aead_xchacha20poly1305_ietf_NPUBBYTES + crypto_aead_xchacha20poly1305_ietf_ABYTES)) {
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


// Vault function to securely store and retrieve keys
// Operation: "STORE" or "RETRIEVE"
// Identifier: unique identifier for the data
std::string cryptography::vault(const std::string& operation, const std::string& identifier, const std::string& data) {
    if (!checkSodium()) return "ERROR: libsodium initialization failed";

    // Determine platform-specific secure storage path
    std::string vaultPath;
    #ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        vaultPath = appdata ? std::string(appdata) + "\\StructuraCost\\.vault" : ".vault";
    #elif __APPLE__
        const char* home = std::getenv("HOME");
        vaultPath = home ? std::string(home) + "/Library/Application Support/StructuraCost/.vault" : ".vault";
    #else
        const char* home = std::getenv("HOME");
        vaultPath = home ? std::string(home) + "/.config/StructuraCost/.vault" : ".vault";
    #endif

    // Generate master key from hardware/system-specific data
    constexpr size_t masterKeyLen = crypto_secretbox_KEYBYTES;
    std::vector<unsigned char> masterKey(masterKeyLen);

    // Use system-specific salt (you should enhance this with machine ID)
    auto computeReleaseCombinedVersion = []() -> std::string {
        // 1) If CI/Build sets COMBINED_VERSION environment variable, use it
        if (const char* env = std::getenv("COMBINED_VERSION")) {
            if (env[0] != '\0') return {env};
        }

        // 2) First, try the build-folder copy where CI might have rendered outputs
        const std::string buildWorkflow = "cmake-build-debug/.github/workflows/release.yml";
        if (std::ifstream bwf(buildWorkflow); bwf.is_open()) {
            std::string line;
            while (std::getline(bwf, line)) {
                // look for a direct combined_version assignment like: combined_version: v1.2.3
                auto pos = line.find("combined_version");
                if (pos != std::string::npos) {
                    if (auto colon = line.find(':', pos); colon != std::string::npos) {
                        std::string val = line.substr(colon + 1);
                        // trim
                        val.erase(0, val.find_first_not_of(" \t\"'"));
                        val.erase(val.find_last_not_of(" \t\"'") + 1);
                        if (!val.empty()) return val;
                    }
                }
            }
            // fall-through to parsing defaults in this build copy below if no direct combined_version
            bwf.clear();
            bwf.seekg(0);
            std::vector<std::string> lines;
            while (std::getline(bwf, line)) lines.push_back(line);

            auto findDefault = [&](const std::string& key)->std::string {
                for (size_t i = 0; i + 1 < lines.size(); ++i) {
                    if (lines[i].find(key) != std::string::npos) {
                        for (size_t j = i; j < std::min(lines.size(), i + 8); ++j) {
                            auto p = lines[j].find("default:");
                            if (p != std::string::npos) {
                                std::string v = lines[j].substr(p + 8);
                                v.erase(0, v.find_first_not_of(" \t\"'"));
                                v.erase(v.find_last_not_of(" \t\"'") + 1);
                                return v;
                            }
                        }
                    }
                }
                return {};
            };

            std::string W = findDefault("version_w");
            std::string X_NAME = findDefault("version_x");
            std::string Y = findDefault("version_y");
            std::string Z = findDefault("version_z");
            std::string ITERATION = findDefault("stage_iteration");

            if (W.empty()) W = "0";
            if (X_NAME.empty()) X_NAME = "stable";
            if (Y.empty()) Y = "0";
            if (Z.empty()) Z = "0";
            if (ITERATION.empty()) ITERATION = "0";

            int X_NUM = 4; std::string STAGE_SUFFIX;
            if (X_NAME == "stable") { X_NUM = 4; STAGE_SUFFIX = ""; }
            else if (X_NAME == "rc") { X_NUM = 3; STAGE_SUFFIX = "-rc"; }
            else if (X_NAME == "beta") { X_NUM = 2; STAGE_SUFFIX = "-beta"; }
            else if (X_NAME == "alpha") { X_NUM = 1; STAGE_SUFFIX = "-alpha"; }
            else if (X_NAME == "pre-alpha") { X_NUM = 0; STAGE_SUFFIX = "-pre-alpha"; }

            try { if (std::stoi(ITERATION) > 0 && X_NAME != "stable") STAGE_SUFFIX += "." + ITERATION; } catch(...) {}
            std::string VERSION_NUMERIC = std::string("v") + W + "." + std::to_string(X_NUM) + "." + Y + "." + Z;
            std::string COMBINED_VERSION = VERSION_NUMERIC + STAGE_SUFFIX;
            if (!COMBINED_VERSION.empty()) return COMBINED_VERSION;
        }

        // 3) Fallback: repo workflow (original logic)
        const std::string repoWorkflow = ".github/workflows/release.yml";
        if (std::ifstream rwf(repoWorkflow); rwf.is_open()) {
            std::string line;
            while (std::getline(rwf, line)) {
                auto pos = line.find("combined_version");
                if (pos != std::string::npos) {
                    if (auto colon = line.find(':', pos); colon != std::string::npos) {
                        std::string val = line.substr(colon + 1);
                        val.erase(0, val.find_first_not_of(" \t\"'"));
                        val.erase(val.find_last_not_of(" \t\"'") + 1);
                        if (!val.empty()) return val;
                    }
                }
            }
            // if not present as explicit, try computing from defaults similar to above
            rwf.clear(); rwf.seekg(0);
            std::vector<std::string> lines;
            while (std::getline(rwf, line)) lines.push_back(line);
            auto findDefault = [&](const std::string& key)->std::string {
                for (size_t i = 0; i + 1 < lines.size(); ++i) {
                    if (lines[i].find(key) != std::string::npos) {
                        for (size_t j = i; j < std::min(lines.size(), i + 8); ++j) {
                            auto p = lines[j].find("default:");
                            if (p != std::string::npos) {
                                std::string v = lines[j].substr(p + 8);
                                v.erase(0, v.find_first_not_of(" \t\"'"));
                                v.erase(v.find_last_not_of(" \t\"'") + 1);
                                return v;
                            }
                        }
                    }
                }
                return {};
            };
            std::string W = findDefault("version_w");
            std::string X_NAME = findDefault("version_x");
            std::string Y = findDefault("version_y");
            std::string Z = findDefault("version_z");
            std::string ITERATION = findDefault("stage_iteration");

            if (W.empty()) W = "0";
            if (X_NAME.empty()) X_NAME = "stable";
            if (Y.empty()) Y = "0";
            if (Z.empty()) Z = "0";
            if (ITERATION.empty()) ITERATION = "0";

            int X_NUM = 4; std::string STAGE_SUFFIX;
            if (X_NAME == "stable") { X_NUM = 4; STAGE_SUFFIX = ""; }
            else if (X_NAME == "rc") { X_NUM = 3; STAGE_SUFFIX = "-rc"; }
            else if (X_NAME == "beta") { X_NUM = 2; STAGE_SUFFIX = "-beta"; }
            else if (X_NAME == "alpha") { X_NUM = 1; STAGE_SUFFIX = "-alpha"; }
            else if (X_NAME == "pre-alpha") { X_NUM = 0; STAGE_SUFFIX = "-pre-alpha"; }

            try { if (std::stoi(ITERATION) > 0 && X_NAME != "stable") STAGE_SUFFIX += "." + ITERATION; } catch(...) {}
            std::string VERSION_NUMERIC = std::string("v") + W + "." + std::to_string(X_NUM) + "." + Y + "." + Z;
            std::string COMBINED_VERSION = VERSION_NUMERIC + STAGE_SUFFIX;
            if (!COMBINED_VERSION.empty()) return COMBINED_VERSION;
        }

        return {"version_tag_unreachable"};
    };

    const std::string systemSalt = computeReleaseCombinedVersion(); // now matches release.yml combined_version when available
    std::vector<unsigned char> salt(crypto_pwhash_SALTBYTES);
    crypto_generichash(salt.data(), salt.size(),
                      reinterpret_cast<const unsigned char*>(systemSalt.data()),
                      systemSalt.size(), nullptr, 0);

    // Derive master key
    if (crypto_pwhash(masterKey.data(), masterKeyLen,
                      systemSalt.data(), systemSalt.size(),
                      salt.data(),
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        return "ERROR: Master key derivation failed";
    }

    if (operation == "STORE") {
        // Encrypt and store data
        std::vector<unsigned char> nonce(crypto_secretbox_NONCEBYTES);
        randombytes_buf(nonce.data(), nonce.size());

        std::vector<unsigned char> ciphertext(data.size() + crypto_secretbox_MACBYTES);
        crypto_secretbox_easy(ciphertext.data(),
                             reinterpret_cast<const unsigned char*>(data.data()),
                             data.size(), nonce.data(), masterKey.data());

        // Store: identifier || nonce || ciphertext
        std::ofstream vault(vaultPath, std::ios::binary | std::ios::app);
        if (!vault.is_open()) {
            sodium_memzero(masterKey.data(), masterKey.size());
            return "ERROR: Cannot open vault file";
        }

        // Write entry: [identifier_len][identifier][nonce][ciphertext_len][ciphertext]
        const uint32_t idLen = identifier.size();
        const uint32_t ctLen = ciphertext.size();
        vault.write(reinterpret_cast<const char*>(&idLen), sizeof(idLen));
        vault.write(identifier.data(), idLen);
        vault.write(reinterpret_cast<const char*>(nonce.data()), nonce.size());
        vault.write(reinterpret_cast<const char*>(&ctLen), sizeof(ctLen));
        vault.write(reinterpret_cast<const char*>(ciphertext.data()), ctLen);
        vault.close();

        sodium_memzero(masterKey.data(), masterKey.size());
        sodium_memzero(ciphertext.data(), ciphertext.size());
        return "SUCCESS: Data stored securely";
    }

    if (operation == "RETRIEVE") {
        std::ifstream vault(vaultPath, std::ios::binary);
        if (!vault.is_open()) {
            sodium_memzero(masterKey.data(), masterKey.size());
            return "ERROR: Vault not found";
        }

        // Search for identifier
        while (vault.good()) {
            uint32_t idLen = 0;
            vault.read(reinterpret_cast<char*>(&idLen), sizeof(idLen));
            if (vault.eof()) break;

            std::string storedId(idLen, '\0');
            vault.read(&storedId[0], idLen);

            std::vector<unsigned char> nonce(crypto_secretbox_NONCEBYTES);
            vault.read(reinterpret_cast<char*>(nonce.data()), nonce.size());

            uint32_t ctLen = 0;
            vault.read(reinterpret_cast<char*>(&ctLen), sizeof(ctLen));

            std::vector<unsigned char> ciphertext(ctLen);
            vault.read(reinterpret_cast<char*>(ciphertext.data()), ctLen);

            if (storedId == identifier) {
                // Decrypt the data
                std::vector<unsigned char> plaintext(ciphertext.size() - crypto_secretbox_MACBYTES);
                if (crypto_secretbox_open_easy(plaintext.data(), ciphertext.data(), ciphertext.size(),
                                              nonce.data(), masterKey.data()) != 0) {
                    sodium_memzero(masterKey.data(), masterKey.size());
                    return "ERROR: Decryption failed - data corrupted";
                }

                std::string result(reinterpret_cast<char*>(plaintext.data()), plaintext.size());
                sodium_memzero(masterKey.data(), masterKey.size());
                sodium_memzero(plaintext.data(), plaintext.size());
                return result;
            }
        }

        sodium_memzero(masterKey.data(), masterKey.size());
        return "ERROR: Identifier not found";
    }

    sodium_memzero(masterKey.data(), masterKey.size());
    return "ERROR: Invalid operation (use STORE or RETRIEVE)";
}
