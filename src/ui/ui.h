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
#include "hello_imgui/hello_imgui.h"

class ui {
    public:
        // Global Variable/s
        static std::string g_failedMessage;
        static std::string g_userName;
        static std::string g_position;

        // Global Methods
        static void constructUI(const std::string &a_title, const std::string& a_fontLocation, int a_widthPx, int a_heightPx, const std::string& a_window);
        static void renderStatCard(const char* childId, const char* title, const ImVec4& titleColor, const std::string& valueText, const char* subtitle, const ImVec2& size);
        static std::string normalizeProjectId(const std::string& input);
        static std::string buildProjectReportOutFile(const std::string& projectID);
        static ImVec2 fullWidthButtonSize(float a_height);
        static void setButtonCenter(const char *text, const ImVec2 &size);
        static void setTextCenter(const char *text);
        static void setTextRight(const char *text);
        static void loadImage(const std::string &p_location, float p_locationXPx, float p_locationYPx, float p_imageSize);
};
#endif //CPET_140_FINAL_PROJECT_UI_H
