#include "app_config.h"

// Global authentication flag (declared extern in app_config.h)
bool g_auth = false;

// Definitions of application configuration variables (match declarations in app_config.h)
#include <string>

std::string appConfig::g_appTitle      = "APP TITLE";
std::string appConfig::g_loginTitle    = "LOGIN";
std::string appConfig::g_errorTitle    = "ERROR";

std::string appConfig::g_fontName      = "fonts/OpenSans-Regular.ttf";
std::string appConfig::g_dbNamePayroll = "payroll.db";
std::string appConfig::g_dbNameTracker = "tracker.db";
std::string appConfig::g_txtNameUser   = "user.txt";

int appConfig::g_defaultWidth  = 1600;
int appConfig::g_defaultHeight = 900;

int appConfig::g_loginWidth  = 500;
int appConfig::g_loginHeight = 415;

int appConfig::g_errorWidth  = 300;
int appConfig::g_errorHeight = 100;
