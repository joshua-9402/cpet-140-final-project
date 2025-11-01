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

#include <iostream>
#include <ostream>
#include <string>
#include "lib/UI.h"
#include "lib/db.h"

const std::string appTitle = "Main Window";
const std::string fontName = "fonts/OpenSans-Regular.ttf";
constexpr int windowWidth = 1600;
constexpr int windowHeight = 900;

int main() {
    // Print whether SQLite is available
    std::cout << std::boolalpha << db::isSqliteAvailable() << std::endl;

    // Attempt to create (or open) a database file named "app.db"
    std::string err;
    bool created = db::createDatabase("app.db", &err);
    std::cout << std::boolalpha << created << std::endl;
    if (!created && !err.empty()) {
        std::cout << "db::createDatabase error: " << err << std::endl;
    }

    constructUI(appTitle, fontName, windowWidth, windowHeight, "main");
    return 0;
}
