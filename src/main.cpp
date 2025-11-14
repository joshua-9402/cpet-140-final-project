/*
 * CpET 140 Final Project — Application entry point
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
 * - Configures title, font, and window size, then calls constructUI().
 */


#include <string>
#include "ui/ui.h"
#include "config/app_config.h"

int main() {
    // Main application loop: show login/main UI based on authentication status
    while (true) {
        const config appConfig;
        if (!g_auth) {
            ui::constructUI(appConfig.g_loginTitle, appConfig.g_fontName, appConfig.g_smallWidth, appConfig.g_smallHeight, "auth"); // Show authentication UI
            if (!g_auth) {break;} // If user didn't log in (closed the window), exit the application
        }
        ui::constructUI(appConfig.g_appTitle, appConfig.g_fontName, appConfig.g_defaultWidth, appConfig.g_defaultHeight, "main"); // Show main UI
    }

    return 0;
}
