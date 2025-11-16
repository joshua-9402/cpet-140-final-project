/*
 * CpET 140 Final Project — Authentication module
 * Payroll and Monitoring System - Authentication module
 *
 * Contributors: Joshua Literal
 *
 * Purpose:
 * - Manages authentication between the desktop application and the mobile application.
 * - Handles user credentials and verification codes.
 *
 * Boundaries:
 * - No UI rendering and no DB, or I/O operations.
 * - Persistence or queries must go through the db adapter (db.h) or other handler / service modules.
 *
 * Notes:
 * - Single-threaded by default; callers must synchronize if accessed from multiple threads or workers.
 * - Ensure secure handling of credentials; avoid logging sensitive information.
 * - Use standard encryption/hashing for passwords where applicable.
 *
 *
 * USER AND PW:
 * - username: test_user
 * - password: test_pass
 *
 * - DEVICE CODE GENERATION:
 *      Handled by the mobile application.
 */

#include "auth.h"
#include <string>
#include <sodium.h>


/*
 * This function is responsible for the connection to and from the mobile application
 */
std::string auth::authGateway(const std::string& username, const std::string& password, const std::string& deviceCode) {
    return username + "_" + password + "_" + deviceCode;
}


bool auth::checkSodium() {
    if (sodium_init() < 0) { return false; }
    return true;
}

// Hash password using libsodium's crypto_pwhash_str (argon2id)
std::string auth::hashPassword(const std::string &password) {
    if (!auth::checkSodium()) return "";

    // Recommended output buffer size from libsodium
    constexpr size_t out_len = crypto_pwhash_STRBYTES;
    std::string out(out_len, '\0');

    if (0 != crypto_pwhash_str(
            &out[0],
            password.c_str(),
            password.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE)) {
        return std::string(); // failure
    }
    // The returned string may contain a terminating '\0' inside; resize to actual C-string length
    out.resize(std::strlen(out.c_str()));
    return out;
}

// Verify password against encoded hash
bool auth::verifyPassword(const std::string &encodedHash, const std::string &password) {
    if (!auth::checkSodium()) return false;

    if (0 == crypto_pwhash_str_verify(encodedHash.c_str(), password.c_str(), password.size())) {
        return true; // match
    }
    return false;
}
