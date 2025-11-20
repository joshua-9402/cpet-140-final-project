Vault function — documentation
===============================

Purpose
-------
The `cryptography::vault` function provides a small secure "vault" utility for storing and retrieving short pieces of sensitive data (secrets) inside a single append-only vault file on disk.

It is intended for simple use-cases inside the project where storing small secret blobs (API keys, encrypted config values, etc.) is needed without pulling in a full secret management service.

API
---
C++ signature (from `cryptography.h` / `cryptography.cpp`):

- std::string cryptography::vault(const std::string& operation,
                                   const std::string& identifier,
                                   const std::string& data);

Parameters:
- operation: either "STORE" or "RETRIEVE" (case-sensitive). Any other value returns an error string.
- identifier: a short identifier (string) used to look up the secret later. Treated as opaque bytes when stored.
- data: For `STORE`, the plaintext data to store (binary-safe via std::string). For `RETRIEVE`, this argument is ignored.

Return value:
- On success, `STORE` returns a string starting with `"SUCCESS"` and `RETRIEVE` returns the plaintext secret.
- On error, the function returns a string that begins with `"ERROR:"` describing the problem.

What the function does (high level)
----------------------------------
- Initializes libsodium (if not already initialized).
- Locates the vault file path depending on platform:
  - Windows: `%APPDATA%\StructuraCost\.vault`
  - macOS: `~/Library/Application Support/StructuraCost/.vault`
  - Linux/Unix: `~/.config/StructuraCost/.vault`
- Derives a master key using a small, deterministic derivation based on a hard-coded `systemSalt` and `crypto_pwhash`.
  - NOTE: Current implementation uses `systemSalt = "StructuraCost_v1.0"`. For production, replace with a machine-unique value.
- Depending on the operation:
  - STORE:
    - Generates a random nonce.
    - Encrypts the provided data using `crypto_secretbox_easy` with the derived master key.
    - Appends an entry to the vault file in the following binary format:
      [uint32_t idLen][identifier bytes][nonce][uint32_t ciphertext_len][ciphertext bytes]
  - RETRIEVE:
    - Scans the vault file sequentially, reading entries as above and comparing identifiers.
    - When a matching identifier is found, decrypts the ciphertext with `crypto_secretbox_open_easy` using the derived master key and returns the plaintext.

On-disk format details
----------------------
Each stored entry appended to the vault file uses the following layout (all multi-byte integer fields are stored in native host byte-order as 32-bit unsigned integers):

- uint32_t idLen          // number of bytes of the identifier
- idLen bytes             // identifier string (not NUL-terminated)
- nonce (crypto_secretbox_NONCEBYTES bytes)
- uint32_t ctLen          // ciphertext length in bytes (includes MAC)
- ctLen bytes             // ciphertext

Security model & limitations
----------------------------
- The function uses libsodium's `crypto_secretbox` (XSalsa20-Poly1305) for authenticated encryption of stored data.
- The vault uses a master key derived via `crypto_pwhash(masterKey, ..., systemSalt, ...)` where `systemSalt` is currently a static value derived from code. This is:
  - Acceptable for local, low-risk scenarios but NOT recommended for high-security needs.
  - In production you should replace `systemSalt` with a machine-unique, persistent per-install salt (e.g., TPM/secure-hardware value, OS-provided machine id, or a user-supplied secret).
- The vault file is append-only; `RETRIEVE` scans sequentially and returns the first matching identifier. There is no index, and no deletion support currently.
- The implementation stores binary lengths using native endianness. If you move the file between different architectures with different endianness, lengths may be interpreted incorrectly.
- The code uses `sodium_memzero` to clear derived secrets when finished.

Error cases
-----------
The function returns human-readable error strings for common failure modes, for example:
- `"ERROR: libsodium initialization failed"` — sodium couldn't initialize.
- `"ERROR: Cannot open vault file"` — vault file not available for reading/writing.
- `"ERROR: Decryption failed - data corrupted"` — ciphertext authentication failed; either the file is corrupted or the master key is wrong.
- `"ERROR: Identifier not found"` — when retrieving and no matching identifier exists.

Usage examples
--------------
Store a secret:

```cpp
#include "security/cryptography.h"

std::string id = "my_api_key";
std::string secret = "S3cr3t-Value-Here";
std::string r = cryptography::vault("STORE", id, secret);
if (r.rfind("SUCCESS", 0) == 0) {
    // stored
} else {
    // handle r which starts with "ERROR:"
}
```

Retrieve the secret later:

```cpp
std::string id = "my_api_key";
std::string result = cryptography::vault("RETRIEVE", id, "");
if (result.rfind("ERROR:", 0) == 0) {
    // handle error
} else {
    std::string secret = result; // plaintext
}
```

Testing locally
---------------
- Run a small program that calls `STORE` and then `RETRIEVE` for the same identifier and checks that the retrieved value equals the original.
- Ensure file permission and placement are correct for your platform (the vault file is created in your user-level config directory).

Recommended improvements
------------------------
- Replace the current `systemSalt` with a machine-unique value (or prompt for a passphrase) to ensure master key uniqueness per machine.
- Use a better metadata/index: store a compact lookup index in a separate file to avoid scanning large vaults.
- Add support for secure deletion / rotation of entries.
- Use fixed byte-order for on-disk integers (e.g., little-endian) to enable cross-platform portability.
- Consider using libsodium's higher-level sealed boxes or an OS-backed key store (Keychain on macOS, Windows DPAPI/CNG, Linux secret services) for stronger key protection.

Troubleshooting
---------------
- If `RETRIEVE` returns `ERROR: Identifier not found`, double-check the identifier string and ensure it was stored.
- If you see `ERROR: Decryption failed - data corrupted`, the vault may be corrupted or the master key derivation has changed; check for changes to `systemSalt` or platform differences.

License / attribution
---------------------
This implementation relies on libsodium (https://libsodium.org/) for crypto primitives. Keep the libsodium license and attribution in your project NOTICE if you redistribute.

Contact / follow-up
-------------------
If you'd like, I can:
- Add a small unit test `test/vault_test.cpp` that validates `STORE`/`RETRIEVE` behavior, or
- Implement key rotation and secure deletion support as a follow-up feature.
