#ifndef CPET_140_FINAL_PROJECT_APP_CONFIG_H
#define CPET_140_FINAL_PROJECT_APP_CONFIG_H

#include <string>

// Application configuration variables (declared once, defined in app_config.cpp)
extern const std::string g_appTitle;            // Main App Title
extern const std::string g_fontName;            // Default font path (relative to assets)
extern const std::string g_dbNamePayroll;       // Database file name for payroll
extern const std::string g_dbNameTracker;       // Database file name for tracker
extern const std::string g_txtNameUser;         // User data file name

extern const int g_defaultWidth;                // Window width in pixels
extern const int g_defaultHeight;               // Window height in pixels

extern const int g_smallWidth;                  // Small window width in pixels
extern const int g_smallHeight;                 // Small window height in pixels

#endif // CPET_140_FINAL_PROJECT_APP_CONFIG_H
