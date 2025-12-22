/*
 * CpET 140 Final Project — Monitor UI module
 * StructuraCost - UI - Monitor module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Provide the monitoring and data-management user interface: employee/project/attendance
 *   viewers and management forms, and helpers to load and present database rows.
 *
 * Boundaries
 * - Presentation only. No direct database persistence or heavy business logic here; delegate to
 *   `monitor` and `db` modules for data access and mutations.
 *
 * Notes
 * - Immediate-mode UI built on HelloImGui/ImGui; functions should be kept small and focused.
 */

#include "monitorUI.h"
#include <string>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <iomanip>

// For debounced per-field validation
#include <chrono>
#include <unordered_map>

#include "../../dependencies/sqlite/sqlite3.h"

#include "hello_imgui/hello_imgui.h"
#include "../../handler/db.h"
#include "../../handler/system.h"
#include "../../config/config.h"
#include "../../core/monitor.h"


struct State {
    bool touched = false;
    bool valid   = true;
};

static std::unordered_map<ImGuiID, State> g_states;
static std::vector<std::string> weekOptions;
static std::vector<std::string> weekDates;

// Debounced validator: validates at most once per second per input field.
// Behavior: when validation returns an empty string (invalid), the textbox background becomes red.
// No inline text, no tooltip — only the red background for invalid state, updated every 1 second.
static void InputTextValidated(const char* p_id, char* p_buffer, int p_bufSize, system::inputType p_validationType, float p_itemWidth = 0.0f) {
    State& l_state = g_states[ImGui::GetID(p_id)];

    // Real-time validation
    l_state.valid = (p_buffer[0] == '\0') || !system::validateInput(p_validationType, p_buffer).empty();;

    // Highlight red if invalid
    if (!l_state.valid)
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0.6f, 0.15f, 0.15f, 1.0f});

    if (p_itemWidth > 0.0f)
        ImGui::SetNextItemWidth(p_itemWidth);

    ImGui::InputText(p_id, p_buffer, p_bufSize);

    if (!l_state.valid)
        ImGui::PopStyleColor();
}



void employeeDatabaseUI()  {
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
        {"SALARY"}, {"REGULAR_HOUR"}, {"ADVANCE"}
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
}


void employeeManagementUI()  {
    static char name[128], position[128], employeeID[128], location[128], salary[128], hoursWorked[128], advance[128];

    ImGui::SetWindowFontScale(1.1f);
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 0.8f, 1.0f), "Employee Management");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::BeginChild("Employee Management", ImVec2(0, 400), true);
    ImGui::Spacing();

    // Employee Form Fields - Row 1
    ImGui::Text("Name:");
    ImGui::SameLine(120.0f);
    InputTextValidated("##name", name, IM_ARRAYSIZE(name), system::NAME, 300.0f);

    ImGui::SameLine(450.0f);
    ImGui::Text("Position:");
    ImGui::SameLine(550.0f);
    InputTextValidated("##position", position, IM_ARRAYSIZE(position), system::POSITION, 250.0f);

    ImGui::SameLine(830.0f);
    ImGui::Text("Employee ID:");
    ImGui::SameLine(950.0f);
    InputTextValidated("##employeeID", employeeID, IM_ARRAYSIZE(employeeID), system::EMPLOYEE_ID, 150.0f);

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
    InputTextValidated("##salary", salary, IM_ARRAYSIZE(salary), system::REGULAR_HOURS, 120.0f);

    ImGui::SameLine(660.0f);
    ImGui::Text("Regular Hours:");
    ImGui::SameLine(780.0f);
    InputTextValidated("##hoursWorked", hoursWorked, IM_ARRAYSIZE(hoursWorked), system::NUMBER, 120.0f);

    ImGui::SameLine(930.0f);
    ImGui::Text("Advance:");
    ImGui::SameLine(1020.0f);
    InputTextValidated("##advance", advance, IM_ARRAYSIZE(advance), system::ADVANCE, 120.0f);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Prepare string copies of current buffers for validation and actions (required by action handlers)
    const std::string nameStr(name);
    const std::string positionStr(position);
    const std::string employeeIDStr(employeeID);
    const std::string locationStr(location);
    const std::string salaryStr(salary);
    const std::string hoursWorkedStr(hoursWorked);
    const std::string advanceStr(advance);

    // Employee Action Buttons
    if (ImGui::Button("Add New Employee", ImVec2(260.0f, 40.0f))) {
        // Validate all inputs before adding
        const std::string validatedName = system::validateInput(system::NAME, nameStr);
        const std::string validatedPosition = system::validateInput(system::POSITION, positionStr);
        const std::string validatedLocation = system::validateInput(system::SITE_LOCATION, locationStr);
        const std::string validatedSalary = system::validateInput(system::NUMBER, salaryStr);
        const std::string validatedHours = system::validateInput(system::NUMBER, hoursWorkedStr);
        const std::string validatedAdvance = system::validateInput(system::ADVANCE, advanceStr);

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
        const std::string validatedEmployeeID = system::validateInput(system::EMPLOYEE_ID, employeeIDStr);

        if (validatedEmployeeID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Employee ID is required for update.\n");
        } else {
            // Validate provided fields (allow partial updates)
            std::string validatedName = nameStr.empty() ? nameStr : system::validateInput(system::NAME, nameStr);
            std::string validatedPosition = positionStr.empty() ? positionStr : system::validateInput(system::POSITION, positionStr);
            std::string validatedLocation = locationStr.empty() ? locationStr : system::validateInput(system::SITE_LOCATION, locationStr);
            std::string validatedSalary = salaryStr.empty() ? salaryStr : system::validateInput(system::REGULAR_HOURS, salaryStr);
            std::string validatedHours = hoursWorkedStr.empty() ? hoursWorkedStr : system::validateInput(system::NUMBER, hoursWorkedStr);
            std::string validatedAdvance = system::validateInput(system::ADVANCE, advanceStr);

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
        if (const std::string validatedEmployeeID = system::validateInput(system::EMPLOYEE_ID, employeeIDStr); validatedEmployeeID.empty()) {
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
        const std::string validatedEmployeeID = system::validateInput(system::EMPLOYEE_ID, employeeIDStr);

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
    employeeDatabaseUI();
    ImGui::EndChild();
}


void attendanceDatabaseUI() {
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
}


void attendanceManagementUI() {
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

    ImGui::Text("Employee ID:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    // Debounced red-only validator: marks red only when non-empty and invalid (checked every 1s)
    InputTextValidated("##attendanceEmployeeID", attendanceEmployeeID, IM_ARRAYSIZE(attendanceEmployeeID), system::EMPLOYEE_ID, 120.0f);

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
    attendanceDatabaseUI();
    ImGui::EndChild();
}


void projectDatabaseUI() {
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
}


void projectManagementUI() {
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
        if (std::string inputStr(projectID); inputStr.length() == 10 && inputStr.substr(0, 4) == "PRJ-") {
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

    // Project Form Fields (use debounced red-only validator)
    ImGui::Text("Project ID:");
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(120.0f);
    InputTextValidated("##projectID", projectID, IM_ARRAYSIZE(projectID), system::PROJECT_ID, 120.0f);

    ImGui::SameLine(270.0f);
    ImGui::Text("Project Name:");
    ImGui::SameLine(400.0f);
    ImGui::SetNextItemWidth(400.0f);
    InputTextValidated("##projectName", projectName, IM_ARRAYSIZE(projectName), system::NAME, 400.0f);

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
    InputTextValidated("##startDate", startDate, IM_ARRAYSIZE(startDate), system::DATE, 200.0f);

    ImGui::SameLine(630.0f);
    ImGui::Text("End Date:");
    ImGui::SameLine(720.0f);
    InputTextValidated("##endDate", endDate, IM_ARRAYSIZE(endDate), system::DATE, 200.0f);

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
        const std::string validatedProjectID = system::validateInput(system::PROJECT_ID, projectIDStr);
        const std::string validatedProjectName = system::validateInput(system::NAME, projectNameStr);
        const std::string validatedStartDate = system::validateInput(system::DATE, startDateStr);
        const std::string validatedEndDate = endDateStr.empty() ? endDateStr : system::validateInput(system::DATE, endDateStr);

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
        const std::string validatedProjectID = system::validateInput(system::PROJECT_ID, projectIDStr);

        if (validatedProjectID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Project ID (PRJ-#####) is required for update.\n");
        } else {
            // Validate other fields if provided
            std::string validatedProjectName = projectNameStr.empty() ? projectNameStr : system::validateInput(system::NAME, projectNameStr);
            std::string validatedStartDate = startDateStr.empty() ? startDateStr : system::validateInput(system::DATE, startDateStr);
            std::string validatedEndDate = endDateStr.empty() ? endDateStr : system::validateInput(system::DATE, endDateStr);

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
        const std::string validatedProjectID = system::validateInput(system::PROJECT_ID, projectIDStr);

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
        const std::string validatedProjectID = system::validateInput(system::PROJECT_ID, projectIDStr);

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
    projectDatabaseUI();

    ImGui::EndChild();
}


void materialDatabaseUI() {
    // ==============================================
    // MATERIALS/EXPENSE DATABASE SECTION
    // ==============================================
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
}


void materialManagementUI() {
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

    // Materials Form Fields (use debounced validator)
    ImGui::Text("Project ID:");
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(120.0f);
    InputTextValidated("##materialProjectID", materialProjectID, IM_ARRAYSIZE(materialProjectID), system::PROJECT_ID, 120.0f);

    ImGui::SameLine(270.0f);
    ImGui::Text("Material ID:");
    ImGui::SameLine(380.0f);
    ImGui::SetNextItemWidth(150.0f);
    InputTextValidated("##materialID", materialID, IM_ARRAYSIZE(materialID), system::MATERIAL_ID, 150.0f);

    ImGui::SameLine(560.0f);
    ImGui::Text("Material Name:");
    ImGui::SameLine(690.0f);
    ImGui::SetNextItemWidth(400.0f);
    InputTextValidated("##materialName", materialName, IM_ARRAYSIZE(materialName), system::NAME, 400.0f);

    ImGui::Spacing();

    ImGui::Text("Quantity:");
    ImGui::SameLine(120.0f);
    ImGui::SetNextItemWidth(150.0f);
    InputTextValidated("##materialQuantity", materialQuantity, IM_ARRAYSIZE(materialQuantity), system::NUMBER, 150.0f);

    ImGui::SameLine(300.0f);
    ImGui::Text("Unit Price (PHP):");
    ImGui::SameLine(450.0f);
    ImGui::SetNextItemWidth(150.0f);
    InputTextValidated("##materialUnitPrice", materialUnitPrice, IM_ARRAYSIZE(materialUnitPrice), system::NUMBER, 150.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Materials Action Buttons
    if (ImGui::Button("Add New Material", ImVec2(350.0f, 40.0f))) {
        const std::string validatedProjectID = system::validateInput(system::PROJECT_ID, materialProjectIDStr);
        const std::string validatedMaterialID = system::validateInput(system::MATERIAL_ID, materialIDStr);
        const std::string validatedMaterialName = system::validateInput(system::NAME, materialNameStr);
        const std::string validatedQuantity = system::validateInput(system::NUMBER, materialQuantityStr);
        const std::string validatedUnitPrice = system::validateInput(system::NUMBER, materialUnitPriceStr);

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
        const std::string validatedProjectID = system::validateInput(system::PROJECT_ID, materialProjectIDStr);
        const std::string validatedMaterialID = system::validateInput(system::MATERIAL_ID, materialIDStr);

        if (validatedProjectID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Valid Project ID (PRJ-#####) is required.\n");
        } else if (validatedMaterialID.empty()) {
            system::logMessage(system::messageClassification::WARNING, "DB: Material ID cannot be empty.\n");
        } else {
            // Validate other fields if provided
            std::string validatedMaterialName = materialNameStr.empty() ? materialNameStr : system::validateInput(system::NAME, materialNameStr);
            std::string validatedQuantity = materialQuantityStr.empty() ? materialQuantityStr : system::validateInput(system::NUMBER, materialQuantityStr);
            std::string validatedUnitPrice = materialUnitPriceStr.empty() ? materialUnitPriceStr : system::validateInput(system::NUMBER, materialUnitPriceStr);

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
        const std::string validatedProjectID = system::validateInput(system::PROJECT_ID, materialProjectIDStr);
        const std::string validatedMaterialID = system::validateInput(system::MATERIAL_ID, materialIDStr);
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
        const std::string validatedProjectID = system::validateInput(system::PROJECT_ID, materialProjectIDStr);
        const std::string validatedMaterialID = system::validateInput(system::MATERIAL_ID, materialIDStr);

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

    materialDatabaseUI();

    ImGui::EndChild();
}


void monitorUI::displayMonitor() {

    ImGui::SetWindowFontScale(1.7f);
    ImGui::Text("Data Management & Monitoring");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();



    employeeManagementUI();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    attendanceManagementUI();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    projectManagementUI();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    materialManagementUI();
}
