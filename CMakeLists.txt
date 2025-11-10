cmake_minimum_required(VERSION 4.0) # minimum CMake version
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
        payroll-and-monitoring-system

        # dependency
        dependencies/sqlite/sqlite3.c
        dependencies/sqlite/sqlite3.h

        # The main application file
        src/main.cpp


        # I/O handlers
        src/handler/db.cpp
        src/handler/db.h

        src/ui/ui.cpp
        src/ui/ui.h

        src/handler/system.cpp
        src/handler/system.h


        # The Systems
        src/lib/payroll.cpp
        src/lib/payroll.h

        src/lib/monitor.cpp
        src/lib/monitor.h


        # Security System
        src/security/auth.cpp
        src/security/auth.h

        src/security/cryptography.cpp
        src/security/cryptography.h


        # Assets location
        ASSETS_LOCATION "assets/"
)
