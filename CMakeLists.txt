cmake_minimum_required(VERSION 4.0)
project(cpet_140_final_project) # project name
set(CMAKE_CXX_STANDARD 26) # set C++ standard to C++26

# Fetch Hello ImGui (automatic download)
include(FetchContent)
FetchContent_Declare(
        hello_imgui
        GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
)
FetchContent_MakeAvailable(hello_imgui)

# Add your app
hello_imgui_add_app(
        pos-and-inventory-system
        src/main.cpp
        ASSETS_LOCATION "assets/"
)
