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

        # SQLite dependency
        dependencies/sqlite/sqlite3.c
        dependencies/sqlite/sqlite3.h

        # The main application file
        src/main.cpp

        # The Database I/O handler
        src/lib/db.cpp
        src/lib/db.h

        # The User Interface handler
        src/lib/UI.cpp
        src/lib/UI.h

        # The Inventory system
        src/lib/inventory.cpp
        src/lib/inventory.h

        # The Point-of-Sale system
        src/lib/pos.cpp
        src/lib/pos.h
        ASSETS_LOCATION "assets/"
)
