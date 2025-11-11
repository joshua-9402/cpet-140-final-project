/*
 * CpET 140 Final Project — UI module
 * Payroll and Monitoring System - UI module
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
 * - Uses C++17 features like std::unordered_map and std::function for flexibility.
 */

#include <iostream>
#include <string>
#include <functional>
#include <unordered_map>
#include <algorithm>    // for transform
#include <ranges>       // added: for std::ranges::transform
#include "hello_imgui/hello_imgui.h"
#include "imgui.h"
#include "ui.h"
#include "../handler/system.h"


// UI registry and UI management
static std::unordered_map<std::string, std::function<void()>> g_uiMap;
static std::function<void()> g_currentUI = nullptr;
static std::function<void()> g_rightUI = nullptr; // Right panel active UI (shown in main two-column layout)
auto g_buttonSizePxSelector = ImVec2(270, 40); // x for width, y for height of buttons
std::string ui::g_failedMessage; // Global failed message for failedUI
const std::string g_userName = "testUserName";
const std::string g_companyName = "testCompanyName";
const std::string g_position = "testPosition";


// Lowercase helper used by both constructUI and switchToUI
static std::string toLower(std::string s) {
    std::ranges::transform(s, s.begin(), [](const unsigned char c){ return std::tolower(c); });
    return s;
}


static ImVec2 fullWidthButtonSize(const float a_height = g_buttonSizePxSelector.y) { return ImVec2(ImGui::GetContentRegionAvail().x, a_height); }


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


static void failedUI() {
    ImGui::Text("%s", ui::g_failedMessage.c_str());
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}


static void accountUI() {
    ImGui::Text("Name:");
    setTextRight(g_userName.c_str());
    ImGui::Text("Position:");
    setTextRight(g_position.c_str());
    ImGui::Text("Company Name");
    setTextRight(g_companyName.c_str());
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
    ImGui::SetCursorPos(ImVec2(8.0f, 200.0f));
    if (ImGui::Button("Summary", fullWidthButtonSize())) { if (g_uiMap.contains("summary")) g_rightUI = g_uiMap["summary"]; }
    if (ImGui::Button("Payroll", fullWidthButtonSize())) { if (g_uiMap.contains("payroll")) g_rightUI = g_uiMap["payroll"]; }
    if (ImGui::Button("Expenses", fullWidthButtonSize())) { if (g_uiMap.contains("monitor")) g_rightUI = g_uiMap["monitor"]; }

    // Top: Account info inside a bordered child, matching the two-column style
    ImGui::Spacing();
    const float lineH = ImGui::GetTextLineHeightWithSpacing();
    const float accountHeight = lineH * 6.0f + 12.0f; // 6 text lines + small padding
    ImGui::BeginChild("AccountPanel", ImVec2(0, accountHeight), true);
    accountUI();
    ImGui::EndChild();
    ImGui::Spacing();

    if (ImGui::Button("Exit", fullWidthButtonSize())) HelloImGui::GetRunnerParams()->appShallExit = true;
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


// Main two-column layout: left = selector, right = active panel (summary/payroll/monitor)
static void mainUI() {
    if (!g_rightUI) g_rightUI = summaryUI; // default right panel

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


void ui::constructUI(const std::string &a_title, const std::string& a_fontLocation, const int a_widthPx, const int a_lengthPx, const std::string& a_window) {
    HelloImGui::RunnerParams params;
    // Ensure HelloImGui does not create a DockSpace: keep the default full-screen window
    // (ProvideFullScreenWindow). No explicit docking toggle available in this version.
    params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenWindow;

    // populate UI registry (ensure it's available before selecting current UI)
    g_uiMap.clear();
    g_uiMap.reserve(6);
    g_uiMap["main"] = mainUI;
    g_uiMap["summary"] = summaryUI;
    g_uiMap["payroll"] = payrollUI;
    g_uiMap["monitor"] = monitorUI;
    g_uiMap["account"] = accountUI;
    g_uiMap["failed"] = failedUI;

    // Load your custom font from assets/fonts, with fallback to default font
    params.callbacks.LoadAdditionalFonts = [a_fontLocation]()
    {
        const ImGuiIO& io = ImGui::GetIO();
        const std::string fontPath = HelloImGui::AssetFileFullPath(a_fontLocation);
        const ImFont* font = nullptr; // fixed: proper pointer type
        if (!fontPath.empty()) {
            font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 22.0f);
        }
        if (!font) {
            // fallback to default font so UI remains readable
            io.Fonts->AddFontDefault();
        }
        io.Fonts->Build();
    };

    // Determine start key and select initial right panel; always render main layout
    if (const std::string startKey = a_window.empty() ? "main" : toLower(a_window); startKey == "payroll") {
        if (g_uiMap.contains("payroll")) g_rightUI = g_uiMap["payroll"]; else g_rightUI = payrollUI;
    }
    else if (startKey == "monitor") {
        if (g_uiMap.contains("monitor")) g_rightUI = g_uiMap["monitor"]; else g_rightUI = monitorUI;
    }
    else if (startKey == "summary" || startKey == "main") {
        if (g_uiMap.contains("summary")) g_rightUI = g_uiMap["summary"]; else g_rightUI = summaryUI;
    }
    else {
        ui::g_failedMessage = "Unknown start window: " + startKey;
        if (g_uiMap.contains("summary")) g_rightUI = g_uiMap["summary"]; else g_rightUI = summaryUI;
    }
    g_currentUI = mainUI;

    //Use a wrapper so we can call the current UI each frame (no queued switching)
    params.callbacks.ShowGui = []() {if (g_currentUI) g_currentUI();};

    // Window and GUI settings
    // clamp sizes to reasonable bounds so caller can't accidentally create tiny or huge windows
    const int l_clampedWidth = std::clamp(a_widthPx, 640, 3840);
    const int l_clampedLength = std::clamp(a_lengthPx, 480, 2160);
    params.appWindowParams.windowGeometry.size = { l_clampedWidth, l_clampedLength };

    // Rename the whole application to "system" if there is no argument/s in the variable "title"
    params.appWindowParams.windowTitle = a_title.empty() ? "system" : a_title;

    HelloImGui::Run(params);
}