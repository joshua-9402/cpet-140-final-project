cmake_minimum_required(VERSION 3.22) # minimum CMake version
project(cpet_140_final_project) # project name
set(CMAKE_CXX_STANDARD 20) # set C++ standard to C++20 (AppleClang doesn't support C++26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent) # Fetch Hello ImGui (automatic download)

# Download and build FreeType automatically to avoid version conflicts
# This ensures HelloImGui has a compatible FreeType version regardless of system version
set(HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED ON CACHE BOOL "Download FreeType if needed" FORCE)

# Download and build GLFW automatically if not found (especially for SUSE)
# This ensures HelloImGui can build even when system GLFW packages are unavailable
set(HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED ON CACHE BOOL "Download GLFW if needed" FORCE)

# Disable Hello ImGui's automatic asset installation - we'll handle it manually
# This prevents absolute path issues on Windows with CPack
set(HELLOIMGUI_ADD_APP_WITH_INSTALL OFF CACHE BOOL "Disable automatic asset install" FORCE)

FetchContent_Declare(
        hello_imgui
        GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
)

FetchContent_MakeAvailable(hello_imgui)

# Add your app
hello_imgui_add_app(
        # Target name
        structuracost

        # dependency
        dependencies/sqlite/sqlite3.c
        dependencies/sqlite/sqlite3.h

        # The main application file
        src/main.cpp


        # App config
        src/config/config.cpp
        src/config/config.h


        # Handlers
        src/handler/db.cpp
        src/handler/db.h

        src/handler/system.cpp
        src/handler/system.h


        # Security
        src/security/auth.cpp
        src/security/auth.h

        src/security/cryptography.cpp
        src/security/cryptography.h


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

# -------------------------------
# libsodium configuration - GitHub Actions compatible
# -------------------------------
set(LIBSODIUM_FOUND FALSE)

# Detect if running in CI environment (GitHub Actions, GitLab CI, Travis, etc.)
set(IS_CI FALSE)
if(DEFINED ENV{CI} OR DEFINED ENV{GITHUB_ACTIONS} OR DEFINED ENV{GITLAB_CI} OR DEFINED ENV{TRAVIS})
    set(IS_CI TRUE)
    message(STATUS "CI environment detected")
endif()

# Strategy 1: Try to find via pkg-config (Linux/Unix/CI)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBSODIUM QUIET libsodium)
    if(LIBSODIUM_FOUND)
        target_include_directories(structuracost PRIVATE ${LIBSODIUM_INCLUDE_DIRS})
        target_link_libraries(structuracost PRIVATE ${LIBSODIUM_LIBRARIES})
        target_link_directories(structuracost PRIVATE ${LIBSODIUM_LIBRARY_DIRS})
        message(STATUS "✓ Found libsodium via pkg-config: ${LIBSODIUM_LIBRARIES}")
    endif()
endif()

# Strategy 2: Try Homebrew paths (macOS - both Apple Silicon and Intel)
if(NOT LIBSODIUM_FOUND AND APPLE)
    set(HOMEBREW_PATHS
        /opt/homebrew             # Apple Silicon (arm64) - GitHub Actions arm64 runners
        /usr/local                # Intel Mac (x86_64) - GitHub Actions x64 runners
    )

    foreach(HOMEBREW_PATH ${HOMEBREW_PATHS})
        if(EXISTS "${HOMEBREW_PATH}/include/sodium.h")
            # Found headers, now look for library
            set(LIB_CANDIDATES
                "${HOMEBREW_PATH}/lib/libsodium.a"
                "${HOMEBREW_PATH}/lib/libsodium.dylib"
            )

            foreach(LIB_PATH ${LIB_CANDIDATES})
                if(EXISTS "${LIB_PATH}")
                    target_include_directories(structuracost PRIVATE "${HOMEBREW_PATH}/include")
                    target_link_libraries(structuracost PRIVATE "${LIB_PATH}")
                    set(LIBSODIUM_FOUND TRUE)
                    set(LIBSODIUM_INCLUDE_DIRS "${HOMEBREW_PATH}/include")
                    set(LIBSODIUM_LIBRARIES "${LIB_PATH}")
                    message(STATUS "✓ Found Homebrew libsodium: ${LIB_PATH}")
                    break()
                endif()
            endforeach()

            if(LIBSODIUM_FOUND)
                break()
            endif()
        endif()
    endforeach()
endif()

# Strategy 3: Try local dependencies folder (all platforms, useful for Windows CI)
if(NOT LIBSODIUM_FOUND)
    set(LOCAL_LIBSODIUM_ROOT "${CMAKE_SOURCE_DIR}/dependencies/libsodium")

    if(EXISTS "${LOCAL_LIBSODIUM_ROOT}/include/sodium.h")
        target_include_directories(structuracost PRIVATE "${LOCAL_LIBSODIUM_ROOT}/include")

        # Platform-specific library paths and names
        if(WIN32)
            # Windows: try both .lib (static/import) and .dll locations
            set(WIN_LIB_PATHS
                "${LOCAL_LIBSODIUM_ROOT}/lib/libsodium.lib"
                "${LOCAL_LIBSODIUM_ROOT}/lib/libsodium.a"
                "${LOCAL_LIBSODIUM_ROOT}/windows/libsodium.lib"
                "${LOCAL_LIBSODIUM_ROOT}/windows/libsodium.a"
                "${LOCAL_LIBSODIUM_ROOT}/Win32/Release/v142/static/libsodium.lib"
                "${LOCAL_LIBSODIUM_ROOT}/x64/Release/v142/static/libsodium.lib"
            )

            foreach(LIB_PATH ${WIN_LIB_PATHS})
                if(EXISTS "${LIB_PATH}")
                    target_link_libraries(structuracost PRIVATE "${LIB_PATH}")
                    set(LIBSODIUM_FOUND TRUE)
                    message(STATUS "✓ Found local libsodium (Windows): ${LIB_PATH}")
                    break()
                endif()
            endforeach()

        elseif(APPLE)
            # macOS: try .dylib (dynamic) and .a (static)
            set(MAC_LIB_PATHS
                "${LOCAL_LIBSODIUM_ROOT}/lib/libsodium.dylib"
                "${LOCAL_LIBSODIUM_ROOT}/lib/libsodium.a"
                "${LOCAL_LIBSODIUM_ROOT}/unix/libsodium.dylib"
                "${LOCAL_LIBSODIUM_ROOT}/unix/libsodium.a"
            )

            foreach(LIB_PATH ${MAC_LIB_PATHS})
                if(EXISTS "${LIB_PATH}")
                    target_link_libraries(structuracost PRIVATE "${LIB_PATH}")
                    set(LIBSODIUM_FOUND TRUE)
                    message(STATUS "✓ Found local libsodium (macOS): ${LIB_PATH}")
                    break()
                endif()
            endforeach()

        elseif(UNIX)
            # Linux: try .so (dynamic) and .a (static)
            set(LINUX_LIB_PATHS
                "${LOCAL_LIBSODIUM_ROOT}/lib/libsodium.so"
                "${LOCAL_LIBSODIUM_ROOT}/lib/libsodium.a"
                "${LOCAL_LIBSODIUM_ROOT}/unix/libsodium.so"
                "${LOCAL_LIBSODIUM_ROOT}/unix/libsodium.a"
            )

            foreach(LIB_PATH ${LINUX_LIB_PATHS})
                if(EXISTS "${LIB_PATH}")
                    target_link_libraries(structuracost PRIVATE "${LIB_PATH}")
                    set(LIBSODIUM_FOUND TRUE)
                    message(STATUS "✓ Found local libsodium (Linux): ${LIB_PATH}")
                    break()
                endif()
            endforeach()
        endif()
    endif()
endif()

# Strategy 4: Try standard system paths (Linux/GitHub Actions Ubuntu runners)
if(NOT LIBSODIUM_FOUND AND UNIX AND NOT APPLE)
    # Common paths where libsodium gets installed via apt/dnf/zypper
    set(SYSTEM_INCLUDE_PATHS
        "/usr/include/sodium.h"
        "/usr/local/include/sodium.h"
    )

    set(FOUND_INCLUDE FALSE)
    foreach(INC_PATH ${SYSTEM_INCLUDE_PATHS})
        if(EXISTS "${INC_PATH}")
            set(FOUND_INCLUDE TRUE)
            break()
        endif()
    endforeach()

    if(FOUND_INCLUDE)
        set(SYSTEM_LIB_PATHS
            "/usr/lib/libsodium.so"
            "/usr/lib/libsodium.a"
            "/usr/local/lib/libsodium.so"
            "/usr/local/lib/libsodium.a"
            "/usr/lib/x86_64-linux-gnu/libsodium.so"
            "/usr/lib/x86_64-linux-gnu/libsodium.a"
            "/usr/lib/aarch64-linux-gnu/libsodium.so"
            "/usr/lib/aarch64-linux-gnu/libsodium.a"
            "/lib/x86_64-linux-gnu/libsodium.so.26"
            "/lib/aarch64-linux-gnu/libsodium.so.26"
        )

        foreach(LIB_PATH ${SYSTEM_LIB_PATHS})
            if(EXISTS "${LIB_PATH}")
                target_link_libraries(structuracost PRIVATE "${LIB_PATH}")
                set(LIBSODIUM_FOUND TRUE)
                message(STATUS "✓ Found system libsodium (Linux): ${LIB_PATH}")
                break()
            endif()
        endforeach()
    endif()
endif()

# Final check: Error if libsodium not found
if(NOT LIBSODIUM_FOUND)
    if(IS_CI)
        message(FATAL_ERROR
            "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
            "  libsodium not found in CI environment!\n"
            "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
            "\n"
            "For GitHub Actions, add this step BEFORE cmake configure:\n"
            "\n"
            "  - name: Install libsodium\n"
            "    run: |\n"
            "      if [[ \"\${{ runner.os }}\" == \"Linux\" ]]; then\n"
            "        sudo apt-get update\n"
            "        sudo apt-get install -y libsodium-dev\n"
            "      elif [[ \"\${{ runner.os }}\" == \"macOS\" ]]; then\n"
            "        brew install libsodium\n"
            "      elif [[ \"\${{ runner.os }}\" == \"Windows\" ]]; then\n"
            "        # Download and place in dependencies/libsodium\n"
            "        curl -L -o libsodium.zip https://download.libsodium.org/libsodium/releases/libsodium-1.0.20-stable-msvc.zip\n"
            "        7z x libsodium.zip -o\${{ github.workspace }}/dependencies/libsodium\n"
            "      fi\n"
            "\n"
            "See doc/LIBSODIUM_SETUP.md for detailed instructions.\n"
        )
    else()
        message(FATAL_ERROR
            "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
            "  libsodium not found!\n"
            "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
            "\n"
            "Install libsodium:\n"
            "  macOS:   brew install libsodium\n"
            "  Ubuntu:  sudo apt-get install libsodium-dev\n"
            "  Fedora:  sudo dnf install libsodium-devel\n"
            "  Arch:    sudo pacman -S libsodium\n"
            "  Windows: Download from https://github.com/jedisct1/libsodium/releases\n"
            "\n"
            "Or place manually in: dependencies/libsodium/\n"
            "  Structure:\n"
            "    dependencies/libsodium/include/sodium.h\n"
            "    dependencies/libsodium/lib/libsodium.[a|so|dylib|lib]\n"
            "\n"
            "See doc/LIBSODIUM_SETUP.md for detailed instructions.\n"
        )
    endif()
endif()


# -------------------------------
# Install rules for packaging
# -------------------------------
# Default install prefix when creating installers (can be overridden)
if(UNIX AND NOT APPLE)
    # Linux: Use /opt/structuracost as install prefix
    set(APP_INSTALL_PREFIX "/opt/structuracost" CACHE PATH "Install prefix used inside installers")
    set(CPACK_PACKAGING_INSTALL_PREFIX "${APP_INSTALL_PREFIX}")
elseif(APPLE)
    # macOS: Use a temporary staging directory to avoid permission issues
    set(CPACK_PACKAGING_INSTALL_PREFIX "/tmp/_cpack_staging")
endif()
# Note: Windows doesn't set CMAKE_INSTALL_PREFIX or CPACK_PACKAGING_INSTALL_PREFIX
# NSIS installer handles its own default paths (e.g., C:\Program Files\StructuraCost)

if(APPLE)
    # Install the .app bundle directly
    install(TARGETS structuracost BUNDLE DESTINATION .)
    # Ensure assets are present inside the bundle Resources
    install(DIRECTORY assets/ DESTINATION "structuracost.app/Contents/Resources/assets")
elseif(WIN32)
    # Install exe at the root folder
    install(TARGETS structuracost RUNTIME DESTINATION ".")
    # Manually install assets with relative path (Hello ImGui auto-install disabled to avoid absolute paths)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/" DESTINATION "assets" PATTERN "*")
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

# Common metadata
# Note: macOS DragNDrop/productbuild generators only accept .rtf, .rtfd, .html, .txt (not .md)
# Using LICENSE.txt for all platforms as it's in plain text format
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt" CACHE FILEPATH "")

# Per-platform DESTDIR handling
# macOS and Linux use CPACK_SET_DESTDIR=ON to install into staging directory (avoids permission issues)
# Windows uses OFF because NSIS expects relative paths and handles install location itself
if(WIN32)
    set(CPACK_SET_DESTDIR OFF)
else()
    set(CPACK_SET_DESTDIR ON)
endif()

# Per-platform default generators (can be overridden with -DCPACK_GENERATOR=...)
if(APPLE)
    # Provide both .dmg (DragNDrop) and .pkg (productbuild)
    set(CPACK_GENERATOR "DragNDrop;productbuild")
    # Link Security framework (Keychain APIs) for keyring support
    find_library(SECURITY_FRAMEWORK Security)
    if(SECURITY_FRAMEWORK)
        target_link_libraries(structuracost PRIVATE ${SECURITY_FRAMEWORK})
        message(STATUS "Linking Security framework for Keychain support: ${SECURITY_FRAMEWORK}")
    else()
        message(WARNING "Security framework not found; keychain support may fail to link")
    endif()
elseif(WIN32)
    # Use NSIS (.exe) only on Windows
    set(CPACK_GENERATOR "NSIS")
    # Link Windows Credential Manager (Advapi32 provides CredWrite/CredRead)
    target_link_libraries(structuracost PRIVATE Advapi32)
    message(STATUS "Linking Advapi32 for Windows Credential Manager support")
else()
    # On Linux, generator will be set from CI per family (DEB or RPM); fallback to TGZ
    set(CPACK_GENERATOR "TGZ")
    # Try to link libsecret (secret-1) if available for native keyring support; otherwise we fallback to secret-tool or file
    find_library(LIBSECRET_LIBRARY NAMES secret-1 libsecret-1)
    if(LIBSECRET_LIBRARY)
        target_link_libraries(structuracost PRIVATE ${LIBSECRET_LIBRARY})
        message(STATUS "Linking libsecret for Linux keyring support: ${LIBSECRET_LIBRARY}")
    else()
        message(STATUS "libsecret not found; using secret-tool/file fallback for key storage")
    endif()
endif()

# NSIS tweaks (Windows)
set(CPACK_NSIS_DISPLAY_NAME "StructuraCost")
set(CPACK_NSIS_PACKAGE_NAME "StructuraCost")
set(CPACK_NSIS_CONTACT "")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH OFF)  # Don't modify system PATH

# DEB/RPM settings (Linux)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Utilities")

# Windows product metadata (for installers)
if(WIN32)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "StructuraCost")
    # NSIS installed icon (executable) and installer shortcuts
    set(CPACK_NSIS_INSTALLED_ICON_NAME "structuracost.exe")
    set(CPACK_PACKAGE_EXECUTABLES "structuracost;StructuraCost")
    set(CPACK_CREATE_DESKTOP_LINKS "structuracost")
    set(CPACK_NSIS_MENU_NAME "StructuraCost")
endif()

include(CPack)