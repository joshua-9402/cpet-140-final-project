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
    // // System check: Ensure cryptography (optional) is initialized properly
    // if (!cryptography::checkSodium()) {
    //     system::logMessage(system::messageClassification::ERROR, "cryptography initialization failed.");
    //     ui::g_failedMessage = "Error: cryptography initialization failed.";
    //     ui::constructUI(appConfig::g_errorTitle, appConfig::g_fontName, appConfig::g_errorWidth, appConfig::g_defaultHeight, "failed");
    // }
    system::logMessage(system::messageClassification::INFO, "cryptography initialized (or skipped) successfully.");

    // Database check: Ensure SQLite is available
    if (!db::isSQLiteAvailable()) {
        system::logMessage(system::messageClassification::ERROR, "sqlite initialization failed.");
        ui::g_failedMessage = "Error: sqlite initialization failed.";
        ui::constructUI(appConfig::g_errorTitle, appConfig::g_fontName, appConfig::g_errorWidth, appConfig::g_defaultHeight, "failed");
    }
    system::logMessage(system::messageClassification::INFO, "sqlite initialized successfully.");
}


void directoryCheck() {
    // Create the necessary directories and database files
    if (!system::searchDirectory(appConfig::g_logsDirectory)) {system::createDirectory(appConfig::g_logsDirectory);}
    if (!system::searchDirectory(appConfig::g_backupDirectory)) {system::createDirectory(appConfig::g_backupDirectory);}
    if (!system::searchDirectory(appConfig::g_dataDirectory)) {system::createDirectory(appConfig::g_dataDirectory);}

    if (!system::searchDirectory(appConfig::g_dataDirectory + appConfig::g_payrollDirectory)) {system::createDirectory(appConfig::g_dataDirectory + appConfig::g_payrollDirectory);}
    if (!system::searchDirectory(appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_payrollAttendanceDirectory)) {
        system::createDirectory(appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_payrollAttendanceDirectory);
    }

    if (!system::searchDirectory(appConfig::g_dataDirectory + appConfig::g_projectDirectory)) {system::createDirectory(appConfig::g_dataDirectory + appConfig::g_projectDirectory);}
    if (!system::searchDirectory(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_projectExpenseDirectory)) {system::createDirectory(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_projectExpenseDirectory);}

    // Only create base DBs if neither plaintext nor encrypted files exist
    const std::string payrollDb = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    if (const std::string payrollEnc = payrollDb + ".enc"; !system::searchFile(payrollDb) && !system::searchFile(payrollEnc)) {
        db::createDatabase(payrollDb);
    }
    const std::string projectDb = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;
    if (const std::string projectEnc = projectDb + ".enc"; !system::searchFile(projectDb) && !system::searchFile(projectEnc)) {
        db::createDatabase(projectDb);
    }
}

int main() {
    // Perform checks before starting the application
    system::logMessage(system::messageClassification::INFO, "Application Self Pre-Check Initiated.");
    systemCheck();
    directoryCheck();
    system::logMessage(system::messageClassification::INFO, "Application Self Pre-Check Completed Successfully.");
    system::logMessage(system::messageClassification::INFO, "Main Application Starting.");

    // Main application loop: show login or main UI based on auth status
    while (true) {
        if (!appConfig::g_auth) {
            // Show login UI
            ui::constructUI(appConfig::g_loginTitle, appConfig::g_fontName, appConfig::g_loginWidth, appConfig::g_loginHeight, "auth");

            // If still not authenticated after login UI exits, user closed the window - exit application
            if (!appConfig::g_auth) {
                system::logMessage(system::messageClassification::INFO, "Application closed from login screen.");
                break;
            }
        } else {
            // Show main UI
            ui::constructUI(appConfig::g_appTitle, appConfig::g_fontName, appConfig::g_defaultWidth, appConfig::g_defaultHeight, "main");

            // If user logged out (g_auth became false), loop back to login
            if (!appConfig::g_auth) {
                system::logMessage(system::messageClassification::INFO, "Returning to login screen.");
                continue;
            }

            // If g_auth is still true, user closed the main window - exit application
            system::logMessage(system::messageClassification::INFO, "Application closed from main screen.");
            break;
        }
    }

    system::logMessage(system::messageClassification::INFO, "Application terminated successfully.");
    return 0;
}