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
# libsodium configuration
# -------------------------------
set(LIBSODIUM_FOUND FALSE)

# Try pkg-config first (Linux/macOS with package manager)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBSODIUM QUIET libsodium)
    if(LIBSODIUM_FOUND)
        target_include_directories(structuracost PRIVATE ${LIBSODIUM_INCLUDE_DIRS})
        target_link_libraries(structuracost PRIVATE ${LIBSODIUM_LIBRARIES})
        message(STATUS "✓ Found libsodium via pkg-config")

        # Minimal fallback: if the linker cannot resolve -lsodium, prefer linking a concrete
        # libsodium library from common Homebrew/system locations so the macOS linker finds it.
        find_library(SODIUM_LIB_FILE NAMES sodium libsodium
            PATHS /opt/homebrew/lib /usr/local/lib /usr/lib
            NO_DEFAULT_PATH)
        if(SODIUM_LIB_FILE)
            target_link_libraries(structuracost PRIVATE "${SODIUM_LIB_FILE}")
            message(STATUS "✓ Also linking concrete libsodium library: ${SODIUM_LIB_FILE}")
        endif()
    endif()
endif()

# Try Homebrew paths (macOS)
if(NOT LIBSODIUM_FOUND AND APPLE)
    foreach(HOMEBREW_PATH /opt/homebrew /usr/local)
        if(EXISTS "${HOMEBREW_PATH}/include/sodium.h")
            foreach(LIB_PATH "${HOMEBREW_PATH}/lib/libsodium.a" "${HOMEBREW_PATH}/lib/libsodium.dylib")
                if(EXISTS "${LIB_PATH}")
                    target_include_directories(structuracost PRIVATE "${HOMEBREW_PATH}/include")
                    target_link_libraries(structuracost PRIVATE "${LIB_PATH}")
                    set(LIBSODIUM_FOUND TRUE)
                    message(STATUS "✓ Found libsodium: ${LIB_PATH}")
                    break()
                endif()
            endforeach()
            if(LIBSODIUM_FOUND)
                break()
            endif()
        endif()
    endforeach()
endif()

# Try local dependencies folder (Windows)
if(NOT LIBSODIUM_FOUND AND WIN32)
    set(LOCAL_LIBSODIUM_ROOT "${CMAKE_SOURCE_DIR}/dependencies/libsodium")
    if(EXISTS "${LOCAL_LIBSODIUM_ROOT}/include/sodium.h")
        target_include_directories(structuracost PRIVATE "${LOCAL_LIBSODIUM_ROOT}/include")
        # Collect candidate libs
        file(GLOB_RECURSE _TMP_LIBS "${LOCAL_LIBSODIUM_ROOT}/*libsodium.lib")
        set(_WIN_LIB_CANDIDATES "")

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            # 64-bit build - prefer libs that mention x64/x86_64/Win64 in the path/name
            foreach(_L ${_TMP_LIBS})
                string(TOUPPER "${_L}" _LU)
                if(_LU MATCHES "X64" OR _LU MATCHES "X86_64" OR _LU MATCHES "WIN64")
                    list(APPEND _WIN_LIB_CANDIDATES "${_L}")
                endif()
            endforeach()
        else()
            # Non-64-bit builds - prefer ARM64 labeled libs if present
            foreach(_L ${_TMP_LIBS})
                string(TOUPPER "${_L}" _LU)
                if(_LU MATCHES "ARM64")
                    list(APPEND _WIN_LIB_CANDIDATES "${_L}")
                endif()
            endforeach()
        endif()

        # If we didn't find an architecture-specific candidate, try a small local find as fallback
        if(_WIN_LIB_CANDIDATES STREQUAL "")
            find_library(SODIUM_LIB_FILE NAMES libsodium sodium
                PATHS "${LOCAL_LIBSODIUM_ROOT}" "${LOCAL_LIBSODIUM_ROOT}/x64" "${LOCAL_LIBSODIUM_ROOT}/X64" "${LOCAL_LIBSODIUM_ROOT}/Win64"
                NO_DEFAULT_PATH)
            if(SODIUM_LIB_FILE)
                list(APPEND _WIN_LIB_CANDIDATES "${SODIUM_LIB_FILE}")
            endif()
        endif()

        # Fallback: take the first discovered library (if any)
        if(_WIN_LIB_CANDIDATES STREQUAL "")
            foreach(_L ${_TMP_LIBS})
                # avoid obvious ARM64 when building x64
                string(TOUPPER "${_L}" _LU)
                if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND _LU MATCHES "ARM64")
                    continue()
                endif()
                list(APPEND _WIN_LIB_CANDIDATES "${_L}")
                break()
            endforeach()
        endif()

        if(NOT _WIN_LIB_CANDIDATES STREQUAL "")
            list(GET _WIN_LIB_CANDIDATES 0 _BEST_LIB)

            # If building 64-bit but the chosen library path contains ARM64, try a quick x64 fallback
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                string(TOUPPER "${_BEST_LIB}" _BEST_UPPER)
                if(_BEST_UPPER MATCHES "ARM64")
                    # Try common x64 subfolders under the local libsodium root
                    find_library(_SODIUM_X64 NAMES libsodium sodium
                        PATHS "${LOCAL_LIBSODIUM_ROOT}/x64" "${LOCAL_LIBSODIUM_ROOT}/X64" "${LOCAL_LIBSODIUM_ROOT}/Win64" "${LOCAL_LIBSODIUM_ROOT}/x86_64"
                        NO_DEFAULT_PATH)
                    if(_SODIUM_X64)
                        set(_BEST_LIB "${_SODIUM_X64}")
                        message(STATUS "✓ Found x64 libsodium fallback: ${_BEST_LIB}")
                    else()
                        message(FATAL_ERROR "libsodium: selected library is ARM64 but target is x64. Please provide an x64 libsodium library under ${LOCAL_LIBSODIUM_ROOT} (e.g., x64/ or Win64/).")
                    endif()
                endif()
            endif()

            target_link_libraries(structuracost PRIVATE "${_BEST_LIB}")
            set(LIBSODIUM_FOUND TRUE)
            message(STATUS "✓ Selected libsodium (Windows, arch filtered): ${_BEST_LIB}")
        else()
            message(WARNING "No suitable Windows libsodium found in ${LOCAL_LIBSODIUM_ROOT}; build may fail or you may need to provide an x64 libsodium for x64 builds")
        endif()
    endif()
endif()

# Try system paths (Linux)
if(NOT LIBSODIUM_FOUND AND UNIX AND NOT APPLE)
    if(EXISTS "/usr/include/sodium.h" OR EXISTS "/usr/local/include/sodium.h")
        foreach(LIB_PATH
            "/usr/lib/libsodium.so"
            "/usr/lib/x86_64-linux-gnu/libsodium.so"
            "/usr/lib/aarch64-linux-gnu/libsodium.so")
            if(EXISTS "${LIB_PATH}")
                target_link_libraries(structuracost PRIVATE "${LIB_PATH}")
                set(LIBSODIUM_FOUND TRUE)
                message(STATUS "✓ Found libsodium: ${LIB_PATH}")
                break()
            endif()
        endforeach()
    endif()
endif()

# Error if not found
if(NOT LIBSODIUM_FOUND)
    message(FATAL_ERROR
        "\nlibsodium not found!\n\n"
        "Install libsodium:\n"
        "  macOS:   brew install libsodium\n"
        "  Ubuntu:  sudo apt-get install libsodium-dev\n"
        "  Fedora:  sudo dnf install libsodium-devel\n"
        "  Windows: Download from https://github.com/jedisct1/libsodium/releases\n"
        "           and place in dependencies/libsodium/\n")
endif()


# -------------------------------
# Install rules for packaging
# -------------------------------
if(UNIX AND NOT APPLE)
    set(APP_INSTALL_PREFIX "/opt/structuracost" CACHE PATH "Install prefix")
    set(CPACK_PACKAGING_INSTALL_PREFIX "${APP_INSTALL_PREFIX}")
elseif(APPLE)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/tmp/_cpack_staging")
endif()

if(APPLE)
    install(TARGETS structuracost BUNDLE DESTINATION .)
    install(DIRECTORY assets/ DESTINATION "structuracost.app/Contents/Resources/assets")
elseif(WIN32)
    install(TARGETS structuracost RUNTIME DESTINATION ".")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/" DESTINATION "assets" PATTERN "*")
else()
    install(TARGETS structuracost RUNTIME DESTINATION bin)
    install(DIRECTORY assets/ DESTINATION assets)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/structuracost.sh "#!/bin/sh\ncd \"${APP_INSTALL_PREFIX}\" || exit 1\nexec ./bin/structuracost \"$@\"\n")
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/structuracost.sh DESTINATION /usr/bin RENAME structuracost)
    install(FILES ${CMAKE_SOURCE_DIR}/packaging/linux/structuracost.desktop DESTINATION share/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/assets/icons/business_logo.png DESTINATION share/icons/hicolor/256x256/apps RENAME structuracost.png)
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/linux/postinst.sh;")
    set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/packaging/linux/postinst.sh")
endif()

# -------------------------------
# CPack configuration
# -------------------------------
set(CPACK_PACKAGE_NAME "structuracost")
set(CPACK_PACKAGE_VENDOR "structuracost")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "StructuraCost - payroll and monitoring system")
if(NOT CPACK_PACKAGE_VERSION)
    set(CPACK_PACKAGE_VERSION "v0.0.0")
endif()
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt" CACHE FILEPATH "")

if(WIN32)
    set(CPACK_SET_DESTDIR OFF)
else()
    set(CPACK_SET_DESTDIR ON)
endif()

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop;productbuild")
    find_library(SECURITY_FRAMEWORK Security)
    if(SECURITY_FRAMEWORK)
        target_link_libraries(structuracost PRIVATE ${SECURITY_FRAMEWORK})
    endif()
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")
    target_link_libraries(structuracost PRIVATE Advapi32)
    set(CPACK_NSIS_DISPLAY_NAME "StructuraCost")
    set(CPACK_NSIS_PACKAGE_NAME "StructuraCost")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "StructuraCost")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "structuracost.exe")
    set(CPACK_PACKAGE_EXECUTABLES "structuracost;StructuraCost")
    set(CPACK_CREATE_DESKTOP_LINKS "structuracost")
else()
    set(CPACK_GENERATOR "TGZ")
    find_library(LIBSECRET_LIBRARY NAMES secret-1 libsecret-1)
    if(LIBSECRET_LIBRARY)
        target_link_libraries(structuracost PRIVATE ${LIBSECRET_LIBRARY})
    endif()
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Utilities")
endif()

include(CPack)