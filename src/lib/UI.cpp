#include "hello_imgui/hello_imgui.h"
#include "imgui.h"
#include <string>

void mainUI() {
    ImGui::Text("Hello, BCpET 1101!");
    ImGui::Text("This is the Main Window");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

void testUI()
{
    ImGui::Text("Hello, BCpET 1101!");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

void failedUI()
{
    ImGui::Text("Failed to launch application/invalid argument.");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

void posUI() {}

void inventoryUI() {}

void constructUI(const std::string &title, const std::string& font_location, const int width, const int length, const std::string& window) {
    HelloImGui::RunnerParams params;

    // Load your custom font from assets/fonts
    params.callbacks.LoadAdditionalFonts = [font_location]()
    {
        const ImGuiIO& io = ImGui::GetIO();
        const std::string fontPath = HelloImGui::AssetFileFullPath(font_location);
        io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 22.0f);
    };

    // Window and GUI settings
    if (window == "main") {
        params.callbacks.ShowGui = mainUI;
    }
    else if (window == "test") {
        params.callbacks.ShowGui = testUI;
    }
    else if (window == "inventory") {
        params.callbacks.ShowGui = inventoryUI;
    }
    else if (window == "pos") {
        params.callbacks.ShowGui = posUI;
    }
    else {
        params.callbacks.ShowGui = failedUI;
    }
    params.appWindowParams.windowTitle = title;
    params.appWindowParams.windowGeometry.size = {width, length};

    HelloImGui::Run(params);
}