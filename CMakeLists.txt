cmake_minimum_required(VERSION 3.22) # minimum CMake version
project(cpet_140_final_project) # project name
set(CMAKE_CXX_STANDARD 20) # set C++ standard to C++20 (AppleClang doesn't support C++26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch Hello ImGui (automatic download)
include(FetchContent)

# Use bundled FreeType to avoid version conflicts with system libraries
set(HELLOIMGUI_USE_FREETYPE_BUNDLE ON CACHE BOOL "Use HelloImGui's bundled FreeType" FORCE)

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

        # App config
        src/config/app_config.cpp
        src/config/app_config.h


        # Handlers
        src/handler/auth.cpp
        src/handler/auth.h

        src/handler/db.cpp
        src/handler/db.h

        src/handler/system.cpp
        src/handler/system.h


        # UI Components
        src/ui/ui.cpp
        src/ui/ui.h


        # The Main Systems
        src/core/payroll.cpp
        src/core/payroll.h

        src/core/monitor.cpp
        src/core/monitor.h


        # Assets location
        ASSETS_LOCATION "assets/"
)

# Add include directory for SQLite3 headers
target_include_directories(payroll-and-monitoring-system PRIVATE
    ${CMAKE_SOURCE_DIR}/dependencies/sqlite
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    message(STATUS "Release build configured with -O3 optimization.")
endif()

# GCC and Clang optimization flags
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Note: Removed -march=native for better CI/CD compatibility
    # LTO can be disabled by setting -DNO_LTO=ON if it causes issues
    if(NOT NO_LTO)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -DNDEBUG")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    endif()
endif()

# MSVC optimization flags
if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /GL /DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG")  # link-time code generation
endif()