/*
 * CpET 140 Final Project — Application entry point
 *
 * Contributors: Joshua Literal
 *
 * Purpose
 * - Starts the application, do any necessary checks, constructs the UI and finally run the application.
 *
 * Boundaries
 * - Entry point only; no business logic or persistence here.
 *
 * Notes
 * - Configures title, font, and window size, then calls constructUI().
 */

#include <ostream>
#include <string>
#include "lib/UI.h"

// Application configuration variables
const std::string g_appTitle = "Main Window";
const std::string g_fontName = "fonts/OpenSans-Regular.ttf";
constexpr int g_windowWidth = 1600;
constexpr int g_windowHeight = 900;


int main() {
    constructUI(g_appTitle, g_fontName, g_windowWidth, g_windowHeight, "main");
    return 0;
}
