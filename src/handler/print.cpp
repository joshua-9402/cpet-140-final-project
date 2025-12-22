/*
 * CpET 140 Final Project — Print Module
 * StructuraCost - Handler - Print Module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Generate and export employee payslips to HTML format
 * - Fetch employee data from the payroll database
 * - Convert company logos to embedded data URIs for offline viewing
 *
 * Boundaries
 * - Interacts with SQLite database for employee data retrieval
 * - Handles file system operations for HTML export and logo embedding
 * - Platform-specific system calls for opening generated HTML files
 *
 * Notes
 * - Generates print-ready HTML with embedded styling and logos
 * - Supports automatic pagination for multiple payslips
 * - Cross-platform file opening (Windows, macOS, Linux)
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "print.h"
// Payroll computations (SSS, PhilHealth, Pag-IBIG, tax)
#include "../core/payroll.h"

namespace {
    // Helper function to open HTML file in default browser
    void openInBrowser(const std::string& filePath) {
        const std::string absPath = std::filesystem::absolute(filePath).string();

#ifdef _WIN32
        std::string command = "start \"\" \"" + absPath + "\"";
#elif defined(__APPLE__)
        std::string command = "open \"" + absPath + "\"";
#else
        #error "Unsupported platform - only Windows and macOS are supported"
#endif
        std::system(command.c_str());
    }
}
#include <iomanip>
#include <vector>

#include <sqlite3.h>

#include "../config/config.h"
#include "system.h"


struct employee {
    int id{};
    std::string name;
    std::string position;
    std::string location;
    double salary{0.0};
    double hoursWorked{0.0};
    double regularHours{40.0};
    double advance{0.0};
    double others{0.0};
    std::string date;

    // Weekly hours breakdown (Sunday to Saturday)
    std::string weekStartDate;
    double sunHours{0.0};
    double monHours{0.0};
    double tueHours{0.0};
    double wedHours{0.0};
    double thuHours{0.0};
    double friHours{0.0};
    double satHours{0.0};
    bool hasWeeklyData{false};
};

struct material {
    std::string id;
    std::string name;
    double quantity{0.0};
    double unitPrice{0.0};
};

struct payrollEmployee {
    std::string id;
    std::string name;
    std::string position;
    double hourlyRate{0.0};
    double totalHours{0.0};
    double totalCost{0.0};
};

struct project {
    std::string id;
    std::string name;
    std::string status;
    std::string startDate;
    std::string note;
    std::vector<material> materials;
    std::vector<payrollEmployee> payrollEmployees;
    double totalMaterialCost{0.0};
    double totalPayrollCost{0.0};
};


// Convert image file to base64 data URI
static std::string imageToDataUri(const std::string& imagePath) {
    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        system::logMessage(system::messageClassification::ERROR, std::string("Cannot open logo file: ") + imagePath + "\n");
        return "";
    }

    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (buffer.empty()) {
        return "";
    }

    // Base64 encoding
    static constexpr char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    result.reserve(((buffer.size() + 2) / 3) * 4);

    for (size_t i = 0; i < buffer.size(); i += 3) {
        const uint32_t b = (buffer[i] << 16) | ((i + 1 < buffer.size() ? buffer[i + 1] : 0) << 8) | (i + 2 < buffer.size() ? buffer[i + 2] : 0);
        result += chars[(b >> 18) & 0x3F];
        result += chars[(b >> 12) & 0x3F];
        result += (i + 1 < buffer.size()) ? chars[(b >> 6) & 0x3F] : '=';
        result += (i + 2 < buffer.size()) ? chars[b & 0x3F] : '=';
    }

    // Determine MIME type
    std::string mime = "image/png";
    auto endsWith = [](const std::string &s, const std::string &suffix) {
        if (s.size() < suffix.size()) return false;
        return s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
    };
    if (endsWith(imagePath, ".jpg") || endsWith(imagePath, ".jpeg")) {
        mime = "image/jpeg";
    } else if (endsWith(imagePath, ".gif")) {
        mime = "image/gif";
    }

    return "data:" + mime + ";base64," + result;
}


// Helper: safely read text column (maybe null)
static std::string colTextSafe(sqlite3_stmt* stmt, const int col) {
    const unsigned char* t = sqlite3_column_text(stmt, col);
    return t ? reinterpret_cast<const char*>(t) : std::string();
}


// Fetch employees from the payroll DB (uses provided dbPath)
std::vector<employee> fetchAllEmployees(const std::string &dbPath) {
    std::vector<employee> list;

    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        system::logMessage(system::messageClassification::ERROR, std::string("fetchAllEmployees: failed to open DB: ") + dbPath + " -> " + (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") + "\n");
        if (db) sqlite3_close(db);
        return list;
    }

    // Select REGULAR_HOUR if present in schema (createDatabase creates it). Column order:
    // EMPLOYEE_ID, NAME, POSITION, SITE_LOCATION, SALARY, REGULAR_HOUR, ADVANCE
    if (const auto sql = "SELECT EMPLOYEE_ID, NAME, POSITION, SITE_LOCATION, SALARY, REGULAR_HOUR, ADVANCE FROM EMPLOYEES ORDER BY EMPLOYEE_ID;"; sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        system::logMessage(system::messageClassification::ERROR, std::string("fetchAllEmployees: prepare failed: ") + (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") + "\n");
        sqlite3_close(db);
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        employee e;
        e.id = sqlite3_column_int(stmt, 0);
        e.name = colTextSafe(stmt, 1);
        e.position = colTextSafe(stmt, 2);
        e.location = colTextSafe(stmt, 3);
        e.salary = sqlite3_column_double(stmt, 4);
        // when REGULAR_HOUR is present we read it at col index 5
        e.regularHours = sqlite3_column_double(stmt, 5);
        // Defensive fallback: if DB has 0 or negative, use company default (52 hours/week)
        if (e.regularHours <= 0.0) e.regularHours = 52.0;
        e.advance = sqlite3_column_double(stmt, 6);
        // others and date are optional in this schema; leave defaults
        list.push_back(e);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return list;
}


std::string makePayslipHtml(const employee& data, const std::string& logo) {
    // Prepare a payroll::Employee to compute gross, contributions, and tax.
    Employee pEmp{};
    pEmp.hourlyRate = data.salary;
    pEmp.hoursWorked = data.hoursWorked;
    // Use per-employee regularHours when available
    pEmp.regularHours = data.regularHours;
    // pEmp.regularHours uses default (40.0) unless changed elsewhere
    const PayrollResult pr = payroll::computePayroll(pEmp);

    // Compute overtime details for display
    const double regularHours = pEmp.regularHours;
    const double overtimeHours = std::max(0.0, data.hoursWorked - regularHours);
    const double overtimeRate = data.salary * 1.5; // 1.5x standard overtime multiplier
    const double overtimePay = overtimeHours * overtimeRate;
    const double gross = pr.grossPay; // use computed gross (includes overtime)

    // Deductions breakdown
    const double deductionAdvance = data.advance;
    const double deductionOthers = data.others;
    const double deductionSSS = pr.sss;
    const double deductionPhilHealth = pr.philHealth; // HMO equivalent
    const double deductionPagIbig = pr.pagIbig;
    const double deductionTax = pr.tax;

    const double totalDeductions = deductionAdvance + deductionOthers + deductionSSS + deductionPhilHealth + deductionPagIbig + deductionTax;
    const double netAfterAll = gross - totalDeductions;

    std::ostringstream o;
    o << "<div class=\"payslip\">\n";

    o << " <div class=\"payslip-header\">\n";
    o << "  <img src=\"" << logo << "\" class=\"company-logo\">\n";
    o << "  <div class=\"company-info\"><div class=\"payslip-title\">OFFICIAL PAYSLIP</div></div>\n";
    o << " </div>\n";

    o << " <div class=\"info-section\">\n";
    o << "  <div class=\"info-columns\">\n";
    o << "   <div class=\"info-column\">\n";
    o << "    <div><span class=\"info-label\">Employee No:</span> " << data.id << "</div>\n";
    o << "    <div><span class=\"info-label\">Name:</span> " << data.name << "</div>\n";
    if (!data.weekStartDate.empty()) {
        o << "    <div><span class=\"info-label\">Week of:</span> " << data.weekStartDate << "</div>\n";
    } else if (!data.date.empty()) {
        o << "    <div><span class=\"info-label\">Pay Period:</span> " << data.date << "</div>\n";
    }
    o << "   </div>\n";
    o << "   <div class=\"info-column\">\n";
    o << "    <div><span class=\"info-label\">Position:</span> " << data.position << "</div>\n";
    o << "    <div><span class=\"info-label\">Site Location:</span> " << data.location << "</div>\n";
    o << "   </div>\n";
    o << "  </div>\n";
    o << " </div>\n";

    // Weekly Hours Display - ALWAYS show, even if hours are 0
    o << " <div class=\"weekly-hours-section\">\n";
    o << "  <div class=\"section-subtitle\">Daily Hours:</div>\n";
    o << "  <div class=\"hours-inline\">\n";
    o << "   <span class=\"day-hours\">Sun: <strong>" << std::fixed << std::setprecision(1) << data.sunHours << "</strong></span>\n";
    o << "   <span class=\"day-hours\">Mon: <strong>" << std::fixed << std::setprecision(1) << data.monHours << "</strong></span>\n";
    o << "   <span class=\"day-hours\">Tue: <strong>" << std::fixed << std::setprecision(1) << data.tueHours << "</strong></span>\n";
    o << "   <span class=\"day-hours\">Wed: <strong>" << std::fixed << std::setprecision(1) << data.wedHours << "</strong></span>\n";
    o << "   <span class=\"day-hours\">Thu: <strong>" << std::fixed << std::setprecision(1) << data.thuHours << "</strong></span>\n";
    o << "   <span class=\"day-hours\">Fri: <strong>" << std::fixed << std::setprecision(1) << data.friHours << "</strong></span>\n";
    o << "   <span class=\"day-hours\">Sat: <strong>" << std::fixed << std::setprecision(1) << data.satHours << "</strong></span>\n";
    o << "   <span class=\"day-hours total-hours\">Total: <strong>" << std::fixed << std::setprecision(1) << data.hoursWorked << " hrs</strong></span>\n";
    o << "  </div>\n";
    o << " </div>\n";

    o << " <table class=\"pay-table\">\n";
    o << "  <tr><th colspan=\"2\">Earnings</th><th colspan=\"2\">Deductions</th></tr>\n";
    o << "  <tr>\n";
    o << "   <td>Rate per Hour:</td><td class=\"amount\">₱" << std::fixed << std::setprecision(2) << data.salary << "</td>\n";
    o << "   <td>Advance:</td><td class=\"amount\">₱" << std::fixed << std::setprecision(2) << deductionAdvance << "</td>\n";
    o << "  </tr>\n";
    o << "  <tr>\n";
    o << "   <td>Regular Hours (per week):</td><td class=\"amount\">" << regularHours << " hrs</td>\n";
    o << "   <td>Others:</td><td class=\"amount\">₱" << std::fixed << std::setprecision(2) << deductionOthers << "</td>\n";
    o << "  </tr>\n";
    o << "  <tr>\n";
    o << "   <td>Hours Worked:</td><td class=\"amount\">" << data.hoursWorked << " hrs</td>\n";
    o << "   <td>SSS:</td><td class=\"amount\">₱" << std::fixed << std::setprecision(2) << deductionSSS << "</td>\n";
    o << "  </tr>\n";
    o << "  <tr>\n";
    o << "   <td></td><td></td>\n";
    o << "   <td>PhilHealth (HMO):</td><td class=\"amount\">₱" << std::fixed << std::setprecision(2) << deductionPhilHealth << "</td>\n";
    o << "  </tr>\n";
    o << "  <tr>\n";
    o << "   <td></td><td></td>\n";
    o << "   <td>Pag-IBIG:</td><td class=\"amount\">₱" << std::fixed << std::setprecision(2) << deductionPagIbig << "</td>\n";
    o << "  </tr>\n";
    o << "  <tr>\n";
    o << "   <td></td><td></td>\n";
    o << "   <td>Tax:</td><td class=\"amount\">₱" << std::fixed << std::setprecision(2) << deductionTax << "</td>\n";
    o << "  </tr>\n";
    // Show overtime only when applicable
    if (overtimeHours > 0.0) {
        o << "  <tr>\n";
        o << "   <td>Overtime (hrs):</td><td class=\"amount\">" << overtimeHours << " hrs</td>\n";
        o << "   <td></td><td></td>\n";
        o << "  </tr>\n";
    }
    o << "  <tr class=\"total-row\">\n";
    o << "   <td><strong>Gross Pay:</strong></td><td class=\"amount\"><strong>₱" << std::fixed << std::setprecision(2) << gross << "</strong></td>\n";
    o << "   <td><strong>Total Deductions:</strong></td><td class=\"amount\"><strong>₱" << std::fixed << std::setprecision(2) << totalDeductions << "</strong></td>\n";
    o << "  </tr>\n";


    o << " </table>\n";

    o << " <div class=\"net-pay\">\n";
    o << "  <div class=\"net-pay-label\">NET PAY:</div>\n";
    o << "  <div class=\"net-pay-amount\">₱" << std::fixed << std::setprecision(2) << netAfterAll << "</div>\n";
    o << " </div>\n";

    o << " <div class=\"signature-section\">\n";
    o << "  <div class=\"signature-box\"><div class=\"signature-line\"></div><div class=\"signature-label\">Employee Signature</div></div>\n";
    o << "  <div class=\"signature-box\"><div class=\"signature-line\"></div><div class=\"signature-label\">Authorized By</div></div>\n";
    o << " </div>\n";

    o << "</div>\n";
    return o.str();
}


// Export payslips to HTML. outFile can be absolute or relative. logoPath is a path that will be embedded in the HTML (relative file:// works).
// Returns true on success, false on failure (no file written).
bool Print::exportPayslipsHtml(const std::string& outFile, const std::string& logoPath) {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    if (std::error_code errorCode; !std::filesystem::exists(dbPath, errorCode)) {
        system::logMessage(system::messageClassification::ERROR, std::string("exportPayslipsHtml: payroll DB not found at ") + dbPath + "\n");
        return false;
    }

    auto employees = fetchAllEmployees(dbPath);
    if (employees.empty()) {
        system::logMessage(system::messageClassification::ERROR, std::string("exportPayslipsHtml: no employees found in payroll DB (") + dbPath + ").\n");
        return false;
    }

    // Fetch weekly attendance data from the WEEKLY_ATTENDANCE table in base payroll DB
    // IMPORTANT: Hours worked ONLY come from weekly attendance, NOT from base payroll
    // Weekly hours section ALWAYS displays, even if hours are 0
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) == SQLITE_OK) {
        for (auto& emp : employees) {
            // Reset all hours to 0 - we ONLY use attendance data
            emp.hoursWorked = 0.0;
            emp.sunHours = 0.0;
            emp.monHours = 0.0;
            emp.tueHours = 0.0;
            emp.wedHours = 0.0;
            emp.thuHours = 0.0;
            emp.friHours = 0.0;
            emp.satHours = 0.0;

            sqlite3_stmt* stmt = nullptr;
            // Get the most recent week's attendance for this employee

            if (const auto sql = "SELECT WEEK_START, SUN, MON, TUE, WED, THU, FRI, SAT FROM WEEKLY_ATTENDANCE WHERE EMPLOYEE_ID = ? ORDER BY WEEK_START DESC LIMIT 1;"; sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, emp.id);

                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    const unsigned char* weekStart = sqlite3_column_text(stmt, 0);
                    emp.weekStartDate = weekStart ? reinterpret_cast<const char*>(weekStart) : "";
                    emp.sunHours = sqlite3_column_double(stmt, 1);
                    emp.monHours = sqlite3_column_double(stmt, 2);
                    emp.tueHours = sqlite3_column_double(stmt, 3);
                    emp.wedHours = sqlite3_column_double(stmt, 4);
                    emp.thuHours = sqlite3_column_double(stmt, 5);
                    emp.friHours = sqlite3_column_double(stmt, 6);
                    emp.satHours = sqlite3_column_double(stmt, 7);

                    // Calculate total hours from daily breakdown (ACTUAL hours worked from attendance)
                    emp.hoursWorked = emp.sunHours + emp.monHours + emp.tueHours +
                                     emp.wedHours + emp.thuHours + emp.friHours + emp.satHours;
                }
                sqlite3_finalize(stmt);
            }
        }
        sqlite3_close(db);
    }

    std::string logoDataUri;
    if (!logoPath.empty() && std::filesystem::exists(logoPath)) {
        logoDataUri = imageToDataUri(logoPath);
    }

    try {
        if (const auto parent = std::filesystem::path(outFile).parent_path(); !parent.empty()) {
            std::filesystem::create_directories(parent);
        }
    } catch (...) {}

    std::ofstream f(outFile);
    if (!f.is_open()) {
        system::logMessage(system::messageClassification::ERROR, std::string("exportPayslipsHtml: cannot open output file: ") + outFile + "\n");
        return false;
    }

    f << R"(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Payslips</title>
<style>
  @page { size: 8.5in 13in; margin: 0.3in 0.25in; }

  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: Arial, sans-serif; -webkit-print-color-adjust: exact; print-color-adjust: exact; }

  .container { width: 8in; height: 12.45in; margin: 0 auto; padding-top: 0.1in; }

  .page {
    display: grid;
    grid-template: repeat(4, 1fr) / 1fr 1fr;
    gap: 0.05in;
    padding: 0.05in 0;
    height: 100%;
  }

  .payslip {
    border: 2px solid #d88c28;
    padding: 3px;
    border-radius: 3px;
    display: flex;
    flex-direction: column;
    overflow: hidden;
    font-size: 6.5px;
    background: white;
  }

  .payslip-header {
    display: flex;
    align-items: center;
    gap: 4px;
    padding-bottom: 2px;
    border-bottom: 2px solid #d88c28;
    margin-bottom: 2px;
  }

  .company-logo { width: 30px; height: 30px; object-fit: contain; }
  .company-info { flex: 1; text-align: center; }
  .payslip-title { font-size: 10px; font-weight: bold; color: #d88c28; }

  .info-section { margin: 2px 0; }
  .info-columns { display: grid; grid-template-columns: 1fr 1fr; gap: 3px; }
  .info-column { display: flex; flex-direction: column; gap: 1px; }
  .info-label { font-weight: bold; color: #555; font-size: 6.5px; }

  .weekly-hours-section { margin: 2px 0; background: #f9f3e8; padding: 2px 3px; border-radius: 2px; border: 1px solid #d88c28; }
  .section-subtitle { font-weight: bold; color: #d88c28; font-size: 6px; margin-bottom: 1px; }
  .hours-inline { display: flex; flex-wrap: wrap; gap: 2px; font-size: 5.5px; line-height: 1.2; }
  .day-hours { padding: 0px 2px; background: white; border-radius: 1px; border: 1px solid #ddd; white-space: nowrap; }
  .day-hours.total-hours { background: #d88c28; color: white; font-weight: bold; padding: 0px 3px; }

  .pay-table { width: 100%; border-collapse: collapse; margin: 1px 0; font-size: 6px; }
  .pay-table th { background: #d88c28; color: white; padding: 1px 2px; border: 1px solid #d88c28; font-size: 6px; }
  .pay-table td { padding: 1px 2px; border: 1px solid #ddd; font-size: 6px; }
  .pay-table td.amount { text-align: right; }
  .pay-table tr.total-row { background: #f9f3e8; }
  .pay-table tr.total-row td { border-top: 2px solid #d88c28; }

  .net-pay {
    display: flex;
    justify-content: space-between;
    background: #d88c28;
    color: white;
    padding: 2px 3px;
    margin: 1px 0;
    border-radius: 2px;
  }
  .net-pay-label { font-size: 7px; font-weight: bold; }
  .net-pay-amount { font-size: 8px; font-weight: bold; }

  .signature-section { display: flex; gap: 5px; margin-top: 3px; }
  .signature-box { flex: 1; text-align: center; }
  .signature-line { border-top: 1px solid #333; margin: 2px 0 1px; }
  .signature-label { font-size: 5.5px; color: #666; }

  @media print {
    .container { width: auto; height: auto; }
    .payslip { break-inside: avoid; }
  }
</style>
</head>
<body>
<div class="container">
)";

    // Render pages explicitly to avoid leaving a trailing blank page.
    constexpr size_t perPage = 8;  // Changed from 12 to 8 to accommodate weekly hours table
    const size_t total = employees.size();
    const size_t totalPages = (total + perPage - 1) / perPage;

    for (size_t p = 0; p < totalPages; ++p) {
        const bool isLast = (p + 1 == totalPages);
        f << "<div class=\"page" << (isLast ? " last-page" : "") << "\">\n";

        const size_t start = p * perPage;
        const size_t end = std::min(start + perPage, total);
        for (size_t i = start; i < end; ++i) {
            f << makePayslipHtml(employees[i], logoDataUri) << "\n";
        }

        f << "</div>\n";
    }

    f << "</div>\n</body>\n</html>\n";
    f.close();

    std::cout << "Exported payslips to " << outFile << "\n";
    openInBrowser(outFile);

    return true;
}


// Export payslips to HTML for a specific week with daily hours breakdown
bool Print::exportPayslipsHtmlForWeek(const std::string& outFile, const std::string& logoPath, const std::string& weekStartDate) {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    if (std::error_code errorCode; !std::filesystem::exists(dbPath, errorCode)) {
        system::logMessage(system::messageClassification::ERROR, std::string("exportPayslipsHtmlForWeek: payroll DB not found at ") + dbPath + "\n");
        return false;
    }

    auto employees = fetchAllEmployees(dbPath);
    if (employees.empty()) {
        system::logMessage(system::messageClassification::ERROR, std::string("exportPayslipsHtmlForWeek: no employees found in payroll DB (") + dbPath + ").\n");
        return false;
    }

    // Fetch weekly attendance data from the WEEKLY_ATTENDANCE table in base payroll DB
    // This table mirrors all attendance data from individual weekly files
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) == SQLITE_OK) {
        for (auto& emp : employees) {
            // Reset all hours to 0 - we ONLY use attendance data
            emp.hoursWorked = 0.0;
            emp.sunHours = 0.0;
            emp.monHours = 0.0;
            emp.tueHours = 0.0;
            emp.wedHours = 0.0;
            emp.thuHours = 0.0;
            emp.friHours = 0.0;
            emp.satHours = 0.0;
            emp.weekStartDate = weekStartDate;

            sqlite3_stmt* stmt = nullptr;
            // Get the attendance for this specific week
            const auto sql = "SELECT SUN, MON, TUE, WED, THU, FRI, SAT FROM WEEKLY_ATTENDANCE WHERE EMPLOYEE_ID = ? AND WEEK_START = ? LIMIT 1;";

            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, emp.id);
                sqlite3_bind_text(stmt, 2, weekStartDate.c_str(), -1, SQLITE_TRANSIENT);

                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    emp.sunHours = sqlite3_column_double(stmt, 0);
                    emp.monHours = sqlite3_column_double(stmt, 1);
                    emp.tueHours = sqlite3_column_double(stmt, 2);
                    emp.wedHours = sqlite3_column_double(stmt, 3);
                    emp.thuHours = sqlite3_column_double(stmt, 4);
                    emp.friHours = sqlite3_column_double(stmt, 5);
                    emp.satHours = sqlite3_column_double(stmt, 6);

                    // Calculate total hours from daily breakdown (ACTUAL hours worked from attendance)
                    emp.hoursWorked = emp.sunHours + emp.monHours + emp.tueHours +
                                     emp.wedHours + emp.thuHours + emp.friHours + emp.satHours;
                }
                sqlite3_finalize(stmt);
            }
        }
        sqlite3_close(db);
    }

    std::string logoDataUri;
    if (!logoPath.empty() && std::filesystem::exists(logoPath)) {
        logoDataUri = imageToDataUri(logoPath);
    }

    try {
        if (const auto parent = std::filesystem::path(outFile).parent_path(); !parent.empty()) {
            std::filesystem::create_directories(parent);
        }
    } catch (...) {}

    std::ofstream f(outFile);
    if (!f.is_open()) {
        system::logMessage(system::messageClassification::ERROR, std::string("exportPayslipsHtmlForWeek: cannot open output file: ") + outFile + "\n");
        return false;
    }

    f << R"(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Payslips - Week of )" << weekStartDate << R"(</title>
<style>
  @page { size: 8.5in 13in; margin: 0.3in 0.25in; }

  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: Arial, sans-serif; -webkit-print-color-adjust: exact; print-color-adjust: exact; }

  .container { width: 8in; height: 12.45in; margin: 0 auto; padding-top: 0.1in; }

  .page {
    display: grid;
    grid-template: repeat(4, 1fr) / 1fr 1fr;
    gap: 0.05in;
    padding: 0.05in 0;
    height: 100%;
  }

  .payslip {
    border: 2px solid #d88c28;
    padding: 3px;
    border-radius: 3px;
    display: flex;
    flex-direction: column;
    overflow: hidden;
    font-size: 6.5px;
    background: white;
  }

  .payslip-header {
    display: flex;
    align-items: center;
    gap: 4px;
    padding-bottom: 2px;
    border-bottom: 2px solid #d88c28;
    margin-bottom: 2px;
  }

  .company-logo { width: 30px; height: 30px; object-fit: contain; }
  .company-info { flex: 1; text-align: center; }
  .payslip-title { font-size: 10px; font-weight: bold; color: #d88c28; }

  .info-section { margin: 2px 0; }
  .info-columns { display: grid; grid-template-columns: 1fr 1fr; gap: 3px; }
  .info-column { display: flex; flex-direction: column; gap: 1px; }
  .info-label { font-weight: bold; color: #555; font-size: 6.5px; }

  .weekly-hours-section { margin: 2px 0; background: #f9f3e8; padding: 2px 3px; border-radius: 2px; border: 1px solid #d88c28; }
  .section-subtitle { font-weight: bold; color: #d88c28; font-size: 6px; margin-bottom: 1px; }
  .hours-inline { display: flex; flex-wrap: wrap; gap: 2px; font-size: 5.5px; line-height: 1.2; }
  .day-hours { padding: 0px 2px; background: white; border-radius: 1px; border: 1px solid #ddd; white-space: nowrap; }
  .day-hours.total-hours { background: #d88c28; color: white; font-weight: bold; padding: 0px 3px; }

  .pay-table { width: 100%; border-collapse: collapse; margin: 1px 0; font-size: 6px; }
  .pay-table th { background: #d88c28; color: white; padding: 1px 2px; border: 1px solid #d88c28; font-size: 6px; }
  .pay-table td { padding: 1px 2px; border: 1px solid #ddd; font-size: 6px; }
  .pay-table td.amount { text-align: right; }
  .pay-table tr.total-row { background: #f9f3e8; }
  .pay-table tr.total-row td { border-top: 2px solid #d88c28; }

  .net-pay {
    display: flex;
    justify-content: space-between;
    background: #d88c28;
    color: white;
    padding: 2px 3px;
    margin: 1px 0;
    border-radius: 2px;
  }
  .net-pay-label { font-size: 7px; font-weight: bold; }
  .net-pay-amount { font-size: 8px; font-weight: bold; }

  .signature-section { display: flex; gap: 5px; margin-top: 3px; }
  .signature-box { flex: 1; text-align: center; }
  .signature-line { border-top: 1px solid #333; margin: 2px 0 1px; }
  .signature-label { font-size: 5.5px; color: #666; }

  @media print {
    .container { width: auto; height: auto; }
    .payslip { break-inside: avoid; }
  }
</style>
</head>
<body>
<div class="container">
)";

    constexpr size_t perPage = 8;
    const size_t total = employees.size();
    const size_t totalPages = (total + perPage - 1) / perPage;

    for (size_t p = 0; p < totalPages; ++p) {
        const bool isLast = (p + 1 == totalPages);
        f << "<div class=\"page" << (isLast ? " last-page" : "") << "\">\n";

        const size_t start = p * perPage;
        const size_t end = std::min(start + perPage, total);
        for (size_t i = start; i < end; ++i) {
            f << makePayslipHtml(employees[i], logoDataUri) << "\n";
        }

        f << "</div>\n";
    }

    f << "</div>\n</body>\n</html>\n";
    f.close();

    std::cout << "Exported payslips for week of " << weekStartDate << " to " << outFile << "\n";
    openInBrowser(outFile);

    return true;
}


// Fetch materials from a project expense database
static std::vector<material> fetchProjectMaterials(const std::string &dbPath) {
    std::vector<material> list;

    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        system::logMessage(system::messageClassification::ERROR, std::string("fetchProjectMaterials: failed to open DB: ") + dbPath + " -> " + (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") + "\n");
        if (db) sqlite3_close(db);
        return list;
    }

    if (const auto sql = "SELECT MATERIAL_ID, MATERIAL_NAME, QUANTITY, UNIT_PRICE FROM MATERIALS ORDER BY MATERIAL_ID;"; sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        system::logMessage(system::messageClassification::ERROR, std::string("fetchProjectMaterials: prepare failed: ") + (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") + "\n");
        sqlite3_close(db);
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        material m;
        m.id = colTextSafe(stmt, 0);
        m.name = colTextSafe(stmt, 1);
        m.quantity = sqlite3_column_double(stmt, 2);
        m.unitPrice = sqlite3_column_double(stmt, 3);
        list.push_back(m);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return list;
}

// Fetch payroll employees from a project expense database
static std::vector<payrollEmployee> fetchProjectPayroll(const std::string &dbPath) {
    std::vector<payrollEmployee> list;

    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        system::logMessage(system::messageClassification::ERROR, std::string("fetchProjectPayroll: failed to open DB: ") + dbPath + " -> " + (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") + "\n");
        if (db) sqlite3_close(db);
        return list;
    }

    if (const auto sql = "SELECT EMPLOYEE_ID, EMPLOYEE_NAME, POSITION, HOURLY_RATE, TOTAL_HOURS, TOTAL_COST FROM PAYROLL_EXPENSES ORDER BY EMPLOYEE_ID;"; sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        system::logMessage(system::messageClassification::ERROR, std::string("fetchProjectPayroll: prepare failed: ") + (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") + "\n");
        sqlite3_close(db);
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        payrollEmployee p;
        p.id = std::to_string(sqlite3_column_int(stmt, 0));
        p.name = colTextSafe(stmt, 1);
        p.position = colTextSafe(stmt, 2);
        p.hourlyRate = sqlite3_column_double(stmt, 3);
        p.totalHours = sqlite3_column_double(stmt, 4);
        p.totalCost = sqlite3_column_double(stmt, 5);
        list.push_back(p);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return list;
}


// Fetch project info from the project database
static project fetchProjectInfo(const std::string &projectId) {
    project proj;
    proj.id = projectId;

    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;

    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        system::logMessage(system::messageClassification::ERROR, std::string("fetchProjectInfo: failed to open DB: ") + dbPath + "\n");
        if (db) sqlite3_close(db);
        return proj;
    }

    const auto sql = "SELECT PROJECT_NAME, STATUS, START_DATE, NOTE FROM PROJECT_LIST WHERE PROJECT_ID = ? LIMIT 1;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        system::logMessage(system::messageClassification::ERROR, std::string("fetchProjectInfo: prepare failed: ") + (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") + "\n");
        sqlite3_close(db);
        return proj;
    }

    sqlite3_bind_text(stmt, 1, projectId.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        proj.name = colTextSafe(stmt, 0);
        proj.status = colTextSafe(stmt, 1);
        proj.startDate = colTextSafe(stmt, 2);
        proj.note = colTextSafe(stmt, 3);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    // Fetch materials from the project expense database
    const std::string materialDbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_projectExpenseDirectory + projectId + ".db";
    if (std::filesystem::exists(materialDbPath)) {
        proj.materials = fetchProjectMaterials(materialDbPath);

        // Calculate total material cost
        for (const auto& mat : proj.materials) {
            proj.totalMaterialCost += mat.quantity * mat.unitPrice;
        }

        // Fetch payroll expenses
        proj.payrollEmployees = fetchProjectPayroll(materialDbPath);

        // Calculate total payroll cost
        for (const auto& emp : proj.payrollEmployees) {
            proj.totalPayrollCost += emp.totalCost;
        }
    }

    return proj;
}


// Generate HTML for project report
static std::string makeProjectReportHtml(const project& proj, const std::string& logo) {
    std::ostringstream o;

    o << R"(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Project Report - )" << proj.id << R"(</title>
<style>
  @page { size: 8.5in 13in; margin: 0.5in 0.5in 0.5in 0.5in; }

  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: Arial, sans-serif; -webkit-print-color-adjust: exact; print-color-adjust: exact; background: white; padding: 0; margin: 0; }

  .report-container { width: 7.5in; margin: 0; border: 2px solid #d88c28; border-radius: 8px; padding: 15px; background: white; }

  .report-header {
    display: flex;
    align-items: center;
    gap: 15px;
    padding-bottom: 12px;
    border-bottom: 3px solid #d88c28;
    margin-bottom: 15px;
  }

  .company-logo { width: 55px; height: 55px; object-fit: contain; }
  .header-info { flex: 1; }
  .report-title { font-size: 22px; font-weight: bold; color: #d88c28; margin-bottom: 4px; }
  .report-subtitle { font-size: 13px; color: #666; }

  .project-info {
    background: #f9f3e8;
    border: 1px solid #d88c28;
    border-radius: 4px;
    padding: 12px;
    margin-bottom: 15px;
  }

  .info-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 8px;
  }

  .info-item {
    margin-bottom: 6px;
  }

  .info-label {
    font-weight: bold;
    color: #555;
    margin-right: 6px;
  }

  .section-title {
    font-size: 17px;
    font-weight: bold;
    color: #d88c28;
    margin: 15px 0 8px;
    padding-bottom: 4px;
    border-bottom: 2px solid #d88c28;
  }

  .materials-table {
    width: 100%;
    border-collapse: collapse;
    margin-bottom: 15px;
  }

  .materials-table th {
    background: #d88c28;
    color: white;
    padding: 8px;
    text-align: left;
    border: 1px solid #d88c28;
    font-size: 14px;
  }

  .materials-table td {
    padding: 6px 8px;
    border: 1px solid #ddd;
    font-size: 13px;
  }

  .materials-table tr:nth-child(even) {
    background: #f9f9f9;
  }

  .materials-table td.amount {
    text-align: right;
  }

  .materials-table tr.total-row {
    background: #f9f3e8;
    font-weight: bold;
  }

  .materials-table tr.total-row td {
    border-top: 2px solid #d88c28;
    padding-top: 10px;
  }

  .summary-box {
    background: #d88c28;
    color: white;
    padding: 12px;
    border-radius: 4px;
    margin: 15px 0;
  }

  .summary-item {
    display: flex;
    justify-content: space-between;
    margin-bottom: 6px;
    font-size: 15px;
  }

  .summary-item.grand-total {
    font-size: 18px;
    font-weight: bold;
    margin-top: 8px;
    padding-top: 8px;
    border-top: 2px solid rgba(255, 255, 255, 0.3);
  }

  .footer {
    text-align: center;
    margin-top: 20px;
    padding-top: 12px;
    border-top: 1px solid #ddd;
    color: #666;
    font-size: 11px;
  }

  .page-break {
    page-break-after: always;
    break-after: page;
    margin: 20px 0;
    padding: 15px 0;
    border-bottom: 3px dashed #d88c28;
    text-align: center;
    color: #d88c28;
    font-weight: bold;
  }

  .page-break::after {
    content: "— CONTINUED ON NEXT PAGE —";
    display: block;
    font-size: 14px;
    margin-top: 10px;
  }

  .continuation-header {
    border-top: 3px solid #d88c28;
    padding-top: 15px;
    margin-top: 20px;
    margin-bottom: 10px;
  }

  .continuation-header::before {
    content: "— CONTINUED FROM PREVIOUS PAGE —";
    display: block;
    text-align: center;
    color: #d88c28;
    font-weight: bold;
    font-size: 14px;
    margin-bottom: 15px;
  }

  @media print {
    body { padding: 0; margin: 0; }
    .report-container { border: 2px solid #d88c28; width: 7.5in; }
    .page-break {
      page-break-after: always;
      border-bottom: none;
    }
    .page-break::after { display: none; }
  }
</style>
</head>
<body>
<div class="report-container">
  <div class="report-header">
    <img src=")" << logo << R"(" class="company-logo" alt="Company Logo">
    <div class="header-info">
      <div class="report-title">PROJECT REPORT</div>
      <div class="report-subtitle">Comprehensive Project Cost Analysis</div>
    </div>
  </div>

  <div class="project-info">
    <div class="info-grid">
      <div class="info-item">
        <span class="info-label">Project ID:</span>
        <span>)" << proj.id << R"(</span>
      </div>
      <div class="info-item">
        <span class="info-label">Status:</span>
        <span>)" << proj.status << R"(</span>
      </div>
      <div class="info-item">
        <span class="info-label">Project Name:</span>
        <span>)" << proj.name << R"(</span>
      </div>
      <div class="info-item">
        <span class="info-label">Start Date:</span>
        <span>)" << proj.startDate << R"(</span>
      </div>
    </div>)";

    if (!proj.note.empty()) {
        o << R"(
    <div class="info-item" style="margin-top: 8px;">
      <span class="info-label">Notes:</span>
      <span>)" << proj.note << R"(</span>
    </div>)";
    }

    o << R"(
  </div>

  <div class="section-title">Materials & Supplies</div>
)";

    if (!proj.materials.empty()) {
        const size_t itemsPerPage = 25; // Adjust based on page size
        const size_t totalItems = proj.materials.size();

        for (size_t pageStart = 0; pageStart < totalItems; pageStart += itemsPerPage) {
            // Add continuation header for pages after the first
            if (pageStart > 0) {
                o << R"(
</div>
<div class="page-break"></div>
<div class="report-container continuation-header">
  <div class="section-title">Materials & Supplies (Continued)</div>
)";
            }

            o << R"(  <table class="materials-table">
    <thead>
      <tr>
        <th>Material ID</th>
        <th>Material Name</th>
        <th style="text-align: right;">Quantity</th>
        <th style="text-align: right;">Unit Price (₱)</th>
        <th style="text-align: right;">Total Cost (₱)</th>
      </tr>
    </thead>
    <tbody>
)";

            const size_t pageEnd = std::min(pageStart + itemsPerPage, totalItems);
            for (size_t i = pageStart; i < pageEnd; ++i) {
                const auto& mat = proj.materials[i];
                const double totalCost = mat.quantity * mat.unitPrice;
                o << "      <tr>\n";
                o << "        <td>" << mat.id << "</td>\n";
                o << "        <td>" << mat.name << "</td>\n";
                o << "        <td class=\"amount\">" << std::fixed << std::setprecision(2) << mat.quantity << "</td>\n";
                o << "        <td class=\"amount\">₱" << std::fixed << std::setprecision(2) << mat.unitPrice << "</td>\n";
                o << "        <td class=\"amount\">₱" << std::fixed << std::setprecision(2) << totalCost << "</td>\n";
                o << "      </tr>\n";
            }

            // Only show total on the last page
            if (pageEnd >= totalItems) {
                o << R"(      <tr class="total-row">
        <td colspan="4" style="text-align: right;">Total Material Cost:</td>
        <td class="amount">₱)" << std::fixed << std::setprecision(2) << proj.totalMaterialCost << R"(</td>
      </tr>
)";
            }

            o << R"(    </tbody>
  </table>
)";
        }
    } else {
        o << R"(  <p style="color: #666; font-style: italic; margin: 15px 0;">No materials recorded for this project.</p>
)";
    }

    // PAYROLL EXPENSES SECTION
    o << R"(
  <div class="section-title">Payroll Expenses</div>
)";

    if (!proj.payrollEmployees.empty()) {
        o << R"(  <table class="materials-table">
    <thead>
      <tr>
        <th>Employee ID</th>
        <th>Employee Name</th>
        <th>Position</th>
        <th style="text-align: right;">Hourly Rate (₱)</th>
        <th style="text-align: right;">Total Hours</th>
        <th style="text-align: right;">Total Cost (₱)</th>
      </tr>
    </thead>
    <tbody>
)";

        for (const auto& emp : proj.payrollEmployees) {
            o << "      <tr>\n";
            o << "        <td>" << emp.id << "</td>\n";
            o << "        <td>" << emp.name << "</td>\n";
            o << "        <td>" << emp.position << "</td>\n";
            o << "        <td class=\"amount\">₱" << std::fixed << std::setprecision(2) << emp.hourlyRate << "</td>\n";
            o << "        <td class=\"amount\">" << std::fixed << std::setprecision(2) << emp.totalHours << "</td>\n";
            o << "        <td class=\"amount\">₱" << std::fixed << std::setprecision(2) << emp.totalCost << "</td>\n";
            o << "      </tr>\n";
        }

        o << R"(      <tr class="total-row">
        <td colspan="5" style="text-align: right;">Total Payroll Cost:</td>
        <td class="amount">₱)" << std::fixed << std::setprecision(2) << proj.totalPayrollCost << R"(</td>
      </tr>
    </tbody>
  </table>
)";
    } else {
        o << R"(  <p style="color: #666; font-style: italic; margin: 15px 0;">No payroll expenses recorded for this project.</p>
)";
    }

    // SUMMARY BOX
    const double grandTotal = proj.totalMaterialCost + proj.totalPayrollCost;

    o << R"(
  <div class="summary-box">
    <div class="summary-item">
      <span>Total Materials:</span>
      <span>₱)" << std::fixed << std::setprecision(2) << proj.totalMaterialCost << R"(</span>
    </div>
    <div class="summary-item">
      <span>Total Payroll:</span>
      <span>₱)" << std::fixed << std::setprecision(2) << proj.totalPayrollCost << R"(</span>
    </div>
    <div class="summary-item grand-total">
      <span>PROJECT TOTAL COST:</span>
      <span>₱)" << std::fixed << std::setprecision(2) << grandTotal << R"(</span>
    </div>
  </div>

  <div class="footer">
    <p>This report was generated automatically by StructuraCost</p>
    <p>Project ID: )" << proj.id << R"( | Generated: )" << std::string(__DATE__) << R"(</p>
  </div>
</div>
</body>
</html>
)";

    return o.str();
}


// Export project report to HTML
bool Print::exportProjectReportHtml(const std::string& projectId, const std::string& outFile, const std::string& logoPath) {
    if (projectId.empty()) {
        system::logMessage(system::messageClassification::ERROR, "exportProjectReportHtml: project ID is empty\n");
        return false;
    }

    const auto proj = fetchProjectInfo(projectId);
    if (proj.name.empty()) {
        system::logMessage(system::messageClassification::ERROR, std::string("exportProjectReportHtml: project not found: ") + projectId + "\n");
        return false;
    }

    std::string logoDataUri;
    if (!logoPath.empty() && std::filesystem::exists(logoPath)) {
        logoDataUri = imageToDataUri(logoPath);
    }

    try {
        if (const auto parent = std::filesystem::path(outFile).parent_path(); !parent.empty()) {
            std::filesystem::create_directories(parent);
        }
    } catch (...) {}

    std::ofstream f(outFile);
    if (!f.is_open()) {
        system::logMessage(system::messageClassification::ERROR, std::string("exportProjectReportHtml: cannot open output file: ") + outFile + "\n");
        return false;
    }

    f << makeProjectReportHtml(proj, logoDataUri);
    f.close();

    std::cout << "Exported project report to " << outFile << "\n";
    openInBrowser(outFile);

    return true;
}

