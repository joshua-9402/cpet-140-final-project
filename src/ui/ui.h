/*
 * CpET 140 Final Project — User Interface module header
 * StructuraCost - UI - User Interface module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Define the main UI construction and rendering functions
 * - Manage user interface state and interactions
 *
 * Boundaries
 * - Interacts with all modules to present data and handle user input
 *
 * Notes
 * - Uses ImGui/HelloImGui for rendering
 * - Manages login, payroll, project monitoring, and summary views
 */

#ifndef CPET_140_FINAL_PROJECT_UI_H
#define CPET_140_FINAL_PROJECT_UI_H

#include <string>

class ui {
    public:
        // Global Variable/s
        static std::string g_failedMessage;
        static std::string g_userName;
        static std::string g_position;

        // Global Methods
        static void constructUI(const std::string &a_title, const std::string& a_fontLocation, int a_widthPx, int a_heightPx, const std::string& a_window);

};
#endif //CPET_140_FINAL_PROJECT_UI_H
