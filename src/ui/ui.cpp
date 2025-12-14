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
 */

#include <string>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "ui.h"

#include "hello_imgui/hello_imgui.h"
#include "../handler/db.h"
#include "../handler/system.h"
#include "../config/config.h"
#include "../security/cryptography.h"
#include "../security/auth.h"

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


// forward-declare the exporter to avoid including implementation here
void exportPayslipsHtml(const std::string& outFile, const std::string& logoPath);


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
    std::ranges::transform(s, s.begin(), [](const unsigned char c){ return std::tolower(c); });
    return s;
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

            ui::g_userName = std::string(username);
            ui::g_position = "";

            if (auto* params = HelloImGui::GetRunnerParams()) {
                params->appShallExit = true;
            }
        }
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
        appConfig::g_auth = false;
        appConfig::g_testMode = false;
        ui::g_userName = "";
        ui::g_position = "";
        // Force the runner to use the login window size on next run, then request the current Run to exit.
        if (auto* params = HelloImGui::GetRunnerParams()) {
            params->appWindowParams.windowGeometry.size = { std::clamp(appConfig::g_loginWidth, 50, 3840), std::clamp(appConfig::g_loginHeight, 50, 2160) };
            // Ensure the runner applies the resize when it exits the current Run
            params->appWindowParams.windowGeometry.resizeAppWindowAtNextFrame = true;
            // Request app to quit so HelloImGui::Run returns and we can restart at login
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
    ImGui::SetCursorPos(ImVec2(40.0f, 135.0f));
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
    ImGui::Text("Summary Panel");
}


static void payrollUI() {
    ImGui::Text("Payroll Management");
    // ImGui::Separator();
    // ImGui::Spacing();
    //
    // // Load employees from the database
    // auto employees = db::getEmployees();
    //
    // // Create a table layout
    // ImGui::Columns(4, "employee_table");
    // ImGui::Text("ID"); ImGui::NextColumn();
    // ImGui::Text("Name"); ImGui::NextColumn();
    // ImGui::Text("Hourly Rate"); ImGui::NextColumn();
    // ImGui::Text("Hours Worked"); ImGui::NextColumn();
    // ImGui::Separator();
    //
    // for (auto& emp : employees) {
    //     ImGui::Text("%d", emp.id); ImGui::NextColumn();
    //     ImGui::Text("%s", emp.name.c_str()); ImGui::NextColumn();
    //     ImGui::Text("%.2f", emp.hourlyRate); ImGui::NextColumn();
    //     ImGui::Text("%.2f", emp.hoursWorked); ImGui::NextColumn();
    // }
    //
    // ImGui::Columns(1);
}


static void monitorUI() {
    static char name[128], position[128], employeeID[128], location[128], salary[128], hoursWorked[128], advance[128];
    constexpr float yAxis = 95.0f;
    constexpr float textboxWidth = 300.0f;

    ImGui::SetWindowFontScale(1.7f);
    ImGui::Text("Monitoring Management");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Text("Employee Information");

    ImGui::SetCursorPos(ImVec2(10.0f, yAxis));
    ImGui::Text("Name:");
    ImGui::SetCursorPos(ImVec2(65.0f, yAxis));
    ImGui::SetNextItemWidth(textboxWidth + 200.0f);
    ImGui::InputText("##name", name, IM_ARRAYSIZE(name));

    ImGui::SetCursorPos(ImVec2(580.0f, yAxis));
    ImGui::Text("Position:");
    ImGui::SetCursorPos(ImVec2(650.0f, yAxis));
    ImGui::SetNextItemWidth(textboxWidth);
    ImGui::InputText("##position", position, IM_ARRAYSIZE(position));

    ImGui::SetCursorPos(ImVec2(975.0f, yAxis));
    ImGui::Text("Employee ID:");
    ImGui::SetCursorPos(ImVec2(1080.0f, yAxis));
    ImGui::SetNextItemWidth(textboxWidth - 110.0f);
    ImGui::InputText("##employeeID", employeeID, IM_ARRAYSIZE(employeeID));

    ImGui::SetCursorPos(ImVec2(10.0, yAxis + 40.0f));
    ImGui::Text("Location / Site:");
    ImGui::SetCursorPos(ImVec2(130.0f, yAxis + 40.0f));
    ImGui::SetNextItemWidth(textboxWidth);
    ImGui::InputText("##location", location, IM_ARRAYSIZE(location));

    ImGui::SetCursorPos(ImVec2(445.0f, yAxis + 40.0f));
    ImGui::Text("Salary (per Hour):");
    ImGui::SetCursorPos(ImVec2(585.0f, yAxis + 40.0f));
    ImGui::SetNextItemWidth(textboxWidth - 200.0f);
    ImGui::InputText("##salary", salary, IM_ARRAYSIZE(salary));

    ImGui::SetCursorPos(ImVec2(700.0f, yAxis + 40.0f));
    ImGui::Text("Hours Worked:");
    ImGui::SetCursorPos(ImVec2(820.0f, yAxis + 40.0f));
    ImGui::SetNextItemWidth(textboxWidth - 200.0f);
    ImGui::InputText("##hoursWorked", hoursWorked, IM_ARRAYSIZE(hoursWorked));

    ImGui::SetCursorPos(ImVec2(940.0f, yAxis + 40.0f));
    ImGui::Text("Advance:");
    ImGui::SetCursorPos(ImVec2(1015.0f, yAxis + 40.0f));
    ImGui::SetNextItemWidth(textboxWidth - 120.0f);
    ImGui::InputText("##advance", advance, IM_ARRAYSIZE(advance));

    const std::string nameStr(name);
    const std::string positionStr(position);
    const std::string employeeIDStr(employeeID);
    const std::string locationStr(location);
    const std::string salaryStr(salary);
    const std::string hoursWorkedStr(hoursWorked);
    const std::string advanceStr(advance);

    ImGui::SetCursorPos(ImVec2(25.0f, yAxis + 90.0f));
    if (ImGui::Button("Add New Employee", ImVec2(400.0f, 100.0f))) {
        if (db::appendDatabase(appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll, "'" + nameStr + "', '" + positionStr + "', '" + locationStr + "', " + salaryStr + ", " + hoursWorkedStr + ", " + advanceStr)) {
            system::logMessage(system::messageClassification::INFO, "DB Test: New employee added successfully.\n");
            name[0] = '\0';
            position[0] = '\0';
            employeeID[0] = '\0';
            location[0] = '\0';
            salary[0] = '\0';
            hoursWorked[0] = '\0';
            advance[0] = '\0';
        } else {
            system::logMessage(system::messageClassification::INFO, "DB Test: Failed to add new employee.\n");
        }
    }

    ImGui::SetCursorPos(ImVec2(447.5f, yAxis + 90.0f));
    if (ImGui::Button("Delete Employee", ImVec2(400.0f, 100.0f))) {
        if (db::deleteRow(appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll, employeeIDStr)) {
            system::logMessage(system::messageClassification::INFO, "DB Test: Employee deleted successfully.\n");
            employeeID[0] = '\0';
        } else {
            system::logMessage(system::messageClassification::ERROR, "DB Test: Failed to delete employee.\n");
        }
    }

    ImGui::SetCursorPos(ImVec2(870.0f, yAxis + 90.0f));
    if (ImGui::Button("Change Data (Employee)", ImVec2(400.0f, 100.0f))) {
        if (const std::string setClause = "NAME='" + nameStr + "', 'POSITION='" + positionStr + "', SITE_LOCATION='" + locationStr + "', SALARY=" + salaryStr + ", HOURS_WORK=" + hoursWorkedStr + ", ADVANCE=" + advanceStr;
            db::updateDatabase(appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll, employeeIDStr, setClause)) {
            system::logMessage(system::messageClassification::INFO, "DB Test: Data updated successfully.\n");
            name[0] = '\0';
            position[0] = '\0';
            employeeID[0] = '\0';
            location[0] = '\0';
            salary[0] = '\0';
            hoursWorked[0] = '\0';
            advance[0] = '\0';
        } else {
            system::logMessage(system::messageClassification::ERROR, "DB Test: Failed to update data.\n");
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Text("Employee Raw Database Viewer");

    // Options (employees-only DB viewer)
    static int s_maxRowsEmployee = 100; // max rows to display
    static bool s_showHeadersEmployee = true;

    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 230.0f));
    ImGui::SetNextItemWidth(110.0f);
    ImGui::InputInt("Rows (max at 100) (for Employee)", &s_maxRowsEmployee, 1, 5);
    if (s_maxRowsEmployee < 1) s_maxRowsEmployee = 1;
    if (s_maxRowsEmployee > 100) s_maxRowsEmployee = 100;

    ImGui::SetCursorPos(ImVec2(390.0f, yAxis + 230.0f));
    ImGui::Checkbox("Show Headers (Employee)", &s_showHeadersEmployee);

    ImGui::Spacing();

    // Resolve DB path and table schema for the employee dataset only
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    struct Column { const char* name; int index; };
    const std::vector<Column> columns = {
        {"EMPLOYEE_ID", 1}, {"NAME", 2}, {"POSITION", 3}, {"SITE_LOCATION", 4},
        {"SALARY", 5}, {"HOURS_WORK", 6}, {"ADVANCE", 7}
        };

    ImGui::BeginChild("DBViewer", ImVec2(0, 300), true);

    if (s_showHeadersEmployee) {
        ImGui::Columns(static_cast<int>(columns.size()), "DBViewer_HeaderCols");
        for (const auto& c : columns) {
            ImGui::Text("%s", c.name);
            ImGui::NextColumn();
        }
        ImGui::Separator();
        ImGui::Columns(1);
    }

    int shownEmployee = 0;
    for (int row = 1; row <= s_maxRowsEmployee; ++row) {
        if (const std::string idCell = db::fetchCell(dbPath, static_cast<size_t>(row), 1); idCell.empty()) break;

        ImGui::PushID(row);
        ImGui::Columns(static_cast<int>(columns.size()), "DBViewer_RowCols");
        for (const auto& c : columns) {
            ImGui::TextWrapped("%s", db::fetchCell(dbPath, static_cast<size_t>(row), static_cast<size_t>(c.index)).c_str());
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::PopID();
        ++shownEmployee;
    }

    ImGui::TextDisabled(shownEmployee == 0 ? "No rows to display." : "Showing %d row(s).", shownEmployee);
    ImGui::EndChild();



    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Text("Employee Weekly Attendance (Sunday - Saturday)");

    static char attendanceEmployeeID[128];
    static char sunHours[128], monHours[128], tueHours[128], wedHours[128], thuHours[128], friHours[128], satHours[128];
    static int selectedWeekIndex = 0;

    // Generate week options for the current year (2025)
    static std::vector<std::string> weekOptions;
    static std::vector<std::string> weekDates; // Store actual date ranges
    if (weekOptions.empty()) {
        // Generate all weeks for 2025 (52-53 weeks)
        for (int month = 1; month <= 12; month++) {
            // Simplified: generate weeks by month (4 weeks per month approx)
            int daysInMonth = 31;
            if (month == 2) daysInMonth = 28;
            else if (month == 4 || month == 6 || month == 9 || month == 11) daysInMonth = 30;

            for (int day = 1; day <= daysInMonth; day += 7) {
                int endDay = std::min(day + 6, daysInMonth);
                std::ostringstream label, dateRange;
                label << std::setfill('0') << std::setw(2) << month << "/"
                      << std::setw(2) << day << "-" << std::setw(2) << endDay;
                dateRange << "2025-" << std::setfill('0') << std::setw(2) << month
                         << "-" << std::setw(2) << day;
                weekOptions.push_back(label.str());
                weekDates.push_back(dateRange.str());
            }
        }
    }

    ImGui::Text("Employee ID:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputText("##attendanceEmployeeID", attendanceEmployeeID, IM_ARRAYSIZE(attendanceEmployeeID));

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

    ImGui::Text("Day:");
    ImGui::SameLine();
    static int selectedDayForHours = 0;
    const char* dayLabels[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    if (ImGui::BeginCombo("##daySelector", dayLabels[selectedDayForHours])) {
        for (int i = 0; i < 7; ++i) {
            bool isSelected = (selectedDayForHours == i);
            if (ImGui::Selectable(dayLabels[i], isSelected)) selectedDayForHours = i;
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::Text(" Hours:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80.0f);

    // Map the per-day buffers into an array so the selected day's buffer is edited by the single textbox.
    static char* dayBuffers[] = { sunHours, monHours, tueHours, wedHours, thuHours, friHours, satHours };
    ImGui::InputText("##dayHours", dayBuffers[selectedDayForHours], IM_ARRAYSIZE(sunHours));

    const std::string attendanceEmpIDStr(attendanceEmployeeID);
    const std::string weekStartStr = weekDates[selectedWeekIndex];
    const std::string weekLabel = weekOptions[selectedWeekIndex];
    const std::string sunHoursStr(sunHours);
    const std::string monHoursStr(monHours);
    const std::string tueHoursStr(tueHours);
    const std::string wedHoursStr(wedHours);
    const std::string thuHoursStr(thuHours);
    const std::string friHoursStr(friHours);
    const std::string satHoursStr(satHours);

    // Format employee ID as EMP-00001
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
    if (ImGui::Button("Add Weekly Attendance", ImVec2(200.0f, 35.0f))) {
        if (!formattedEmpID.empty() && !weekStartStr.empty()) {
            // Attendance DB path: data/payroll/2025/12/01-07.db
            const int currentYear = 2025; // Current year
            const std::string attendanceDbPath = appConfig::g_dataDirectory +
                                                appConfig::g_payrollDirectory +
                                                std::to_string(currentYear) + "/" +
                                                weekLabel + ".db";
            const std::string a_data = "'" + formattedEmpID + "', '" + weekStartStr + "', " + sunHoursStr + ", " + monHoursStr + ", " +
                                      tueHoursStr + ", " + wedHoursStr + ", " + thuHoursStr + ", " +
                                      friHoursStr + ", " + satHoursStr;

            if (db::appendDatabase(attendanceDbPath, a_data)) {
                system::logMessage(system::messageClassification::INFO, "Attendance: Weekly attendance added successfully.\n");
                attendanceEmployeeID[0] = '\0';
                sunHours[0] = '\0';
                monHours[0] = '\0';
                tueHours[0] = '\0';
                wedHours[0] = '\0';
                thuHours[0] = '\0';
                friHours[0] = '\0';
                satHours[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::INFO, "Attendance: Failed to add weekly attendance.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "Attendance: Employee ID and Week are required.\n");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete Week Attendance", ImVec2(200.0f, 35.0f))) {
        if (!formattedEmpID.empty() && !weekStartStr.empty()) {
            const int currentYear = 2025;
            const std::string attendanceDbPath = appConfig::g_dataDirectory +
                                                appConfig::g_payrollDirectory +
                                                std::to_string(currentYear) + "/" +
                                                weekLabel + ".db";
            if (db::deleteRow(attendanceDbPath, formattedEmpID)) {
                system::logMessage(system::messageClassification::INFO, "Attendance: Week attendance deleted successfully.\n");
                attendanceEmployeeID[0] = '\0';
                sunHours[0] = '\0';
                monHours[0] = '\0';
                tueHours[0] = '\0';
                wedHours[0] = '\0';
                thuHours[0] = '\0';
                friHours[0] = '\0';
                satHours[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "Attendance: Failed to delete week attendance.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "Attendance: Employee ID and Week are required.\n");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Update Week Attendance", ImVec2(200.0f, 35.0f))) {
        if (!formattedEmpID.empty() && !weekStartStr.empty()) {
            const int currentYear = 2025;
            const std::string attendanceDbPath = appConfig::g_dataDirectory +
                                                appConfig::g_payrollDirectory +
                                                std::to_string(currentYear) + "/" +
                                                weekLabel + ".db";
            const std::string setClause = "SUN=" + sunHoursStr + ", MON=" + monHoursStr + ", TUE=" + tueHoursStr +
                                         ", WED=" + wedHoursStr + ", THU=" + thuHoursStr + ", FRI=" + friHoursStr +
                                         ", SAT=" + satHoursStr;
            if (db::updateDatabase(attendanceDbPath, formattedEmpID, setClause)) {
                system::logMessage(system::messageClassification::INFO, "Attendance: Week attendance updated successfully.\n");
                attendanceEmployeeID[0] = '\0';
                sunHours[0] = '\0';
                monHours[0] = '\0';
                tueHours[0] = '\0';
                wedHours[0] = '\0';
                thuHours[0] = '\0';
                friHours[0] = '\0';
                satHours[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "Attendance: Failed to update week attendance.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "Attendance: Employee ID and Week are required.\n");
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Text("Weekly Attendance Viewer");

    static int s_maxRowsAttendance = 50;
    static bool s_showHeadersAttendance = true;
    static int viewerWeekIndex = 0;

    ImGui::Text("Week:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    if (ImGui::BeginCombo("##viewerWeekSelector", weekOptions[viewerWeekIndex].c_str())) {
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
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputInt("Max Rows", &s_maxRowsAttendance, 5, 25);
    if (s_maxRowsAttendance < 1) s_maxRowsAttendance = 1;
    if (s_maxRowsAttendance > 100) s_maxRowsAttendance = 100;

    ImGui::SameLine();
    ImGui::Checkbox("Show Headers", &s_showHeadersAttendance);

    const std::string viewerWeekLabel = weekOptions[viewerWeekIndex];
    const int currentYear = 2025;
    const std::string dbPathAttendance = appConfig::g_dataDirectory +
                                        appConfig::g_payrollDirectory +
                                        std::to_string(currentYear) + "/" +
                                        viewerWeekLabel + ".db";

    struct ColumnAttendance { const char* name; int index; };
    const std::vector<ColumnAttendance> columnsAttendance = {
        {"EMPLOYEE_ID", 1}, {"WEEK_START", 2}, {"SUN", 3}, {"MON", 4}, {"TUE", 5}, {"WED", 6}, {"THU", 7}, {"FRI", 8}, {"SAT", 9}
    };

    ImGui::BeginChild("DBViewerAttendance", ImVec2(0, 250), true);

    if (s_showHeadersAttendance) {
        ImGui::Columns(static_cast<int>(columnsAttendance.size()), "DBViewer_HeaderCols_Attendance");
        for (const auto& c : columnsAttendance) {
            ImGui::Text("%s", c.name);
            ImGui::NextColumn();
        }
        ImGui::Separator();
        ImGui::Columns(1);
    }

    int shownAttendance = 0;
    for (int row = 1; row <= s_maxRowsAttendance; ++row) {
        if (const std::string idCell = db::fetchCell(dbPathAttendance, static_cast<size_t>(row), 1); idCell.empty()) break;

        ImGui::PushID(30000 + row);
        ImGui::Columns(static_cast<int>(columnsAttendance.size()), "DBViewer_RowCols_Attendance");
        for (const auto& c : columnsAttendance) {
            ImGui::TextWrapped("%s", db::fetchCell(dbPathAttendance, static_cast<size_t>(row), static_cast<size_t>(c.index)).c_str());
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::PopID();
        ++shownAttendance;
    }

    ImGui::TextDisabled(shownAttendance == 0 ? "No rows to display." : "Showing %d row(s).", shownAttendance);

    ImGui::EndChild();




    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    static char projectID[128], projectName[16384], status[128], startDate[128], notes[16384];

    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 1060.0f));
    ImGui::Text("Project List Information");

    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 1090.0f));
    ImGui::Text("Project ID:");
    ImGui::SetCursorPos(ImVec2(100.0f, yAxis + 1090.0f));
    ImGui::SetNextItemWidth(textboxWidth - 240.0f);
    ImGui::InputText("##projectID", projectID, IM_ARRAYSIZE(projectID));

    ImGui::SetCursorPos(ImVec2(180.0f, yAxis + 1090.0f));
    ImGui::Text("Project Name:");
    ImGui::SetCursorPos(ImVec2(295.0f, yAxis + 1090.0f));
    ImGui::SetNextItemWidth(textboxWidth + 200.0f);
    ImGui::InputText("##projectName", projectName, IM_ARRAYSIZE(projectName));

    ImGui::SetCursorPos(ImVec2(810.0f, yAxis + 1090.0f));
    ImGui::Text("Status:");
    ImGui::SetCursorPos(ImVec2(870.0f, yAxis + 1090.0f));
    ImGui::SetNextItemWidth(textboxWidth - 180.0f);
    static int statusIndex = 0;
    const char* statusOptions[] = { "Active", "Completed", "On-Hold", "In Progress" };
    ImGui::Combo("##status", &statusIndex, statusOptions, IM_ARRAYSIZE(statusOptions));
    std::strncpy(status, statusOptions[statusIndex], sizeof(status) - 1);
    status[sizeof(status) - 1] = '\0';

    ImGui::SetCursorPos(ImVec2(1000.0f, yAxis + 1090.0f));
    ImGui::Text("Start Date: ");
    ImGui::SetCursorPos(ImVec2(1090.0f, yAxis + 1090.0f));
    ImGui::SetNextItemWidth(textboxWidth - 120.0f);
    ImGui::InputText("##startDate", startDate, IM_ARRAYSIZE(startDate));

    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 1130.0f));
    ImGui::Text("Notes:");
    ImGui::SetCursorPos(ImVec2(70.0f, yAxis + 1130.0f));
    ImGui::SetNextItemWidth(textboxWidth + 900.0f);
    ImGui::InputText("##notes", notes, IM_ARRAYSIZE(notes));

    // projectID handling is done below (formatted as PRJ-xxxxx). Keep other fields.
    const std::string projectNameStr(projectName);
    const std::string statusStr(status);
    const std::string startDateStr(startDate);
    const std::string notesStr(notes);

    // Enforce display format: PRJ-xxxxx (five digits). Extract digits from input, normalize
    std::string projectIDStr; // displayed and stored (PRJ-00001)
    {
        std::string inputStr(projectID);

        // Check if input already has correct PRJ-xxxxx format (to avoid re-formatting every frame)
        if (inputStr.length() == 10 && inputStr.substr(0, 4) == "PRJ-") {
            // Already formatted correctly, use as-is
            projectIDStr = inputStr;
        } else {
            // Extract only digits from input
            std::string digits;
            for (char ch : inputStr) {
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    digits.push_back(ch);
                }
            }

            if (!digits.empty()) {
                // normalize to integer value (remove stray leading zeros) then format for display
                long long idVal = 0;
                try { idVal = std::stoll(digits); } catch (...) { idVal = 0; }
                // display and DB: zero-padded to 5 digits with PRJ- prefix
                std::ostringstream oss; oss << std::setw(5) << std::setfill('0') << idVal;
                projectIDStr = std::string("PRJ-") + oss.str();
                // Write formatted ID back to the input buffer so user sees PRJ-xxxxx
                std::strncpy(projectID, projectIDStr.c_str(), IM_ARRAYSIZE(projectID));
                projectID[IM_ARRAYSIZE(projectID)-1] = '\0';
            } else {
                projectIDStr.clear();
            }
        }
    }

    ImGui::SetCursorPos(ImVec2(25.0f, yAxis + 1170.0f));
    if (ImGui::Button("Add New Project", ImVec2(400.0f, 100.0f))) {
        // Append project fields including PROJECT_ID: PROJECT_ID, PROJECT_NAME, STATUS, START_DATE, NOTE
        if (!projectIDStr.empty()) {
            const std::string p_data = "'" + projectIDStr + "', '" + projectNameStr + "', '" + statusStr + "', '" + startDateStr + "', '" + notesStr + "'";
            if (db::appendDatabase(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject, p_data)) {
                system::logMessage(system::messageClassification::INFO, "DB Test: New project added successfully.\n");
                db::createDatabase(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject + appConfig::g_projectExpenseDirectory + projectIDStr + ".db");
                projectID[0] = '\0';
                projectName[0] = '\0';
                status[0] = '\0';
                startDate[0] = '\0';
                notes[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::INFO, "DB Test: Failed to add new project.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "DB Test: Project ID is required.\n");
        }
    }

    ImGui::SetCursorPos(ImVec2(447.5f, yAxis + 1170.0f));
    if (ImGui::Button("Delete Project", ImVec2(400.0f, 100.0f))) {
        if (!projectIDStr.empty()) {
            if (db::deleteRow(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject, projectIDStr)) {
                system::logMessage(system::messageClassification::INFO, "DB Test: Project deleted successfully.\n");
                system::deleteFile(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject + appConfig::g_projectExpenseDirectory + projectIDStr + ".db");
                projectID[0] = '\0';
                projectName[0] = '\0';
                status[0] = '\0';
                startDate[0] = '\0';
                notes[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "DB Test: Failed to delete project.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "DB Test: Project ID is required.\n");
        }
    }

    ImGui::SetCursorPos(ImVec2(870.0f, yAxis + 1170.0f));
    if (ImGui::Button("Change Data (Project)", ImVec2(400.0f, 100.0f))) {
        if (!projectIDStr.empty()) {
            if (const std::string setClause = "PROJECT_NAME='" + projectNameStr + "', STATUS='" + statusStr + "', START_DATE='" + startDateStr + "', NOTE='" + notesStr + "'";
                db::updateDatabase(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject, projectIDStr, setClause)) {
                system::logMessage(system::messageClassification::INFO, "DB Test: Project data updated successfully.\n");
                projectID[0] = '\0';
                projectName[0] = '\0';
                status[0] = '\0';
                startDate[0] = '\0';
                notes[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "DB Test: Failed to update project data.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "DB Test: Project ID is required.\n");
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 1280.0f));
    ImGui::Text("Project Raw Database Viewer");

    // Options (projects-only DB viewer)
    static int s_maxRowsProjects = 100;
    static bool s_showHeadersProjects = true;

    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 1310.0f));
    ImGui::SetNextItemWidth(110.0f);
    ImGui::InputInt("Rows (max at 100) (for Projects)", &s_maxRowsProjects, 1, 5);
    if (s_maxRowsProjects < 1) s_maxRowsProjects = 1;
    if (s_maxRowsProjects > 100) s_maxRowsProjects = 100;

    ImGui::SetCursorPos(ImVec2(390.0f, yAxis + 1310.0f));
    ImGui::Checkbox("Show Headers (Projects)", &s_showHeadersProjects);

    ImGui::Spacing();

    // Resolve DB path and table schema for the project dataset only
    const std::string dbPathProjects = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;
    struct ColumnProjects { const char* name; int index; };
    const std::vector<Column> columnsProjects = {
        {"PROJECT_ID", 1}, {"PROJECT_NAME", 2}, {"STATUS", 3}, {"START_DATE", 4}, {"NOTE", 5}
        };

    ImGui::BeginChild("DBViewerProjects", ImVec2(0, 300), true);

    if (s_showHeadersProjects) {
        ImGui::Columns(static_cast<int>(columnsProjects.size()), "DBViewer_HeaderCols_Projects");
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

        ImGui::PushID(10000 + row); // Unique ID offset for projects
        ImGui::Columns(static_cast<int>(columnsProjects.size()), "DBViewer_RowCols_Projects");
        for (const auto& c : columnsProjects) {
            ImGui::TextWrapped("%s", db::fetchCell(dbPathProjects, static_cast<size_t>(row), static_cast<size_t>(c.index)).c_str());
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::PopID();
        ++shownProjects;
    }

    ImGui::TextDisabled(shownProjects == 0 ? "No rows to display." : "Showing %d row(s).", shownProjects);
    ImGui::EndChild();

    static char materialProjectID[128], materialID[128], materialName[16384], materialQuantity[128], materialUnitPrice[128];

    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 1660.0f));
    ImGui::Text("Project Materials / Expense Information");

    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 1690.0f));
    ImGui::Text("Project ID:");
    ImGui::SetCursorPos(ImVec2(100.0f, yAxis + 1690.0f));
    ImGui::SetNextItemWidth(textboxWidth - 240.0f);
    ImGui::InputText("##materialProjectID", materialProjectID, IM_ARRAYSIZE(materialProjectID));

    ImGui::SetCursorPos(ImVec2(180.0f, yAxis + 1690.0f));
    ImGui::Text("Material ID:");
    ImGui::SetCursorPos(ImVec2(280.0f, yAxis + 1690.0f));
    ImGui::SetNextItemWidth(textboxWidth - 120.0f);
    ImGui::InputText("##materialID", materialID, IM_ARRAYSIZE(materialID));

    ImGui::SetCursorPos(ImVec2(470.0f, yAxis + 1690.0f));
    ImGui::Text("Material Name:");
    ImGui::SetCursorPos(ImVec2(590.0f, yAxis + 1690.0f));
    ImGui::SetNextItemWidth(textboxWidth + 100.0f);
    ImGui::InputText("##materialName", materialName, IM_ARRAYSIZE(materialName));

    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 1730.0f));
    ImGui::Text("Quantity:");
    ImGui::SetCursorPos(ImVec2(90.0f, yAxis + 1730.0f));
    ImGui::SetNextItemWidth(textboxWidth - 200.0f);
    ImGui::InputText("##materialQuantity", materialQuantity, IM_ARRAYSIZE(materialQuantity));

    ImGui::SetCursorPos(ImVec2(220.0f, yAxis + 1730.0f));
    ImGui::Text("Unit Price (PHP):");
    ImGui::SetCursorPos(ImVec2(350.0f, yAxis + 1730.0f));
    ImGui::SetNextItemWidth(textboxWidth - 200.0f);
    ImGui::InputText("##materialUnitPrice", materialUnitPrice, IM_ARRAYSIZE(materialUnitPrice));

    // Format material project ID to PRJ-xxxxx
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

    const std::string materialIDStr(materialID);
    const std::string materialNameStr(materialName);
    const std::string materialQuantityStr(materialQuantity);
    const std::string materialUnitPriceStr(materialUnitPrice);

    ImGui::SetCursorPos(ImVec2(25.0f, yAxis + 1770.0f));
    if (ImGui::Button("Add New Material", ImVec2(400.0f, 100.0f))) {
        if (!materialProjectIDStr.empty() && !materialIDStr.empty()) {
            const std::string expenseDbPath = appConfig::g_dataDirectory +
                                             appConfig::g_projectDirectory +
                                             appConfig::g_projectExpenseDirectory +
                                             materialProjectIDStr + ".db";
            const std::string m_data = "'" + materialIDStr + "', '" + materialNameStr + "', " + materialQuantityStr + ", " + materialUnitPriceStr;

            if (db::appendDatabase(expenseDbPath, m_data)) {
                system::logMessage(system::messageClassification::INFO, "DB Test: New material added successfully.\n");
                materialProjectID[0] = '\0';
                materialID[0] = '\0';
                materialName[0] = '\0';
                materialQuantity[0] = '\0';
                materialUnitPrice[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::INFO, "DB Test: Failed to add new material.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "DB Test: Project ID and Material ID are required.\n");
        }
    }

    ImGui::SetCursorPos(ImVec2(447.5f, yAxis + 1770.0f));
    if (ImGui::Button("Delete Material", ImVec2(400.0f, 100.0f))) {
        if (!materialProjectIDStr.empty() && !materialIDStr.empty()) {
            const std::string expenseDbPath = appConfig::g_dataDirectory +
                                             appConfig::g_projectDirectory +
                                             appConfig::g_projectExpenseDirectory +
                                             materialProjectIDStr + ".db";
            if (db::deleteRow(expenseDbPath, materialIDStr)) {
                system::logMessage(system::messageClassification::INFO, "DB Test: Material deleted successfully.\n");
                materialProjectID[0] = '\0';
                materialID[0] = '\0';
                materialName[0] = '\0';
                materialQuantity[0] = '\0';
                materialUnitPrice[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "DB Test: Failed to delete material.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "DB Test: Project ID and Material ID are required.\n");
        }
    }

    ImGui::SetCursorPos(ImVec2(870.0f, yAxis + 1770.0f));
    if (ImGui::Button("Change Data (Material)", ImVec2(400.0f, 100.0f))) {
        if (!materialProjectIDStr.empty() && !materialIDStr.empty()) {
            const std::string expenseDbPath = appConfig::g_dataDirectory +
                                             appConfig::g_projectDirectory +
                                             appConfig::g_projectExpenseDirectory +
                                             materialProjectIDStr + ".db";
            const std::string setClause = "MATERIAL_NAME='" + materialNameStr + "', QUANTITY=" + materialQuantityStr + ", UNIT_PRICE=" + materialUnitPriceStr;
            if (db::updateDatabase(expenseDbPath, materialIDStr, setClause)) {
                system::logMessage(system::messageClassification::INFO, "DB Test: Material data updated successfully.\n");
                materialProjectID[0] = '\0';
                materialID[0] = '\0';
                materialName[0] = '\0';
                materialQuantity[0] = '\0';
                materialUnitPrice[0] = '\0';
            } else {
                system::logMessage(system::messageClassification::ERROR, "DB Test: Failed to update material data.\n");
            }
        } else {
            system::logMessage(system::messageClassification::ERROR, "DB Test: Project ID and Material ID are required.\n");
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 1880.0f));
    ImGui::Text("Materials Database Viewer (Per Project)");

    static int s_maxRowsMaterials = 100;
    static bool s_showHeadersMaterials = true;
    static char viewerProjectID[128] = "";

    ImGui::SetCursorPos(ImVec2(10.0f, yAxis + 1910.0f));
    ImGui::Text("Project ID to View:");
    ImGui::SetCursorPos(ImVec2(160.0f, yAxis + 1910.0f));
    ImGui::SetNextItemWidth(textboxWidth - 190.0f);
    ImGui::InputText("##viewerProjectID", viewerProjectID, IM_ARRAYSIZE(viewerProjectID));

    ImGui::SetCursorPos(ImVec2(290.0f, yAxis + 1910.0f));
    ImGui::SetNextItemWidth(110.0f);
    ImGui::InputInt("Rows (max at 100) (for Materials)", &s_maxRowsMaterials, 1, 5);
    if (s_maxRowsMaterials < 1) s_maxRowsMaterials = 1;
    if (s_maxRowsMaterials > 100) s_maxRowsMaterials = 100;

    ImGui::SetCursorPos(ImVec2(670.0f, yAxis + 1910.0f));
    ImGui::Checkbox("Show Headers (Materials)", &s_showHeadersMaterials);

    ImGui::Spacing();

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
            }
        }
    }

    const std::string dbPathMaterials = appConfig::g_dataDirectory +
                                       appConfig::g_projectDirectory +
                                       appConfig::g_projectExpenseDirectory +
                                       viewerProjectIDStr + ".db";

    struct ColumnMaterials { const char* name; int index; };
    const std::vector<ColumnMaterials> columnsMaterials = {
        {"MATERIAL_ID", 1}, {"MATERIAL_NAME", 2}, {"QUANTITY", 3}, {"UNIT_PRICE", 4}
    };

    ImGui::BeginChild("DBViewerMaterials", ImVec2(0, 300), true);

    if (!viewerProjectIDStr.empty()) {
        if (s_showHeadersMaterials) {
            ImGui::Columns(static_cast<int>(columnsMaterials.size()), "DBViewer_HeaderCols_Materials");
            for (const auto& c : columnsMaterials) {
                ImGui::Text("%s", c.name);
                ImGui::NextColumn();
            }
            ImGui::Separator();
            ImGui::Columns(1);
        }

        int shownMaterials = 0;
        for (int row = 1; row <= s_maxRowsMaterials; ++row) {
            if (const std::string idCell = db::fetchCell(dbPathMaterials, static_cast<size_t>(row), 1); idCell.empty()) break;

            ImGui::PushID(20000 + row); // Unique ID offset for materials
            ImGui::Columns(static_cast<int>(columnsMaterials.size()), "DBViewer_RowCols_Materials");
            for (const auto& c : columnsMaterials) {
                ImGui::TextWrapped("%s", db::fetchCell(dbPathMaterials, static_cast<size_t>(row), static_cast<size_t>(c.index)).c_str());
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::PopID();
            ++shownMaterials;
        }

        ImGui::TextDisabled(shownMaterials == 0 ? "No rows to display." : "Showing %d row(s).", shownMaterials);
    } else {
        ImGui::TextDisabled("Enter a Project ID to view materials.");
    }

    ImGui::EndChild();
}


static void testUI() {
    ImGui::Text("THIS IS ONLY FOR TESTING/DEBUGGING PURPOSE ONLY");

    static char passkey[128] = "";
    static std::string hashedPasskey;
    static std::string salt;
    static bool passkeyHashed = false;

    ImGui::Text("Passkey");
    ImGui::InputText("##passkey", passkey, IM_ARRAYSIZE(passkey));

    if (ImGui::Button("Hash and Salt Passkey")) {
        std::string passkeyStr(passkey);
        hashedPasskey = cryptography::toHex(std::vector<unsigned char>(passkeyStr.begin(), passkeyStr.end()));
        passkeyHashed = true;
    }


    if (passkeyHashed) {
        ImGui::Spacing();
        const float wrapWidth = ImGui::GetContentRegionAvail().x;
        const float height = ImGui::GetTextLineHeightWithSpacing() +
                             ImGui::CalcTextSize(hashedPasskey.c_str(), nullptr, false, wrapWidth).y;
        ImGui::BeginChild("PasskeyDisplayPanel", ImVec2(0.0f, height), true);
        ImGui::TextWrapped("Hashed Passkey: %s", hashedPasskey.c_str());
        ImGui::EndChild();
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

    // TEST FOR DB
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("DB Test");

    static char nameBuf[128] = "";
    static char positionBuf[128] = "";
    static char locationBuf[128] = "";
    static char salaryBuf[128] = "";
    static char hoursWorkedBuf[128] = "";
    static char advanceBuf[128] = "";

    ImGui::Text("Name");
    ImGui::InputText("##name", nameBuf, IM_ARRAYSIZE(nameBuf));

    ImGui::Text("Position");
    ImGui::InputText("##position", positionBuf, IM_ARRAYSIZE(positionBuf));

    ImGui::Text("Location Site");
    ImGui::InputText("##location", locationBuf, IM_ARRAYSIZE(locationBuf));

    ImGui::Text("Salary");
    ImGui::InputText("##salary", salaryBuf, IM_ARRAYSIZE(salaryBuf));

    ImGui::Text("Hours Worked");
    ImGui::InputText("##hoursWorked", hoursWorkedBuf, IM_ARRAYSIZE(hoursWorkedBuf));

    ImGui::Text("Advance Payment");
    ImGui::InputText("##advance", advanceBuf, IM_ARRAYSIZE(advanceBuf));

    const std::string nameStr(nameBuf);
    const std::string positionStr(positionBuf);
    const std::string locationStr(locationBuf);
    const std::string salaryStr(salaryBuf);
    const std::string hoursWorkedStr(hoursWorkedBuf);
    const std::string advanceStr(advanceBuf);

    if (ImGui::Button("Add New Employee")) {
        if (db::appendDatabase(appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll, "'" + nameStr + "', '" + positionStr + "', '" + locationStr + "', " + salaryStr + ", " + hoursWorkedStr + ", " + advanceStr)) {
            system::logMessage(system::messageClassification::INFO, "DB Test: New employee added successfully.\n");
        } else {
            system::logMessage(system::messageClassification::INFO, "DB Test: Failed to add new employee.\n");
        }
    }

    ImGui::Separator();
    ImGui::Spacing();

    static char employeeId[8] = "";
    ImGui::Text("Employee ID");
    ImGui::InputText("##employeeId", employeeId, IM_ARRAYSIZE(employeeId));
    const std::string employeeIdStr(employeeId);

    static char column[64] = "";
    ImGui::Text("Column Name");
    ImGui::InputText("##column", column, IM_ARRAYSIZE(column));
    const std::string columnStr(column);

    static char newValue[128] = "";
    ImGui::Text("New Value");
    ImGui::InputText("##newValue", newValue, IM_ARRAYSIZE(newValue));
    const std::string newValueStr(newValue);

    if (ImGui::Button("Delete Employee")) {
        if (db::deleteRow(appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll, employeeId)) {
            system::logMessage(system::messageClassification::INFO, "DB Test: Employee deleted successfully.\n");
        } else {
            system::logMessage(system::messageClassification::ERROR, "DB Test: Failed to delete employee.\n");
        }
    }

    if (ImGui::Button("Change Data")) {
        if (const std::string setClause = columnStr + " = '" + newValueStr + "'"; db::updateDatabase(appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll, employeeIdStr, setClause)) {
            system::logMessage(system::messageClassification::INFO, "DB Test: Data updated successfully.\n");
        } else {
            system::logMessage(system::messageClassification::ERROR, "DB Test: Failed to update data.\n");
        }
    }

    // Payslip printing test
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Payslip Printing Test");

    if (ImGui::Button("Print Payslips")) {
        if (const std::string logoPath = HelloImGui::AssetFileFullPath("icons/business_logo.png"); system::printPayslips("", logoPath, {})) {
            system::logMessage(system::messageClassification::INFO, "Payslip Test: Payslips exported successfully.\n");
        } else {
            system::logMessage(system::messageClassification::ERROR, "Payslip Test: Failed to export payslips.\n");
        }
    }

    // Project report printing test
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Project Report Printing Test");

    static char projectIdInput[32] = "PRJ-00001";
    ImGui::InputText("Project ID", projectIdInput, sizeof(projectIdInput));

    if (ImGui::Button("Print Project Report")) {
        if (const std::string logoPath = HelloImGui::AssetFileFullPath("icons/business_logo.png"); system::printProjectReport(projectIdInput, logoPath)) {
            system::logMessage(system::messageClassification::INFO, "Project Report Test: Report exported successfully.\n");
        } else {
            system::logMessage(system::messageClassification::ERROR, "Project Report Test: Failed to export report.\n");
        }
    }

    // ---------------------------------------------------------------------
    // Employee Database Viewer
    // ---------------------------------------------------------------------
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Employee Database Viewer");

    // Options for Employee DB
    static int s_maxRowsEmp = 50;
    static bool s_showHeadersEmp = true;
    ImGui::InputInt("Max Rows##Emp", &s_maxRowsEmp, 5, 25);
    if (s_maxRowsEmp < 1) s_maxRowsEmp = 1;
    ImGui::Checkbox("Show Headers##Emp", &s_showHeadersEmp);

    ImGui::Spacing();

    // Employee DB configuration
    std::string dbPathEmp = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    struct Column { const char* name; int index; };
    std::vector<Column> columnsEmp = {
        {"EMPLOYEE_ID", 1},
        {"NAME", 2},
        {"POSITION", 3},
        {"SITE_LOCATION", 4},
        {"SALARY", 5},
        {"HOURS_WORK", 6},
        {"ADVANCE", 7}
    };

    // Display area for Employee DB
    ImGui::BeginChild("EmployeeDBViewer", ImVec2(0, 200), true);

    // Render headers
    if (s_showHeadersEmp) {
        ImGui::Columns(static_cast<int>(columnsEmp.size()), "EmpDB_HeaderCols");
        for (const auto& c : columnsEmp) {
            ImGui::Text("%s", c.name);
            ImGui::NextColumn();
        }
        ImGui::Separator();
        ImGui::Columns(1);
    }

    // Render rows
    int shownEmp = 0;
    for (int row = 1; row <= s_maxRowsEmp; ++row) {
        const std::string idCell = db::fetchCell(dbPathEmp, static_cast<size_t>(row), 1);
        if (idCell.empty()) break;

        ImGui::PushID(row);
        ImGui::Columns(static_cast<int>(columnsEmp.size()), "EmpDB_RowCols");
        for (const auto& c : columnsEmp) {
            const std::string cell = db::fetchCell(dbPathEmp, static_cast<size_t>(row), static_cast<size_t>(c.index));
            ImGui::TextWrapped("%s", cell.c_str());
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::PopID();
        ++shownEmp;
    }

    if (shownEmp == 0) {
        ImGui::TextDisabled("No rows to display.");
    } else {
        ImGui::TextDisabled("Showing %d row(s).", shownEmp);
    }

    ImGui::EndChild();

    // ---------------------------------------------------------------------
    // Project Database Viewer
    // ---------------------------------------------------------------------
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Project Database Viewer");

    // Options for Project DB
    static int s_maxRowsProj = 50;
    static bool s_showHeadersProj = true;
    ImGui::InputInt("Max Rows##Proj", &s_maxRowsProj, 5, 25);
    if (s_maxRowsProj < 1) s_maxRowsProj = 1;
    ImGui::Checkbox("Show Headers##Proj", &s_showHeadersProj);

    ImGui::Spacing();

    // Project DB configuration
    std::string dbPathProj = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;
    std::vector<Column> columnsProj = {
        {"PROJECT_ID", 1},
        {"PROJECT_NAME", 2},
        {"STATUS", 3},
        {"START_DATE", 4},
        {"NOTE", 5}
    };

    // Display area for Project DB
    ImGui::BeginChild("ProjectDBViewer", ImVec2(0, 200), true);

    // Render headers
    if (s_showHeadersProj) {
        ImGui::Columns(static_cast<int>(columnsProj.size()), "ProjDB_HeaderCols");
        for (const auto& c : columnsProj) {
            ImGui::Text("%s", c.name);
            ImGui::NextColumn();
        }
        ImGui::Separator();
        ImGui::Columns(1);
    }

    // Render rows
    int shownProj = 0;
    for (int row = 1; row <= s_maxRowsProj; ++row) {
        if (const std::string idCell = db::fetchCell(dbPathProj, static_cast<size_t>(row), 1); idCell.empty()) break;

        ImGui::PushID(1000 + row); // Offset ID to avoid conflicts with employee viewer
        ImGui::Columns(static_cast<int>(columnsProj.size()), "ProjDB_RowCols");
        for (const auto& c : columnsProj) {
            const std::string cell = db::fetchCell(dbPathProj, static_cast<size_t>(row), static_cast<size_t>(c.index));
            ImGui::TextWrapped("%s", cell.c_str());
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::PopID();
        ++shownProj;
    }

    if (shownProj == 0) {
        ImGui::TextDisabled("No rows to display.");
    } else {
        ImGui::TextDisabled("Showing %d row(s).", shownProj);
    }

    ImGui::EndChild();
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

    // Ensure HelloImGui does not create a DockSpace: keep the default full-screen window
    // (ProvideFullScreenWindow). No explicit docking toggle is available in this version.
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

    // Load a custom font with only the default ASCII character set to save memory.
    // By providing this callback, we take control of the font loading.
    params.callbacks.LoadAdditionalFonts = [a_fontLocation]() {
        const ImGuiIO& io = ImGui::GetIO();
        // Clear any existing fonts to ensure we only load what we need.
        io.Fonts->Clear();

        const std::string fontPath = HelloImGui::AssetFileFullPath(a_fontLocation);

        ImFontConfig fontConfig;
        // This is the crucial part for memory saving: load only the default character set.
        fontConfig.GlyphRanges = io.Fonts->GetGlyphRangesDefault();

        if (!fontPath.empty()) {
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 22.0f, &fontConfig);
        } else {
            // As a fallback, load ImGui's default font if our custom one fails, but still with the limited character set.
            io.Fonts->AddFontDefault(&fontConfig);
        }
    };

    // PostInit: set application icon using HelloImGui backend pointers (GLFW/SDL)
    params.callbacks.PostInit = []() {
        // Resolve icon path from assets; prefer app_icon.png, fallback to business_logo.png
        std::string iconPath = HelloImGui::AssetFileFullPath("icons/app_icon.png");
        if (iconPath.empty()) iconPath = HelloImGui::AssetFileFullPath("icons/business_logo.png");
        if (iconPath.empty()) return; // nothing to do

        // Load image via stb_image as RGBA
        int w = 0, h = 0, comps = 0;
        unsigned char* pixels = stbi_load(iconPath.c_str(), &w, &h, &comps, 4);
        if (!pixels) return;

        auto* params = HelloImGui::GetRunnerParams();
        if (!params) {
            stbi_image_free(pixels);
            return;
        }
        auto bp = params->backendPointers;

    #ifdef UI_HAVE_GLFW
        if (bp.glfwWindow) {
            GLFWimage img;
            img.width = w;
            img.height = h;
            img.pixels = pixels;
            // Set the GLFW window icon (GLFW makes its own copy)
            glfwSetWindowIcon(reinterpret_cast<GLFWwindow*>(bp.glfwWindow), 1, &img);
            stbi_image_free(pixels);
            return;
        }
    #endif

    #ifdef UI_HAVE_SDL
        if (bp.sdlWindow) {
            // Create an SDL_Surface from the RGBA pixel data
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

        // If no supported backend, free pixels
        stbi_image_free(pixels);
    };

    // Determine a start key and select the initial UI

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

    //Use a wrapper so we can call the current UI each frame (no queued switching)
    params.callbacks.ShowGui = []() {if (g_currentUI) g_currentUI();};

    // Window and GUI settings
    // clamp sizes to reasonable bounds so caller can't accidentally create tiny or huge windows
    const int l_clampedWidth = std::clamp(a_widthPx, 50, 3840);
    const int l_clampedLength = std::clamp(a_heightPx, 50, 2160);
    params.appWindowParams.windowGeometry.size = { l_clampedWidth, l_clampedLength };

    // Rename the whole application to "system" if there is no argument/s in the variable "title"
    params.appWindowParams.windowTitle = a_title.empty() ? "system" : a_title;

    HelloImGui::Run(params);
}
