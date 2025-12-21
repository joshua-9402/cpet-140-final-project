#include "summaryUI.h"
#include <string>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include "hello_imgui/hello_imgui.h"
#include "../ui.h"
#include "../../handler/db.h"
#include "../../handler/print.h"
#include "../../handler/system.h"
#include "../../config/config.h"
#include "../../core/monitor.h"


void summary::displaySummary() {
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
    ui::renderStatCard("EmployeeCard", "Total Employees", accentColor, std::to_string(totalEmployees), "Active in payroll", ImVec2(cardWidth, cardHeight));

    ImGui::SameLine();

    // Project Count Card
    ui::renderStatCard("ProjectCard", "Total Projects", accentColor, std::to_string(totalProjects), "Managed projects", ImVec2(cardWidth, cardHeight));

    ImGui::SameLine();

    // Active Projects Card
    ui::renderStatCard("ActiveCard", "Active Projects", accentColor, std::to_string(activeProjects), "In progress/active", ImVec2(cardWidth, cardHeight));

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

        // Helper lambda to get day of week (0=Sunday, 6=Saturday)
        auto getDayOfWeek = [](int year, int month, const int day) -> int {
            if (month < 3) {
                month += 12;
                year--;
            }
            const int q = day;
            const int m = month;
            const int k = year % 100;
            const int j = year / 100;
            const int h = (q + ((13 * (m + 1)) / 5) + k + (k / 4) + (j / 4) - (2 * j)) % 7;
            return (h + 6) % 7;
        };

        auto getDaysInMonth = [currentYear](int month) -> int {
            if (month == 2) {
                const bool isLeap = (currentYear % 4 == 0 && currentYear % 100 != 0) || (currentYear % 400 == 0);
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
        std::string projectIDStr = ui::normalizeProjectId(std::string(projectIDForReport));
        if (!projectIDStr.empty()) {
            const std::string outFile = ui::buildProjectReportOutFile(projectIDStr);
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