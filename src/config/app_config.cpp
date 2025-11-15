#include "app_config.h"

// Global authentication flag
bool g_auth = false;

// Definitions of application configuration constants
static constexpr std::string APP_TITLE = "APP TITLE";
static constexpr std::string LOGIN_TITLE = "LOGIN";

static const std::string FONT_NAME = "fonts/OpenSans-Regular.ttf";
static const std::string DB_NAME_PAYROLL = "payroll.db";
static const std::string DB_NAME_TRACKER = "tracker.db";
static const std::string TXT_NAME_USER = "user.txt";

static constexpr int DEFAULT_WIDTH = 1600;
static constexpr int DEFAULT_HEIGHT = 900;

static constexpr int SMALL_UI_WIDTH = 500;
static constexpr int SMALL_UI_HEIGHT = 415;

// Config class constructor - initializes all const members
config::config()
    : g_appTitle(APP_TITLE)
    , g_fontName(FONT_NAME)
    , g_dbNamePayroll(DB_NAME_PAYROLL)
    , g_dbNameTracker(DB_NAME_TRACKER)
    , g_txtNameUser(TXT_NAME_USER)
    , g_defaultWidth(DEFAULT_WIDTH)
    , g_defaultHeight(DEFAULT_HEIGHT)
    , g_smallWidth(SMALL_UI_WIDTH)
    , g_smallHeight(SMALL_UI_HEIGHT)
    , g_loginTitle(LOGIN_TITLE)
{
}
