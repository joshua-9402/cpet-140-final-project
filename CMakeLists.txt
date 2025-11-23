cmake_minimum_required(VERSION 3.22) # minimum CMake version
project(cpet_140_final_project) # project name
set(CMAKE_CXX_STANDARD 20) # set C++ standard to C++20 (AppleClang doesn't support C++26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent) # Fetch Hello ImGui (automatic download)

FetchContent_Declare(
        hello_imgui
        GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
)
set(HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED ON CACHE BOOL "Download FreeType if needed" FORCE)
set(HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED ON CACHE BOOL "Download GLFW if needed" FORCE)
set(HELLOIMGUI_ADD_APP_WITH_INSTALL OFF CACHE BOOL "Disable automatic asset install" FORCE)

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

########################################################################################################################
#                                                                                                                      #
#                                                THIS AREA IS OFF LIMITS                                               #
#                                                                                                                      #
########################################################################################################################

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
if(NOT PKG_CONFIG_FOUND)
    message(STATUS "pkg-config not found; skipping pkg-config detection for libsodium")
else()
    pkg_check_modules(LIBSODIUM QUIET libsodium)
endif()

# Minimal macOS Homebrew link dir fallback: ensure linker searches Homebrew lib path
if(APPLE AND (EXISTS "/opt/homebrew/lib/libsodium.dylib" OR EXISTS "/opt/homebrew/lib/libsodium.a"))
        target_link_directories(structuracost PRIVATE "/opt/homebrew/lib")
endif()

if(APPLE AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.13" AND (EXISTS "/opt/homebrew/lib/libsodium.dylib" OR EXISTS "/opt/homebrew/lib/libsodium.a"))
    target_link_options(structuracost PRIVATE "-L/opt/homebrew/lib")
endif()

# Check for libsodium in /usr/local/lib without nesting
if(EXISTS "/usr/local/lib/libsodium.dylib" OR EXISTS "/usr/local/lib/libsodium.a")
    set(_SODIUM_USRLIB_EXISTS TRUE)
else()
    set(_SODIUM_USRLIB_EXISTS FALSE)
endif()

if(APPLE AND _SODIUM_USRLIB_EXISTS)
        target_link_directories(structuracost PRIVATE "/usr/local/lib")
endif()

if(APPLE AND _SODIUM_USRLIB_EXISTS)
            target_link_options(structuracost PRIVATE "-L/usr/local/lib")
endif()

if(LIBSODIUM_FOUND)
    target_include_directories(structuracost PRIVATE ${LIBSODIUM_INCLUDE_DIRS})

    find_library(SODIUM_LIB_FILE NAMES sodium libsodium
        PATHS /opt/homebrew/lib /usr/local/lib /usr/lib
        NO_DEFAULT_PATH)

    if(SODIUM_LIB_FILE)
        set(_SODIUM_LINK "${SODIUM_LIB_FILE}")
    else()
        set(_SODIUM_LINK ${LIBSODIUM_LIBRARIES})
    endif()

    target_link_libraries(structuracost PRIVATE ${_SODIUM_LINK})
endif()

# Try Homebrew paths (macOS)
if(NOT LIBSODIUM_FOUND AND APPLE)
    foreach(HOMEBREW_PATH /opt/homebrew /usr/local)
        if(NOT EXISTS "${HOMEBREW_PATH}/include/sodium.h")
            continue()
        endif()

            set(_LIB_A "${HOMEBREW_PATH}/lib/libsodium.a")
            set(_LIB_DY "${HOMEBREW_PATH}/lib/libsodium.dylib")
        set(_SELECTED_SODIUM "")

            if(EXISTS "${_LIB_A}")
            set(_SELECTED_SODIUM "${_LIB_A}")
            elseif(EXISTS "${_LIB_DY}")
            set(_SELECTED_SODIUM "${_LIB_DY}")
        endif()

        if(NOT "${_SELECTED_SODIUM}" STREQUAL "")
                target_include_directories(structuracost PRIVATE "${HOMEBREW_PATH}/include")
            target_link_libraries(structuracost PRIVATE "${_SELECTED_SODIUM}")
                set(LIBSODIUM_FOUND TRUE)
                break()
        endif()
    endforeach()
endif()

# Try local dependencies folder (Windows)
if(NOT LIBSODIUM_FOUND AND WIN32)
    set(LOCAL_LIBSODIUM_ROOT "${CMAKE_SOURCE_DIR}/dependencies/libsodium")
    if(NOT EXISTS "${LOCAL_LIBSODIUM_ROOT}/include/sodium.h")
        return()
    endif()

        target_include_directories(structuracost PRIVATE "${LOCAL_LIBSODIUM_ROOT}/include")
        file(GLOB_RECURSE _TMP_LIBS "${LOCAL_LIBSODIUM_ROOT}/*libsodium.lib")

    # Determine target architecture (ARM64 vs X64)
    set(_TARGET_ARCH "X64")
        if(DEFINED CMAKE_SYSTEM_PROCESSOR)
            string(TOUPPER "${CMAKE_SYSTEM_PROCESSOR}" _SYS_CPU)
            if(_SYS_CPU MATCHES "ARM64")
                set(_TARGET_ARCH "ARM64")
            endif()
    elseif(DEFINED CMAKE_GENERATOR_PLATFORM)
            string(TOUPPER "${CMAKE_GENERATOR_PLATFORM}" _GEN_PLAT)
            if(_GEN_PLAT MATCHES "ARM64")
                set(_TARGET_ARCH "ARM64")
        endif()
        endif()

    # Filter libraries by architecture
    set(_WIN_LIB_CANDIDATES "")
        foreach(_L ${_TMP_LIBS})
            string(TOUPPER "${_L}" _LU)
            if(_TARGET_ARCH STREQUAL "ARM64")
            if(_LU MATCHES "ARM64" AND NOT _LU MATCHES "X64|AMD64")
                    list(APPEND _WIN_LIB_CANDIDATES "${_L}")
                endif()
        else()
                    if(NOT _LU MATCHES "ARM64")
                        list(APPEND _WIN_LIB_CANDIDATES "${_L}")
                    endif()
                endif()
            endforeach()

    if(_WIN_LIB_CANDIDATES)
            list(GET _WIN_LIB_CANDIDATES 0 _BEST_LIB)
            target_link_libraries(structuracost PRIVATE "${_BEST_LIB}")
            set(LIBSODIUM_FOUND TRUE)
endif()
endif()

# Try system paths (Linux)
if(NOT LIBSODIUM_FOUND AND UNIX AND NOT APPLE AND (EXISTS "/usr/include/sodium.h" OR EXISTS "/usr/local/include/sodium.h"))
        foreach(LIB_PATH
            "/usr/lib/libsodium.so"
            "/usr/lib/x86_64-linux-gnu/libsodium.so"
            "/usr/lib/aarch64-linux-gnu/libsodium.so")
            if(EXISTS "${LIB_PATH}")
                target_link_libraries(structuracost PRIVATE "${LIB_PATH}")
                set(LIBSODIUM_FOUND TRUE)
                break()
            endif()
        endforeach()
    endif()

# Error if not found
if(NOT LIBSODIUM_FOUND)
    return()
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