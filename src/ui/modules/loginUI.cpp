#include "loginUI.h"
#include <string>
#include <sstream>
#include <iomanip>
#include "hello_imgui/hello_imgui.h"
#include "../ui.h"
#include "../../config/config.h"
#include "../../security/auth.h"


void loginUI::displayLogin() {
    static char username[128] = "";
    static char password[128] = "";
    static std::string loginErrorMessage;
    constexpr float textboxWidth = 460.0f;

    ui::loadImage("icons/business_logo.png", 0.05f, 10.1f, 70.0f);
    ImGui::SetCursorPos(ImVec2(18.0f, 90.0f));
    ImGui::Text("Welcome, please log in to continue");

    ImGui::SetCursorPos(ImVec2(24.0f, 150.0f));
    ImGui::Text("%s", "Username: ");


    ImGui::SetCursorPos(ImVec2(18.0f, 180.0f));
    ImGui::SetNextItemWidth(textboxWidth); // width in pixels
    ImGui::InputText("##username", username, IM_ARRAYSIZE(username));

    ImGui::SetCursorPos(ImVec2(24.0f, 220.0f));
    ImGui::Text("%s", "Password: ");

    ImGui::SetCursorPos(ImVec2(18.0f, 250.0f));
    ImGui::SetNextItemWidth(textboxWidth); // width in pixels
    ImGui::InputText("##password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

    ImGui::SetCursorPos(ImVec2(25.0f, 310.0f));
    if (ui::setButtonCenter("Log In", ui::fullWidthButtonSize(35)), ImGui::IsItemClicked()){
        if (auth::testAuth(username, password)) {
            appConfig::g_auth = true;
            appConfig::g_testMode = true;

            username[0] = '\0';
            password[0] = '\0';

            if (auto* params = HelloImGui::GetRunnerParams()) {
                params->appShallExit = true;
            }
        } else if (auth::testDeployAuth(username, password)) {
            appConfig::g_auth = true;
            appConfig::g_testMode = false;

            username[0] = '\0';
            password[0] = '\0';

            if (auto* params = HelloImGui::GetRunnerParams()) {
                params->appShallExit = true;
            }
        } else if (auth::mainAuth(username, password)) {
            appConfig::g_auth = true;
            appConfig::g_testMode = false;

            username[0] = '\0';
            password[0] = '\0';

            // ui::g_userName and ui::g_position are already set by mainAuth()

            if (auto* params = HelloImGui::GetRunnerParams()) {
                params->appShallExit = true;
            }
        } else {
            loginErrorMessage = "Invalid username/password or decryption failed.";
        }
    }

    if (!loginErrorMessage.empty()) {
        ImGui::SetCursorPos(ImVec2(25.0f, 280.0f));
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", loginErrorMessage.c_str());
    }

    ImGui::SetCursorPos(ImVec2(25.0f, 360.0f));
    if (ui::setButtonCenter("Exit App", ui::fullWidthButtonSize(35)), ImGui::IsItemClicked()) {
        if (auto* params = HelloImGui::GetRunnerParams()) {
            params->appShallExit = true;
        }
    }
}
