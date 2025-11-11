#ifndef CPET_140_FINAL_PROJECT_UI_H
#define CPET_140_FINAL_PROJECT_UI_H

#include <string>

class ui {
    public:
    // Global Variable/s
    static std::string g_failedMessage;

    // Global Methods
    static void constructUI(const std::string &a_title, const std::string& a_fontLocation, int a_widthPx, int a_lengthPx, const std::string& a_window);

};
#endif //CPET_140_FINAL_PROJECT_UI_H
