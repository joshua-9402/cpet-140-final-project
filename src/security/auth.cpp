#include "auth.h"
#include <string>
#include "../ui/ui.h"
#include "../main.cpp"


/*
 * This function is responsible for the connection to and from the mobile application
 *
 * For more information, please refer to the mobile application's repository
 */
std::string authGateway(const std::string& username, const std::string& password, const std::string& verificationCode) {
    if (verificationCode.empty()) {
        ui::g_failedMessage = "error: invalid verification code";
        ui::constructUI(g_appTitle, g_fontName, g_windowWidth, g_windowHeight, "failedUI");
    }
    return username + "_" + password + "_" + verificationCode;
}