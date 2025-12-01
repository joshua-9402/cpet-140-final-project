#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

#include <sqlite3.h>

#include "../config/config.h"

struct SimpleEmp {
    int id{};
    std::string name;
    std::string position;
    std::string location;
    double salary{0.0};
    double hoursWorked{0.0};
    double advance{0.0};
    double others{0.0};
    std::string date;
};

// Helper: safely read text column (maybe null)
static std::string colTextSafe(sqlite3_stmt* stmt, const int col) {
    const unsigned char* t = sqlite3_column_text(stmt, col);
    return t ? reinterpret_cast<const char*>(t) : std::string();
}

// Fetch employees from the payroll DB (uses provided dbPath)
std::vector<SimpleEmp> fetchAllEmployees(const std::string &dbPath) {
    std::vector<SimpleEmp> list;

    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "fetchAllEmployees: failed to open DB: " << dbPath << " -> " << (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") << "\n";
        if (db) sqlite3_close(db);
        return list;
    }

    if (const auto sql = "SELECT EMPLOYEE_ID, NAME, POSITION, SITE_LOCATION, SALARY, HOURS_WORK, ADVANCE FROM EMPLOYEES ORDER BY EMPLOYEE_ID;"; sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "fetchAllEmployees: prepare failed: " << (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") << "\n";
        sqlite3_close(db);
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SimpleEmp e;
        e.id = sqlite3_column_int(stmt, 0);
        e.name = colTextSafe(stmt, 1);
        e.position = colTextSafe(stmt, 2);
        e.location = colTextSafe(stmt, 3);
        e.salary = sqlite3_column_double(stmt, 4);
        e.hoursWorked = sqlite3_column_double(stmt, 5);
        e.advance = sqlite3_column_double(stmt, 6);
        // others and date are optional in this schema; leave defaults
        list.push_back(e);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return list;
}

// Generate a single payslip HTML using the template/logo
std::string makePayslipHtml(const SimpleEmp& e, const std::string& logo) {
    const double gross = e.salary * e.hoursWorked;
    const double total = gross - e.advance - e.others;

    std::ostringstream o;
    o << "<div class=\"payslip\">\n";
    o << " <div class=\"header\">\n";
    if (!logo.empty()) {
        o << "  <img src=\"" << logo << "\" class=\"logo\">\n";
    }
    o << "  <div class=\"title\">OFFICIAL PAYSLIP</div>\n";
    o << " </div>\n";

    o << " <div class=\"fields\">\n";
    o << "  <strong>Employee No:</strong> " << e.id << "<br>\n";
    if (!e.date.empty()) o << "  <strong>Date:</strong> " << e.date << "<br>\n";
    o << "  <strong>Employee Name:</strong> " << e.name << "<br>\n";
    o << "  <strong>Position:</strong> " << e.position << "<br>\n";
    o << "  <strong>Location:</strong> " << e.location << "<br>\n";
    o << "  <strong>Salary (rate):</strong> " << e.salary << "<br>\n";
    o << "  <strong>Hours Worked:</strong> " << e.hoursWorked << "<br>\n";
    o << " </div>\n";

    o << " <table class=\"weekly-table\">\n";
    o << "  <tr>\n";
    o << "    <th>SUNDAY</th><th>MONDAY</th><th>TUESDAY</th><th>WEDNESDAY</th>";
    o << "<th>THURSDAY</th><th>FRIDAY</th><th>SATURDAY</th>\n";
    o << "  </tr>\n";
    o << "  <tr>\n";
    o << "    <td></td><td></td><td></td><td></td><td></td><td></td><td></td>\n";
    o << "  </tr>\n";
    o << " </table>\n";

    o << " <div class=\"footer-fields\">\n";
    o << "  <strong>Advance:</strong> " << e.advance << "<br>\n";
    o << "  <strong>Others:</strong> " << e.others << "<br>\n";
    o << "  <strong>Gross:</strong> " << gross << "<br>\n";
    o << "  <strong>Total Payable:</strong> " << total << "<br>\n";
    o << " </div>\n";

    o << "</div>\n";
    return o.str();
}

// Export payslips to HTML. outFile can be absolute or relative. logoPath is a path that will be embedded in the HTML (relative file:// works).
// Returns true on success, false on failure (no file written).
bool exportPayslipsHtml(const std::string& outFile, const std::string& logoPath) {
    // build full payroll DB path from appConfig
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    // If DB file does not exist, fail with message (UI can create sample DB first)
    if (std::error_code errorCode; !std::filesystem::exists(dbPath, errorCode)) {
        std::cerr << "exportPayslipsHtml: payroll DB not found at " << dbPath << "\n";
        return false;
    }

    const auto employees = fetchAllEmployees(dbPath);
    if (employees.empty()) {
        std::cerr << "exportPayslipsHtml: no employees found in payroll DB (" << dbPath << "). Nothing to export.\n";
        return false;
    }

    // Ensure output directory exists
    try {
        if (const auto parent = std::filesystem::path(outFile).parent_path(); !parent.empty()) std::filesystem::create_directories(parent);
    } catch (...) {
        // ignore
    }

    std::ofstream f(outFile);
    if (!f.is_open()) {
        std::cerr << "exportPayslipsHtml: cannot open output file: " << outFile << "\n";
        return false;
    }

    f << R"(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Payslips</title>
<style>
  body { margin:20px; font-family:Arial, sans-serif; }
  .page { display:grid; grid-template-columns:1fr 1fr; grid-template-rows:repeat(4,auto); gap:20px; margin-bottom:40px; page-break-after:always; }
  .payslip { border:2px solid #d88c28; padding:15px; border-radius:6px; }
  .header { display:flex; align-items:center; }
  .logo { width:110px; margin-right:20px; }
  .title { font-size:26px; font-weight:bold; color:#d88c28; }
  .fields { margin-top:10px; font-size:14px; line-height:1.5em; }
  .weekly-table { width:100%; border-collapse:collapse; margin-top:10px; }
  .weekly-table th, .weekly-table td { border:1px solid #d88c28; padding:4px; font-size:12px; text-align:center; }
  .footer-fields { margin-top:10px; font-size:14px; }
  @media print { body{margin:0;padding:0;} .page{page-break-after:always;} }
</style>
</head>
<body>
)";

    int count = 0;
    f << "<div class=\"page\">\n";
    for (size_t i = 0; i < employees.size(); ++i) {
        f << makePayslipHtml(employees[i], logoPath) << "\n";
        ++count;
        if (count == 8 && i + 1 < employees.size()) {
            f << "</div>\n<div class=\"page\">\n";
            count = 0;
        }
    }
    f << "</div>\n</body>\n</html>\n";
    f.close();

    std::cout << "Exported payslips to " << outFile << "\n";
    return true;
}
