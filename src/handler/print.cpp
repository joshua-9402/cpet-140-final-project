/*
 * CpET 140 Final Project — Print Module
 * StructuraCost - Handler - Print Module
*
* Contributors:
*  Joshua Literal
*
* Purpose
 *  - Generate and export employee payslips to HTML format
 *  - Fetch employee data from the payroll database
 *  - Convert company logos to embedded data URIs for offline viewing
*
* Boundaries
 *  - Interacts with SQLite database for employee data retrieval
 *  - Handles file system operations for HTML export and logo embedding
 *  - Platform-specific system calls for opening generated HTML files
*
* Notes
 *  - Generates print-ready HTML with embedded styling and logos
 *  - Supports automatic pagination for multiple payslips
 *  - Cross-platform file opening (Windows, macOS, Linux)
*/


#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <filesystem>

#include <sqlite3.h>

#include "../config/config.h"


struct employee {
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

struct material {
    std::string id;
    std::string name;
    double quantity{0.0};
    double unitPrice{0.0};
};

struct project {
    std::string id;
    std::string name;
    std::string status;
    std::string startDate;
    std::string note;
    std::vector<material> materials;
    double totalMaterialCost{0.0};
};


// Convert image file to base64 data URI
static std::string imageToDataUri(const std::string& imagePath) {
    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open logo file: " << imagePath << "\n";
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
    if (imagePath.ends_with(".jpg") || imagePath.ends_with(".jpeg")) {
        mime = "image/jpeg";
    } else if (imagePath.ends_with(".gif")) {
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
        employee e;
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


std::string makePayslipHtml(const employee& data, const std::string& logo) {
    const double gross = data.salary * data.hoursWorked;
    const double total = gross - data.advance - data.others;

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
    if (!data.date.empty()) {
        o << "    <div><span class=\"info-label\">Pay Period:</span> " << data.date << "</div>\n";
    }
    o << "   </div>\n";
    o << "   <div class=\"info-column\">\n";
    o << "    <div><span class=\"info-label\">Position:</span> " << data.position << "</div>\n";
    o << "    <div><span class=\"info-label\">Site Location:</span> " << data.location << "</div>\n";
    o << "   </div>\n";
    o << "  </div>\n";
    o << " </div>\n";

    o << " <table class=\"pay-table\">\n";
    o << "  <tr><th colspan=\"2\">Earnings</th><th colspan=\"2\">Deductions</th></tr>\n";
    o << "  <tr>\n";
    o << "   <td>Rate per Hour:</td><td class=\"amount\">₱" << std::fixed << std::setprecision(2) << data.salary << "</td>\n";
    o << "   <td>Advance:</td><td class=\"amount\">₱" << std::fixed << std::setprecision(2) << data.advance << "</td>\n";
    o << "  </tr>\n";
    o << "  <tr>\n";
    o << "   <td>Hours Worked:</td><td class=\"amount\">" << data.hoursWorked << " hrs</td>\n";
    o << "   <td>Others:</td><td class=\"amount\">₱" << std::fixed << std::setprecision(2) << data.others << "</td>\n";
    o << "  </tr>\n";
    o << "  <tr class=\"total-row\">\n";
    o << "   <td><strong>Gross Pay:</strong></td><td class=\"amount\"><strong>₱" << std::fixed << std::setprecision(2) << gross << "</strong></td>\n";
    o << "   <td><strong>Total Deductions:</strong></td><td class=\"amount\"><strong>₱" << std::fixed << std::setprecision(2) << (data.advance + data.others) << "</strong></td>\n";
    o << "  </tr>\n";
    o << " </table>\n";

    o << " <div class=\"net-pay\">\n";
    o << "  <div class=\"net-pay-label\">NET PAY:</div>\n";
    o << "  <div class=\"net-pay-amount\">₱" << std::fixed << std::setprecision(2) << total << "</div>\n";
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
bool exportPayslipsHtml(const std::string& outFile, const std::string& logoPath) {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    if (std::error_code errorCode; !std::filesystem::exists(dbPath, errorCode)) {
        std::cerr << "exportPayslipsHtml: payroll DB not found at " << dbPath << "\n";
        return false;
    }

    const auto employees = fetchAllEmployees(dbPath);
    if (employees.empty()) {
        std::cerr << "exportPayslipsHtml: no employees found in payroll DB (" << dbPath << ").\n";
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
        std::cerr << "exportPayslipsHtml: cannot open output file: " << outFile << "\n";
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
    grid-template: repeat(6, 1fr) / 1fr 1fr;
    gap: 0.05in;
    padding: 0.05in 0;
    height: 100%;
  }

  .payslip {
    border: 2px solid #d88c28;
    padding: 4px;
    border-radius: 4px;
    display: flex;
    flex-direction: column;
    overflow: hidden;
    font-size: 7px;
    background: white;
  }

  .payslip-header {
    display: flex;
    align-items: center;
    gap: 5px;
    padding-bottom: 3px;
    border-bottom: 2px solid #d88c28;
    margin-bottom: 3px;
  }

  .company-logo { width: 35px; height: 35px; object-fit: contain; }
  .company-info { flex: 1; text-align: center; }
  .payslip-title { font-size: 12px; font-weight: bold; color: #d88c28; }

  .info-section { margin: 4px 0 4px; }
  .info-columns { display: grid; grid-template-columns: 1fr 1fr; gap: 6px; }
  .info-column { display: flex; flex-direction: column; gap: 2px; }
  .info-label { font-weight: bold; color: #555; }

  .pay-table { width: 100%; border-collapse: collapse; margin: 1px 0 2px; font-size: 7px; }
  .pay-table th { background: #d88c28; color: white; padding: 2px; border: 1px solid #d88c28; font-size: 7px; }
  .pay-table td { padding: 2px 3px; border: 1px solid #ddd; }
  .pay-table td.amount { text-align: right; }
  .pay-table tr.total-row { background: #f9f3e8; }
  .pay-table tr.total-row td { border-top: 2px solid #d88c28; }

  .net-pay {
    display: flex;
    justify-content: space-between;
    background: #d88c28;
    color: white;
    padding: 3px 4px;
    margin-bottom: 2px;
    border-radius: 2px;
  }
  .net-pay-label { font-size: 8px; font-weight: bold; }
  .net-pay-amount { font-size: 9px; font-weight: bold; }

  .signature-section { display: flex; gap: 8px; margin-top: 15px; }
  .signature-box { flex: 1; text-align: center; }
  .signature-line { border-top: 1px solid #333; margin: 4px 0 1px; }
  .signature-label { font-size: 6.5px; color: #666; }

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
    constexpr size_t perPage = 12;
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

    const std::string absPath = std::filesystem::absolute(outFile).string();

#ifdef _WIN32
    std::string command = "start \"\" \"" + absPath + "\"";
    std::system(command.c_str());
#elif __APPLE__
    std::string command = "open \"" + absPath + "\"";
    std::system(command.c_str());
#else
    std::string command = "xdg-open \"" + absPath + "\"";
    std::system(command.c_str());
#endif

    return true;
}


// Fetch materials from a project expense database
static std::vector<material> fetchProjectMaterials(const std::string &dbPath) {
    std::vector<material> list;

    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "fetchProjectMaterials: failed to open DB: " << dbPath << " -> " << (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") << "\n";
        if (db) sqlite3_close(db);
        return list;
    }

    if (const auto sql = "SELECT MATERIAL_ID, MATERIAL_NAME, QUANTITY, UNIT_PRICE FROM MATERIALS ORDER BY MATERIAL_ID;"; sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "fetchProjectMaterials: prepare failed: " << (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") << "\n";
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


// Fetch project info from the project database
static project fetchProjectInfo(const std::string &projectId) {
    project proj;
    proj.id = projectId;

    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;

    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "fetchProjectInfo: failed to open DB: " << dbPath << "\n";
        if (db) sqlite3_close(db);
        return proj;
    }

    const auto sql = "SELECT PROJECT_NAME, STATUS, START_DATE, NOTE FROM PROJECT_LIST WHERE PROJECT_ID = ? LIMIT 1;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "fetchProjectInfo: prepare failed: " << (sqlite3_errmsg(db) ? sqlite3_errmsg(db) : "unknown") << "\n";
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
  @page { size: 8.5in 13in; margin: 0.5in; }

  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: Arial, sans-serif; -webkit-print-color-adjust: exact; print-color-adjust: exact; background: white; padding: 20px; }

  .report-container { max-width: 7.5in; margin: 0 auto; border: 2px solid #d88c28; border-radius: 8px; padding: 20px; background: white; }

  .report-header {
    display: flex;
    align-items: center;
    gap: 15px;
    padding-bottom: 15px;
    border-bottom: 3px solid #d88c28;
    margin-bottom: 20px;
  }

  .company-logo { width: 60px; height: 60px; object-fit: contain; }
  .header-info { flex: 1; }
  .report-title { font-size: 24px; font-weight: bold; color: #d88c28; margin-bottom: 5px; }
  .report-subtitle { font-size: 14px; color: #666; }

  .project-info {
    background: #f9f3e8;
    border: 1px solid #d88c28;
    border-radius: 4px;
    padding: 15px;
    margin-bottom: 20px;
  }

  .info-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 10px;
  }

  .info-item {
    margin-bottom: 8px;
  }

  .info-label {
    font-weight: bold;
    color: #555;
    margin-right: 8px;
  }

  .section-title {
    font-size: 18px;
    font-weight: bold;
    color: #d88c28;
    margin: 20px 0 10px;
    padding-bottom: 5px;
    border-bottom: 2px solid #d88c28;
  }

  .materials-table {
    width: 100%;
    border-collapse: collapse;
    margin-bottom: 20px;
  }

  .materials-table th {
    background: #d88c28;
    color: white;
    padding: 10px;
    text-align: left;
    border: 1px solid #d88c28;
  }

  .materials-table td {
    padding: 8px 10px;
    border: 1px solid #ddd;
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
    padding-top: 12px;
  }

  .summary-box {
    background: #d88c28;
    color: white;
    padding: 15px;
    border-radius: 4px;
    margin: 20px 0;
  }

  .summary-item {
    display: flex;
    justify-content: space-between;
    margin-bottom: 8px;
    font-size: 16px;
  }

  .summary-item.grand-total {
    font-size: 20px;
    font-weight: bold;
    margin-top: 10px;
    padding-top: 10px;
    border-top: 2px solid rgba(255, 255, 255, 0.3);
  }

  .footer {
    text-align: center;
    margin-top: 30px;
    padding-top: 15px;
    border-top: 1px solid #ddd;
    color: #666;
    font-size: 12px;
  }

  @media print {
    .report-container { border: 2px solid #d88c28; }
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
    <div class="info-item" style="margin-top: 10px;">
      <span class="info-label">Notes:</span>
      <span>)" << proj.note << R"(</span>
    </div>)";
    }

    o << R"(
  </div>

  <div class="section-title">Materials & Supplies</div>
)";

    if (!proj.materials.empty()) {
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

        for (const auto& mat : proj.materials) {
            const double totalCost = mat.quantity * mat.unitPrice;
            o << "      <tr>\n";
            o << "        <td>" << mat.id << "</td>\n";
            o << "        <td>" << mat.name << "</td>\n";
            o << "        <td class=\"amount\">" << std::fixed << std::setprecision(2) << mat.quantity << "</td>\n";
            o << "        <td class=\"amount\">₱" << std::fixed << std::setprecision(2) << mat.unitPrice << "</td>\n";
            o << "        <td class=\"amount\">₱" << std::fixed << std::setprecision(2) << totalCost << "</td>\n";
            o << "      </tr>\n";
        }

        o << R"(      <tr class="total-row">
        <td colspan="4" style="text-align: right;">Total Material Cost:</td>
        <td class="amount">₱)" << std::fixed << std::setprecision(2) << proj.totalMaterialCost << R"(</td>
      </tr>
    </tbody>
  </table>
)";
    } else {
        o << R"(  <p style="color: #666; font-style: italic; margin: 20px 0;">No materials recorded for this project.</p>
)";
    }

    o << R"(
  <div class="summary-box">
    <div class="summary-item">
      <span>Total Materials:</span>
      <span>₱)" << std::fixed << std::setprecision(2) << proj.totalMaterialCost << R"(</span>
    </div>
    <div class="summary-item grand-total">
      <span>PROJECT TOTAL COST:</span>
      <span>₱)" << std::fixed << std::setprecision(2) << proj.totalMaterialCost << R"(</span>
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
bool exportProjectReportHtml(const std::string& projectId, const std::string& outFile, const std::string& logoPath) {
    if (projectId.empty()) {
        std::cerr << "exportProjectReportHtml: project ID is empty\n";
        return false;
    }

    const auto proj = fetchProjectInfo(projectId);
    if (proj.name.empty()) {
        std::cerr << "exportProjectReportHtml: project not found: " << projectId << "\n";
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
        std::cerr << "exportProjectReportHtml: cannot open output file: " << outFile << "\n";
        return false;
    }

    f << makeProjectReportHtml(proj, logoDataUri);
    f.close();

    std::cout << "Exported project report to " << outFile << "\n";

    const std::string absPath = std::filesystem::absolute(outFile).string();

#ifdef _WIN32
    std::string command = "start \"\" \"" + absPath + "\"";
    std::system(command.c_str());
#elif __APPLE__
    std::string command = "open \"" + absPath + "\"";
    std::system(command.c_str());
#else
    std::string command = "xdg-open \"" + absPath + "\"";
    std::system(command.c_str());
#endif

    return true;
}

