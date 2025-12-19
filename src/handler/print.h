/*
 * CpET 140 Final Project — Print/Export module header
 * StructuraCost - Handler - Print module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Export payslips and project reports to HTML format
 * - Generate printable documents for payroll and project management
 *
 * Boundaries
 * - Interacts with database module to retrieve data
 * - Generates HTML files for browser viewing and printing
 *
 * Notes
 * - HTML output includes embedded styles and logos
 * - Supports both payslip and project report generation
 */

#ifndef CPET_140_FINAL_PROJECT_PRINT_H
#define CPET_140_FINAL_PROJECT_PRINT_H

#include <string>

class Print {
public:
    // Export payslips to HTML file (current week).
    // outFile: path to output HTML file
    // logoPath: path to logo image (will be embedded in HTML)
    // Returns true on success, false on failure
    static bool exportPayslipsHtml(const std::string& outFile, const std::string& logoPath);

    // Export payslips to HTML file for a specific week.
    // outFile: path to output HTML file
    // logoPath: path to logo image (will be embedded in HTML)
    // weekStartDate: ISO date string (YYYY-MM-DD) for the week start (Sunday)
    // Returns true on success, false on failure
    static bool exportPayslipsHtmlForWeek(const std::string& outFile, const std::string& logoPath, const std::string& weekStartDate);

    // Export project report to HTML file.
    // projectId: the project ID (e.g., "PRJ-00001")
    // outFile: path to output HTML file
    // logoPath: path to logo image (will be embedded in HTML)
    // Returns true on success, false on failure
    static bool exportProjectReportHtml(const std::string& projectId, const std::string& outFile, const std::string& logoPath);
};

// Backward compatibility - free function wrappers
inline bool exportPayslipsHtml(const std::string& outFile, const std::string& logoPath) {
    return Print::exportPayslipsHtml(outFile, logoPath);
}

inline bool exportPayslipsHtmlForWeek(const std::string& outFile, const std::string& logoPath, const std::string& weekStartDate) {
    return Print::exportPayslipsHtmlForWeek(outFile, logoPath, weekStartDate);
}

inline bool exportProjectReportHtml(const std::string& projectId, const std::string& outFile, const std::string& logoPath) {
    return Print::exportProjectReportHtml(projectId, outFile, logoPath);
}

#endif //CPET_140_FINAL_PROJECT_PRINT_H