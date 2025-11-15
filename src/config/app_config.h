#ifndef CPET_140_FINAL_PROJECT_APP_CONFIG_H
#define CPET_140_FINAL_PROJECT_APP_CONFIG_H

#include <string>

// Global authentication flag (defined in app_config.cpp)
extern bool g_auth;

// Application configuration variables (declared once, defined in app_config.cpp)
class config {
public:
    config(); // Constructor

    std::string g_appTitle;            // Main App Title
    std::string g_loginTitle;          // Login Window Title

    std::string g_fontName;            // Default font path (relative to assets)
    std::string g_dbNamePayroll;       // Database file name for payroll
    std::string g_dbNameTracker;       // Database file name for tracker
    std::string g_txtNameUser;         // User data file name

    int g_defaultWidth;                // Window width in pixels
    int g_defaultHeight;               // Window height in pixels

    int g_smallWidth;                  // Small window width in pixels
    int g_smallHeight;                 // Small window height in pixels
};

#endif // CPET_140_FINAL_PROJECT_APP_CONFIG_H
