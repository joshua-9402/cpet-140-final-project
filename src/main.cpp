/*
 * CpET 140 Final Project — Application entry point
 *
 * Contributors: Joshua Literal
 *
 * Purpose
 * - Starts the application, do any necessary checks, constructs the UI and finally run the application.
 * - Handles auth, incoming, and outgoing data from / to the system
 *
 * Boundaries
 * - Entry point only; no business logic or persistence here.
 *
 * Notes
 * - Configures title, font, and window size, then calls constructUI().
 */


#include <string>
#include "ui/ui.h"


// Application configuration variables
const std::string g_appTitle = "Main Window";
const std::string g_fontName = "fonts/OpenSans-Regular.ttf";
constexpr int g_windowWidth = 1600;
constexpr int g_windowHeight = 900;


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


/*
 *
 */
std::string ioHandler(const std::string& io, const std::string& command) {
    return "val " +  io + " " + command;
}


int main() {
    ui::constructUI(g_appTitle, g_fontName, g_windowWidth, g_windowHeight, "main");
    return 0;
}
