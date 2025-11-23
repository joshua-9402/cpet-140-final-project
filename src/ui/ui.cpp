/*
 * CpET 140 Final Project — UI module
 * StructuraCost - UI - UI module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Immediate-mode UI built with HelloImGui/ImGui. Manages a registry of UIs, switching between them, font loading,
 *   and window setup.
 * - Provides the main two-column layout with navigation on the left and active panel on the right.
 *
 * Boundaries
 * - Presentation only. No business logic or persistence here. Call payroll.cpp and monitor.cpp modules for operations.
 *
 * Notes
 * - UI switching is immediate (case-insensitive). App exit uses HelloImGui runner.
 * - This part is OFF LIMITS especially to constructUI(), the global variables, and to the UI registry/map.
 */

#include <string>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "ui.h"

#include "hello_imgui/hello_imgui.h"
#include "../handler/system.h"
#include "../config/config.h"
#include "../security/cryptography.h"
#include "../security/auth.h"


// UI registry and UI management
static std::unordered_map<std::string, std::function<void()>> g_uiMap;
static std::function<void()> g_currentUI = nullptr;
static std::function<void()> g_rightUI = nullptr; // Right panel active UI (shown in main two-column layout)
auto g_buttonSizePxSelector = ImVec2(270, 40); // x for width, y for height of buttons
std::string ui::g_failedMessage; // Global failed message for failedUI
std::string g_accountNumber = "NA";
std::string g_userName;
std::string g_position;

// Track whether the failed login modal is open
static bool g_failedPopupOpen = false;


// Lowercase helper used by both constructUI and switchToUI
static std::string toLower(std::string s) {
    std::ranges::transform(s, s.begin(), [](const unsigned char c){ return std::tolower(c); });
    return s;
}


static ImVec2 fullWidthButtonSize(const float a_height = g_buttonSizePxSelector.y) { return {ImGui::GetContentRegionAvail().x, a_height}; }


void setButtonCenter(const char* text, const ImVec2& size){
    const float windowWidth = ImGui::GetWindowSize().x;
    const float buttonWidth = size.x;

    // Move to center position
    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    ImGui::Button(text, size);
}


void setTextCenter(const char* text){
    const float windowWidth = ImGui::GetWindowSize().x;
    const float textWidth = ImGui::CalcTextSize(text).x;

    // Move to center position
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text("%s", text);
}


void setTextRight(const char* text){
    const float windowWidth = ImGui::GetWindowSize().x;
    const float textWidth = ImGui::CalcTextSize(text).x;

    // Position the cursor so that text ends at the right edge
    ImGui::SetCursorPosX(windowWidth - textWidth - ImGui::GetStyle().WindowPadding.x);
    ImGui::Text("%s", text);
}


static void loadImage(const std::string& p_location, const float p_locationXPx, const float p_locationYPx, const float p_imageSize) {
    static HelloImGui::ImageAndSize imageFile;

    if constexpr (true) {imageFile = HelloImGui::ImageAndSizeFromAsset(p_location.c_str());}
    if (imageFile.textureId != static_cast<ImTextureID>(0)) {
        const float imageSize = p_imageSize;
        const float imagePosX = (ImGui::GetWindowWidth() - imageSize) * p_locationXPx;
        ImGui::SetCursorPosX(imagePosX);
        ImGui::SetCursorPosY(p_locationYPx);
        ImGui::Image(imageFile.textureId, ImVec2(imageSize, imageSize));
    }
}


static void failedUI() {
    ImGui::Text("%s", ui::g_failedMessage.c_str());
    if (ImGui::Button("Exit")) {
        system::appShutdown();
    }
}


static void loginUI() {
    static char username[128] = "";
    static char password[128] = "";
    constexpr int textboxPaddingX = 8;
    constexpr int textboxPaddingY = 5;
    constexpr float textboxWidth = 460.0f;

    loadImage("icons/business_logo.png", 0.05f, 10.1f, 70.0f);
    ImGui::SetCursorPos(ImVec2(18.0f, 90.0f));
    ImGui::Text("Welcome, please log in to continue");

    ImGui::SetCursorPos(ImVec2(24.0f, 150.0f));
    ImGui::Text("Username");

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(textboxPaddingX, textboxPaddingY)); // custom padding
    ImGui::SetCursorPos(ImVec2(18.0f, 180.0f));
    ImGui::SetNextItemWidth(textboxWidth); // width in pixels
    ImGui::InputText("##username", username, IM_ARRAYSIZE(username));
    ImGui::PopStyleVar();

    ImGui::SetCursorPos(ImVec2(24.0f, 220.0f));
    ImGui::Text("Password");

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(textboxPaddingX, textboxPaddingY)); // custom padding
    ImGui::SetCursorPos(ImVec2(18.0f, 250.0f));
    ImGui::SetNextItemWidth(textboxWidth); // width in pixels
    ImGui::InputText("##password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
    ImGui::PopStyleVar();

    ImGui::SetCursorPos(ImVec2(25.0f, 310.0f));
    if (setButtonCenter("Log In", fullWidthButtonSize(35)), ImGui::IsItemClicked()){

        // First, validate inputs are provided. Show failed popup when either field is empty.
        if (username[0] == '\0' || password[0] == '\0') {
            username[0] = '\0';
            password[0] = '\0';
            ui::g_failedMessage = "Error: Username and password are required.";
            appConfig::g_auth = false;

            // Mark the modal open and instruct ImGui to open it
            g_failedPopupOpen = true;
            ImGui::OpenPopup("Failed##login");
        }

        // Inputs present — proceed with authentication checks
        if (auth::testAuth(username, password)) {
            g_userName = std::string(username);
            appConfig::g_auth = true;
            appConfig::g_testMode = true;
            username[0] = '\0';
            password[0] = '\0';
            system::appShutdown();
        } else if (auth::testDeployAuth(username, password)) {
            g_userName = std::string(username);
            appConfig::g_auth = true;
            appConfig::g_testMode = false;
            username[0] = '\0';
            password[0] = '\0';
            system::appShutdown();
        } else {
            // Invalid credentials: show failed UI modal (reuse same mechanism)
            ui::g_failedMessage = "Error: Invalid username or password.";
            appConfig::g_auth = false;

            g_failedPopupOpen = true;
            ImGui::OpenPopup("Failed##login");
        }
    }

    // Render the failed modal if it's open. This must be called every frame so the popup stays visible
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

        if (g_failedPopupOpen || ImGui::IsPopupOpen("Failed##login")) {
            // Ensure the window size/pos are set for when the popup opens
            ImGui::SetNextWindowSize(ImVec2(420.0f, 180.0f), ImGuiCond_Appearing);
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }

        // Pass pointer so ImGui will clear flag when modal is closed by user actions
        if (ImGui::BeginPopupModal("Failed##login", &g_failedPopupOpen, flags)) {
            failedUI();
            ImGui::Spacing();
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                g_failedPopupOpen = false;
            }
            ImGui::EndPopup();
        }
    }

    ImGui::SetCursorPos(ImVec2(25.0f, 360.0f));
    if (setButtonCenter("Exit App", fullWidthButtonSize(35)), ImGui::IsItemClicked()) {system::appShutdown();}
}

static void accountUI() {
    // Load business logo only once on first call
    loadImage("icons/business_logo.png", 0.1f, 10.0f, 70.1f);
    loadImage("icons/user_icon.png", 0.8f, 10.0f, 60.0f);

    if (!appConfig::g_testMode) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f); // Small vertical spacing
        setTextRight(g_accountNumber.c_str());
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f); // Small vertical spacing
        setTextRight(g_userName.c_str());
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f); // Small vertical spacing
        setTextRight(g_position.c_str());
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f); // Small vertical spacing
    if (ImGui::Button("Log Out", fullWidthButtonSize(40))) {
        appConfig::g_auth = false;
        system::appShutdown();
    }
}


static void selectorUI() {
    // Greeting and username
    std::string l_greetings;
    if (const int hour = system::fetchTime(system::PartDateTime::HOUR); hour >= 0 && hour < 12) {
        l_greetings = "Good Morning,";
    }
    else if (hour >= 12 && hour < 18) {
        l_greetings = "Good Afternoon,";
    }
    else {
        l_greetings = "Good Evening,";
    }

    ImGui::SetCursorPos(ImVec2(20.0f, 10.0f));
    setTextCenter("appLogo"); // Placeholder for logo

    ImGui::SetCursorPos(ImVec2(20.0f, 80.0f)); // x = padding from left, y = small padding from top
    ImGui::SetWindowFontScale(1.7f); // Larger greeting
    ImGui::Text("%s", l_greetings.c_str());
    ImGui::SetWindowFontScale(1.0f); // Reset font scale
    ImGui::SetCursorPos(ImVec2(40.0f, 115.0f));
    ImGui::Text("%s", g_userName.c_str());

    // Navigation buttons control the right pane
    if (appConfig::g_testMode) {
        ImGui::SetCursorPos(ImVec2(8.0f, 210.0f));
        if (ImGui::Button("Test/Debug", fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("test")) g_rightUI = g_uiMap["test"]; }
    }
    else {
        ImGui::SetCursorPos(ImVec2(8.0f, 210.0f));
        if (ImGui::Button("Summary", fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("summary")) g_rightUI = g_uiMap["summary"]; }
        ImGui::SetCursorPos(ImVec2(8.0f, 320.0f));
        if (ImGui::Button("Payroll", fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("payroll")) g_rightUI = g_uiMap["payroll"]; }
        ImGui::SetCursorPos(ImVec2(8.0f, 430.0f));
        if (ImGui::Button("Expenses", fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("monitor")) g_rightUI = g_uiMap["monitor"]; }
    }

    // Top: Account info inside a bordered child, matching the two-column style
    ImGui::Spacing();
    ImGui::SetCursorPos(ImVec2(8.0f, 600.0f));
    const float lineH = ImGui::GetTextLineHeightWithSpacing();
    const float accountHeight = lineH * 10.0f + 12.0f; // 6 text lines + small padding

    // Push a custom background color (RGB: 0.25, 0.25, 0.25) for the next ImGui child window.
    // This creates a lighter gray panel that visually separates the account info from the surrounding UI
    // Value of rgb / 255 (e.g., the value is 177, then 177/255 = 0.69f)
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));

    ImGui::BeginChild("AccountPanel", ImVec2(0, accountHeight), true);
    accountUI();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::Spacing();
}


static void summaryUI() {
    ImGui::Text("Summary Panel");
}


static void payrollUI() {
    ImGui::Text("This is the Payroll UI");
}


static void monitorUI() {
    ImGui::Text("This is the Monitoring UI");
}


void testUI() {
    ImGui::Text("THIS IS ONLY FOR TESTING/DEBUGGING PURPOSE ONLY");

    static int keySizeBits = 128; // Default key size
    ImGui::InputInt("Key Size (bits)", &keySizeBits, 8, 64);

    static bool keyGenerated = false;
    static std::vector<unsigned char> storedKeyRaw;
    static std::string storedKeyHashed;
    static std::string storedKeyHashedSalted;
    static int storedKeySizeBits = 0;

    if (ImGui::Button("Generate Key")) {
        storedKeyRaw = cryptography::generateKey(keySizeBits);
        storedKeyHashed = cryptography::toHex(storedKeyRaw);
        storedKeyHashedSalted = cryptography::saltKey(storedKeyHashed);
        storedKeySizeBits = keySizeBits;
        keyGenerated = true;
    }

    if (keyGenerated) {
        ImGui::Spacing();
        const float wrapWidth = ImGui::GetContentRegionAvail().x;
        const float height = ImGui::GetTextLineHeightWithSpacing() +
                             ImGui::CalcTextSize(storedKeyHashed.c_str(), nullptr, false, wrapWidth).y +
                             ImGui::CalcTextSize(storedKeyHashedSalted.c_str(), nullptr, false, wrapWidth).y;
        ImGui::BeginChild("KeyDisplayPanel", ImVec2(0, height), true);
        ImGui::Text("The key size in bits is: %d", storedKeySizeBits);
        ImGui::TextWrapped("The key in hex: %s", storedKeyHashed.c_str());
        ImGui::TextWrapped("The key in hex and salted: %s", storedKeyHashedSalted.c_str());
        ImGui::EndChild();
    }

    // Test encryption and decryption
    static const std::string location = "test.txt";
    static std::string encryptionStatus;
    static std::string decryptionStatus;

    ImGui::Spacing();
    ImGui::Text("Encryption/Decryption Test");

    if (ImGui::Button("Encrypt")) {
        // Ensure we have a key; generate with the user-selected size if missing
        if (!keyGenerated || storedKeyRaw.empty()) {
            storedKeyRaw = cryptography::generateKey(keySizeBits);
            storedKeyHashed = cryptography::toHex(storedKeyRaw);
            storedKeyHashedSalted = cryptography::saltKey(storedKeyHashed);
            storedKeySizeBits = keySizeBits;
            keyGenerated = true;
        }

        if (storedKeyRaw.empty()) {
            encryptionStatus = "ERROR: Key generation failed. Cannot encrypt.";
        } else {
            if (cryptography::encryptFile(location, storedKeyRaw)) {
                encryptionStatus = "SUCCESS: File encrypted to " + location + ".enc";
                system::deleteFile(location); // Delete the unencrypted file after successful encryption
            } else {
                encryptionStatus = "ERROR: Encryption failed. Check if file exists and key is correct.";
            }
        }
    }

    if (ImGui::Button("Decrypt")) {
        if (!keyGenerated || storedKeyRaw.empty()) {
            decryptionStatus = "ERROR: No key available. Generate or load a key before decrypting.";
        } else {
            if (cryptography::decryptFile(location + ".enc", storedKeyRaw)) {
                decryptionStatus = "SUCCESS: File decrypted (replaced " + location + ".enc with decrypted content)";
                system::deleteFile(location + ".enc"); // Delete the encrypted file after successful decryption
            } else {
                decryptionStatus = "ERROR: Decryption failed. Check if encrypted file exists and key is correct.";
            }
        }
    }

    if (!encryptionStatus.empty()) {
        ImGui::Spacing();
        if (encryptionStatus.find("SUCCESS") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", encryptionStatus.c_str());
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", encryptionStatus.c_str());
        }
    }

    if (!decryptionStatus.empty()) {
        ImGui::Spacing();
        if (decryptionStatus.find("SUCCESS") != std::string::npos) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", decryptionStatus.c_str());
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", decryptionStatus.c_str());
        }
    }
}


// Main two-column layout: left = selector (with account), right = active panel (summary/payroll/monitor)
static void mainUI() {
    if (appConfig::g_testMode) {
        g_rightUI = testUI; // default right panel if in test mode
    } else {
        g_rightUI = summaryUI; // default right panel if not in test mode
    }

    // Full-viewport, borderless root so the main UI is in the app window itself
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // Use work area to avoid overlapping HelloImGui menu/dockspace areas
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    constexpr ImGuiWindowFlags rootFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoDocking; // prevent docking overlay from capturing inputs
    // Nudge focus to our root window for the first frames to ensure interactivity
    static int s_focusFrames = 30;
    if (s_focusFrames > 0) { ImGui::SetNextWindowFocus(); --s_focusFrames; }
    ImGui::Begin("##MainRoot", nullptr, rootFlags);

    const float total = ImGui::GetContentRegionAvail().x;
    const float leftWidth = std::clamp(total * 0.18f, 220.0f, 480.0f);
    // Left pane: fixed-width child so it does not move when resizing
    ImGui::BeginChild("LeftPane", ImVec2(leftWidth, 0), true);
    selectorUI();
    ImGui::EndChild();

    ImGui::SameLine();

    // Right pane: fills remaining width
    ImGui::BeginChild("RightPane", ImVec2(0, 0), true);
    if (g_rightUI) g_rightUI();
    ImGui::EndChild();

    ImGui::End();
}


void ui::constructUI(const std::string &a_title, const std::string& a_fontLocation, const int a_widthPx, const int a_heightPx, const std::string& a_window) {
    HelloImGui::RunnerParams params;

    namespace fs = std::filesystem;
    for (const auto &base : {fs::current_path(), fs::path(__FILE__).parent_path().parent_path().parent_path()}) {
        if (const fs::path assets = base / "assets"; fs::exists(assets)) {
            HelloImGui::SetAssetsFolder(assets.string());
            break;
        }
    }

    // Ensure HelloImGui does not create a DockSpace: keep the default full-screen window
    // (ProvideFullScreenWindow). No explicit docking toggle available in this version.
    params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenWindow;

    // populate UI registry (ensure it's available before selecting current UI)
    g_uiMap.clear();
    g_uiMap.reserve(7);
    g_uiMap["auth"] = loginUI;
    g_uiMap["main"] = mainUI;
    g_uiMap["summary"] = summaryUI;
    g_uiMap["payroll"] = payrollUI;
    g_uiMap["monitor"] = monitorUI;
    g_uiMap["test"] = testUI;
    g_uiMap["failed"] = failedUI;

    // Load a custom font with only the default ASCII character set to save memory.
    // By providing this callback, we take control of font loading.
    params.callbacks.LoadAdditionalFonts = [a_fontLocation]() {
        const ImGuiIO& io = ImGui::GetIO();
        // Clear any existing fonts to ensure we only load what we need.
        io.Fonts->Clear();

        const std::string fontPath = HelloImGui::AssetFileFullPath(a_fontLocation);

        ImFontConfig fontConfig;
        // This is the crucial part for memory saving: load only the default character set.
        fontConfig.GlyphRanges = io.Fonts->GetGlyphRangesDefault();

        if (!fontPath.empty()) {
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 22.0f, &fontConfig);
        } else {
            // As a fallback, load ImGui's default font if our custom one fails, but still with the limited character set.
            io.Fonts->AddFontDefault(&fontConfig);
        }
    };

    // Determine start key and select initial UI

    if (const std::string startKey = a_window.empty() ? "main" : toLower(a_window); startKey == "auth") {
        g_currentUI = loginUI;
    }
    else if (startKey == "payroll") {
        if (g_uiMap.contains("payroll")) g_rightUI = g_uiMap["payroll"]; else g_rightUI = payrollUI;
        g_currentUI = mainUI;
    }
    else if (startKey == "monitor") {
        if (g_uiMap.contains("monitor")) g_rightUI = g_uiMap["monitor"]; else g_rightUI = monitorUI;
        g_currentUI = mainUI;
    }
    else if (startKey == "test") {
        if (g_uiMap.contains("test")) g_rightUI = g_uiMap["test"]; else g_rightUI = testUI;
        g_currentUI = mainUI;
    }
    else if (startKey == "summary" || startKey == "main") {
        if (g_uiMap.contains("summary")) g_rightUI = g_uiMap["summary"]; else g_rightUI = summaryUI;
        g_currentUI = mainUI;
    }
    else {
        ui::g_failedMessage = "Unknown start window: " + startKey;
        if (g_uiMap.contains("summary")) g_rightUI = g_uiMap["summary"]; else g_rightUI = summaryUI;
        g_currentUI = mainUI;
    }

    //Use a wrapper so we can call the current UI each frame (no queued switching)
    params.callbacks.ShowGui = []() {if (g_currentUI) g_currentUI();};

    // Window and GUI settings
    // clamp sizes to reasonable bounds so caller can't accidentally create tiny or huge windows
    const int l_clampedWidth = std::clamp(a_widthPx, 50, 3840);
    const int l_clampedLength = std::clamp(a_heightPx, 50, 2160);
    params.appWindowParams.windowGeometry.size = { l_clampedWidth, l_clampedLength };

    // Rename the whole application to "system" if there is no argument/s in the variable "title"
    params.appWindowParams.windowTitle = a_title.empty() ? "system" : a_title;

    HelloImGui::Run(params);
}
