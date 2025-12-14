#ifndef CPET_140_FINAL_PROJECT_PRINT_H
#define CPET_140_FINAL_PROJECT_PRINT_H

#include <string>

// Export payslips to HTML file.
// outFile: path to output HTML file
// logoPath: path to logo image (will be embedded in HTML)
// Returns true on success, false on failure
bool exportPayslipsHtml(const std::string& outFile, const std::string& logoPath);

// Export project report to HTML file.
// projectId: the project ID (e.g., "PRJ-00001")
// outFile: path to output HTML file
// logoPath: path to logo image (will be embedded in HTML)
// Returns true on success, false on failure
bool exportProjectReportHtml(const std::string& projectId, const std::string& outFile, const std::string& logoPath);

#endif //CPET_140_FINAL_PROJECT_PRINT_H