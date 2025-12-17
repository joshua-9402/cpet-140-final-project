/*
 * CpET 140 Final Project — Application Configuration header
 * StructuraCost - Config - Application Configuration
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Define global configuration variables for the application
 * - Centralize application settings and constants
 *
 * Boundaries
 * - Provides read-only configuration data to all modules
 *
 * Notes
 * - All variables are declared static and defined in config.cpp
 * - Configuration includes paths, window dimensions, and application metadata
 */

#ifndef CPET_140_FINAL_PROJECT_APP_CONFIG_H
#define CPET_140_FINAL_PROJECT_APP_CONFIG_H

#include <string>

// Application configuration variables (declared once, defined in app_config.cpp)
class appConfig {
    public:
    // Global authentication flag (defined in app_config.cpp)
        static bool g_auth;
        static bool g_testMode;

        static std::string g_appTitle;            // Main App Title
        static std::string g_loginTitle;          // Login Window Title
        static std::string g_errorTitle;          // Error Window Title

        static std::string g_fontName;            // Default font path (relative to assets)
        static std::string g_assetsDirectory;     // Assets directory path
        static std::string g_dataDirectory;       // Data directory path
        static std::string g_projectDirectory;    // Projects directory path
        static std::string g_payrollDirectory;    // Payroll database directory path
        static std::string g_projectExpenseDirectory; // Project expenses directory path
        static std::string g_payrollAttendanceDirectory; // Payroll attendance directory path

        static std::string g_dbNamePayroll;       // Database file name for payroll
        static std::string g_dbNameProject;       // Database file name for tracker

        static int g_defaultWidth;                // Window width in pixels
        static int g_defaultHeight;               // Window height in pixels

        static int g_loginWidth;                  // Login window width in pixels
        static int g_loginHeight;                 // Login window height in pixels

        static int g_errorWidth;                  // Error window width in pixels
        static int g_errorHeight;                 // Error window height in pixels
};

#endif // CPET_140_FINAL_PROJECT_APP_CONFIG_H
