cmake_minimum_required(VERSION 3.22) # minimum CMake version
project(cpet_140_final_project) # project name
set(CMAKE_CXX_STANDARD 20) # set C++ standard to C++20 (AppleClang doesn't support C++26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch Hello ImGui (automatic download)
include(FetchContent)

# Download and build FreeType automatically to avoid version conflicts
# This ensures HelloImGui has a compatible FreeType version regardless of system version
set(HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED ON CACHE BOOL "Download FreeType if needed" FORCE)

# Download and build GLFW automatically if not found (especially for SUSE)
# This ensures HelloImGui can build even when system GLFW packages are unavailable
set(HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED ON CACHE BOOL "Download GLFW if needed" FORCE)

FetchContent_Declare(
        hello_imgui
        GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
)
FetchContent_MakeAvailable(hello_imgui)

# Add your app
hello_imgui_add_app(
        structuracost

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
target_include_directories(structuracost PRIVATE
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

# -------------------------------
# Install rules for packaging
# -------------------------------
# Default install prefix when creating installers (can be overridden)
# Note: Only set for Linux; macOS and Windows installers handle their own paths
if(UNIX AND NOT APPLE)
    set(APP_INSTALL_PREFIX "/opt/structuracost" CACHE PATH "Install prefix used inside installers")
    set(CPACK_PACKAGING_INSTALL_PREFIX "${APP_INSTALL_PREFIX}")
elseif(APPLE)
    # For macOS, use a relative prefix to avoid permission issues during packaging
    set(CPACK_PACKAGING_INSTALL_PREFIX "/")
endif()

if(APPLE)
    # Install the .app bundle at the root of the package (DMG)
    install(TARGETS structuracost BUNDLE DESTINATION .)
    # Ensure assets are present inside the bundle Resources
    install(DIRECTORY assets/ DESTINATION "structuracost.app/Contents/Resources/assets")
elseif(WIN32)
    # Install exe at the root folder, with assets alongside
    install(TARGETS structuracost RUNTIME DESTINATION .)
    install(DIRECTORY assets/ DESTINATION assets)
else()
    # Linux: install under /opt/structuracost with bin and assets
    install(TARGETS structuracost RUNTIME DESTINATION bin)
    install(DIRECTORY assets/ DESTINATION assets)
    # Provide a launcher in /usr/bin that cds into install prefix then runs the app
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/structuracost.sh "#!/bin/sh\ncd \"${APP_INSTALL_PREFIX}\" || exit 1\nexec ./bin/structuracost \"$@\"\n")
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/structuracost.sh DESTINATION /usr/bin RENAME structuracost)

    # Install .desktop and icon
    install(FILES ${CMAKE_SOURCE_DIR}/packaging/linux/structuracost.desktop DESTINATION share/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/assets/icons/business_logo.png DESTINATION share/icons/hicolor/256x256/apps RENAME structuracost.png)
    # Hook post-install: DEB and RPM specific, plus generic fallback
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/linux/postinst.sh;")
    set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/packaging/linux/postinst.sh")
    set(CPACK_POST_INSTALL_SCRIPT "${CMAKE_SOURCE_DIR}/packaging/linux/postinst.sh")
endif()

# -------------------------------
# CPack configuration (installers)
# -------------------------------
set(CPACK_PACKAGE_NAME "structuracost")
set(CPACK_PACKAGE_VENDOR "structuracost")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "StructuraCost - payroll and monitoring system")
# Version can be overridden at configure time: -DCPACK_PACKAGE_VERSION=v1.2.3
if(NOT CPACK_PACKAGE_VERSION)
    set(CPACK_PACKAGE_VERSION "v0.0.0")
endif()

# Common metadata (omit license for now as requested)
# set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE" CACHE FILEPATH "")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md" CACHE FILEPATH "")

# Per-platform DESTDIR handling
# Windows and macOS need CPACK_SET_DESTDIR=OFF to use relative install paths
# Linux needs it ON to respect CPACK_PACKAGING_INSTALL_PREFIX
if(WIN32 OR APPLE)
    set(CPACK_SET_DESTDIR OFF)
else()
    set(CPACK_SET_DESTDIR ON)
endif()

# Per-platform default generators (can be overridden with -DCPACK_GENERATOR=...)
if(APPLE)
    # Provide both .dmg (DragNDrop) and .pkg (productbuild)
    set(CPACK_GENERATOR "DragNDrop;productbuild")
elseif(WIN32)
    # Provide both NSIS (.exe) and WiX (.msi)
    set(CPACK_GENERATOR "NSIS;WIX")
else()
    # On Linux, generator will be set from CI per family (DEB or RPM); fallback to TGZ
    set(CPACK_GENERATOR "TGZ")
endif()

# NSIS tweaks (Windows)
set(CPACK_NSIS_DISPLAY_NAME "StructuraCost")
set(CPACK_NSIS_PACKAGE_NAME "StructuraCost")
set(CPACK_NSIS_CONTACT "")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

# WiX (Windows .msi) - set a stable upgrade GUID
# Generate your own GUID for production; this constant ensures upgrades work
set(CPACK_WIX_UPGRADE_GUID "8D2B4E2A-9E4C-4F08-9F6F-6C9A8F5C2F11")

# DEB/RPM settings (Linux)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Utilities")

# Windows product metadata (for installers)
if(WIN32)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "StructuraCost")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/assets/icons/business_logo.png")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "structuracost.exe")
    # Start Menu and Desktop shortcuts
    set(CPACK_PACKAGE_EXECUTABLES "structuracost;StructuraCost")
    set(CPACK_CREATE_DESKTOP_LINKS "structuracost")
    set(CPACK_NSIS_MENU_NAME "StructuraCost")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "StructuraCost")
endif()

include(CPack)
