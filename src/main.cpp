/*
 * CpET 140 Final Project — Application entry point
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Starts the application, do any necessary checks, constructs the UI and finally run the application.
 * - Uses configuration variables for title, font, and window size.
 *
 * Boundaries
 * - Entry point only; no business logic or persistence here.
 *
 * Notes
 * - Check for any dependencies or required files/directories before starting.
 * - Loop to show login UI if not authenticated, then main UI if authenticated.
 * - Configures title, font, and window size, then calls constructUI().
 */


#include <string>
#include "ui/ui.h"
#include "config/app_config.h"
#include "handler/auth.h"
#include "handler/system.h"
#include "handler/db.h"


void systemCheck() {
    // System check: Ensure libsodium is initialized properly
    if (!auth::checkSodium()) {
        ui::g_failedMessage = "Error: libsodium initialization failed.";
        ui::constructUI(appConfig::g_errorTitle, appConfig::g_fontName, appConfig::g_errorWidth, appConfig::g_defaultHeight, "failed");
    }

    // Create necessary directories and files
    if (!system::searchDirectory("data")) {system::createDirectory("data");}
    if (!system::searchDirectory("projects")) {system::createDirectory("projects");}
    if (!db::searchFileText("data/users.txt")) {db::createFileText("data/users.txt");}
    if (db::searchDatabase(appConfig::g_dbNamePayroll) == false) {db::createDatabase(appConfig::g_dbNamePayroll);}
    if (db::searchDatabase(appConfig::g_dbNameTracker) == false) {db::createDatabase(appConfig::g_dbNameTracker);}
}

int main() {
    // Perform system checks before starting the application
    systemCheck();

    // Main application loop
    while (true) {
        if (!g_auth) {
            ui::constructUI(appConfig::g_loginTitle, appConfig::g_fontName, appConfig::g_loginWidth, appConfig::g_loginHeight, "auth"); // Show authentication UI
            if (!g_auth) {break;} // If user didn't log in (closed the window), exit the application
        }
        ui::constructUI(appConfig::g_appTitle, appConfig::g_fontName, appConfig::g_defaultWidth, appConfig::g_defaultHeight, "main"); // Show main UI
    }

    return 0;
}
