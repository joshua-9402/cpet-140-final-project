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
#include <thread>
#include <atomic>

#include "ui/ui.h"
#include "config/config.h"
#include "handler/system.h"
#include "handler/db.h"
#include "security/cryptography.h"


void systemCheck() {
    // System check: Ensure libsodium is initialized properly
    if (!cryptography::checkSodium()) {
        system::logMessage(system::messageClassification::ERROR, "libsodium initialization failed.\n");
        ui::g_failedMessage = "Error: libsodium initialization failed.";
        ui::constructUI(appConfig::g_errorTitle, appConfig::g_fontName, appConfig::g_errorWidth, appConfig::g_defaultHeight, "failed");
    }
    system::logMessage(system::messageClassification::INFO, "libsodium initialized successfully.\n");

    // Database check: Ensure SQLite is available
    if (!db::isSQLiteAvailable()) {
        system::logMessage(system::messageClassification::ERROR, "sqlite initialization failed.\n");
        ui::g_failedMessage = "Error: sqlite initialization failed.";
        ui::constructUI(appConfig::g_errorTitle, appConfig::g_fontName, appConfig::g_errorWidth, appConfig::g_defaultHeight, "failed");
    }
    system::logMessage(system::messageClassification::INFO, "sqlite initialized successfully.\n");

    // Create the necessary directories and database files
    if (!system::searchDirectory("logs")) {system::createDirectory("logs");}
    if (!system::searchDirectory("backup")) {system::createDirectory("backup");}
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

static std::atomic<bool> s_runBackground{true};

void runUIFlow() {
    // Show login if not authenticated
    if (!appConfig::g_auth) {
        ui::constructUI(appConfig::g_loginTitle, appConfig::g_fontName, appConfig::g_loginWidth, appConfig::g_loginHeight, "auth");
        // When the Run returns, check if authentication was set. If not, user chose to exit — stop the app.
        if (!appConfig::g_auth) {
            s_runBackground.store(false, std::memory_order_relaxed);
            system::logMessage(system::messageClassification::INFO, "Application exiting.\n");
            return;
        }
    }

    // Store auth state before showing main UI
    const bool wasAuthenticated = appConfig::g_auth;

    // Show main UI
    ui::constructUI(appConfig::g_appTitle, appConfig::g_fontName, appConfig::g_defaultWidth, appConfig::g_defaultHeight, "main");

    // After main UI closes, check if we should restart (logout was pressed)
    if (wasAuthenticated && !appConfig::g_auth) {
        // User logged out - restart the UI flow
        runUIFlow();
    }
}

int main() {
    // Capture terminal output (stdout/stderr) into the system logger
    system::setCaptureStdStreams(true);

    // Perform checks before starting the application
    system::logMessage(system::messageClassification::INFO, "Application Self Pre-Check Initiated.\n");
    systemCheck();
    system::logMessage(system::messageClassification::INFO, "Application Self Pre-Check Completed Successfully.\n");
    system::logMessage(system::messageClassification::INFO, "Main Application Starting.\n");

    // Background thread to monitor and rearrange employee IDs and project IDs periodically.
    std::thread([] {
        while (s_runBackground.load(std::memory_order_relaxed)) {
            // Avoid touching databases while not authenticated (maybe encrypted)
            if (appConfig::g_auth) {
                if (db::checkEmployeeChanges()) {
                    db::rearrangeEmployeeIDs();
                }
                if (db::checkProjectChanges()) {
                    db::rearrangeProjectIDs();
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();

    // Start the UI flow
    runUIFlow();

    // Perform cleanup after UI flow exits
    system::appShutdown();
    return 0;
}