/*
 * CpET 140 Final Project — Application entry point
 *
 * Contributors: Joshua Literal
 *
 * Purpose
 * - Starts the application and constructs the UI runner.
 *
 * Boundaries
 * - Entry point only; no business logic or persistence here.
 *
 * Notes
 * - Configures title, font, and window size, then calls constructUI().
 */

#include <string>
#include "lib/UI.h"

const std::string appTitle = "Main Window";
const std::string fontName = "fonts/OpenSans-Regular.ttf";
constexpr int windowWidth = 1600;
constexpr int windowHeight = 900;

int main() {
    constructUI(appTitle, fontName,windowWidth, windowHeight, "main");

    return 0;
}
