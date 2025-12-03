#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <filesystem>

#include <sqlite3.h>

#include "../config/config.h"

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

std::string makePayslipHtml(const SimpleEmp& data, const std::string& logo) {
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
