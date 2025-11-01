#include "hello_imgui/hello_imgui.h"
#include "imgui.h"
#include <string>
#include <functional>
#include <unordered_map>

// UI registry and current UI management
static std::unordered_map<std::string, std::function<void()>> g_uiMap;
static std::function<void()> g_currentUI = nullptr;
static std::string g_nextUIName;
static bool g_pendingSwitch = false;

void failedUI()
{
    ImGui::Text("Failed to launch application/invalid argument.");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

// Request a switch to another UI by name. Perform the switch immediately.
void switchToUI(const std::string& name)
{
    auto it = g_uiMap.find(name);
    if (it != g_uiMap.end()) g_currentUI = it->second;
    else g_currentUI = failedUI;
}

void mainUI() {
    ImGui::Text("Hello, BCpET 1101!");
    ImGui::Text("This is the Main Window");

    if (ImGui::Button("Switch to POS")) switchToUI("pos");
    if (ImGui::Button("Switch to Inventory")) switchToUI("inventory");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

void testUI()
{
    ImGui::Text("Hello, BCpET 1101!");
    ImGui::Text("This is the Test Window");

    if (ImGui::Button("Switch to Main")) switchToUI("main");
    if (ImGui::Button("Switch to POS")) switchToUI("pos");
    if (ImGui::Button("Switch to Inventory")) switchToUI("inventory");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

void posUI() {
    ImGui::Text("This is the POS UI");

    if (ImGui::Button("Switch to Main")) switchToUI("main");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

void inventoryUI() {
    ImGui::Text("This is the Inventory UI");

    if (ImGui::Button("Switch to Main")) switchToUI("main");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

void constructUI(const std::string &title, const std::string& font_location, const int width, const int length, const std::string& window) {
    HelloImGui::RunnerParams params;

    // Load your custom font from assets/fonts
    params.callbacks.LoadAdditionalFonts = [font_location]()
    {
        const ImGuiIO& io = ImGui::GetIO();
        const std::string fontPath = HelloImGui::AssetFileFullPath(font_location);
        io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 22.0f);
    };

    // populate UI registry
    g_uiMap.clear();
    g_uiMap["main"] = mainUI;
    g_uiMap["test"] = testUI;
    g_uiMap["inventory"] = inventoryUI;
    g_uiMap["pos"] = posUI;
    g_uiMap["failed"] = failedUI;

    // initialize current UI based on requested window
    auto it = g_uiMap.find(window);
    if (it != g_uiMap.end()) {
        g_currentUI = it->second;
    } else {
        g_currentUI = failedUI;
    }
    g_pendingSwitch = false;
    g_nextUIName.clear();

    // Use a wrapper so we can call the current UI each frame (no queued switching)
    params.callbacks.ShowGui = []() {
        if (g_currentUI) g_currentUI();
    };

    // Window and GUI settings
    params.appWindowParams.windowTitle = title;
    params.appWindowParams.windowGeometry.size = {width, length};

    HelloImGui::Run(params);
}