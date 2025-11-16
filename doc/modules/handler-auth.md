# Authentication module (auth.h / auth.cpp)

This document explains the `auth` module found in `src/handler/auth.h` and `src/handler/auth.cpp`.
It describes the public API, behavior, data shapes, error modes, security considerations, build requirements, and usage examples.

## Purpose

The `auth` module provides authentication and cryptographic helpers used by the application. Main responsibilities:

- Password hashing and verification (libsodium `crypto_pwhash` / Argon2id)
- Generating and managing symmetric keys for database encryption
- Encrypting/decrypting raw database content using libsodium secret-key authenticated encryption (XSalsa20-Poly1305)
- Small helper `authGateway` used by the (mobile/desktop) gateway logic (application specific)

The module deliberately focuses on cryptographic operations and avoids performing UI or persistent storage (DB I/O). Higher-level code should call into `auth` and handle persistence, user input, or UI prompts.

## Files

- `src/handler/auth.h` — public header with API declarations
- `src/handler/auth.cpp` — implementation using libsodium

There is related documentation and utilities in `doc/modules/handler-keyring.md` for key storage.

## Public API (summary)

In `auth.h` the following public static methods are declared on class `auth`:

- std::string auth::authGateway(const std::string& username, const std::string& password, const std::string& deviceCode)
  - Lightweight helper used for the gateway logic. Returns a combined string from the inputs (application-specific format).

- bool auth::checkSodium()
  - Initializes libsodium via `sodium_init()` and returns true on success, false on failure. Callers should check this before using other libsodium functionality.

- std::string auth::hashPassword(const std::string &password)
  - Creates a secure encoded password hash using libsodium's `crypto_pwhash_str` (Argon2id). The returned string encodes parameters and salt and can be stored in the database.
  - Returns empty string on failure.

- bool auth::verifyPassword(const std::string &encodedHash, const std::string &password)
  - Verifies a password against an encoded hash using `crypto_pwhash_str_verify`.
  - Returns true when the password matches, false otherwise.

- std::vector<unsigned char> auth::generateDatabaseKey()
  - Generates and returns a cryptographically secure random key suitable for `crypto_secretbox` (32 bytes: `crypto_secretbox_KEYBYTES`). Returns an empty vector on failure.
  - Note: in older headers this may appear as `generateKey()`.

- std::vector<unsigned char> auth::encryptDatabase(const std::vector<unsigned char>& plainData, const std::vector<unsigned char>& key)
  - Encrypts `plainData` using `crypto_secretbox_easy` (XSalsa20-Poly1305). The output format is: <nonce(24 bytes)> + <ciphertext_with_MAC>
  - Requires `key.size() == crypto_secretbox_KEYBYTES`. Returns empty vector on error.

- std::vector<unsigned char> auth::decryptDatabase(const std::vector<unsigned char>& encryptedData, const std::vector<unsigned char>& key)
  - Decrypts `encryptedData` produced by `encryptDatabase`. Expects nonce prepended. Validates MAC and returns plaintext or empty vector on failure.

## Data shapes

- Password hash: `std::string` encoded by libsodium (`crypto_pwhash_str`) — contains algorithm parameters, salt, and hash (ASCII-safe string). Example value should be treated as opaque when stored.
- Database key: `std::vector<unsigned char>` with size `crypto_secretbox_KEYBYTES` (usually 32 bytes).
- Encrypted blob: `std::vector<unsigned char>` with size at least `crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES`.
  - Format: nonce (24 bytes) | ciphertext (plaintext size + MAC_bytes)

## Error modes and return conventions

- Functions return empty string / empty vector when a fatal or validation error occurs (e.g., libsodium not initialized, wrong key size, decryption failure).
- `verifyPassword` returns false on failure or mismatch.
- The module uses `auth::checkSodium()` to ensure libsodium is initialized. Callers can call `checkSodium()` first or rely on the helpers to call it internally (the current implementation checks internally).

## Security considerations

- Password hashing: `hashPassword` uses libsodium's recommended interactive limits (`crypto_pwhash_OPSLIMIT_INTERACTIVE` and `crypto_pwhash_MEMLIMIT_INTERACTIVE`). For higher security (server-side) consider using `MODERATE`/`SENSITIVE` limits and updating the strategy for hashing cost upgrades.

- Database encryption: Uses `crypto_secretbox_easy` (XSalsa20-Poly1305) which provides authenticated encryption. The implementation generates a random nonce per encryption and stores it in plaintext prepended to the ciphertext.
  - Always use a unique random nonce per encryption. The implementation uses libsodium `randombytes_buf` to generate nonces.
  - Key management is critical — the module only generates keys; it does not manage secure storage. Use the OS keychain (see `doc/modules/handler-keyring.md`) or a KMS for production.

- Never commit keys, password hashes, or unencrypted backups to source control.

## Build / runtime dependencies

- libsodium: required for password hashing, random bytes, and secretbox encryption. The project `CMakeLists.txt` contains logic to find libsodium via pkg-config, Homebrew paths (macOS), or a repo-local `dependencies/libsodium` folder.
  - On macOS: install via Homebrew `brew install libsodium` (expected at `/opt/homebrew` on Apple Silicon)
  - On Debian/Ubuntu: `sudo apt-get install libsodium-dev`
  - On Fedora: `sudo dnf install libsodium-devel`

- The code requires linking libsodium at link time (CMake handles that in the repo). Without libsodium the build will fail at configure time.

## Usage examples

### Hash a password and verify

```cpp
#include "handler/auth.h"
#include <iostream>

int main() {
    std::string password = "mysecret";

    // Create a hash (store this string in DB)
    std::string encoded = auth::hashPassword(password);
    if (encoded.empty()) {
        std::cerr << "Hashing failed" << std::endl;
        return 1;
    }

    // Later: verify
    if (auth::verifyPassword(encoded, password)) {
        std::cout << "Password OK" << std::endl;
    } else {
        std::cout << "Wrong password" << std::endl;
    }
}
```

### Encrypting a database file (short)

```cpp
// Generate or retrieve the key from secure storage
auto key = auth::generateDatabaseKey(); // store securely

// Read file bytes (implementation omitted) -> std::vector<unsigned char> dbBytes

auto encrypted = auth::encryptDatabase(dbBytes, key);
// write encrypted to payroll.db.enc
```

### Decrypting (short)

```cpp
// load key from secure storage
// read payroll.db.enc into encrypted vector
auto plain = auth::decryptDatabase(encrypted, key);
// write plain back to payroll.db
```

---

# Database Encryption Guide

The `auth` module provides functions to encrypt and decrypt database files using libsodium's XSalsa20-Poly1305 authenticated encryption.

## Functions

### generateDatabaseKey()
Generates a random 32-byte encryption key.

```cpp
std::vector<unsigned char> key = auth::generateDatabaseKey();
```

### encryptDatabase(plainData, key)
Encrypts database content using the provided key.

Parameters:
- plainData: Raw database content as `std::vector<unsigned char>`
- key: 32-byte encryption key (generated by `generateDatabaseKey()`)

Returns: Encrypted data with nonce prepended, or empty vector on failure

```cpp
std::vector<unsigned char> encrypted = auth::encryptDatabase(plainData, key);
```

### decryptDatabase(encryptedData, key)
Decrypts database content using the same key used for encryption.

Parameters:
- encryptedData: Encrypted data with prepended nonce
- key: 32-byte decryption key (must match encryption key)

Returns: Decrypted plaintext data, or empty vector on failure

```cpp
std::vector<unsigned char> decrypted = auth::decryptDatabase(encrypted, key);
```

## Usage Example

### Encrypting a Database File

```cpp
#include "handler/auth.h"
#include <fstream>
#include <vector>

// Read database file into memory
std::ifstream dbFile("payroll.db", std::ios::binary);
std::vector<unsigned char> plainData((std::istreambuf_iterator<char>(dbFile)),
                                      std::istreambuf_iterator<char>());
dbFile.close();

// Generate encryption key (store this securely!)
std::vector<unsigned char> key = auth::generateDatabaseKey();

// Encrypt the database
std::vector<unsigned char> encryptedData = auth::encryptDatabase(plainData, key);

if (!encryptedData.empty()) {
    // Write encrypted data to file
    std::ofstream outFile("payroll.db.enc", std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(encryptedData.data()), 
                  encryptedData.size());
    outFile.close();
    
    std::cout << "Database encrypted successfully!" << std::endl;
} else {
    std::cerr << "Encryption failed!" << std::endl;
}
```

### Decrypting a Database File

```cpp
#include "handler/auth.h"
#include <fstream>
#include <vector>

// Read encrypted file
std::ifstream encFile("payroll.db.enc", std::ios::binary);
std::vector<unsigned char> encryptedData((std::istreambuf_iterator<char>(encFile)),
                                          std::istream_iterator<char>());
encFile.close();

// Load the encryption key (from secure storage)
// Prefer using the keyring helper: see doc/modules/handler-keyring.md
std::vector<unsigned char> key = loadKeyFromSecureStorage();

// Decrypt the database
std::vector<unsigned char> decryptedData = auth::decryptDatabase(encryptedData, key);

if (!decryptedData.empty()) {
    // Write decrypted data back to database file
    std::ofstream dbFile("payroll.db", std::ios::binary);
    dbFile.write(reinterpret_cast<const char*>(decryptedData.data()), 
                 decryptedData.size());
    dbFile.close();
    
    std::cout << "Database decrypted successfully!" << std::endl;
} else {
    std::cerr << "Decryption failed! Wrong key or corrupted data." << std::endl;
}
```

### Storing the Encryption Key Securely

Important: The encryption key must be stored securely. Options:

1. Operating System Keychain (recommended)
   - See `doc/modules/handler-keyring.md` for cross-platform helpers

2. User Password-Derived Key:
   ```cpp
   // Derive key from user password using Argon2
   std::string userPassword = "user_password";
   unsigned char salt[crypto_pwhash_SALTBYTES];
   randombytes_buf(salt, sizeof salt);
   
   std::vector<unsigned char> key(crypto_secretbox_KEYBYTES);
   crypto_pwhash(key.data(), key.size(),
                 userPassword.c_str(), userPassword.size(),
                 salt,
                 crypto_pwhash_OPSLIMIT_INTERACTIVE,
                 crypto_pwhash_MEMLIMIT_INTERACTIVE,
                 crypto_pwhash_ALG_DEFAULT);
   ```

3. Hardware Security Module (HSM)

4. Environment Variable (development only)

## Security Notes

- Key Management: Store the encryption key separately and securely.
- Nonce: A random nonce is generated for each encryption operation (stored with ciphertext).
- Authentication: XSalsa20-Poly1305 provides authenticated encryption (detects tampering).
- Key Size: Always use 32-byte keys (`crypto_secretbox_KEYBYTES`).
- Initialization: libsodium is automatically initialized via `checkSodium()`.

## Error Handling

All functions return empty vectors on failure. Common failure scenarios:

- Encryption:
  - Invalid key size (not 32 bytes)
  - Empty input data
  - Libsodium not initialized

- Decryption:
  - Invalid key size
  - Data too small (corrupted)
  - Wrong decryption key
  - Tampered/corrupted ciphertext

## Performance Considerations

- Large Databases: For very large databases (>100MB), consider encrypting only sensitive tables or using SQLCipher.
- Memory: The entire database is loaded into memory during encryption/decryption.

## Integration with SQLite

For transparent database encryption with SQLite, you can:

1. Encrypt/Decrypt on Open/Close:
   - Decrypt when opening the database
   - Encrypt when closing the database

2. Use SQLCipher: A SQLite extension with built-in encryption (recommended for production)

Example workflow:
```cpp
// On database open
std::vector<unsigned char> encryptedDB = readFile("payroll.db.enc");
std::vector<unsigned char> decryptedDB = auth::decryptDatabase(encryptedDB, key);
writeFile("payroll.db", decryptedDB);
sqlite3_open("payroll.db", &db);

// ... use database ...

// On database close
sqlite3_close(db);
std::vector<unsigned char> plainDB = readFile("payroll.db");
std::vector<unsigned char> encrypted = auth::encryptDatabase(plainDB, key);
writeFile("payroll.db.enc", encrypted);
std::remove("payroll.db"); // Delete unencrypted file
```

## Testing

Test your encryption/decryption implementation:

```cpp
// Test round-trip encryption/decryption
std::vector<unsigned char> original = {1, 2, 3, 4, 5};
auto key = auth::generateDatabaseKey();
auto encrypted = auth::encryptDatabase(original, key);
auto decrypted = auth::decryptDatabase(encrypted, key);

assert(original == decrypted);
std::cout << "Encryption test passed!" << std::endl;
```
