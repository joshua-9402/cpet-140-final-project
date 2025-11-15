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
 */

#include "auth.h"
#include <string>
#include "../ui/ui.h"
#include "../config/app_config.h"


const config appConfig; // Create an instance of the config


/*
 * This function is responsible for the connection to and from the mobile application
 *
 * For more information, please refer to the mobile application's repository
 *
 * The generation of device code is handled by the mobile application
 * deviceCode = username + password + device + appGeneratedString (based on time) + salting / hashing
 */
std::string authGateway(const std::string& username, const std::string& password, const std::string& deviceCode) {
    if (deviceCode.empty()) {
        ui::g_failedMessage = "error: invalid verification code";
        ui::constructUI(appConfig.g_appTitle, appConfig.g_fontName, appConfig.g_defaultWidth, appConfig.g_defaultHeight, "failedUI");
    }
    return username + "_" + password + "_" + deviceCode;
}