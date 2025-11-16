#ifndef CPET_140_FINAL_PROJECT_APP_CONFIG_H
#define CPET_140_FINAL_PROJECT_APP_CONFIG_H

#include <string>

// Global authentication flag (defined in app_config.cpp)
extern bool g_auth;

// Application configuration variables (declared once, defined in app_config.cpp)
class appConfig {
    public:
        static std::string g_appTitle;            // Main App Title
        static std::string g_loginTitle;          // Login Window Title
        static std::string g_errorTitle;          // Error Window Title

        static std::string g_fontName;            // Default font path (relative to assets)
        static std::string g_dbNamePayroll;       // Database file name for payroll
        static std::string g_dbNameTracker;       // Database file name for tracker
        static std::string g_txtNameUser;         // User data file name

        static int g_defaultWidth;                // Window width in pixels
        static int g_defaultHeight;               // Window height in pixels

        static int g_loginWidth;                  // Login window width in pixels
        static int g_loginHeight;                 // Login window height in pixels

        static int g_errorWidth;                  // Error window width in pixels
        static int g_errorHeight;                 // Error window height in pixels
};

#endif // CPET_140_FINAL_PROJECT_APP_CONFIG_H
