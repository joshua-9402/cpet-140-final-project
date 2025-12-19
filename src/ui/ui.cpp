/*
 * CpET 140 Final Project — UI module
 * StructuraCost - UI - UI module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Immediate-mode UI built with HelloImGui/ImGui. Manages a registry of UIs, switching between them, font loading,
 *   and window setup.
 * - Provides the main two-column layout with navigation on the left and an active panel on the right.
 *
 * Boundaries
 * - Presentation only. No business logic or persistence here. Call payroll.cpp and monitor.cpp modules for operations.
 *
 * Notes
 * - UI switching is immediate (case-insensitive). App exit uses HelloImGui runner.
 * - This part is OFF LIMITS, especially to constructUI(), the global variables, and to the UI registry/map.
 *
 * jslbuilders@victoria
 */

#include <string>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "../../dependencies/sqlite/sqlite3.h"
#include "ui.h"

#include "hello_imgui/hello_imgui.h"
#include "../handler/db.h"
#include "../handler/print.h"
#include "../handler/system.h"
#include "../config/config.h"
#include "../security/cryptography.h"
#include "../security/auth.h"
#include "../core/monitor.h"

// stb_image is provided by the deps used in the project (hello_imgui). We include it here for icon loading.
// If the header isn't available on the include path, these functions are declared manually as fallback prototypes.
#if __has_include("stb_image.h")
#include "stb_image.h"
#else
extern "C" unsigned char* stbi_load(const char *filename, int *x, int *y, int *comp, int req_comp);
extern "C" void stbi_image_free(void *retval_from_stbi_load);
#endif

#if __has_include(<GLFW/glfw3.h>)
#include <GLFW/glfw3.h>
#define UI_HAVE_GLFW 1
#endif

#if __has_include(<SDL.h>)
#include <SDL.h>
#define UI_HAVE_SDL 1
#endif


// UI registry and UI management
static std::unordered_map<std::string, std::function<void()>> g_uiMap;
static std::function<void()> g_currentUI = nullptr;
static std::function<void()> g_rightUI = nullptr; // Right panel active UI (shown in the main two-column layout)
auto g_buttonSizePxSelector = ImVec2(270, 40); // x for width, y for height of buttons
std::string ui::g_failedMessage; // Global failed message for failedUI
std::string ui::g_userName;
std::string ui::g_position;


// Lowercase helper used by both constructUI and switchToUI
static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}


// Small helpers to keep UI code concise without changing behavior
static void renderStatCard(
    const char* childId,
    const char* title,
    const ImVec4& titleColor,
    const std::string& valueText,
    const char* subtitle,
    const ImVec2& size
) {
    ImGui::BeginChild(childId, size, true);
    ImGui::SetCursorPosY(15.0f);
    ImGui::SetCursorPosX(20.0f);
    ImGui::TextColored(titleColor, "%s", title);
    ImGui::SetCursorPosX(20.0f);
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("%s", valueText.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::SetCursorPosY(size.y - 30.0f);
    ImGui::SetCursorPosX(20.0f);
    ImGui::TextDisabled("%s", subtitle);
    ImGui::EndChild();
}

// Keep exact normalization semantics used previously for PRJ-xxxxx formatting
static std::string normalizeProjectId(const std::string& input) {
    std::string projectIDStr = input;
    if (!projectIDStr.empty()) {
        if (projectIDStr.length() != 10 || projectIDStr.substr(0, 4) != "PRJ-") {
            std::string digits;
            for (char ch : projectIDStr) {
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    digits.push_back(ch);
                }
            }
            if (!digits.empty()) {
                long long idVal = 0;
                try { idVal = std::stoll(digits); } catch (...) { idVal = 0; }
                std::ostringstream oss;
                oss << "PRJ-" << std::setw(5) << std::setfill('0') << idVal;
                projectIDStr = oss.str();
            }
        }
    }
    return projectIDStr;
}

static std::string buildProjectReportOutFile(const std::string& projectID) {
    return appConfig::g_dataDirectory + "project_report_" + projectID + "_" +
           std::to_string(system::fetchTime(system::PartDateTime::YEAR)) + "_" +
           std::to_string(system::fetchTime(system::PartDateTime::MONTH)) + "_" +
           std::to_string(system::fetchTime(system::PartDateTime::DAY)) + ".html";
}


static ImVec2 fullWidthButtonSize(const float a_height = g_buttonSizePxSelector.y) { return {ImGui::GetContentRegionAvail().x, a_height}; }


void setButtonCenter(const char* text, const ImVec2& size){
    const float windowWidth = ImGui::GetWindowSize().x;
    const float buttonWidth = size.x;

    // Move to the center position
    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    ImGui::Button(text, size);
}


void setTextCenter(const char* text){
    const float windowWidth = ImGui::GetWindowSize().x;
    const float textWidth = ImGui::CalcTextSize(text).x;

    // Move to the center position
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text("%s", text);
}


void setTextRight(const char* text){
    const float windowWidth = ImGui::GetWindowSize().x;
    const float textWidth = ImGui::CalcTextSize(text).x;

    // Position the cursor so that the text ends at the right edge
    ImGui::SetCursorPosX(windowWidth - textWidth - ImGui::GetStyle().WindowPadding.x);
    ImGui::Text("%s", text);
}


static void loadImage(const std::string& p_location, const float p_locationXPx, const float p_locationYPx, const float p_imageSize) {
    static HelloImGui::ImageAndSize imageFile;

    if constexpr (true) {imageFile = HelloImGui::ImageAndSizeFromAsset(p_location.c_str());}
    if (imageFile.textureId != static_cast<ImTextureID>(0)) {
        const float imageSize = p_imageSize;
        const float imagePosX = (ImGui::GetWindowWidth() - imageSize) * p_locationXPx;
        ImGui::SetCursorPosX(imagePosX);
        ImGui::SetCursorPosY(p_locationYPx);
        ImGui::Image(imageFile.textureId, ImVec2(imageSize, imageSize));
    }
}


static void failedUI() {
    ImGui::Text("%s", ui::g_failedMessage.c_str());
    if (ImGui::Button("Exit")) {
        if (auto* params = HelloImGui::GetRunnerParams()) {
            params->appShallExit = true;
        }
    }
}


static void loginUI() {
    const std::string displayUsername = "Username: ";
    const std::string displayPassword = "Password: ";
    static char username[128] = "";
    static char password[128] = "";
    static std::string loginErrorMessage;
    constexpr float textboxWidth = 460.0f;

    loadImage("icons/business_logo.png", 0.05f, 10.1f, 70.0f);
    ImGui::SetCursorPos(ImVec2(18.0f, 90.0f));
    ImGui::Text("Welcome, please log in to continue");

    ImGui::SetCursorPos(ImVec2(24.0f, 150.0f));
    ImGui::Text("%s", displayUsername.c_str());


    ImGui::SetCursorPos(ImVec2(18.0f, 180.0f));
    ImGui::SetNextItemWidth(textboxWidth); // width in pixels
    ImGui::InputText("##username", username, IM_ARRAYSIZE(username));

    ImGui::SetCursorPos(ImVec2(24.0f, 220.0f));
    ImGui::Text("%s", displayPassword.c_str());

    ImGui::SetCursorPos(ImVec2(18.0f, 250.0f));
    ImGui::SetNextItemWidth(textboxWidth); // width in pixels
    ImGui::InputText("##password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

    ImGui::SetCursorPos(ImVec2(25.0f, 310.0f));
    if (setButtonCenter("Log In", fullWidthButtonSize(35)), ImGui::IsItemClicked()){
        if (auth::testAuth(username, password)) {
            appConfig::g_auth = true;
            appConfig::g_testMode = true;

            username[0] = '\0';
            password[0] = '\0';

            if (auto* params = HelloImGui::GetRunnerParams()) {
                params->appShallExit = true;
            }
        } else if (auth::testDeployAuth(username, password)) {
            appConfig::g_auth = true;
            appConfig::g_testMode = false;

            username[0] = '\0';
            password[0] = '\0';

            if (auto* params = HelloImGui::GetRunnerParams()) {
                params->appShallExit = true;
            }
        } else if (auth::mainAuth(username, password)) {
            appConfig::g_auth = true;
            appConfig::g_testMode = false;

            username[0] = '\0';
            password[0] = '\0';

            // ui::g_userName and ui::g_position are already set by mainAuth()

            if (auto* params = HelloImGui::GetRunnerParams()) {
                params->appShallExit = true;
            }
        } else {
            loginErrorMessage = "Invalid username/password or decryption failed.";
        }
    }

    if (!loginErrorMessage.empty()) {
        ImGui::SetCursorPos(ImVec2(25.0f, 280.0f));
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", loginErrorMessage.c_str());
    }

    ImGui::SetCursorPos(ImVec2(25.0f, 360.0f));
    if (setButtonCenter("Exit App", fullWidthButtonSize(35)), ImGui::IsItemClicked()) {
        if (auto* params = HelloImGui::GetRunnerParams()) {
            params->appShallExit = true;
        }
    }
}

static void accountUI() {
    // Load business logo only once on the first call
    loadImage("icons/business_logo.png", 0.1f, 10.0f, 70.1f);
    loadImage("icons/user_icon.png", 0.8f, 10.0f, 60.0f);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f); // Small vertical spacing
    setTextRight(ui::g_userName.c_str());
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f); // Small vertical spacing
    setTextRight(ui::g_position.c_str());

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f); // Small vertical spacing
    if (ImGui::Button("Log Out", fullWidthButtonSize(40))) {
        // Encrypt all DBs before logging out (only if a session key exists)
        if (security::DBEncryptionSession::hasKey()) {
            security::DBEncryptionSession::encryptAllDbs();
            security::DBEncryptionSession::clear();
        }
        appConfig::g_auth = false;
        appConfig::g_testMode = false;
        ui::g_userName = "";
        ui::g_position = "";
        // Force the runner to use the login window size on next run, then request the current Run to exit.
        if (auto* params = HelloImGui::GetRunnerParams()) {
            params->appWindowParams.windowGeometry.size = { std::clamp(appConfig::g_loginWidth, 50, 3840), std::clamp(appConfig::g_loginHeight, 50, 2160) };
            // Ensure the runner applies the resize when it exits the current Run
            params->appWindowParams.windowGeometry.resizeAppWindowAtNextFrame = true;
            // Request app to quit so HelloImGui::Run returns, and we can restart at login
            params->appShallExit = true;
        }
    }
}


static void selectorUI() {
    // Greeting and username
    std::string l_greetings;
    if (const int hour = system::fetchTime(system::PartDateTime::HOUR); hour >= 0 && hour < 12) {
        l_greetings = "Good Morning,";
    }
    else if (hour >= 12 && hour < 18) {
        l_greetings = "Good Afternoon,";
    }
    else {
        l_greetings = "Good Evening,";
    }

    ImGui::SetCursorPos(ImVec2(20.0f, 10.0f));
    loadImage("icons/app_icon.png", 0.5f, 10.0f, 64.0f); // Centered application logo from assets/app_logo.png
    ImGui::SetCursorPos(ImVec2(87.0f, 65.0f));
    ImGui::Text("StructuraCost");

    ImGui::SetCursorPos(ImVec2(20.0f, 110.0f)); // x = padding from left, y = small padding from the top
    ImGui::SetWindowFontScale(1.7f); // Larger greeting
    ImGui::Text("%s", l_greetings.c_str());
    ImGui::SetWindowFontScale(1.0f); // Reset font scale
    ImGui::SetCursorPos(ImVec2(60.0f, 155.0f));
    ImGui::Text("%s", ui::g_userName.c_str());

    // Navigation buttons control the right pane
    if (appConfig::g_testMode) {
        ImGui::SetCursorPos(ImVec2(8.0f, 220.0f));
        if (ImGui::Button("Test/Debug", fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("test")) g_rightUI = g_uiMap["test"]; }
    }
    else {
        ImGui::SetCursorPos(ImVec2(8.0f, 220.0f));
        if (ImGui::Button("Summary", fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("summary")) g_rightUI = g_uiMap["summary"]; }
        ImGui::SetCursorPos(ImVec2(8.0f, 330.0f));
        if (ImGui::Button("Payroll Module", fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("payroll")) g_rightUI = g_uiMap["payroll"]; }
        ImGui::SetCursorPos(ImVec2(8.0f, 440.0f));
        if (ImGui::Button("Monitoring Module", fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("monitor")) g_rightUI = g_uiMap["monitor"]; }
    }

    // Top: Account info inside a bordered child, matching the two-column style
    ImGui::Spacing();
    ImGui::SetCursorPos(ImVec2(8.0f, 650.0f));
    const float lineH = ImGui::GetTextLineHeightWithSpacing();
    const float accountHeight = lineH * 8.5f;

    // Push a custom background color (RGB: 0.25, 0.25, 0.25) for the next ImGui child window.
    // This creates a lighter gray panel that visually separates the account info from the surrounding UI
    // Value of rgb / 255 (e.g., the value is 177, then 177/255 = 0.69f)
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));

    ImGui::BeginChild("AccountPanel", ImVec2(0, accountHeight), true);
    accountUI();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::Spacing();
}


static void summaryUI() {
    ImGui::SetWindowFontScale(1.7f);
    ImGui::Text("Dashboard Overview");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Separator();
    ImGui::Spacing();

    // Delegate counts to monitor module
    const auto payrollSummary = monitor::computePayrollSummary();
    const auto projectSummary = monitor::computeProjectSummary();
    const int totalEmployees = payrollSummary.totalEmployees;
    const int totalProjects = projectSummary.totalProjects;
    const int activeProjects = projectSummary.activeProjects;

    // Statistics Cards
    constexpr float cardWidth = 280.0f;
    constexpr float cardHeight = 120.0f;
    constexpr auto cardColor = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    constexpr auto accentColor = ImVec4(0.2f, 0.6f, 0.8f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, cardColor);

    // Employee Count Card
    renderStatCard("EmployeeCard", "Total Employees", accentColor, std::to_string(totalEmployees), "Active in payroll", ImVec2(cardWidth, cardHeight));

    ImGui::SameLine();

    // Project Count Card
    renderStatCard("ProjectCard", "Total Projects", accentColor, std::to_string(totalProjects), "Managed projects", ImVec2(cardWidth, cardHeight));

    ImGui::SameLine();

    // Active Projects Card
    renderStatCard("ActiveCard", "Active Projects", accentColor, std::to_string(activeProjects), "In progress/active", ImVec2(cardWidth, cardHeight));

    ImGui::SameLine();

    // System Status Card (kept as-is to preserve colored value text)
    ImGui::BeginChild("SystemCard", ImVec2(cardWidth, cardHeight), true);
    ImGui::SetCursorPosY(15.0f);
    ImGui::SetCursorPosX(20.0f);
    ImGui::TextColored(accentColor, "System Status");
    ImGui::SetCursorPosX(20.0f);
    ImGui::SetCursorPosY(50.0f);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "ONLINE");
    ImGui::SetCursorPosY(cardHeight - 30.0f);
    ImGui::SetCursorPosX(20.0f);
    ImGui::TextDisabled("All systems operational");
    ImGui::EndChild();

    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Recent Projects Section
    ImGui::SetWindowFontScale(1.1f);
    ImGui::Text("Recent Projects");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::BeginChild("RecentProjects", ImVec2(0, 250), true);

    // Table headers
    ImGui::Columns(5, "ProjectTable");
    ImGui::Separator();
    ImGui::Text("Project ID"); ImGui::NextColumn();
    ImGui::Text("Project Name"); ImGui::NextColumn();
    ImGui::Text("Status"); ImGui::NextColumn();
    ImGui::Text("Start Date"); ImGui::NextColumn();
    ImGui::Text("Notes"); ImGui::NextColumn();
    ImGui::Separator();

    // Display all projects (up to 1000)
    const std::string projectDB = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;
    int displayedProjects = 0;
    for (int row = 1; row <= 1000; ++row) {
        const std::string projectID = db::fetchCell(projectDB, static_cast<size_t>(row), 1);
        if (projectID.empty()) break;

        const std::string projectName = db::fetchCell(projectDB, static_cast<size_t>(row), 2);
        const std::string status = db::fetchCell(projectDB, static_cast<size_t>(row), 3);
        const std::string startDate = db::fetchCell(projectDB, static_cast<size_t>(row), 4);
        const std::string notes = db::fetchCell(projectDB, static_cast<size_t>(row), 5);

        // Color code by status
        ImVec4 statusColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
        if (status == "Active" || status == "In Progress") {
            statusColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
        } else if (status == "Completed") {
            statusColor = ImVec4(0.2f, 0.6f, 0.8f, 1.0f);
        } else if (status == "On-Hold") {
            statusColor = ImVec4(0.8f, 0.6f, 0.2f, 1.0f);
        }

        ImGui::Text("%s", projectID.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", projectName.c_str()); ImGui::NextColumn();
        ImGui::TextColored(statusColor, "%s", status.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", startDate.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", notes.c_str()); ImGui::NextColumn();
        ImGui::Separator();

        ++displayedProjects;
    }

    if (displayedProjects == 0) {
        ImGui::Columns(1);
        ImGui::TextDisabled("No projects available");
    }

    ImGui::Columns(1);
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Employee Overview Section
    ImGui::SetWindowFontScale(1.1f);
    ImGui::Text("List of Employees");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::BeginChild("RecentEmployees", ImVec2(0, 250), true);

    // Table headers
    ImGui::Columns(4, "EmployeeTable");
    ImGui::Separator();
    ImGui::Text("Employee No."); ImGui::NextColumn();
    ImGui::Text("Name"); ImGui::NextColumn();
    ImGui::Text("Position"); ImGui::NextColumn();
    ImGui::Text("Site Location"); ImGui::NextColumn();
    ImGui::Separator();

    // Display all employees (up to 1000)
    int displayedEmployees = 0;
    const auto empRows = monitor::listEmployees(1000);
    for (const auto& er : empRows) {
        ImGui::Text("%s", er.id.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", er.name.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", er.position.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", er.siteLocation.c_str()); ImGui::NextColumn();
        ImGui::Separator();
        ++displayedEmployees;
    }

    if (displayedEmployees == 0) {
        ImGui::Columns(1);
        ImGui::TextDisabled("No employees available");
    }

    ImGui::Columns(1);
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Action Buttons Section
    ImGui::SetWindowFontScale(1.1f);
    ImGui::Text("Generate Reports");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    static char projectIDForReport[128] = "";

    // Generate week options for payslip dropdown (same logic as weekly attendance)
    static std::vector<std::string> payslipWeekOptions;
    static std::vector<std::string> payslipWeekDates;
    static int selectedPayslipWeekIndex = 0;

    if (payslipWeekOptions.empty()) {
        const int currentYear = system::fetchTime(system::PartDateTime::YEAR);
        const int currentMonth = system::fetchTime(system::PartDateTime::MONTH);
        const int currentDay = system::fetchTime(system::PartDateTime::DAY);

        // Helper lambda to get day of week (0=Sunday, 6=Saturday)
        auto getDayOfWeek = [](int year, int month, int day) -> int {
            if (month < 3) {
                month += 12;
                year--;
            }
            int q = day;
            int m = month;
            int k = year % 100;
            int j = year / 100;
            int h = (q + ((13 * (m + 1)) / 5) + k + (k / 4) + (j / 4) - (2 * j)) % 7;
            return (h + 6) % 7;
        };

        auto getDaysInMonth = [currentYear](int month) -> int {
            if (month == 2) {
                bool isLeap = (currentYear % 4 == 0 && currentYear % 100 != 0) || (currentYear % 400 == 0);
                return isLeap ? 29 : 28;
            }
            if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
            return 31;
        };

        for (int month = 1; month <= 12; month++) {
            const int daysInMonth = getDaysInMonth(month);
            for (int day = 1; day <= daysInMonth; day++) {
                if (getDayOfWeek(currentYear, month, day) == 0) {
                    int endDay = day + 6;
                    int endMonth = month;
                    if (endDay > daysInMonth) {
                        endDay -= daysInMonth;
                        endMonth = (month % 12) + 1;
                    }

                    std::ostringstream label, dateRange;
                    label << std::setfill('0') << std::setw(2) << month << "/"
                          << std::setw(2) << day;
                    if (endMonth != month) {
                        label << "-" << std::setw(2) << endMonth << "/" << std::setw(2) << endDay;
                    } else {
                        label << "-" << std::setw(2) << endDay;
                    }

                    dateRange << currentYear << "-" << std::setfill('0')
                              << std::setw(2) << month << "-" << std::setw(2) << day;

                    payslipWeekOptions.push_back(label.str());
                    payslipWeekDates.push_back(dateRange.str());
                }
            }
        }
    }

    // Payslip Reports - All in one row
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 0.8f, 1.0f), "Payslip Reports:");
    ImGui::SameLine(200.0f);

    if (ImGui::Button("Print Payslips (Current)", ImVec2(280.0f, 40.0f))) {
        const std::string outFile = appConfig::g_dataDirectory + "payslips_" +
                                   std::to_string(system::fetchTime(system::PartDateTime::YEAR)) + "_" +
                                   std::to_string(system::fetchTime(system::PartDateTime::MONTH)) + "_" +
                                   std::to_string(system::fetchTime(system::PartDateTime::DAY)) + ".html";

        if (const std::string logoPath = HelloImGui::AssetFileFullPath("icons/business_logo.png"); exportPayslipsHtml(outFile, logoPath)) {
            system::logMessage(system::messageClassification::INFO, "Payslips exported successfully to: " + outFile + "\n");
        } else {
            system::logMessage(system::messageClassification::ERROR, "Failed to export payslips\n");
        }
    }

    ImGui::SameLine();
    ImGui::Text("Week:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    if (ImGui::BeginCombo("##weekSelectorPayslip", payslipWeekOptions[selectedPayslipWeekIndex].c_str())) {
        for (int i = 0; i < payslipWeekOptions.size(); i++) {
            bool isSelected = (selectedPayslipWeekIndex == i);
            if (ImGui::Selectable(payslipWeekOptions[i].c_str(), isSelected)) {
                selectedPayslipWeekIndex = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();

    if (ImGui::Button("Print Payslips for Week", ImVec2(250.0f, 40.0f))) {
        const std::string& weekDateStr = payslipWeekDates[selectedPayslipWeekIndex];
        const std::string outFile = appConfig::g_dataDirectory + "payslips_week_" + weekDateStr + ".html";

        if (const std::string logoPath = HelloImGui::AssetFileFullPath("icons/business_logo.png"); exportPayslipsHtmlForWeek(outFile, logoPath, weekDateStr)) {
            system::logMessage(system::messageClassification::INFO, "Weekly payslips exported successfully to: " + outFile + "\n");
        } else {
            system::logMessage(system::messageClassification::ERROR, "Failed to export weekly payslips\n");
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Project Reports - All in one row
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 0.8f, 1.0f), "Project Reports:");
    ImGui::SameLine(200.0f);
    ImGui::Text("Project ID:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputText("##projectIDReport", projectIDForReport, IM_ARRAYSIZE(projectIDForReport));
    ImGui::SameLine();

    if (ImGui::Button("Print Project Report", ImVec2(280.0f, 40.0f))) {
        std::string projectIDStr = normalizeProjectId(std::string(projectIDForReport));
        if (!projectIDStr.empty()) {
            const std::string outFile = buildProjectReportOutFile(projectIDStr);
            const std::string logoPath = HelloImGui::AssetFileFullPath("icons/business_logo.png");
            if (exportProjectReportHtml(projectIDStr, outFile, logoPath)) {
                system::logMessage(system::messageClassification::INFO, "Project report exported successfully to: " + outFile + "\n");
            } else {
                system::logMessage(system::messageClassification::ERROR, "Failed to export project report for " + projectIDStr + "\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "Please enter a Project ID\n");
        }
    }
    ImGui::Spacing();

    // Footer with current date/time
    const int currentYear = system::fetchTime(system::PartDateTime::YEAR);
    const int currentMonth = system::fetchTime(system::PartDateTime::MONTH);
    const int currentDay = system::fetchTime(system::PartDateTime::DAY);
    const int currentHour = system::fetchTime(system::PartDateTime::HOUR);
    const int currentMinute = system::fetchTime(system::PartDateTime::MINUTE);

    ImGui::Separator();
    ImGui::TextDisabled("Last Updated: %04d-%02d-%02d %02d:%02d",
                        currentYear, currentMonth, currentDay, currentHour, currentMinute);
}


static void payrollUI() {
    ImGui::SetWindowFontScale(1.7f);
    ImGui::Text("Payroll Management");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Separator();
    ImGui::Spacing();

    // Delegate stats computation to core/monitor module
    const auto payrollSummary = monitor::computePayrollSummary();
    const int totalEmployees = payrollSummary.totalEmployees;
    const double totalSalaryExpense = payrollSummary.totalSalaryExpense;
    const double totalAdvances = payrollSummary.totalAdvances;

    // Statistics Cards
    constexpr float cardWidth = 380.0f;
    constexpr float cardHeight = 120.0f;
    constexpr auto cardColor = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    constexpr auto accentColor = ImVec4(0.2f, 0.6f, 0.8f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, cardColor);

    // Total Employees Card
    renderStatCard("PayrollEmployeeCard", "Total Employees", accentColor, std::to_string(totalEmployees), "Active employees", ImVec2(cardWidth, cardHeight));

    ImGui::SameLine();

    // Total Salary Expense Card
    {
        std::ostringstream oss; oss.setf(std::ios::fixed); oss << std::setprecision(2) << totalSalaryExpense;
        renderStatCard("SalaryExpenseCard", "Total Salary Expense", accentColor, oss.str(), "Total payroll cost", ImVec2(cardWidth, cardHeight));
    }

    ImGui::SameLine();

    // Total Advances Card
    {
        std::ostringstream oss; oss.setf(std::ios::fixed); oss << std::setprecision(2) << totalAdvances;
        renderStatCard("AdvancesCard", "Total Advances", accentColor, oss.str(), "Employee advances", ImVec2(cardWidth, cardHeight));
    }

    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Employee Payroll Table
    ImGui::SetWindowFontScale(1.1f);
    ImGui::Text("All Employee Payroll Records");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::BeginChild("PayrollTable", ImVec2(0, 0), true);

    // Table headers
    ImGui::Columns(7, "PayrollColumns");
    ImGui::Separator();
    ImGui::Text("Employee No."); ImGui::NextColumn();
    ImGui::Text("Name"); ImGui::NextColumn();
    ImGui::Text("Position"); ImGui::NextColumn();
    ImGui::Text("Site Location"); ImGui::NextColumn();
    ImGui::Text("Hourly Rate"); ImGui::NextColumn();
    ImGui::Text("Hours Worked"); ImGui::NextColumn();
    ImGui::Text("Advance"); ImGui::NextColumn();
    ImGui::Separator();

    // Display all employees (up to 1000)
    int displayedEmployees = 0;
    const auto payrollRows = monitor::listEmployees(1000);
    for (const auto& er : payrollRows) {
        ImGui::Text("%s", er.id.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", er.name.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", er.position.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", er.siteLocation.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", er.salary.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", er.hoursWork.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", er.advance.c_str()); ImGui::NextColumn();
        ImGui::Separator();
        ++displayedEmployees;
    }

    if (displayedEmployees == 0) {
        ImGui::Columns(1);
        ImGui::TextDisabled("No employee payroll records available");
    } else {
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::TextDisabled("Showing %d employee(s)", displayedEmployees);
    }

    ImGui::EndChild();
}


static void monitorUI() {
    static char name[128], position[128], employeeID[128], location[128], salary[128], hoursWorked[128], advance[128];

    ImGui::SetWindowFontScale(1.7f);
    ImGui::Text("Data Management & Monitoring");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Separator();
    ImGui::Spacing();


    // ==============================================
    // EMPLOYEE MANAGEMENT SECTION
    // ==============================================
    ImGui::SetWindowFontScale(1.1f);
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 0.8f, 1.0f), "Employee Management");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::BeginChild("Employee Management", ImVec2(0, 400), true);
    ImGui::Spacing();

    // Convert to strings for validation
    const std::string nameStr(name);
    const std::string positionStr(position);
    const std::string employeeIDStr(employeeID);
    const std::string locationStr(location);
    const std::string salaryStr(salary);
    const std::string hoursWorkedStr(hoursWorked);
    const std::string advanceStr(advance);

    // Real-time validation
    const bool nameValid = nameStr.empty() || !system::validateInput(system::ValidationType::NAME, nameStr).empty();
    const bool positionValid = positionStr.empty() || !system::validateInput(system::ValidationType::POSITION, positionStr).empty();
    const bool employeeIDValid = employeeIDStr.empty() || !system::validateInput(system::ValidationType::EMPLOYEE_ID, employeeIDStr).empty();
    const bool locationValid = locationStr.empty() || !system::validateInput(system::ValidationType::SITE_LOCATION, locationStr).empty();
    const bool salaryValid = salaryStr.empty() || !system::validateInput(system::ValidationType::SALARY, salaryStr).empty();
    const bool hoursValid = hoursWorkedStr.empty() || !system::validateInput(system::ValidationType::HOURS, hoursWorkedStr).empty();
    const bool advanceValid = advanceStr.empty() || !system::validateInput(system::ValidationType::ADVANCE, advanceStr).empty();

    // Employee Form Fields - Row 1
    ImGui::Text("Name:");
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(300.0f);
    if (!nameValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##name", name, IM_ARRAYSIZE(name));
    if (!nameValid) ImGui::PopStyleColor();

    ImGui::SameLine(450.0f);
    ImGui::Text("Position:");
    ImGui::SameLine(550.0f);
    ImGui::SetNextItemWidth(250.0f);
    if (!positionValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##position", position, IM_ARRAYSIZE(position));
    if (!positionValid) ImGui::PopStyleColor();

    ImGui::SameLine(830.0f);
    ImGui::Text("Employee ID:");
    ImGui::SameLine(950.0f);
    ImGui::SetNextItemWidth(150.0f);
    if (!employeeIDValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##employeeID", employeeID, IM_ARRAYSIZE(employeeID));
    if (!employeeIDValid) ImGui::PopStyleColor();

    ImGui::Spacing();

    // Employee Form Fields - Row 2
    ImGui::Text("Site Location:");
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(250.0f);

    // Build site location options: Main Office, Warehouse, and all project IDs
    static std::vector<std::string> siteLocationOptions;
    static int selectedLocationIndex = 0;
    static bool optionsInitialized = false;

    // Refresh project list periodically or on first load
    static int refreshCounter = 0;
    if (!optionsInitialized || refreshCounter++ % 60 == 0) { // Refresh every 60 frames
        siteLocationOptions.clear();
        siteLocationOptions.push_back("Main Office");
        siteLocationOptions.push_back("Warehouse");

        // Add all project IDs from database
        std::vector<std::string> projectIDs = monitor::listProjectIDs();
        for (const auto& projId : projectIDs) {
            siteLocationOptions.push_back(projId);
        }
        optionsInitialized = true;

        // Try to match current location value to dropdown index
        const std::string currentLocation(location);
        for (size_t i = 0; i < siteLocationOptions.size(); ++i) {
            if (siteLocationOptions[i] == currentLocation) {
                selectedLocationIndex = static_cast<int>(i);
                break;
            }
        }
    }

    // Create array of const char* for ImGui::Combo
    std::vector<const char*> locationCStrings;
    for (const auto& opt : siteLocationOptions) {
        locationCStrings.push_back(opt.c_str());
    }

    if (ImGui::Combo("##location", &selectedLocationIndex, locationCStrings.data(), static_cast<int>(locationCStrings.size()))) {
        // Update location buffer when selection changes
        std::strncpy(location, siteLocationOptions[selectedLocationIndex].c_str(), sizeof(location) - 1);
        location[sizeof(location) - 1] = '\0';
    }

    ImGui::SameLine(400.0f);
    ImGui::Text("Hourly Rate:");
    ImGui::SameLine(510.0f);
    ImGui::SetNextItemWidth(120.0f);
    if (!salaryValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##salary", salary, IM_ARRAYSIZE(salary));
    if (!salaryValid) ImGui::PopStyleColor();

    ImGui::SameLine(660.0f);
    ImGui::Text("Hours Worked:");
    ImGui::SameLine(780.0f);
    ImGui::SetNextItemWidth(120.0f);
    if (!hoursValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##hoursWorked", hoursWorked, IM_ARRAYSIZE(hoursWorked));
    if (!hoursValid) ImGui::PopStyleColor();

    ImGui::SameLine(930.0f);
    ImGui::Text("Advance:");
    ImGui::SameLine(1020.0f);
    ImGui::SetNextItemWidth(120.0f);
    if (!advanceValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##advance", advance, IM_ARRAYSIZE(advance));
    if (!advanceValid) ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Employee Action Buttons
    if (ImGui::Button("Add New Employee", ImVec2(260.0f, 40.0f))) {
        // Validate all inputs before adding
        const std::string validatedName = system::validateInput(system::ValidationType::NAME, nameStr);
        const std::string validatedPosition = system::validateInput(system::ValidationType::POSITION, positionStr);
        const std::string validatedLocation = system::validateInput(system::ValidationType::SITE_LOCATION, locationStr);
        const std::string validatedSalary = system::validateInput(system::ValidationType::SALARY, salaryStr);
        const std::string validatedHours = system::validateInput(system::ValidationType::HOURS, hoursWorkedStr);
        const std::string validatedAdvance = system::validateInput(system::ValidationType::ADVANCE, advanceStr);

        if (validatedName.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Name is required and must be valid (max 100 characters).\n");
        } else if (validatedPosition.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Position is required and must be valid (max 50 characters).\n");
        } else if (validatedLocation.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Site location is required and must be valid.\n");
        } else if (validatedSalary.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Hourly rate must be a positive number.\n");
        } else if (validatedHours.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Hours worked must be a valid number (0-168).\n");
        } else if (validatedAdvance.empty() && !advanceStr.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Advance must be a non-negative number if provided.\n");
        } else {
            if (monitor::addEmployee(validatedName, validatedPosition, validatedLocation, validatedSalary, validatedHours, validatedAdvance)) {
                system::logMessage(system::messageClassification::INFO, "DB: New employee added successfully.\n");
                name[0] = '\0'; position[0] = '\0'; employeeID[0] = '\0'; location[0] = '\0';
                salary[0] = '\0'; hoursWorked[0] = '\0'; advance[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "DB: Failed to add new employee.\n");
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Update Employee", ImVec2(260.0f, 40.0f))) {
        const std::string validatedEmployeeID = system::validateInput(system::ValidationType::EMPLOYEE_ID, employeeIDStr);

        if (validatedEmployeeID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Employee ID is required for update.\n");
        } else {
            // Validate provided fields (allow partial updates)
            std::string validatedName = nameStr.empty() ? nameStr : system::validateInput(system::ValidationType::NAME, nameStr);
            std::string validatedPosition = positionStr.empty() ? positionStr : system::validateInput(system::ValidationType::POSITION, positionStr);
            std::string validatedLocation = locationStr.empty() ? locationStr : system::validateInput(system::ValidationType::SITE_LOCATION, locationStr);
            std::string validatedSalary = salaryStr.empty() ? salaryStr : system::validateInput(system::ValidationType::SALARY, salaryStr);
            std::string validatedHours = hoursWorkedStr.empty() ? hoursWorkedStr : system::validateInput(system::ValidationType::HOURS, hoursWorkedStr);
            std::string validatedAdvance = system::validateInput(system::ValidationType::ADVANCE, advanceStr);

            bool hasValidationError = false;
            if (!nameStr.empty() && validatedName.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid name provided.\n");
                hasValidationError = true;
            } else if (!positionStr.empty() && validatedPosition.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid position provided.\n");
                hasValidationError = true;
            } else if (!locationStr.empty() && validatedLocation.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid location provided.\n");
                hasValidationError = true;
            } else if (!salaryStr.empty() && validatedSalary.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid salary provided.\n");
                hasValidationError = true;
            } else if (!hoursWorkedStr.empty() && validatedHours.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid hours worked provided.\n");
                hasValidationError = true;
            } else if (!advanceStr.empty() && validatedAdvance.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid advance provided.\n");
                hasValidationError = true;
            }

            if (!hasValidationError) {
                if (monitor::updateEmployee(validatedEmployeeID, validatedName, validatedPosition, validatedLocation, validatedSalary, validatedHours, validatedAdvance)) {
                    system::logMessage(system::messageClassification::INFO, "DB: Employee data updated successfully.\n");
                    name[0] = '\0'; position[0] = '\0'; employeeID[0] = '\0'; location[0] = '\0';
                    salary[0] = '\0'; hoursWorked[0] = '\0'; advance[0] = '\0';
                } else {
                    system::logMessage(system::messageClassification::ERROR, "DB: Failed to update employee data.\n");
                }
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete Employee", ImVec2(260.0f, 40.0f))) {
        if (const std::string validatedEmployeeID = system::validateInput(system::ValidationType::EMPLOYEE_ID, employeeIDStr); validatedEmployeeID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Employee ID is required for deletion.\n");
        } else {
            if (monitor::deleteEmployee(validatedEmployeeID)) {
                system::logMessage(system::messageClassification::INFO, "DB: Employee deleted successfully.\n");
                employeeID[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "DB: Failed to delete employee.\n");
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Load Employee", ImVec2(260.0f, 40.0f))) {
        const std::string validatedEmployeeID = system::validateInput(system::ValidationType::EMPLOYEE_ID, employeeIDStr);

        if (validatedEmployeeID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Employee ID is required to load employee data.\n");
        } else {
            // Search for employee in database
            const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
            bool foundEmployee = false;

            for (int row = 1; row <= 1000; ++row) {
                std::string currentID = db::fetchCell(dbPath, static_cast<size_t>(row), 1);
                if (currentID.empty()) break;

                if (currentID == validatedEmployeeID) {
                    // Found the employee - populate all fields
                    std::string empName = db::fetchCell(dbPath, static_cast<size_t>(row), 2);
                    std::string empPosition = db::fetchCell(dbPath, static_cast<size_t>(row), 3);
                    std::string empLocation = db::fetchCell(dbPath, static_cast<size_t>(row), 4);
                    std::string empSalary = db::fetchCell(dbPath, static_cast<size_t>(row), 5);
                    std::string empHours = db::fetchCell(dbPath, static_cast<size_t>(row), 6);
                    std::string empAdvance = db::fetchCell(dbPath, static_cast<size_t>(row), 7);

                    // Copy to input buffers
                    std::strncpy(name, empName.c_str(), sizeof(name) - 1);
                    name[sizeof(name) - 1] = '\0';

                    std::strncpy(position, empPosition.c_str(), sizeof(position) - 1);
                    position[sizeof(position) - 1] = '\0';

                    std::strncpy(location, empLocation.c_str(), sizeof(location) - 1);
                    location[sizeof(location) - 1] = '\0';

                    std::strncpy(salary, empSalary.c_str(), sizeof(salary) - 1);
                    salary[sizeof(salary) - 1] = '\0';

                    std::strncpy(hoursWorked, empHours.c_str(), sizeof(hoursWorked) - 1);
                    hoursWorked[sizeof(hoursWorked) - 1] = '\0';

                    std::strncpy(advance, empAdvance.c_str(), sizeof(advance) - 1);
                    advance[sizeof(advance) - 1] = '\0';

                    // Update site location dropdown index to match loaded location
                    for (size_t i = 0; i < siteLocationOptions.size(); ++i) {
                        if (siteLocationOptions[i] == empLocation) {
                            selectedLocationIndex = static_cast<int>(i);
                            break;
                        }
                    }

                    foundEmployee = true;
                    system::logMessage(system::messageClassification::INFO,
                        "DB: Loaded employee data for ID " + validatedEmployeeID + " (" + empName + ")\n");
                    break;
                }
            }

            if (!foundEmployee) {
                system::logMessage(system::messageClassification::WARNING,
                    "DB: Employee ID " + validatedEmployeeID + " not found in database.\n");
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Employee Database Viewer
    ImGui::Text("Employee Database Viewer");
    static int s_maxRowsEmployee = 100;
    static bool s_showHeadersEmployee = true;

    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputInt("Max Rows##emp", &s_maxRowsEmployee, 10, 50);
    if (s_maxRowsEmployee < 1) s_maxRowsEmployee = 1;
    if (s_maxRowsEmployee > 100) s_maxRowsEmployee = 100;

    ImGui::SameLine();
    ImGui::Checkbox("Show Headers##emp", &s_showHeadersEmployee);

    struct Column { const char* name; };
    const std::vector<Column> columns = {
        {"EMPLOYEE_ID"}, {"NAME"}, {"POSITION"}, {"SITE_LOCATION"},
        {"SALARY"}, {"HOURS_WORK"}, {"ADVANCE"}
    };

    ImGui::BeginChild("EmployeeDBViewer", ImVec2(0, 0), true);

    if (s_showHeadersEmployee) {
        ImGui::Columns(static_cast<int>(columns.size()), "EmpViewerHeader");
        for (const auto& c : columns) {
            ImGui::Text("%s", c.name);
            ImGui::NextColumn();
        }
        ImGui::Separator();
        ImGui::Columns(1);
    }

    int shownEmployee = 0;
    const auto rows = monitor::listEmployees(s_maxRowsEmployee);
    for (size_t i = 0; i < rows.size(); ++i) {
        const auto& er = rows[i];
        ImGui::PushID(static_cast<int>(i) + 10000);
        ImGui::Columns(static_cast<int>(columns.size()), "EmpViewerRow");
        ImGui::TextWrapped("%s", er.id.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", er.name.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", er.position.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", er.siteLocation.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", er.salary.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", er.hoursWork.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", er.advance.c_str()); ImGui::NextColumn();
        ImGui::Columns(1);
        ImGui::PopID();
        ++shownEmployee;
    }

    ImGui::TextDisabled(shownEmployee == 0 ? "No employees to display." : "Showing %d employee(s).", shownEmployee);
    ImGui::EndChild();

    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Spacing();

    // ==============================================
    // ATTENDANCE MANAGEMENT SECTION
    // ==============================================
    ImGui::SetWindowFontScale(1.1f);
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 0.8f, 1.0f), "Weekly Attendance Management");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::BeginChild("AttendanceManagement", ImVec2(0, 420), true);
    ImGui::Spacing();

    static char attendanceEmployeeID[128];
    static char sunHours[128], monHours[128], tueHours[128], wedHours[128], thuHours[128], friHours[128], satHours[128];
    static int selectedWeekIndex = 0;

    // Generate week options aligned to Sunday-Saturday
    static std::vector<std::string> weekOptions;
    static std::vector<std::string> weekDates;
    if (weekOptions.empty()) {
        const int currentYear = system::fetchTime(system::PartDateTime::YEAR);

        // Helper lambda to get day of week (0=Sunday, 6=Saturday)
        auto getDayOfWeek = [](int year, int month, int day) -> int {
            // Zeller's congruence algorithm
            if (month < 3) {
                month += 12;
                year--;
            }
            int q = day;
            int m = month;
            int k = year % 100;
            int j = year / 100;
            int h = (q + ((13 * (m + 1)) / 5) + k + (k / 4) + (j / 4) - (2 * j)) % 7;
            // Convert to 0=Sunday format
            return (h + 6) % 7;
        };

        // Helper to get days in month
        auto getDaysInMonth = [currentYear](int month) -> int {
            if (month == 2) {
                // Check for leap year
                bool isLeap = (currentYear % 4 == 0 && currentYear % 100 != 0) || (currentYear % 400 == 0);
                return isLeap ? 29 : 28;
            }
            if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
            return 31;
        };

        // Generate all Sundays in the year
        for (int month = 1; month <= 12; month++) {
            int daysInMonth = getDaysInMonth(month);

            for (int day = 1; day <= daysInMonth; day++) {
                int dayOfWeek = getDayOfWeek(currentYear, month, day);

                // If this is a Sunday, create a week entry
                if (dayOfWeek == 0) {
                    int endDay = day + 6;
                    int endMonth = month;

                    // Check if week spans into next month
                    if (endDay > daysInMonth) {
                        endDay = endDay - daysInMonth;
                        endMonth = month + 1;
                        if (endMonth > 12) endMonth = 1;
                    }

                    std::ostringstream label, dateRange;
                    if (endMonth == month) {
                        // Week is within same month
                        label << std::setfill('0') << std::setw(2) << month << "/"
                              << std::setw(2) << day << "-" << std::setw(2) << endDay;
                    } else {
                        // Week spans two months
                        label << std::setfill('0') << std::setw(2) << month << "/"
                              << std::setw(2) << day << "-" << std::setw(2) << endMonth << "/"
                              << std::setw(2) << endDay;
                    }

                    dateRange << currentYear << "-" << std::setfill('0') << std::setw(2) << month
                             << "-" << std::setw(2) << day;
                    weekOptions.push_back(label.str());
                    weekDates.push_back(dateRange.str());
                }
            }
        }
    }

    // Convert to strings for validation
    const std::string attendanceEmpIDStr(attendanceEmployeeID);

    // Validation for employee ID
    const bool attendanceEmpIDValid = attendanceEmpIDStr.empty() || !system::validateInput(system::ValidationType::EMPLOYEE_ID, attendanceEmpIDStr).empty();

    ImGui::Text("Employee ID:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    if (!attendanceEmpIDValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##attendanceEmployeeID", attendanceEmployeeID, IM_ARRAYSIZE(attendanceEmployeeID));
    if (!attendanceEmpIDValid) ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::Text("  Week:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    if (ImGui::BeginCombo("##weekSelector", weekOptions[selectedWeekIndex].c_str())) {
        for (int i = 0; i < weekOptions.size(); i++) {
            bool isSelected = (selectedWeekIndex == i);
            if (ImGui::Selectable(weekOptions[i].c_str(), isSelected)) {
                selectedWeekIndex = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::Text("  Day:");
    ImGui::SameLine();
    static int selectedDayForHours = 0;
    const char* dayLabels[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::BeginCombo("##daySelector", dayLabels[selectedDayForHours])) {
        for (int i = 0; i < 7; ++i) {
            bool isSelected = (selectedDayForHours == i);
            if (ImGui::Selectable(dayLabels[i], isSelected)) selectedDayForHours = i;
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::Text("  Hours:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80.0f);

    static char* dayBuffers[] = { sunHours, monHours, tueHours, wedHours, thuHours, friHours, satHours };
    ImGui::InputText("##dayHours", dayBuffers[selectedDayForHours], IM_ARRAYSIZE(sunHours));

    const std::string weekStartStr = weekDates[selectedWeekIndex];
    const std::string weekLabel = weekOptions[selectedWeekIndex];
    const std::string sunHoursStr(sunHours);
    const std::string monHoursStr(monHours);
    const std::string tueHoursStr(tueHours);
    const std::string wedHoursStr(wedHours);
    const std::string thuHoursStr(thuHours);
    const std::string friHoursStr(friHours);
    const std::string satHoursStr(satHours);

    // Format employee ID
    std::string formattedEmpID;
    {
        std::string digits;
        for (char ch : attendanceEmpIDStr) {
            if (std::isdigit(static_cast<unsigned char>(ch))) {
                digits.push_back(ch);
            }
        }
        if (!digits.empty()) {
            long long idVal = 0;
            try { idVal = std::stoll(digits); } catch (...) { idVal = 0; }
            std::ostringstream oss; oss << "EMP-" << std::setw(5) << std::setfill('0') << idVal;
            formattedEmpID = oss.str();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Attendance Action Buttons
    if (ImGui::Button("Add Weekly Attendance", ImVec2(350.0f, 40.0f))) {
        if (!formattedEmpID.empty() && !weekStartStr.empty()) {
            // For logging only (UI presentation), keep the same safe label formatting
            std::string safeWeekLabel = weekLabel;
            std::ranges::replace(safeWeekLabel, '/', '-');

            const bool ok = monitor::addWeeklyAttendance(
                attendanceEmpIDStr,
                weekLabel,
                weekStartStr,
                sunHoursStr,
                monHoursStr,
                tueHoursStr,
                wedHoursStr,
                thuHoursStr,
                friHoursStr,
                satHoursStr
            );

            if (ok) {
                system::logMessage(system::messageClassification::INFO, "Attendance: Weekly attendance added successfully to " + safeWeekLabel + "\n");
                attendanceEmployeeID[0] = '\0';
                sunHours[0] = '\0'; monHours[0] = '\0'; tueHours[0] = '\0'; wedHours[0] = '\0';
                thuHours[0] = '\0'; friHours[0] = '\0'; satHours[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "Attendance: Failed to add weekly attendance.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "Attendance: Employee ID and Week are required.\n");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Update Weekly Attendance", ImVec2(350.0f, 40.0f))) {
        if (!formattedEmpID.empty() && !weekStartStr.empty()) {
            const bool ok = monitor::updateWeeklyAttendance(
                attendanceEmpIDStr,
                weekLabel,
                weekStartStr,
                sunHoursStr,
                monHoursStr,
                tueHoursStr,
                wedHoursStr,
                thuHoursStr,
                friHoursStr,
                satHoursStr
            );
            if (ok) {
                system::logMessage(system::messageClassification::INFO, "Attendance: Week attendance updated successfully.\n");
                attendanceEmployeeID[0] = '\0';
                sunHours[0] = '\0'; monHours[0] = '\0'; tueHours[0] = '\0'; wedHours[0] = '\0';
                thuHours[0] = '\0'; friHours[0] = '\0'; satHours[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "Attendance: Failed to update week attendance.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "Attendance: Employee ID and Week are required.\n");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete Weekly Attendance", ImVec2(350.0f, 40.0f))) {
        if (!formattedEmpID.empty() && !weekStartStr.empty()) {
            const bool ok = monitor::deleteWeeklyAttendance(
                attendanceEmpIDStr,
                weekLabel,
                weekStartStr
            );
            if (ok) {
                system::logMessage(system::messageClassification::INFO, "Attendance: Week attendance deleted successfully.\n");
                attendanceEmployeeID[0] = '\0';
                sunHours[0] = '\0'; monHours[0] = '\0'; tueHours[0] = '\0'; wedHours[0] = '\0';
                thuHours[0] = '\0'; friHours[0] = '\0'; satHours[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "Attendance: Failed to delete week attendance.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "Attendance: Employee ID and Week are required.\n");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Load Weekly Attendance", ImVec2(350.0f, 40.0f))) {
        if (!formattedEmpID.empty() && !weekStartStr.empty()) {
            // Search for attendance in database
            const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

            sqlite3* db = nullptr;
            if (sqlite3_open(dbPath.c_str(), &db) == SQLITE_OK) {
                sqlite3_stmt* stmt = nullptr;
                const char* sql = "SELECT SUN, MON, TUE, WED, THU, FRI, SAT FROM WEEKLY_ATTENDANCE WHERE EMPLOYEE_ID = ? AND WEEK_START = ? LIMIT 1;";

                if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                    // Extract numeric employee ID
                    std::string digits;
                    for (char ch : attendanceEmpIDStr) {
                        if (std::isdigit(static_cast<unsigned char>(ch))) {
                            digits.push_back(ch);
                        }
                    }
                    int empIdNum = 0;
                    if (!digits.empty()) {
                        try { empIdNum = std::stoi(digits); } catch (...) { empIdNum = 0; }
                    }

                    sqlite3_bind_int(stmt, 1, empIdNum);
                    sqlite3_bind_text(stmt, 2, weekStartStr.c_str(), -1, SQLITE_TRANSIENT);

                    if (sqlite3_step(stmt) == SQLITE_ROW) {
                        // Found the attendance record - populate all fields
                        double sunVal = sqlite3_column_double(stmt, 0);
                        double monVal = sqlite3_column_double(stmt, 1);
                        double tueVal = sqlite3_column_double(stmt, 2);
                        double wedVal = sqlite3_column_double(stmt, 3);
                        double thuVal = sqlite3_column_double(stmt, 4);
                        double friVal = sqlite3_column_double(stmt, 5);
                        double satVal = sqlite3_column_double(stmt, 6);

                        // Copy to input buffers
                        snprintf(sunHours, sizeof(sunHours), "%.1f", sunVal);
                        snprintf(monHours, sizeof(monHours), "%.1f", monVal);
                        snprintf(tueHours, sizeof(tueHours), "%.1f", tueVal);
                        snprintf(wedHours, sizeof(wedHours), "%.1f", wedVal);
                        snprintf(thuHours, sizeof(thuHours), "%.1f", thuVal);
                        snprintf(friHours, sizeof(friHours), "%.1f", friVal);
                        snprintf(satHours, sizeof(satHours), "%.1f", satVal);

                        system::logMessage(system::messageClassification::INFO, "Attendance: Weekly attendance loaded successfully for " + formattedEmpID + "\n");
                    } else {
                        system::logMessage(system::messageClassification::WARNING, "Attendance: No attendance record found for Employee " + formattedEmpID + " for week " + weekStartStr + "\n");
                    }
                    sqlite3_finalize(stmt);
                } else {
                    system::logMessage(system::messageClassification::ERROR, "Attendance: Database query failed\n");
                }
                sqlite3_close(db);
            } else {
                system::logMessage(system::messageClassification::ERROR, "Attendance: Failed to open database\n");
            }
        } else {
            system::logMessage(system::messageClassification::WARNING, "Attendance: Employee ID and Week are required to load attendance data.\n");
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Attendance Database Viewer
    ImGui::Text("Weekly Attendance Viewer");
    static int s_maxRowsAttendance = 50;
    static bool s_showHeadersAttendance = true;
    static int viewerWeekIndex = 0;

    ImGui::Text("Week:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    if (!weekOptions.empty() && ImGui::BeginCombo("##viewerWeekSelector", weekOptions[viewerWeekIndex].c_str())) {
        for (int i = 0; i < weekOptions.size(); i++) {
            bool isSelected = (viewerWeekIndex == i);
            if (ImGui::Selectable(weekOptions[i].c_str(), isSelected)) {
                viewerWeekIndex = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputInt("Max Rows##att", &s_maxRowsAttendance, 10, 25);
    if (s_maxRowsAttendance < 1) s_maxRowsAttendance = 1;
    if (s_maxRowsAttendance > 100) s_maxRowsAttendance = 100;

    ImGui::SameLine();
    ImGui::Checkbox("Show Headers##att", &s_showHeadersAttendance);

    const std::string viewerWeekLabel = weekOptions[viewerWeekIndex];

    struct ColumnAttendance { const char* name; };
    const std::vector<ColumnAttendance> columnsAttendance = {
        {"EMPLOYEE_ID"}, {"WEEK_START"}, {"SUN"}, {"MON"}, {"TUE"}, {"WED"}, {"THU"}, {"FRI"}, {"SAT"}
    };

    ImGui::BeginChild("AttendanceDBViewer", ImVec2(0, 0), true);

    if (s_showHeadersAttendance) {
        ImGui::Columns(static_cast<int>(columnsAttendance.size()), "AttViewerHeader");
        for (const auto& c : columnsAttendance) {
            ImGui::Text("%s", c.name);
            ImGui::NextColumn();
        }
        ImGui::Separator();
        ImGui::Columns(1);
    }

    int shownAttendance = 0;
    const auto attRows = monitor::listWeeklyAttendance(viewerWeekLabel, s_maxRowsAttendance);
    for (size_t i = 0; i < attRows.size(); ++i) {
        const auto& ar = attRows[i];
        ImGui::PushID(30000 + static_cast<int>(i));
        ImGui::Columns(static_cast<int>(columnsAttendance.size()), "AttViewerRow");
        ImGui::TextWrapped("%s", ar.employeeId.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", ar.weekStartIso.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", ar.sun.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", ar.mon.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", ar.tue.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", ar.wed.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", ar.thu.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", ar.fri.c_str()); ImGui::NextColumn();
        ImGui::TextWrapped("%s", ar.sat.c_str()); ImGui::NextColumn();
        ImGui::Columns(1);
        ImGui::PopID();
        ++shownAttendance;
    }

    ImGui::TextDisabled(shownAttendance == 0 ? "No attendance records to display." : "Showing %d attendance record(s).", shownAttendance);
    ImGui::EndChild();

    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Spacing();

    // ==============================================
    // PROJECT MANAGEMENT SECTION
    // ==============================================
    ImGui::SetWindowFontScale(1.1f);
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 0.8f, 1.0f), "Project Management");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::BeginChild("ProjectManagement", ImVec2(0, 650), true);
    ImGui::Spacing();

    static char projectID[128], projectName[16384], status[128], startDate[128], endDate[128], notes[16384];

    // Convert to strings for validation
    const std::string projectNameStr(projectName);
    const std::string statusStr(status);
    const std::string startDateStr(startDate);
    const std::string endDateStr(endDate);
    const std::string notesStr(notes);

    // Format project ID
    std::string projectIDStr;
    {
        std::string inputStr(projectID);

        if (inputStr.length() == 10 && inputStr.substr(0, 4) == "PRJ-") {
            projectIDStr = inputStr;
        } else {
            std::string digits;
            for (char ch : inputStr) {
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    digits.push_back(ch);
                }
            }

            if (!digits.empty()) {
                long long idVal = 0;
                try { idVal = std::stoll(digits); } catch (...) { idVal = 0; }
                std::ostringstream oss;
                oss << "PRJ-" << std::setw(5) << std::setfill('0') << idVal;
                projectIDStr = oss.str();
            } else {
                projectIDStr.clear();
            }
        }
    }

    // Real-time validation
    const bool projectIDValid = projectIDStr.empty() || !system::validateInput(system::ValidationType::PROJECT_ID_FORMAT, projectIDStr).empty();
    const bool projectNameValid = projectNameStr.empty() || !system::validateInput(system::ValidationType::NAME, projectNameStr).empty();
    const bool startDateValid = startDateStr.empty() || !system::validateInput(system::ValidationType::DATE_FORMAT, startDateStr).empty();
    const bool endDateValid = endDateStr.empty() || !system::validateInput(system::ValidationType::DATE_FORMAT, endDateStr).empty();

    // Project Form Fields
    ImGui::Text("Project ID:");
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(120.0f);
    if (!projectIDValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##projectID", projectID, IM_ARRAYSIZE(projectID));
    if (!projectIDValid) ImGui::PopStyleColor();

    ImGui::SameLine(270.0f);
    ImGui::Text("Project Name:");
    ImGui::SameLine(400.0f);
    ImGui::SetNextItemWidth(400.0f);
    if (!projectNameValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##projectName", projectName, IM_ARRAYSIZE(projectName));
    if (!projectNameValid) ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::Text("Status:");
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(150.0f);
    static int statusIndex = 0;
    const char* statusOptions[] = { "Active", "Completed", "On-Hold", "In Progress" };
    ImGui::Combo("##status", &statusIndex, statusOptions, IM_ARRAYSIZE(statusOptions));
    std::strncpy(status, statusOptions[statusIndex], sizeof(status) - 1);
    status[sizeof(status) - 1] = '\0';

    ImGui::SameLine(300.0f);
    ImGui::Text("Start Date:");
    ImGui::SameLine(400.0f);
    ImGui::SetNextItemWidth(200.0f);
    if (!startDateValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##startDate", startDate, IM_ARRAYSIZE(startDate));
    if (!startDateValid) ImGui::PopStyleColor();

    ImGui::SameLine(630.0f);
    ImGui::Text("End Date:");
    ImGui::SameLine(720.0f);
    ImGui::SetNextItemWidth(200.0f);
    if (!endDateValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##endDate", endDate, IM_ARRAYSIZE(endDate));
    if (!endDateValid) ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::Text("Notes:");
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(1000.0f);
    ImGui::InputText("##notes", notes, IM_ARRAYSIZE(notes));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Project Action Buttons
    if (ImGui::Button("Add New Project", ImVec2(260.0f, 40.0f))) {
        // Validate project ID format
        const std::string validatedProjectID = system::validateInput(system::ValidationType::PROJECT_ID_FORMAT, projectIDStr);
        const std::string validatedProjectName = system::validateInput(system::ValidationType::NAME, projectNameStr);
        const std::string validatedStartDate = system::validateInput(system::ValidationType::DATE_FORMAT, startDateStr);
        const std::string validatedEndDate = endDateStr.empty() ? endDateStr : system::validateInput(system::ValidationType::DATE_FORMAT, endDateStr);

        if (validatedProjectID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Project ID must follow format PRJ-##### (e.g., PRJ-00001).\n");
        } else if (validatedProjectName.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Project name is required (max 100 characters).\n");
        } else if (validatedStartDate.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Start date must follow format YYYY-MM-DD.\n");
        } else if (!endDateStr.empty() && validatedEndDate.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: End date must follow format YYYY-MM-DD if provided.\n");
        } else {
            if (monitor::addProject(validatedProjectID, validatedProjectName, statusStr, validatedStartDate, validatedEndDate, notesStr)) {
                system::logMessage(system::messageClassification::INFO, "DB: New project added successfully.\n");
                projectID[0] = '\0'; projectName[0] = '\0'; status[0] = '\0'; startDate[0] = '\0'; endDate[0] = '\0'; notes[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "DB: Failed to add new project.\n");
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Update Project", ImVec2(260.0f, 40.0f))) {
        const std::string validatedProjectID = system::validateInput(system::ValidationType::PROJECT_ID_FORMAT, projectIDStr);

        if (validatedProjectID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Project ID (PRJ-#####) is required for update.\n");
        } else {
            // Validate other fields if provided
            std::string validatedProjectName = projectNameStr.empty() ? projectNameStr : system::validateInput(system::ValidationType::NAME, projectNameStr);
            std::string validatedStartDate = startDateStr.empty() ? startDateStr : system::validateInput(system::ValidationType::DATE_FORMAT, startDateStr);
            std::string validatedEndDate = endDateStr.empty() ? endDateStr : system::validateInput(system::ValidationType::DATE_FORMAT, endDateStr);

            bool hasValidationError = false;
            if (!projectNameStr.empty() && validatedProjectName.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid project name provided.\n");
                hasValidationError = true;
            } else if (!startDateStr.empty() && validatedStartDate.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid start date format (use YYYY-MM-DD).\n");
                hasValidationError = true;
            } else if (!endDateStr.empty() && validatedEndDate.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid end date format (use YYYY-MM-DD).\n");
                hasValidationError = true;
            }

            if (!hasValidationError) {
                if (const std::string setClause = "PROJECT_NAME='" + validatedProjectName + "', STATUS='" + statusStr + "', START_DATE='" + validatedStartDate + "', END_DATE='" + validatedEndDate + "', NOTE='" + notesStr + "'";
                    db::updateDatabase(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject, validatedProjectID, setClause)) {
                    system::logMessage(system::messageClassification::INFO, "DB: Project data updated successfully.\n");
                    projectID[0] = '\0'; projectName[0] = '\0'; status[0] = '\0'; startDate[0] = '\0'; endDate[0] = '\0'; notes[0] = '\0';
                } else {
                    system::logMessage(system::messageClassification::ERROR, "DB: Failed to update project data.\n");
                }
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete Project", ImVec2(260.0f, 40.0f))) {
        const std::string validatedProjectID = system::validateInput(system::ValidationType::PROJECT_ID_FORMAT, projectIDStr);

        if (validatedProjectID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Project ID (PRJ-#####) is required for deletion.\n");
        } else {
            if (db::deleteRow(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject, validatedProjectID)) {
                system::logMessage(system::messageClassification::INFO, "DB: Project deleted successfully.\n");
                system::deleteFile(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_projectExpenseDirectory + validatedProjectID + ".db");
                projectID[0] = '\0'; projectName[0] = '\0'; status[0] = '\0'; startDate[0] = '\0'; notes[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "DB: Failed to delete project.\n");
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Load Project", ImVec2(260.0f, 40.0f))) {
        const std::string validatedProjectID = system::validateInput(system::ValidationType::PROJECT_ID_FORMAT, projectIDStr);

        if (validatedProjectID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Project ID is required to load project data.\n");
        } else {
            // Search for project in database
            const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;
            bool foundProject = false;

            for (int row = 1; row <= 1000; ++row) {
                std::string currentID = db::fetchCell(dbPath, static_cast<size_t>(row), 1);
                if (currentID.empty()) break;

                if (currentID == validatedProjectID) {
                    // Found the project - populate all fields
                    std::string prjName = db::fetchCell(dbPath, static_cast<size_t>(row), 2);
                    std::string prjStatus = db::fetchCell(dbPath, static_cast<size_t>(row), 3);
                    std::string prjStartDate = db::fetchCell(dbPath, static_cast<size_t>(row), 4);
                    std::string prjEndDate = db::fetchCell(dbPath, static_cast<size_t>(row), 5);
                    std::string prjNotes = db::fetchCell(dbPath, static_cast<size_t>(row), 6);

                    // Copy to input buffers
                    std::strncpy(projectName, prjName.c_str(), sizeof(projectName) - 1);
                    projectName[sizeof(projectName) - 1] = '\0';

                    std::strncpy(startDate, prjStartDate.c_str(), sizeof(startDate) - 1);
                    startDate[sizeof(startDate) - 1] = '\0';

                    std::strncpy(endDate, prjEndDate.c_str(), sizeof(endDate) - 1);
                    endDate[sizeof(endDate) - 1] = '\0';

                    std::strncpy(notes, prjNotes.c_str(), sizeof(notes) - 1);
                    notes[sizeof(notes) - 1] = '\0';

                    // Update status dropdown index to match loaded status
                    for (int i = 0; i < IM_ARRAYSIZE(statusOptions); ++i) {
                        if (std::string(statusOptions[i]) == prjStatus) {
                            statusIndex = i;
                            std::strncpy(status, statusOptions[i], sizeof(status) - 1);
                            status[sizeof(status) - 1] = '\0';
                            break;
                        }
                    }

                    foundProject = true;
                    system::logMessage(system::messageClassification::INFO,
                        "DB: Loaded project data for ID " + validatedProjectID + " (" + prjName + ")\n");
                    break;
                }
            }

            if (!foundProject) {
                system::logMessage(system::messageClassification::WARNING,
                    "DB: Project ID " + validatedProjectID + " not found in database.\n");
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Project Database Viewer
    ImGui::Text("Project Database Viewer");
    static int s_maxRowsProjects = 100;
    static bool s_showHeadersProjects = true;

    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputInt("Max Rows##prj", &s_maxRowsProjects, 10, 50);
    if (s_maxRowsProjects < 1) s_maxRowsProjects = 1;
    if (s_maxRowsProjects > 100) s_maxRowsProjects = 100;

    ImGui::SameLine();
    ImGui::Checkbox("Show Headers##prj", &s_showHeadersProjects);

    const std::string dbPathProjects = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;
    struct ColumnProjects { const char* name; int index; };
    const std::vector<ColumnProjects> columnsProjects = {
        {"PROJECT_ID", 1}, {"PROJECT_NAME", 2}, {"STATUS", 3}, {"START_DATE", 4}, {"END_DATE", 5}, {"NOTE", 6}
    };

    ImGui::BeginChild("ProjectDBViewer", ImVec2(0, 0), true);

    if (s_showHeadersProjects) {
        ImGui::Columns(static_cast<int>(columnsProjects.size()), "PrjViewerHeader");
        for (const auto& c : columnsProjects) {
            ImGui::Text("%s", c.name);
            ImGui::NextColumn();
        }
        ImGui::Separator();
        ImGui::Columns(1);
    }

    int shownProjects = 0;
    for (int row = 1; row <= s_maxRowsProjects; ++row) {
        if (const std::string idCell = db::fetchCell(dbPathProjects, static_cast<size_t>(row), 1); idCell.empty()) break;

        ImGui::PushID(10000 + row);
        ImGui::Columns(static_cast<int>(columnsProjects.size()), "PrjViewerRow");
        for (const auto& c : columnsProjects) {
            ImGui::TextWrapped("%s", db::fetchCell(dbPathProjects, static_cast<size_t>(row), static_cast<size_t>(c.index)).c_str());
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::PopID();
        ++shownProjects;
    }

    ImGui::TextDisabled(shownProjects == 0 ? "No projects to display." : "Showing %d project(s).", shownProjects);
    ImGui::EndChild();

    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Spacing();

    // ==============================================
    // PROJECT MATERIALS/EXPENSE SECTION
    // ==============================================
    ImGui::SetWindowFontScale(1.1f);
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 0.8f, 1.0f), "Project Materials & Expenses");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::BeginChild("MaterialsManagement", ImVec2(0, 650), true);
    ImGui::Spacing();

    static char materialProjectID[128], materialID[128], materialName[16384], materialQuantity[128], materialUnitPrice[128];

    // Convert to strings and format project ID
    const std::string materialIDStr(materialID);
    const std::string materialNameStr(materialName);
    const std::string materialQuantityStr(materialQuantity);
    const std::string materialUnitPriceStr(materialUnitPrice);

    // Format material project ID
    std::string materialProjectIDStr;
    {
        std::string inputStr(materialProjectID);

        if (inputStr.length() == 10 && inputStr.substr(0, 4) == "PRJ-") {
            materialProjectIDStr = inputStr;
        } else {
            std::string digits;
            for (char ch : inputStr) {
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    digits.push_back(ch);
                }
            }

            if (!digits.empty()) {
                long long idVal = 0;
                try { idVal = std::stoll(digits); } catch (...) { idVal = 0; }
                std::ostringstream oss; oss << std::setw(5) << std::setfill('0') << idVal;
                materialProjectIDStr = std::string("PRJ-") + oss.str();
                std::strncpy(materialProjectID, materialProjectIDStr.c_str(), IM_ARRAYSIZE(materialProjectID));
                materialProjectID[IM_ARRAYSIZE(materialProjectID)-1] = '\0';
            } else {
                materialProjectIDStr.clear();
            }
        }
    }

    // Real-time validation
    const bool matProjectIDValid = materialProjectIDStr.empty() || !system::validateInput(system::ValidationType::PROJECT_ID_FORMAT, materialProjectIDStr).empty();
    const bool matIDValid = materialIDStr.empty() || !system::validateInput(system::ValidationType::MATERIAL_ID, materialIDStr).empty();
    const bool matNameValid = materialNameStr.empty() || !system::validateInput(system::ValidationType::NAME, materialNameStr).empty();
    const bool matQuantityValid = materialQuantityStr.empty() || !system::validateInput(system::ValidationType::QUANTITY, materialQuantityStr).empty();
    const bool matPriceValid = materialUnitPriceStr.empty() || !system::validateInput(system::ValidationType::SALARY, materialUnitPriceStr).empty();

    // Materials Form Fields
    ImGui::Text("Project ID:");
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(120.0f);
    if (!matProjectIDValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##materialProjectID", materialProjectID, IM_ARRAYSIZE(materialProjectID));
    if (!matProjectIDValid) ImGui::PopStyleColor();

    ImGui::SameLine(270.0f);
    ImGui::Text("Material ID:");
    ImGui::SameLine(380.0f);
    ImGui::SetNextItemWidth(150.0f);
    if (!matIDValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##materialID", materialID, IM_ARRAYSIZE(materialID));
    if (!matIDValid) ImGui::PopStyleColor();

    ImGui::SameLine(560.0f);
    ImGui::Text("Material Name:");
    ImGui::SameLine(690.0f);
    ImGui::SetNextItemWidth(400.0f);
    if (!matNameValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##materialName", materialName, IM_ARRAYSIZE(materialName));
    if (!matNameValid) ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::Text("Quantity:");
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(150.0f);
    if (!matQuantityValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##materialQuantity", materialQuantity, IM_ARRAYSIZE(materialQuantity));
    if (!matQuantityValid) ImGui::PopStyleColor();

    ImGui::SameLine(300.0f);
    ImGui::Text("Unit Price (PHP):");
    ImGui::SameLine(450.0f);
    ImGui::SetNextItemWidth(150.0f);
    if (!matPriceValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
    ImGui::InputText("##materialUnitPrice", materialUnitPrice, IM_ARRAYSIZE(materialUnitPrice));
    if (!matPriceValid) ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Materials Action Buttons
    if (ImGui::Button("Add New Material", ImVec2(350.0f, 40.0f))) {
        const std::string validatedProjectID = system::validateInput(system::ValidationType::PROJECT_ID_FORMAT, materialProjectIDStr);
        const std::string validatedMaterialID = system::validateInput(system::ValidationType::MATERIAL_ID, materialIDStr);
        const std::string validatedMaterialName = system::validateInput(system::ValidationType::NAME, materialNameStr);
        const std::string validatedQuantity = system::validateInput(system::ValidationType::QUANTITY, materialQuantityStr);
        const std::string validatedUnitPrice = system::validateInput(system::ValidationType::SALARY, materialUnitPriceStr);

        if (validatedProjectID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Project ID (PRJ-#####) is required.\n");
        } else if (validatedMaterialID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Material ID cannot be empty.\n");
        } else if (validatedMaterialName.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Material name is required (max 100 characters).\n");
        } else if (validatedQuantity.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Quantity must be a non-negative number.\n");
        } else if (validatedUnitPrice.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Unit price must be a positive number.\n");
        } else {
            const std::string expenseDbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                                             appConfig::g_projectExpenseDirectory + validatedProjectID + ".db";
            const std::string m_data = "'" + validatedMaterialID + "', '" + validatedMaterialName + "', " + validatedQuantity + ", " + validatedUnitPrice;

            if (db::appendDatabase(expenseDbPath, m_data)) {
                system::logMessage(system::messageClassification::INFO, "DB: New material added successfully.\n");
                materialProjectID[0] = '\0'; materialID[0] = '\0'; materialName[0] = '\0';
                materialQuantity[0] = '\0'; materialUnitPrice[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "DB: Failed to add new material.\n");
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Update Material", ImVec2(350.0f, 40.0f))) {
        const std::string validatedProjectID = system::validateInput(system::ValidationType::PROJECT_ID_FORMAT, materialProjectIDStr);
        const std::string validatedMaterialID = system::validateInput(system::ValidationType::MATERIAL_ID, materialIDStr);

        if (validatedProjectID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Project ID (PRJ-#####) is required.\n");
        } else if (validatedMaterialID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Material ID cannot be empty.\n");
        } else {
            // Validate other fields if provided
            std::string validatedMaterialName = materialNameStr.empty() ? materialNameStr : system::validateInput(system::ValidationType::NAME, materialNameStr);
            std::string validatedQuantity = materialQuantityStr.empty() ? materialQuantityStr : system::validateInput(system::ValidationType::QUANTITY, materialQuantityStr);
            std::string validatedUnitPrice = materialUnitPriceStr.empty() ? materialUnitPriceStr : system::validateInput(system::ValidationType::SALARY, materialUnitPriceStr);

            bool hasValidationError = false;
            if (!materialNameStr.empty() && validatedMaterialName.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid material name provided.\n");
                hasValidationError = true;
            } else if (!materialQuantityStr.empty() && validatedQuantity.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid quantity provided.\n");
                hasValidationError = true;
            } else if (!materialUnitPriceStr.empty() && validatedUnitPrice.empty()) {
                system::logMessage(system::messageClassification::WARNING, "DB: Invalid unit price provided.\n");
                hasValidationError = true;
            }

            if (!hasValidationError) {
                const std::string expenseDbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                                                 appConfig::g_projectExpenseDirectory + validatedProjectID + ".db";
                const std::string setClause = "MATERIAL_NAME='" + validatedMaterialName + "', QUANTITY=" + validatedQuantity + ", UNIT_PRICE=" + validatedUnitPrice;

                if (db::updateDatabase(expenseDbPath, validatedMaterialID, setClause)) {
                    system::logMessage(system::messageClassification::INFO, "DB: Material data updated successfully.\n");
                    materialProjectID[0] = '\0'; materialID[0] = '\0'; materialName[0] = '\0';
                    materialQuantity[0] = '\0'; materialUnitPrice[0] = '\0';
                } else {
                    system::logMessage(system::messageClassification::ERROR, "DB: Failed to update material data.\n");
                }
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete Material", ImVec2(350.0f, 40.0f))) {
        const std::string validatedProjectID = system::validateInput(system::ValidationType::PROJECT_ID_FORMAT, materialProjectIDStr);
        const std::string validatedMaterialID = system::validateInput(system::ValidationType::MATERIAL_ID, materialIDStr);

        if (validatedProjectID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Project ID (PRJ-#####) is required for deletion.\n");
        } else if (validatedMaterialID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Material ID cannot be empty.\n");
        } else {
            const std::string expenseDbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                                             appConfig::g_projectExpenseDirectory + validatedProjectID + ".db";

            if (db::deleteRow(expenseDbPath, validatedMaterialID)) {
                system::logMessage(system::messageClassification::INFO, "DB: Material deleted successfully.\n");
                materialProjectID[0] = '\0'; materialID[0] = '\0'; materialName[0] = '\0';
                materialQuantity[0] = '\0'; materialUnitPrice[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "DB: Failed to delete material.\n");
            }
        }
    }

    if (ImGui::Button("Load Material", ImVec2(350.0f, 40.0f))) {
        const std::string validatedProjectID = system::validateInput(system::ValidationType::PROJECT_ID_FORMAT, materialProjectIDStr);
        const std::string validatedMaterialID = system::validateInput(system::ValidationType::MATERIAL_ID, materialIDStr);

        if (validatedProjectID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Project ID (PRJ-#####) is required to load material data.\n");
        } else if (validatedMaterialID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Material ID is required to load material data.\n");
        } else {
            // Search for material in database
            const std::string expenseDbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                                             appConfig::g_projectExpenseDirectory + validatedProjectID + ".db";

            if (!std::filesystem::exists(expenseDbPath)) {
                system::logMessage(system::messageClassification::WARNING, "DB: Project expense database not found for " + validatedProjectID + "\n");
            } else {
                bool foundMaterial = false;

                for (int row = 1; row <= 1000; ++row) {
                    std::string currentID = db::fetchCell(expenseDbPath, static_cast<size_t>(row), 1);
                    if (currentID.empty()) break;

                    if (currentID == validatedMaterialID) {
                        // Found the material - populate all fields
                        std::string matName = db::fetchCell(expenseDbPath, static_cast<size_t>(row), 2);
                        std::string matQuantity = db::fetchCell(expenseDbPath, static_cast<size_t>(row), 3);
                        std::string matUnitPrice = db::fetchCell(expenseDbPath, static_cast<size_t>(row), 4);

                        // Copy to input buffers
                        std::strncpy(materialName, matName.c_str(), sizeof(materialName) - 1);
                        materialName[sizeof(materialName) - 1] = '\0';

                        std::strncpy(materialQuantity, matQuantity.c_str(), sizeof(materialQuantity) - 1);
                        materialQuantity[sizeof(materialQuantity) - 1] = '\0';

                        std::strncpy(materialUnitPrice, matUnitPrice.c_str(), sizeof(materialUnitPrice) - 1);
                        materialUnitPrice[sizeof(materialUnitPrice) - 1] = '\0';

                        system::logMessage(system::messageClassification::INFO, "DB: Material data loaded successfully for " + validatedMaterialID + "\n");
                        foundMaterial = true;
                        break;
                    }
                }

                if (!foundMaterial) {
                    system::logMessage(system::messageClassification::WARNING, "DB: Material " + validatedMaterialID + " not found in project " + validatedProjectID + "\n");
                }
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Materials Database Viewer
    ImGui::Text("Materials/Expense Database Viewer");
    static int s_maxRowsMaterials = 100;
    static bool s_showHeadersMaterials = true;
    static char viewerProjectID[128];

    ImGui::Text("Project ID:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("##viewerProjectID", viewerProjectID, IM_ARRAYSIZE(viewerProjectID));

    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputInt("Max Rows##mat", &s_maxRowsMaterials, 10, 50);
    if (s_maxRowsMaterials < 1) s_maxRowsMaterials = 1;
    if (s_maxRowsMaterials > 100) s_maxRowsMaterials = 100;

    ImGui::SameLine();
    ImGui::Checkbox("Show Headers##mat", &s_showHeadersMaterials);

    // Format viewer project ID
    std::string viewerProjectIDStr;
    {
        std::string inputStr(viewerProjectID);
        if (inputStr.length() == 10 && inputStr.substr(0, 4) == "PRJ-") {
            viewerProjectIDStr = inputStr;
        } else {
            std::string digits;
            for (char ch : inputStr) {
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    digits.push_back(ch);
                }
            }
            if (!digits.empty()) {
                long long idVal = 0;
                try { idVal = std::stoll(digits); } catch (...) { idVal = 0; }
                std::ostringstream oss; oss << std::setw(5) << std::setfill('0') << idVal;
                viewerProjectIDStr = std::string("PRJ-") + oss.str();
                std::strncpy(viewerProjectID, viewerProjectIDStr.c_str(), IM_ARRAYSIZE(viewerProjectID));
                viewerProjectID[IM_ARRAYSIZE(viewerProjectID)-1] = '\0';
            } else {
                viewerProjectIDStr.clear();
            }
        }
    }

    const std::string dbPathMaterials = appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                                       appConfig::g_projectExpenseDirectory + viewerProjectIDStr + ".db";
    struct ColumnMaterials { const char* name; int index; };
    const std::vector<ColumnMaterials> columnsMaterials = {
        {"MATERIAL_ID", 1}, {"MATERIAL_NAME", 2}, {"QUANTITY", 3}, {"UNIT_PRICE", 4}
    };

    ImGui::BeginChild("MaterialsDBViewer", ImVec2(0, 0), true);

    if (s_showHeadersMaterials) {
        ImGui::Columns(static_cast<int>(columnsMaterials.size()), "MatViewerHeader");
        for (const auto& c : columnsMaterials) {
            ImGui::Text("%s", c.name);
            ImGui::NextColumn();
        }
        ImGui::Separator();
        ImGui::Columns(1);
    }

    int shownMaterials = 0;
    if (!viewerProjectIDStr.empty()) {
        for (int row = 1; row <= s_maxRowsMaterials; ++row) {
            if (const std::string idCell = db::fetchCell(dbPathMaterials, static_cast<size_t>(row), 1); idCell.empty()) break;

            ImGui::PushID(20000 + row);
            ImGui::Columns(static_cast<int>(columnsMaterials.size()), "MatViewerRow");
            for (const auto& c : columnsMaterials) {
                ImGui::TextWrapped("%s", db::fetchCell(dbPathMaterials, static_cast<size_t>(row), static_cast<size_t>(c.index)).c_str());
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::PopID();
            ++shownMaterials;
        }
    }

    if (viewerProjectIDStr.empty()) {
        ImGui::TextDisabled("Enter a Project ID to view materials");
    } else if (shownMaterials == 0) {
        ImGui::TextDisabled("No materials to display for this project");
    } else {
        ImGui::TextDisabled("Showing %d material(s)", shownMaterials);
    }

    ImGui::EndChild();

    ImGui::EndChild();
}


static void testUI() {
    ImGui::Text("THIS IS ONLY FOR TESTING/DEBUGGING PURPOSE ONLY");

    static char passkey[128] = "";
    static std::string hashedPasskey;
    static bool passkeyHashed = false;

    ImGui::Text("Passkey");
    ImGui::InputText("##passkey", passkey, IM_ARRAYSIZE(passkey));

    if (ImGui::Button("Hash Passkey")) {
        const std::string passkeyStr(passkey);
        if (!passkeyStr.empty()) {
            // Use 32 bytes (256 bits) for hash length - standard SHA-256 equivalent
            hashedPasskey = cryptography::hashKey(passkey, 32);
            passkeyHashed = true;
            std::cout << "Button clicked! Passkey: '" << passkeyStr << "'" << std::endl;
            std::cout << "Hashed result: '" << hashedPasskey << "'" << std::endl;
            std::cout << "Hash length: " << hashedPasskey.length() << std::endl;
        } else {
            passkeyHashed = false;
            hashedPasskey.clear();
            std::cout << "Button clicked but passkey is empty!" << std::endl;
        }
    }

    ImGui::Spacing();

    // Always show if the flag is set, even if hash is empty (for debugging)
    if (passkeyHashed) {
        ImGui::Text("Passkey was hashed!");
        ImGui::Spacing();

        if (!hashedPasskey.empty()) {
            const float height = ImGui::GetTextLineHeightWithSpacing() * 3;
            ImGui::BeginChild("PasskeyDisplayPanel", ImVec2(0.0f, height), true);
            ImGui::TextWrapped("Hashed Passkey:");
            ImGui::TextWrapped("%s", hashedPasskey.c_str());
            ImGui::EndChild();
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Hash result is empty! Check console output.");
        }
    }

    static int keySizeBits = 128; // Default key size
    ImGui::InputInt("Key Size (bits)", &keySizeBits, 8, 64);

    static bool keyGenerated = false;
    static std::vector<unsigned char> storedKeyRaw;
    static std::string storedKeyHashed;
    static std::string storedKeyHashedSalted;
    static int storedKeySizeBits = 0;

    if (ImGui::Button("Generate Key")) {
        storedKeyRaw = cryptography::generateKey(keySizeBits);
        storedKeyHashed = cryptography::toHex(storedKeyRaw);
        storedKeyHashedSalted = cryptography::saltKey(storedKeyHashed);
        storedKeySizeBits = keySizeBits;
        keyGenerated = true;
    }

    if (keyGenerated) {
        ImGui::Spacing();
        const float wrapWidth = ImGui::GetContentRegionAvail().x;
        const float height = ImGui::GetTextLineHeightWithSpacing() +
                             ImGui::CalcTextSize(storedKeyHashed.c_str(), nullptr, false, wrapWidth).y +
                             ImGui::CalcTextSize(storedKeyHashedSalted.c_str(), nullptr, false, wrapWidth).y;
        ImGui::BeginChild("KeyDisplayPanel", ImVec2(0, height), true);
        ImGui::Text("The key size in bits is: %d", storedKeySizeBits);
        ImGui::TextWrapped("The key in hex: %s", storedKeyHashed.c_str());
        ImGui::TextWrapped("The key in hex and salted: %s", storedKeyHashedSalted.c_str());
        ImGui::EndChild();
    }

    // Test encryption and decryption
    static const std::string location = "test.txt";
    static std::string encryptionStatus;
    static std::string decryptionStatus;

    ImGui::Spacing();
    ImGui::Text("Encryption/Decryption Test");

    if (ImGui::Button("Encrypt")) {
        // Ensure we have a key; generate with the user-selected size if missing
        if (!keyGenerated || storedKeyRaw.empty()) {
            storedKeyRaw = cryptography::generateKey(keySizeBits);
            storedKeyHashed = cryptography::toHex(storedKeyRaw);
            storedKeyHashedSalted = cryptography::saltKey(storedKeyHashed);
            storedKeySizeBits = keySizeBits;
            keyGenerated = true;
        }

        if (storedKeyRaw.empty()) {
            encryptionStatus = "ERROR: Key generation failed. Cannot encrypt.";
        } else {
            if (cryptography::encryptFile(location, storedKeyRaw)) {
                encryptionStatus = "SUCCESS: File encrypted to " + location + ".enc";
                system::deleteFile(location); // Delete the unencrypted file after successful encryption
            } else {
                encryptionStatus = "ERROR: Encryption failed. Check if file exists and key is correct.";
            }
        }
    }

    if (ImGui::Button("Decrypt")) {
        if (!keyGenerated || storedKeyRaw.empty()) {
            decryptionStatus = "ERROR: No key available. Generate or load a key before decrypting.";
        } else {
            if (cryptography::decryptFile(location + ".enc", storedKeyRaw)) {
                decryptionStatus = "SUCCESS: File decrypted (replaced " + location + ".enc with decrypted content)";
                system::deleteFile(location + ".enc"); // Delete the encrypted file after successful decryption
            } else {
                decryptionStatus = "ERROR: Decryption failed. Check if encrypted file exists and key is correct.";
            }
        }
    }

    if (!encryptionStatus.empty()) {
        ImGui::Spacing();
        if (encryptionStatus.find("SUCCESS") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", encryptionStatus.c_str());
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", encryptionStatus.c_str());
        }
    }

    if (!decryptionStatus.empty()) {
        ImGui::Spacing();
        if (decryptionStatus.find("SUCCESS") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", decryptionStatus.c_str());
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", decryptionStatus.c_str());
        }
    }
}


// Main two-column layout: left = selector (with an account), right = active panel (summary/payroll/monitor)
static void mainUI() {
    // Only set a default right panel on the first call, not every frame
    static bool initialized = false;
    if (!initialized) {
        if (appConfig::g_testMode) {
            g_rightUI = testUI; // default right panel if in test mode
        } else {
            g_rightUI = summaryUI; // default right panel if not in test mode
        }
        initialized = true;
    }

    // Full-viewport, borderless root, so the main UI is in the app window itself
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // Use work area to avoid overlapping HelloImGui menu/dock space areas
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    constexpr ImGuiWindowFlags rootFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoDocking; // to prevent docking overlay from capturing inputs,
    // Nudge focus to our root window for the first frames to ensure interactivity
    static int s_focusFrames = 30;
    if (s_focusFrames > 0) { ImGui::SetNextWindowFocus(); --s_focusFrames; }
    ImGui::Begin("##MainRoot", nullptr, rootFlags);

    const float total = ImGui::GetContentRegionAvail().x;
    const float leftWidth = std::clamp(total * 0.18f, 220.0f, 480.0f);
    // Left pane: fixed-width child so it does not move when resizing
    ImGui::BeginChild("LeftPane", ImVec2(leftWidth, 0), true);
    selectorUI();
    ImGui::EndChild();

    ImGui::SameLine();

    // Right pane: fills remaining width
    ImGui::BeginChild("RightPane", ImVec2(0, 0), true);
    if (g_rightUI) g_rightUI();
    ImGui::EndChild();

    ImGui::End();
}


void ui::constructUI(const std::string &a_title, const std::string& a_fontLocation, const int a_widthPx, const int a_heightPx, const std::string& a_window) {
    HelloImGui::RunnerParams params;

    namespace fs = std::filesystem;
    for (const auto &base : {fs::current_path(), fs::path(__FILE__).parent_path().parent_path().parent_path()}) {
        if (const fs::path assets = base / "assets"; fs::exists(assets)) {
            HelloImGui::SetAssetsFolder(assets.string());
            break;
        }
    }

    params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenWindow;

    // populate the UI registry (ensure it's available before selecting the current UI)
    g_uiMap.clear();
    g_uiMap.reserve(7);
    g_uiMap["auth"] = loginUI;
    g_uiMap["main"] = mainUI;
    g_uiMap["summary"] = summaryUI;
    g_uiMap["payroll"] = payrollUI;
    g_uiMap["monitor"] = monitorUI;
    g_uiMap["test"] = testUI;
    g_uiMap["failed"] = failedUI;

    params.callbacks.LoadAdditionalFonts = [a_fontLocation]() {
        const ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        const std::string fontPath = HelloImGui::AssetFileFullPath(a_fontLocation);

        ImFontConfig fontConfig;
        fontConfig.GlyphRanges = io.Fonts->GetGlyphRangesDefault();

        if (!fontPath.empty()) {
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 22.0f, &fontConfig);
        } else {
            io.Fonts->AddFontDefault(&fontConfig);
        }
    };

    params.callbacks.PostInit = [&params]() {
        std::string iconPath = HelloImGui::AssetFileFullPath("icons/app_icon.png");
        if (iconPath.empty()) iconPath = HelloImGui::AssetFileFullPath("icons/business_logo.png");
        if (iconPath.empty()) return;

        int w = 0, h = 0, comps = 0;
        unsigned char* pixels = stbi_load(iconPath.c_str(), &w, &h, &comps, 4);
        if (!pixels) return;

        auto bp = params.backendPointers;

    #ifdef UI_HAVE_GLFW
        if (bp.glfwWindow) {
            GLFWimage img;
            img.width = w;
            img.height = h;
            img.pixels = pixels;
            glfwSetWindowIcon(static_cast<GLFWwindow*>(bp.glfwWindow), 1, &img);
            stbi_image_free(pixels);
            return;
        }
#endif

#ifdef UI_HAVE_SDL
        if (bp.sdlWindow) {
            SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
                pixels, w, h, 32, w * 4, SDL_PIXELFORMAT_RGBA32);
            if (surf) {
                SDL_SetWindowIcon(reinterpret_cast<SDL_Window*>(bp.sdlWindow), surf);
                SDL_FreeSurface(surf);
            }
            stbi_image_free(pixels);
            return;
        }
#endif

        stbi_image_free(pixels);
    };

    if (const std::string startKey = a_window.empty() ? "main" : toLower(a_window); startKey == "auth") {
        g_currentUI = loginUI;
    }
    else if (startKey == "payroll") {
        if (g_uiMap.contains("payroll")) g_rightUI = g_uiMap["payroll"]; else g_rightUI = payrollUI;
        g_currentUI = mainUI;
    }
    else if (startKey == "monitor") {
        if (g_uiMap.contains("monitor")) g_rightUI = g_uiMap["monitor"]; else g_rightUI = monitorUI;
        g_currentUI = mainUI;
    }
    else if (startKey == "test") {
        if (g_uiMap.contains("test")) g_rightUI = g_uiMap["test"]; else g_rightUI = testUI;
        g_currentUI = mainUI;
    }
    else if (startKey == "summary" || startKey == "main") {
        if (g_uiMap.contains("summary")) g_rightUI = g_uiMap["summary"]; else g_rightUI = summaryUI;
        g_currentUI = mainUI;
    }

    params.callbacks.ShowGui = []() {if (g_currentUI) g_currentUI();};

    const int l_clampedWidth = std::clamp(a_widthPx, 50, 3840);
    const int l_clampedLength = std::clamp(a_heightPx, 50, 2160);
    params.appWindowParams.windowGeometry.size = { l_clampedWidth, l_clampedLength };
    params.appWindowParams.windowTitle = a_title.empty() ? "system" : a_title;

    HelloImGui::Run(params);
}

