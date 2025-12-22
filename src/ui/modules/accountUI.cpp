/*
 * CpET 140 Final Project — Account UI module
 * StructuraCost - UI - Account module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Display and manage company account settings, logos, and business details.
 *
 * Boundaries
 * - UI-only: delegates persistence to `db` and `system` handlers.
 *
 * Notes
 * - Should be lightweight and not perform blocking I/O on the UI thread.
 */

#include "accountUI.h"
#include "../../ui/ui.h"
//#include "../../security/cryptography.h"
#include "../../config/config.h"
#include "../../handler/system.h"
#include "hello_imgui/hello_imgui.h"

void account::displayAccount() {
    // Load business logo only once on the first call
    ui::loadImage("icons/business_logo.png", 0.1f, 10.0f, 70.1f);
    ui::loadImage("icons/user_icon.png", 0.8f, 10.0f, 60.0f);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f); // Small vertical spacing
    ui::setTextRight(ui::g_userName.c_str());
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f); // Small vertical spacing
    ui::setTextRight(ui::g_position.c_str());

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f); // Small vertical spacing
    if (ImGui::Button("Log Out", ui::fullWidthButtonSize(40))) {
        // Encrypt all DBs before logging out (only if a session key exists)
        // if (security::DBEncryptionSession::hasKey()) {
        //     security::DBEncryptionSession::encryptAllDbs();
        //     security::DBEncryptionSession::clear();
        // }
        appConfig::g_auth = false;
        appConfig::g_testMode = false;
        ui::g_userName = "";
        ui::g_position = "";
        // Force the runner to use the login window size on next run, then request the current Run to exit.
        if (auto* params = HelloImGui::GetRunnerParams()) {
            params->appWindowParams.windowGeometry.size = { std::clamp(appConfig::g_loginWidth, 50, 3840), std::clamp(appConfig::g_loginHeight, 50, 2160) };
            // Ensure the runner applies the resize when it exits the current Run
            params->appWindowParams.windowGeometry.resizeAppWindowAtNextFrame = true;
            // Request app to quit so HelloImGui::Run returns, and we can restart at login
            system::logMessage(system::messageClassification::INFO, "User successfully logged out.");
            params->appShallExit = true;
            // After exiting, the main() function will check g_auth and reconstruct the login UI
        }
    }
}