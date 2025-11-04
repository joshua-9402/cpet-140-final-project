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
 */

#include "hello_imgui/hello_imgui.h"
#include "imgui.h"
#include <string>
#include <functional>
#include <unordered_map>
#include <algorithm>    // for transform
#include <ranges>       // added: for std::ranges::transform
#include <cctype>       // added: for std::tolower

// UI registry and current UI management
static std::unordered_map<std::string, std::function<void()>> g_uiMap;
static std::function<void()> g_currentUI = nullptr;
static std::string g_failedMessage; // holds message shown by failedUI

// Lowercase helper used by both constructUI and switchToUI
static std::string toLower(std::string s)
{
    std::ranges::transform(s, s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

void failedUI()
{
    ImGui::Text("%s", g_failedMessage.c_str());
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

// Request a switch to another UI by name. Perform the switch immediately (case-insensitive).
void switchToUI(const std::string& name)
{
    const std::string key = toLower(name);
    auto it = g_uiMap.find(key);
    if (it != g_uiMap.end()) {
        g_currentUI = it->second;
    } else {
        g_failedMessage = "Unknown UI: " + name;
        g_currentUI = failedUI;
    }
}

void mainUI() {
    ImGui::Text("Hello, BCpET 1101!");
    ImGui::Text("This is the Main Window");

    if (ImGui::Button("Switch to POS")) switchToUI("pos");
    if (ImGui::Button("Switch to Inventory")) switchToUI("inventory");
    if (ImGui::Button("Close Application"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

void posUI() {
    ImGui::Text("This is the POS UI");

    if (ImGui::Button("Switch to Main")) switchToUI("main");
}

void inventoryUI() {
    ImGui::Text("This is the Inventory UI");

    if (ImGui::Button("Switch to Main")) switchToUI("main");
}

void constructUI(const std::string &title, const std::string& font_location, const int width, const int length, const std::string& window) {
    HelloImGui::RunnerParams params;

    // populate UI registry (ensure it's available before selecting current UI)
    g_uiMap.clear();

    g_uiMap.reserve(4);
    g_uiMap["main"] = mainUI;
    g_uiMap["inventory"] = inventoryUI;
    g_uiMap["pos"] = posUI;
    g_uiMap["failed"] = failedUI;

    // Load your custom font from assets/fonts, with fallback to default font
    params.callbacks.LoadAdditionalFonts = [font_location]()
    {
        ImGuiIO& io = ImGui::GetIO();
        const std::string fontPath = HelloImGui::AssetFileFullPath(font_location);
        ImFont* font = nullptr; // fixed: proper pointer type
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
    const std::string startKey = window.empty() ? "main" : toLower(window);

    // initialize current UI based on requested window (case-insensitive)
    auto it = g_uiMap.find(startKey);
    if (it != g_uiMap.end()) {
        g_currentUI = it->second;
    } else {
        // fallback to main if unknown, but keep failed registered for explicit errors
        g_currentUI = g_uiMap.contains("main") ? g_uiMap["main"] : failedUI;
    }

    // Use a wrapper so we can call the current UI each frame (no queued switching)
    params.callbacks.ShowGui = []() {
        if (g_currentUI) g_currentUI();
    };

    // Window and GUI settings
    // clamp sizes to reasonable bounds so caller can't accidentally create tiny or huge windows
    int clampedW = std::clamp(width, 640, 3840);
    int clampedH = std::clamp(length, 480, 2160);
    params.appWindowParams.windowTitle = title.empty() ? "App" : title;
    params.appWindowParams.windowGeometry.size = { clampedW, clampedH };

    HelloImGui::Run(params);
}