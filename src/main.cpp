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
    ui::constructUI(g_appTitle, g_fontName, g_defaultWidth, g_defaultHeight, "main");
    return 0;
}
