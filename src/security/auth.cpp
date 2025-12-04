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
#include "../ui/ui.h"


bool auth::testAuth(const std::string& username, const std::string& password) {
    if ( username == "test" && password == "test") {
        ui::g_userName = "INTERNAL TEST";
        ui::g_position = "INTERNAL TEST";
        return true;
    }
    return false;
}


bool auth::testDeployAuth(const std::string& username, const std::string& password) {
    if ( username == "name" && password == "pass") {
        ui::g_userName = "TESTDRIVE_POSITION";
        ui::g_position = "TESTDRIVE_POSITION";
        return true;
    }
    return false;
}


bool auth::basicAuth(const std::string& username, const std::string& password) {
    if (username == "user" && password == "pass") {
        ui::g_userName = "STANDARD USER";
        ui::g_position = "STANDARD USER";
        return true;
    }
    return false;
}


bool auth::adminAuth(const std::string& username, const std::string& password) {
    if (username == "admin" && password == "admin") return true;
    return false;
}


