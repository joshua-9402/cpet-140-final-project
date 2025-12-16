/*
 * CpET 140 Final Project — Authentication module header
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

#ifndef CPET_140_FINAL_PROJECT_AUTH_H
#define CPET_140_FINAL_PROJECT_AUTH_H

#include <string>


class auth {
    public:
        static bool testAuth(const std::string& username, const std::string& password);

        static bool testDeployAuth(const std::string &username, const std::string &password);

        static bool adminAuth(const std::string& username, const std::string& password);

        static bool mainAuth(const std::string &username, const std::string &password);
};


#endif //CPET_140_FINAL_PROJECT_AUTH_H