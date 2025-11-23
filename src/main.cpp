/*
 * CpET 140 Final Project — Application entry point
 * StructuraCost - Main application entry point
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
#include "config/config.h"
#include "handler/system.h"
#include "handler/db.h"
#include "security/cryptography.h"


void systemCheck() {
    // System check: Ensure libsodium is initialized properly
    if (!cryptography::checkSodium()) {
        ui::g_failedMessage = "Error: libsodium initialization failed.";
        ui::constructUI(appConfig::g_errorTitle, appConfig::g_fontName, appConfig::g_errorWidth, appConfig::g_defaultHeight, "failed");
    }

    // Database check: Ensure SQLite is available
    if (!db::isSQLiteAvailable()) {
        ui::g_failedMessage = "Error: sqlite initialization failed.";
        ui::constructUI(appConfig::g_errorTitle, appConfig::g_fontName, appConfig::g_errorWidth, appConfig::g_defaultHeight, "failed");
    }

    // Create necessary directories and database files
    if (!system::searchDirectory("logs")) {system::createDirectory("logs");}
    if (!system::searchDirectory("backup")) {system::createDirectory("backup");}
    if (!system::searchDirectory(appConfig::g_dataDirectory)) {system::createDirectory(appConfig::g_dataDirectory);}
    if (!system::searchDirectory(appConfig::g_dataDirectory + appConfig::g_projectDirectory)) {system::createDirectory(appConfig::g_dataDirectory + appConfig::g_projectDirectory);}

    if (!system::searchFile(appConfig::g_dataDirectory + appConfig::g_dbNamePayroll)) {db::createDatabase(appConfig::g_dataDirectory + appConfig::g_dbNamePayroll);}
    if (!system::searchFile(appConfig::g_dataDirectory + appConfig::g_dbNameTracker)) {db::createDatabase(appConfig::g_dataDirectory + appConfig::g_dbNameTracker);}
}


void authenticationCheck() {

}

int main() {
    // Perform checks before starting the application
    systemCheck();
    authenticationCheck();

    // Main application loop
    while (true) {
        if (!appConfig::g_auth) {
            ui::constructUI(appConfig::g_loginTitle, appConfig::g_fontName, appConfig::g_loginWidth, appConfig::g_loginHeight, "auth"); // Show authentication UI
            if (!appConfig::g_auth) {break;} // If user didn't log in (closed the window), exit the application
        }
        ui::constructUI(appConfig::g_appTitle, appConfig::g_fontName, appConfig::g_defaultWidth, appConfig::g_defaultHeight, "main"); // Show main UI
    }

    return 0;
}
