#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

#include <sqlite3.h>

#include "../config/config.h"

// Base64 encoding table
static constexpr char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Convert binary data to base64 string
static std::string base64_encode(const unsigned char* bytes_to_encode, size_t in_len) {
    std::string ret;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(int j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (int j = 0; j < i + 1; j++)
            ret += base64_chars[char_array_4[j]];

        while(i++ < 3)
            ret += '=';
    }

    return ret;
}

// Convert image file to base64 data URI
static std::string imageToDataUri(const std::string& imagePath) {
    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open logo file: " << imagePath << "\n";
        return "";
    }

    // Read file into vector
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (buffer.empty()) {
        return "";
    }

    // Encode to base64
    std::string base64 = base64_encode(buffer.data(), buffer.size());

    // Determine MIME type from extension
    std::string mimeType = "image/png";
    if (imagePath.ends_with(".jpg") || imagePath.ends_with(".jpeg")) {
        mimeType = "image/jpeg";
    } else if (imagePath.ends_with(".gif")) {
        mimeType = "image/gif";
    }

    return "data:" + mimeType + ";base64," + base64;
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
    o << "<div class=\"payslip\" style=\"min-height:170px; box-sizing:border-box; overflow:visible; font-size:12px;\">\n";
    o << " <div class=\"header\" style=\"min-height:30px; display:flex; align-items:center;\">\n";
    o << "  <img src=\"" << logo << "\" class=\"logo\" style=\"width:96px; margin-right:10px;\">\n";
    o << "  <div class=\"title\" style=\"font-size:24px; font-weight:bold; color:#d88c28;\">OFFICIAL PAYSLIP</div>\n";
    o << " </div>\n";

    // Fields area: left column = employee info, right column = financials (aligned)
    o << " <div class=\"fields\" style=\"display:flex; gap:6px; align-items:flex-start; font-size:15px; margin-top:5px;\">\n";
    o << "  <div style=\"flex:1; line-height:1.3em; overflow:hidden;\">\n";
    o << "   <div style=\"white-space:nowrap;text-overflow:ellipsis;overflow:hidden;\"><strong>Employee No:</strong> " << data.id << "</div>\n";
    if (!data.date.empty()) o << "   <div style=\"white-space:nowrap;text-overflow:ellipsis;overflow:hidden;\"><strong>Date:</strong> " << data.date << "</div>\n";
    o << "   <div style=\"white-space:nowrap;text-overflow:ellipsis;overflow:hidden;\"><strong>Employee Name:</strong> " << data.name << "</div>\n";
    o << "   <div style=\"white-space:nowrap;text-overflow:ellipsis;overflow:hidden;\"><strong>Position:</strong> " << data.position << "</div>\n";
    o << "   <div style=\"white-space:nowrap;text-overflow:ellipsis;overflow:hidden;\"><strong>Location:</strong> " << data.location << "</div>\n";
    o << "   <div style=\"white-space:nowrap;text-overflow:ellipsis;overflow:hidden;\"><strong>Salary (rate):</strong> " << data.salary << "</div>\n";
    o << "   <div style=\"white-space:nowrap;text-overflow:ellipsis;overflow:hidden;\"><strong>Hours Worked:</strong> " << data.hoursWorked << "</div>\n";
    o << "  </div>\n";
    o << "  <div style=\"width:160px; line-height:1.3em; overflow:hidden; text-align:right;\">\n";
    o << "   <div><strong>Advance:</strong> " << data.advance << "</div>\n";
    o << "   <div><strong>Others:</strong> " << data.others << "</div>\n";
    o << "   <div><strong>Gross:</strong> " << gross << "</div>\n";
    o << "   <div style=\"font-weight:bold; font-size:20px; margin-top:4px;\"><strong>Total Payable:</strong> <strong>" << total << "</strong></div>\n";
    o << "  </div>\n";
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

    // Convert logo to data URI for embedding
    std::string logoDataUri;
    if (!logoPath.empty() && std::filesystem::exists(logoPath)) {
        logoDataUri = imageToDataUri(logoPath);
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
        f << makePayslipHtml(employees[i], logoDataUri) << "\n";
        ++count;
        if (count == 8 && i + 1 < employees.size()) {
            f << "</div>\n<div class=\"page\">\n";
            count = 0;
        }
    }
    f << "</div>\n</body>\n</html>\n";
    f.close();

    std::cout << "Exported payslips to " << outFile << "\n";

    // Auto-open the HTML file in default browser
    const std::string absPath = std::filesystem::absolute(outFile).string();

#ifdef _WIN32
    // Windows: use 'start' command
    std::string command = "start \"\" \"" + absPath + "\"";
    std::system(command.c_str());
#elif __APPLE__
    // macOS: use 'open' command
    std::string command = "open \"" + absPath + "\"";
    std::system(command.c_str());
#else
    // Linux: use 'xdg-open' command
    std::string command = "xdg-open \"" + absPath + "\"";
    std::system(command.c_str());
#endif

    return true;
}
