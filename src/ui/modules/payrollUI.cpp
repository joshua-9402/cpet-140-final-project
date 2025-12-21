/*
 * CpET 140 Final Project — Payroll UI module
 * StructuraCost - UI - Payroll module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Render payroll-related user interfaces: payroll lists, payslip generation, and payroll reports.
 *
 * Boundaries
 * - Presentation only. No direct persistence or heavy payroll calculation here; delegate to `payroll` and `db` modules.
 *
 * Notes
 * - Keep UI logic thin; heavy logic and calculations must remain in `core/payroll.cpp`.
 */

#include "payrollUI.h"
#include <string>
#include <filesystem>
#include <sstream>
#include <iomanip>


#include "hello_imgui/hello_imgui.h"
#include "../ui.h"
#include "../../handler/db.h"
#include "../../core/monitor.h"

void payrollUI::displayPayroll() {
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
    ui::renderStatCard("PayrollEmployeeCard", "Total Employees", accentColor, std::to_string(totalEmployees), "Active employees", ImVec2(cardWidth, cardHeight));

    ImGui::SameLine();

    // Total Salary Expense Card
    {
        std::ostringstream oss; oss.setf(std::ios::fixed); oss << std::setprecision(2) << totalSalaryExpense;
        ui::renderStatCard("SalaryExpenseCard", "Total Salary Expense", accentColor, oss.str(), "Total payroll cost", ImVec2(cardWidth, cardHeight));
    }

    ImGui::SameLine();

    // Total Advances Card
    {
        std::ostringstream oss; oss.setf(std::ios::fixed); oss << std::setprecision(2) << totalAdvances;
        ui::renderStatCard("AdvancesCard", "Total Advances", accentColor, oss.str(), "Employee advances", ImVec2(cardWidth, cardHeight));
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
    ImGui::Text("Regular Hours (per week)"); ImGui::NextColumn();
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
