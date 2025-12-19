# Cryptography module (`security/cryptography.h` / `security/cryptography.cpp`)

## Purpose

Provides cryptographic helpers used by the application: libsodium initialization check, hashing, random key generation, hex conversion, password/key salting, file encryption/decryption, and a small vault utility to store short secrets.

This module wraps libsodium primitives and provides application-level file formats and key derivation behaviors used elsewhere in the project.

## Files

- `src/security/cryptography.h`
- `src/security/cryptography.cpp`

## Public API (summary)

- `bool cryptography::checkSodium()`
  - Initializes libsodium via `sodium_init()` and returns `true` on success (sodium available and initialized).

- `std::string cryptography::hashKey(const std::string &key)`
  - Computes a fixed-length generichash (Blake2b) of the input `key` and returns the hex-encoded string. Returns empty string on failure or if libsodium is unavailable.

- `std::vector<unsigned char> cryptography::generateKey(size_t keyBits)`
  - Generates a cryptographically secure random key of size `keyBits` (bits). Requirements in the current implementation:
    - `keyBits` must be >= 32 and <= 8192 and divisible by 8.
    - Returns an empty vector on invalid input or failure.

- `std::string cryptography::toHex(const std::vector<unsigned char>& key)`
  - Returns a lowercase hex string representation of binary `key` bytes.

- `std::string cryptography::saltKey(const std::string &key)`
  - Generates a random salt (`crypto_pwhash_SALTBYTES`) and runs `crypto_pwhash` to derive a byte buffer of length equal to `key.size()` using `crypto_pwhash_OPSLIMIT_MODERATE` / `crypto_pwhash_MEMLIMIT_MODERATE`. Returns the hex of the derived bytes or empty string on failure.

- `bool cryptography::encryptFile(const std::string &filePath, const std::vector<unsigned char> &key)`
  - Encrypts the file at `filePath` and writes output to `filePath + ".enc"`.
  - Steps (implementation details):
    1. Rejects empty `key` vector.
    2. Generates a random salt (size `crypto_pwhash_SALTBYTES`) and derives a `required`-byte key (`crypto_aead_xchacha20poly1305_ietf_KEYBYTES`) using `crypto_pwhash(..., salt, OPSLIMIT_MODERATE, MEMLIMIT_MODERATE, ...)` with the provided `key` bytes as the password material.
    3. Reads the entire input file into memory.
    4. Generates a random nonce of size `crypto_aead_xchacha20poly1305_ietf_NPUBBYTES`.
    5. Encrypts with `crypto_aead_xchacha20poly1305_ietf_encrypt` producing ciphertext (including MAC).
    6. Writes to the output `.enc` file the concatenation: `salt || nonce || ciphertext`.
    7. Zeroes derived key material with `sodium_memzero` before returning.
  - Returns `true` on success, `false` on error (I/O, derivation failure, or encryption failure).

- `bool cryptography::decryptFile(const std::string &filePath, const std::vector<unsigned char> &key, std::string *errorMsg = nullptr)`
  - Decrypts a file previously written by `encryptFile` (expects format `salt || nonce || ciphertext`) and overwrites the input file path with decrypted plaintext (the implementation opens the same path for writing after successful decryption).
  - Steps:
    1. Rejects empty key and returns error.
    2. Reads the whole file; validates size >= salt + nonce + auth tag.
    3. Extracts salt, nonce, and ciphertext.
    4. Derives the same `required`-byte key via `crypto_pwhash` using the given salt and provided key material.
    5. Calls `crypto_aead_xchacha20poly1305_ietf_decrypt`; on success writes the plaintext back to `filePath` with truncation (replacing the .enc file with plaintext in place in current implementation).
    6. Zeroes derived key material.
  - Returns `true` on success; on failure it returns `false` and optionally sets `*errorMsg` with one of the human-readable status strings present in code (for example: "Empty key provided", "Cannot open encrypted file: ...", "File too small to be valid encrypted file", "Key derivation failed during decrypt", "Decryption failed: wrong key or corrupted file", "Cannot write decrypted file").

- `std::string cryptography::vault(const std::string &operation, const std::string &identifier, const std::string &data)`
  - Small append-only vault for storing and retrieving short secrets.
  - Supported `operation` values: "STORE" and "RETRIEVE" (case-sensitive in current implementation).
  - Behavior (STORE):
    1. Initializes libsodium (`checkSodium`).
    2. Locates platform-specific vault path:
       - Windows: `%APPDATA%/StructuraCost/.vault`
       - macOS: `~/Library/Application Support/StructuraCost/.vault`
    3. Computes a `systemSalt` using a helper that attempts (in order): `COMBINED_VERSION` env var, a build-folder copy of `.github/workflows/release.yml` under `cmake-build-debug/`, then `.github/workflows/release.yml` at repo root; if none found it returns `"version_tag_unreachable"`.
    4. Hashes `systemSalt` to create a salt buffer and derives a `crypto_secretbox_KEYBYTES` master key via `crypto_pwhash(..., OPSLIMIT_INTERACTIVE, MEMLIMIT_INTERACTIVE, ...)`.
    5. Encrypts `data` with `crypto_secretbox_easy` using a random nonce of `crypto_secretbox_NONCEBYTES` and appends an entry to the vault file: `[uint32_t idLen][identifier bytes][nonce][uint32_t ctLen][ciphertext bytes]`.
    6. Zeroes master key and ciphertext buffers before returning.
    7. Returns an ASCII status string: on success "SUCCESS: Data stored securely" or on error an "ERROR: ..." message.
  - Behavior (RETRIEVE):
    1. Opens the vault file and sequentially reads entries of the format above.
    2. When `storedId == identifier` is found, attempts `crypto_secretbox_open_easy` with the derived master key and returns plaintext string on success.
    3. If not found, returns `"ERROR: Identifier not found"`.
    4. On failure (vault not found, decryption failed) returns appropriate `ERROR:` messages.

## Data shapes

- Keys: `std::vector<unsigned char>` for binary keys; `std::string` for hex / textual representations.
- File formats:
  - Encrypted file: `salt (crypto_pwhash_SALTBYTES) || nonce (crypto_aead_xchacha20poly1305_ietf_NPUBBYTES) || ciphertext` (ciphertext includes MAC)
  - Vault entry: `uint32_t idLen | idLen bytes identifier | nonce (crypto_secretbox_NONCEBYTES) | uint32_t ctLen | ctLen bytes ciphertext`

## Error modes and return conventions

- Binary APIs return `bool` for success/failure. `decryptFile` optionally provides a human-readable error string pointer.
- `vault` returns string status messages that start with `SUCCESS:` or `ERROR:`.
- Functions return empty strings or empty vectors on fatal errors.

## Security considerations

- The module uses libsodium primitives and attempts to zero derived key material with `sodium_memzero` after use — this is good practice.
- Current `generateKey` enforces a minimum/maximum and byte-alignment check; callers must request a valid `keyBits` value.
- `saltKey`, `encryptFile`, and `decryptFile` rely on `crypto_pwhash` with moderate/interactive limits for key derivation — review the chosen limits for your threat model and hardware.
- `encryptFile`/`decryptFile` read whole files into memory; avoid using these functions for extremely large files or adapt to streaming encryption modes.
- `vault` uses a build/release-derived `systemSalt`; the implementation attempts to match CI release tags for deterministic master key derivation — if you change release workflows, update the vault salt selection logic accordingly.

## Usage examples

Hash a key and salt it:

```cpp
std::string raw = "my-secret";
std::string h = cryptography::hashKey(raw);
std::string salted = cryptography::saltKey(h);
```

Generate a 256-bit key (must be multiple of 8):

```cpp
auto key = cryptography::generateKey(256); // 32 bytes
if (key.empty()) {
    // handle invalid size or failure
}
```

Encrypt and decrypt a file:

```cpp
std::vector<unsigned char> key = cryptography::generateKey(256);
if (!cryptography::encryptFile("test.txt", key)) {
    // handle encryption failure
}

std::string err;
if (!cryptography::decryptFile("test.txt.enc", key, &err)) {
    // err contains human readable message
}
```

Store and retrieve a small secret in the vault:

```cpp
auto status = cryptography::vault("STORE", "api_key", "secret-value");
if (status.rfind("SUCCESS:", 0) == 0) {
    // stored
}

auto retrieved = cryptography::vault("RETRIEVE", "api_key", "");
if (retrieved.rfind("ERROR:", 0) == 0) {
    // handle error
} else {
    // plaintext secret in `retrieved`
}
```

## Troubleshooting & debugging tips

- If functions fail with linker errors related to `sodium` symbols (e.g., `__imp_crypto_aead_xchacha20poly1305_ietf_encrypt`), ensure libsodium is installed and linked for the target architecture and toolchain. On Windows ensure the MSVC toolset and libsodium build flavor (x86/x64/ARM64) match.

- If `generateKey` returns an empty vector, check `keyBits` validity (must be >=32, <=8192 and divisible by 8). The code currently enforces these limits.

- If `encryptFile` or `decryptFile` fails with "Key derivation failed" or "Decryption failed" messages, verify the `key` you supplied is the same bytes used during encryption (the code derives a new runtime key from the supplied `key` and a per-file random salt).

- `decryptFile` overwrites the provided encrypted file path with the decrypted content in the current implementation — be careful: keep backups before testing on important data.

- `vault` master key depends on a release/build-derived value. If you change release or CI workflow version tags, the master key derivation will change and previously stored vault entries will become unreadable. Consider using a machine-unique persistent secret for production.

---

## Repository sync note

This document was generated to reflect the exact implementation found in `src/security/cryptography.h` and `src/security/cryptography.cpp` (sync date: 2025-11-29). Update this doc if the implementation changes.

