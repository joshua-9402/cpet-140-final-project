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
 * - Provides the main two-column layout with navigation on the left and an active panel on the right.
 *
 * Boundaries
 * - Presentation only. No business logic or persistence here. Call payroll.cpp and monitor.cpp modules for operations.
 *
 * Notes
 * - UI switching is immediate (case-insensitive). App exit uses HelloImGui runner.
 * - This part is OFF LIMITS, especially to constructUI(), the global variables, and to the UI registry/map.
 *
 * jslbuilders@victoria
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
#include "../ui/modules/accountUI.h"
#include "modules/loginUI.h"
#include "modules/summaryUI.h"
#include "modules/monitorUI.h"
#include "modules/payrollUI.h"
#include "modules/testUI.h"


// stb_image is provided by hello_imgui dependencies for icon loading.
// Fallback prototypes are declared if the header is unavailable.
#if __has_include("stb_image.h")
#include "stb_image.h"
#else
    extern "C" {
        unsigned char* stbi_load(const char* filename, int* x, int* y, int* comp, int req_comp);
        void stbi_image_free(void* retval_from_stbi_load);
    }
#endif

// Conditional includes for backend window icon support
#if __has_include(<GLFW/glfw3.h>)
#include <GLFW/glfw3.h>
#define UI_HAVE_GLFW 1
#endif

#if __has_include(<SDL.h>)
#include <SDL.h>
#define UI_HAVE_SDL 1
#endif


// UI registry and UI management
static std::unordered_map<std::string, std::function<void()>> g_uiMap;
static std::function<void()> g_currentUI = nullptr;
static std::function<void()> g_rightUI = nullptr; // Right panel active UI (shown in the main two-column layout)
auto g_buttonSizePxSelector = ImVec2(270, 40); // x for width, y for height of buttons
std::string ui::g_failedMessage; // Global failed message for failedUI
std::string ui::g_userName;
std::string ui::g_position;



// Lowercase helper used by both constructUI and switchToUI
static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}


// Small helpers to keep UI code concise without changing behavior
void ui::renderStatCard(
    const char* childId,
    const char* title,
    const ImVec4& titleColor,
    const std::string& valueText,
    const char* subtitle,
    const ImVec2& size
) {
    ImGui::BeginChild(childId, size, true);
    ImGui::SetCursorPosY(15.0f);
    ImGui::SetCursorPosX(20.0f);
    ImGui::TextColored(titleColor, "%s", title);
    ImGui::SetCursorPosX(20.0f);
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("%s", valueText.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::SetCursorPosY(size.y - 30.0f);
    ImGui::SetCursorPosX(20.0f);
    ImGui::TextDisabled("%s", subtitle);
    ImGui::EndChild();
}

// Keep exact normalization semantics used previously for PRJ-xxxxx formatting
std::string ui::normalizeProjectId(const std::string& input) {
    std::string projectIDStr = input;
    if (!projectIDStr.empty()) {
        if (projectIDStr.length() != 10 || projectIDStr.substr(0, 4) != "PRJ-") {
            std::string digits;
            for (char ch : projectIDStr) {
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    digits.push_back(ch);
                }
            }
            if (!digits.empty()) {
                long long idVal = 0;
                try { idVal = std::stoll(digits); } catch (...) { idVal = 0; }
                std::ostringstream oss;
                oss << "PRJ-" << std::setw(5) << std::setfill('0') << idVal;
                projectIDStr = oss.str();
            }
        }
    }
    return projectIDStr;
}

std::string ui::buildProjectReportOutFile(const std::string& projectID) {
    return appConfig::g_dataDirectory + "project_report_" + projectID + "_" +
           std::to_string(system::fetchTime(system::PartDateTime::YEAR)) + "_" +
           std::to_string(system::fetchTime(system::PartDateTime::MONTH)) + "_" +
           std::to_string(system::fetchTime(system::PartDateTime::DAY)) + ".html";
}


ImVec2 ui::fullWidthButtonSize(const float a_height = g_buttonSizePxSelector.y) { return {ImGui::GetContentRegionAvail().x, a_height}; }


void ui::setButtonCenter(const char* text, const ImVec2& size){
    const float windowWidth = ImGui::GetWindowSize().x;
    const float buttonWidth = size.x;

    // Move to the center position
    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    ImGui::Button(text, size);
}


void ui::setTextCenter(const char* text){
    const float windowWidth = ImGui::GetWindowSize().x;
    const float textWidth = ImGui::CalcTextSize(text).x;

    // Move to the center position
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text("%s", text);
}


void ui::setTextRight(const char* text){
    const float windowWidth = ImGui::GetWindowSize().x;
    const float textWidth = ImGui::CalcTextSize(text).x;

    // Position the cursor so that the text ends at the right edge
    ImGui::SetCursorPosX(windowWidth - textWidth - ImGui::GetStyle().WindowPadding.x);
    ImGui::Text("%s", text);
}


void ui::loadImage(const std::string& p_location, const float p_locationXPx, const float p_locationYPx, const float p_imageSize) {
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
        if (auto* params = HelloImGui::GetRunnerParams()) {
            params->appShallExit = true;
        }
    }
}


static void loginUI() {
    loginUI::displayLogin();
}

static void accountUI() {
    account::displayAccount();
}


static void summaryUI() {
    summary::displaySummary();
}


static void payrollUI() {
    payrollUI::displayPayroll();
}


static void monitorUI() {
    monitorUI::displayMonitor();
}


static void testUI() {
    testUI::displayTest();
}


static void selectorUI() {
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
    ui::loadImage("icons/app_icon.png", 0.5f, 10.0f, 64.0f); // Centered application logo from assets/app_logo.png
    ImGui::SetCursorPos(ImVec2(87.0f, 65.0f));
    ImGui::Text("StructuraCost");

    ImGui::SetCursorPos(ImVec2(20.0f, 110.0f)); // x = padding from left, y = small padding from the top
    ImGui::SetWindowFontScale(1.7f); // Larger greeting
    ImGui::Text("%s", l_greetings.c_str());
    ImGui::SetWindowFontScale(1.0f); // Reset font scale
    ImGui::SetCursorPos(ImVec2(60.0f, 155.0f));
    ImGui::Text("%s", ui::g_userName.c_str());

    // Navigation buttons control the right pane
    if (appConfig::g_testMode) {
        ImGui::SetCursorPos(ImVec2(8.0f, 220.0f));
        if (ImGui::Button("Test/Debug", ui::fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("test")) g_rightUI = g_uiMap["test"]; }
    }
    else {
        ImGui::SetCursorPos(ImVec2(8.0f, 220.0f));
        if (ImGui::Button("Summary", ui::fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("summary")) g_rightUI = g_uiMap["summary"]; }
        ImGui::SetCursorPos(ImVec2(8.0f, 330.0f));
        if (ImGui::Button("Payroll Module", ui::fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("payroll")) g_rightUI = g_uiMap["payroll"]; }
        ImGui::SetCursorPos(ImVec2(8.0f, 440.0f));
        if (ImGui::Button("Monitoring Module", ui::fullWidthButtonSize(g_buttonSizePxSelector.y + 50))) { if (g_uiMap.contains("monitor")) g_rightUI = g_uiMap["monitor"]; }
    }

    // Top: Account info inside a bordered child, matching the two-column style
    ImGui::Spacing();
    ImGui::SetCursorPos(ImVec2(8.0f, 650.0f));
    const float lineH = ImGui::GetTextLineHeightWithSpacing();
    const float accountHeight = lineH * 8.5f;

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


// Main two-column layout: left = selector (with an account), right = active panel (summary/payroll/monitor)
static void mainUI() {
    // Only set a default right panel on the first call, not every frame
    static bool initialized = false;
    if (!initialized) {
        if (appConfig::g_testMode) {
            g_rightUI = testUI; // default right panel if in test mode
        } else {
            g_rightUI = summaryUI; // default right panel if not in test mode
        }
        initialized = true;
    }

    // Full-viewport, borderless root, so the main UI is in the app window itself
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // Use work area to avoid overlapping HelloImGui menu/dock space areas
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    constexpr ImGuiWindowFlags rootFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoDocking; // to prevent docking overlay from capturing inputs,
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

    params.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenWindow;

    // populate the UI registry (ensure it's available before selecting the current UI)
    g_uiMap.clear();
    g_uiMap.reserve(7);
    g_uiMap["auth"] = loginUI;
    g_uiMap["main"] = mainUI;
    g_uiMap["summary"] = summaryUI;
    g_uiMap["payroll"] = payrollUI;
    g_uiMap["monitor"] = monitorUI;
    g_uiMap["test"] = testUI;
    g_uiMap["failed"] = failedUI;

    params.callbacks.LoadAdditionalFonts = [a_fontLocation]() {
        const ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        const std::string fontPath = HelloImGui::AssetFileFullPath(a_fontLocation);

        ImFontConfig fontConfig;
        fontConfig.GlyphRanges = io.Fonts->GetGlyphRangesDefault();

        if (!fontPath.empty()) {
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 22.0f, &fontConfig);
        } else {
            io.Fonts->AddFontDefault(&fontConfig);
        }
    };

    params.callbacks.PostInit = [&params]() {
        std::string iconPath = HelloImGui::AssetFileFullPath("icons/app_icon.png");
        if (iconPath.empty()) iconPath = HelloImGui::AssetFileFullPath("icons/business_logo.png");
        if (iconPath.empty()) return;

        int w = 0, h = 0, comps = 0;
        unsigned char* pixels = stbi_load(iconPath.c_str(), &w, &h, &comps, 4);
        if (!pixels) return;

        auto bp = params.backendPointers;

    #ifdef UI_HAVE_GLFW
        if (bp.glfwWindow) {
            GLFWimage img;
            img.width = w;
            img.height = h;
            img.pixels = pixels;
            glfwSetWindowIcon(static_cast<GLFWwindow*>(bp.glfwWindow), 1, &img);
            stbi_image_free(pixels);
            return;
        }
#endif

#ifdef UI_HAVE_SDL
        if (bp.sdlWindow) {
            SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
                pixels, w, h, 32, w * 4, SDL_PIXELFORMAT_RGBA32);
            if (surf) {
                SDL_SetWindowIcon(reinterpret_cast<SDL_Window*>(bp.sdlWindow), surf);
                SDL_FreeSurface(surf);
            }
            stbi_image_free(pixels);
            return;
        }
#endif

        stbi_image_free(pixels);
    };

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

    params.callbacks.ShowGui = []() {if (g_currentUI) g_currentUI();};

    const int l_clampedWidth = std::clamp(a_widthPx, 50, 3840);
    const int l_clampedLength = std::clamp(a_heightPx, 50, 2160);
    params.appWindowParams.windowGeometry.size = { l_clampedWidth, l_clampedLength };
    params.appWindowParams.windowTitle = a_title.empty() ? "system" : a_title;

    HelloImGui::Run(params);
}

