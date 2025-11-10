/*
 * CpET 140 Final Project — UI module
 *
 * Contributors: Joshua Literal
 *
 * Purpose
 * - Immediate-mode UI built with HelloImGui/ImGui. Manages a registry of UIs,
 *   switching between them, font loading, and window setup.
 *
 * Boundaries
 * - Presentation only: no business logic or persistence here. Call POS/Inventory
 *   modules for operations.
 *
 * Notes
 * - UI switching is immediate (case-insensitive). App exit uses HelloImGui runner.
 * - This part is OFF LIMITS especially to constructUI()
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


// UI registry and current UI management
static std::unordered_map<std::string, std::function<void()>> g_uiMap;
static std::function<void()> g_currentUI = nullptr;
std::string ui::g_failedMessage;


// UI element size variables
auto g_buttonSizePx = ImVec2(200, 40); // x for width, y for height


static void failedUI() {
    ImGui::Text("%s", ui::g_failedMessage.c_str());
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}


// Lowercase helper used by both constructUI and switchToUI
static std::string toLower(std::string s) {
    std::ranges::transform(s, s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}


// Request a switch to another UI by name. Perform the switch immediately (case-insensitive).
static void switchToUI(const std::string& name) {
    const std::string key = toLower(name);
    if (const auto it = g_uiMap.find(key); it != g_uiMap.end()) {
        g_currentUI = it->second;
    } else {
        ui::g_failedMessage = "Unknown UI: " + name;
        g_currentUI = failedUI;
    }
}


static void selectorUI() {
    ImGui::Text("Hello, BCpET 1101!");

    if (ImGui::Button("Payroll UI", g_buttonSizePx)) switchToUI("payroll");
    if (ImGui::Button("Monitoring System", g_buttonSizePx)) switchToUI("monitor");

    if (ImGui::Button("Exit", g_buttonSizePx))HelloImGui::GetRunnerParams()->appShallExit = true;}


static void payrollUI() {
    ImGui::Text("This is the Payroll UI");

    if (ImGui::Button("Switch to Main")) switchToUI("main");
}


static void monitorUI() {
    ImGui::Text("This is the Monitoring UI");

    if (ImGui::Button("Switch to Main")) switchToUI("main");
}


void ui::constructUI(const std::string &a_title, const std::string& a_fontLocation, const int a_widthPx, const int a_lengthPX, const std::string& a_window) {
    HelloImGui::RunnerParams params;

    // populate UI registry (ensure it's available before selecting current UI)
    g_uiMap.clear();
    g_uiMap.reserve(4);
    g_uiMap["main"] = selectorUI;
    g_uiMap["monitor"] = monitorUI;
    g_uiMap["payroll"] = payrollUI;
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

    // normalize and sanitize inputs (case-insensitive start key)
    const std::string startKey = a_window.empty() ? "main" : toLower(a_window);

    // initialize current UI based on requested window (case-insensitive)
    if (const auto it = g_uiMap.find(startKey); it != g_uiMap.end()) {
        g_currentUI = it->second;
    } else {
        // fallback to main if unknown, but keep failed registered for explicit errors
        g_currentUI = g_uiMap.contains("main") ? g_uiMap["main"] : failedUI;
    }

    /*
     * Use a wrapper so we can call the current UI each frame (no queued switching)
     *
     * !!!!!!!!!!!!!!! DO NOT DELETE THIS PART !!!!!!!!!!!!!!!
     */
    params.callbacks.ShowGui = []() {if (g_currentUI) g_currentUI();};

    // Window and GUI settings
    // clamp sizes to reasonable bounds so caller can't accidentally create tiny or huge windows
    const int l_clampedWidth = std::clamp(a_widthPx, 640, 3840);
    const int l_clampedLength = std::clamp(a_lengthPX, 480, 2160);
    params.appWindowParams.windowGeometry.size = { l_clampedWidth, l_clampedLength };

    // Rename the whole application to "system" if there is no argument/s in the variable "title"
    params.appWindowParams.windowTitle = a_title.empty() ? "system" : a_title;

    HelloImGui::Run(params);
}