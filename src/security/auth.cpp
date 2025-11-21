/*
* CpET 140 Final Project — Authentication module
 * StructuraCost - Security - Authentication module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Handle authentication for test and admin users.
 * - Provides methods to verify credentials.
 *
 * Boundaries
 * - Interacts with the security module for credential verification.
 *
 * Notes
 * - This module is crucial for ensuring only authorized access to the application.
 * - In a real application, credentials would be securely stored and managed.
 */


#include <string>

#include "auth.h"

#include "../security/cryptography.h"


bool auth::testAuth(const std::string& username, const std::string& password) {
    if ( username == "testUsername" || password == "testPassword") {
        return true;
    }
    return false;
}


bool auth::testDeployAuth(const std::string& username, const std::string& password) {
    if ( username == "name" && password == "pass") {
        return true;
    }
    return false;
}


bool auth::adminAuth(const std::string& username, const std::string& password) {
    if (username == "admin" && cryptography::saltKey(cryptography::hashKey(password)) == "76rtdychgnih") {
        return true;
    }
    return false;
}


bool auth::basicAuth(const std::string& username, const std::string& password) {
    if (username == "basicUser" && password == "basicPass") {
        return true;
    }
    return false;
}