#include "hello_imgui/hello_imgui.h"
#include "imgui.h"

void MainGUI()
{
    ImGui::Text("Hello, BCpET 1101!");
    if (ImGui::Button("Exit"))
        HelloImGui::GetRunnerParams()->appShallExit = true;
}

void POS() {}

void Inventory() {}


int main()
{
    HelloImGui::RunnerParams params;

    // Load your custom font from assets/fonts
    params.callbacks.LoadAdditionalFonts = []()
    {
        const ImGuiIO& io = ImGui::GetIO();
        const std::string fontPath = HelloImGui::AssetFileFullPath("fonts/OpenSans-Regular.ttf");
        io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 22.0f);
    };

    // Window and GUI settings
    params.callbacks.ShowGui = MainGUI;
    params.appWindowParams.windowTitle = "CpET 140 - Final Project";
    params.appWindowParams.windowGeometry.size = {1600, 900};

    HelloImGui::Run(params);
    return 0;
}