# Authentication module (`security/auth.h` / `security/auth.cpp`)

## Purpose

Implements a small authentication helper used by the application for built-in demo and admin authentication flows. The module provides a few simple, hard-coded checks (used for development/test/demo flows) and integrates with the `cryptography` module for a salted key check in the admin path.

> Important: this module uses in-code test credentials and a simple salted comparison for admin authentication. It is not a complete or secure authentication system and should be replaced with a proper user database and password hashing strategy for production.

## Files

- `src/security/auth.h`
- `src/security/auth.cpp`

## Public API (summary)

- `static bool auth::testAuth(const std::string &username, const std::string &password)`
  - Returns `true` when credentials equal `test` / `test`.
  - Side-effects: sets `ui::g_userName = "INTERNAL TEST"` and `ui::g_position = "INTERNAL TEST"` on success.

- `static bool auth::testDeployAuth(const std::string &username, const std::string &password)`
  - Returns `true` when credentials equal `name` / `pass`.
  - Side-effects: sets `ui::g_userName = "TESTDRIVE_POSITION"` and `ui::g_position = "TESTDRIVE_POSITION"` on success.

- `static bool auth::adminAuth(const std::string &username, const std::string &password)`
  - Admin check: returns `true` when `username == "admin"` and `cryptography::saltKey(cryptography::hashKey(password)) == "76rtdychgnih"`.
  - Side-effects: sets `ui::g_userName = "ADMIN"` and `ui::g_position = "ADMIN"` on success.
  - Note: this uses `cryptography::hashKey` and `cryptography::saltKey` functions for a deterministic (but simplistic) check.

- `static bool auth::basicAuth(const std::string &username, const std::string &password)`
  - Returns `true` when credentials equal `basicUser` / `basicPass`.
  - No UI side-effects in the current implementation.

All functions return `false` when checks fail.

## Data shapes

- Inputs: `std::string` for username and password.
- Outputs: `bool` (success/failure). On success some functions also mutate `ui::` globals.

## Error modes and return conventions

- Boolean return only. No exceptions are thrown by these functions.
- Failure generally means credentials did not match expected hard-coded values.

## Security considerations

- THIS MODULE IS FOR DEMO/TEST PURPOSES ONLY:
  - Credentials are hard-coded in source — not suitable for real users.
  - The admin check uses `cryptography::hashKey` and `cryptography::saltKey` to compare a derived value to a constant; this is weak and brittle (and the hard-coded constant reveals intended password equivalence).
  - Do not rely on these functions for production authentication.

- Recommended production changes:
  - Replace with proper password storage using libsodium's `crypto_pwhash_str` (Argon2id) and `crypto_pwhash_str_verify`.
  - Store user records in a secured database and use per-user salts.
  - Use secure session management and avoid writing plain passwords to memory longer than necessary.

## Build / runtime dependencies

- Calls into `src/security/cryptography.*` for `hashKey` and `saltKey` functions.
- Mutates UI state via `src/ui/ui.h` (global variables `ui::g_userName`, `ui::g_position`).
- Requires libsodium indirectly if `cryptography` uses it.

## Usage examples

```cpp
#include "security/auth.h"

if (auth::testAuth("test", "test")) {
    // ui::g_userName and ui::g_position are updated by the function
}

if (auth::adminAuth("admin", "my-secret-password")) {
    // admin access granted (per the simplistic check in code)
}
```

## Troubleshooting & debugging tips

- If an expected login doesn't succeed:
  - Verify you are using the exact literal credentials as present in the source (e.g., `test/test`, `name/pass`, `basicUser/basicPass`).
  - For `adminAuth`, ensure the `cryptography::hashKey` and `cryptography::saltKey` functions are available and return expected values — a mismatch in those functions will cause admin auth to fail.
  - If UI globals are not updated after successful auth, ensure `ui::g_userName` and `ui::g_position` symbols are defined and linked (they are declared in `src/ui/ui.h` and defined in `src/ui/ui.cpp`). Linker errors or duplicate symbols can prevent expected behavior.

- If you see unresolved linker symbols (related to `cryptography` or `sodium`), verify libsodium is properly installed and the correct library (matching the target architecture) is linked. See `doc/LIBSODIUM_SETUP.md` for CI/runtime guidance.

---

## Repository sync note

Updated to match the real implementation in `src/security/auth.h` and `src/security/auth.cpp` (sync date: 2025-11-29). Replace these demo checks with a proper authentication backend before production use.
