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

# Ensure the application icon (assets/icons/app_icon.png) is visible to the hello_imgui packaging
# helper. hello_imgui's macOS support will look for assets/app_settings/icon.png and convert it to
# icon.icns during bundle creation. We copy the project's icon into the binary tree so the helper
# can find it.
if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icons/app_icon.png")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/assets/app_settings")
    configure_file("${CMAKE_SOURCE_DIR}/assets/icons/app_icon.png"
                   "${CMAKE_BINARY_DIR}/assets/app_settings/icon.png"
                   COPYONLY)
endif()

# Platform-specific source files
set(PLATFORM_SOURCES "")
if(APPLE)
    list(APPEND PLATFORM_SOURCES src/ui/set_app_icon_macos.mm)
endif()

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

        src/handler/print.cpp
        src/handler/print.h

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

        ${PLATFORM_SOURCES}


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

    set(_WIN_LIB_CANDIDATES "")

    # Primary: prefer explicit arch markers in path (x64/Win64/X86_64/AMD64 for X64; ARM64 for ARM64)
    foreach(_L ${_TMP_LIBS})
        string(TOUPPER "${_L}" _LU)
        if(_TARGET_ARCH STREQUAL "X64")
            if(_LU MATCHES "X64" OR _LU MATCHES "X86_64" OR _LU MATCHES "AMD64" OR _LU MATCHES "WIN64")
                # exclude obvious Win32/x86 matches
                if(NOT (_LU MATCHES "\\bWIN32\\b" OR _LU MATCHES "\\bX86\\b"))
                    list(APPEND _WIN_LIB_CANDIDATES "${_L}")
                endif()
            endif()
        else()
            if(_LU MATCHES "ARM64")
                list(APPEND _WIN_LIB_CANDIDATES "${_L}")
            endif()
        endif()
    endforeach()

    # Secondary: look in common arch subdirs
    if(_WIN_LIB_CANDIDATES STREQUAL "")
        if(_TARGET_ARCH STREQUAL "X64")
            find_library(_SODIUM_X64 NAMES libsodium sodium
                PATHS "${LOCAL_LIBSODIUM_ROOT}/x64" "${LOCAL_LIBSODIUM_ROOT}/X64" "${LOCAL_LIBSODIUM_ROOT}/Win64" "${LOCAL_LIBSODIUM_ROOT}/x86_64"
                NO_DEFAULT_PATH)
            if(_SODIUM_X64)
                list(APPEND _WIN_LIB_CANDIDATES "${_SODIUM_X64}")
            endif()
        else()
            find_library(_SODIUM_ARM64 NAMES libsodium sodium
                PATHS "${LOCAL_LIBSODIUM_ROOT}/ARM64" "${LOCAL_LIBSODIUM_ROOT}/arm64"
                NO_DEFAULT_PATH)
            if(_SODIUM_ARM64)
                list(APPEND _WIN_LIB_CANDIDATES "${_SODIUM_ARM64}")
            endif()
        endif()
    endif()

    # Final fallback: pick first non-obvious mismatch
    if(_WIN_LIB_CANDIDATES STREQUAL "")
        foreach(_L ${_TMP_LIBS})
            string(TOUPPER "${_L}" _LU)
            if(_TARGET_ARCH STREQUAL "X64")
                if(NOT (_LU MATCHES "ARM64" OR _LU MATCHES "\\bWIN32\\b" OR _LU MATCHES "\\bX86\\b"))
                    list(APPEND _WIN_LIB_CANDIDATES "${_L}")
                    break()
                endif()
            else()
                if(_LU MATCHES "ARM64")
                    list(APPEND _WIN_LIB_CANDIDATES "${_L}")
                    break()
                endif()
            endif()
        endforeach()
    endif()

    if(_WIN_LIB_CANDIDATES STREQUAL "")
        message(FATAL_ERROR "No suitable libsodium library found for target architecture ${_TARGET_ARCH} under ${LOCAL_LIBSODIUM_ROOT}. Provide prebuilt libsodium for the target (e.g., x64/ or ARM64/).")
    endif()

    list(GET _WIN_LIB_CANDIDATES 0 _BEST_LIB)
    string(TOUPPER "${_BEST_LIB}" _BEST_UP)

    # Sanity: ensure selected library roughly matches requested arch
    if(_TARGET_ARCH STREQUAL "X64" AND _BEST_UP MATCHES "ARM64")
        message(FATAL_ERROR "Selected libsodium is ARM64 but target is X64: ${_BEST_LIB}")
    elseif(_TARGET_ARCH STREQUAL "ARM64" AND (_BEST_UP MATCHES "X64" OR _BEST_UP MATCHES "WIN32" OR _BEST_UP MATCHES "\\bX86\\b"))
        message(FATAL_ERROR "Selected libsodium is X64/Win32 but target is ARM64: ${_BEST_LIB}")
    endif()

    target_link_libraries(structuracost PRIVATE "${_BEST_LIB}")
    set(LIBSODIUM_FOUND TRUE)
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

    # Ensure libsodium runtime is bundled into the app (if we linked a dylib from dependencies)
    file(GLOB_RECURSE _FOUND_SODIUM "${CMAKE_SOURCE_DIR}/dependencies/libsodium/*libsodium*.dylib")
    foreach(_s ${_FOUND_SODIUM})
        if(EXISTS "${_s}")
            install(FILES "${_s}" DESTINATION "structuracost.app/Contents/Frameworks")
        endif()
    endforeach()

    # copy app icon into app resources (in addition to hello_imgui's conversion)
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icons/app_icon.png")
        install(FILES "${CMAKE_SOURCE_DIR}/assets/icons/app_icon.png" DESTINATION "structuracost.app/Contents/Resources/")
    endif()
elseif(WIN32)
    install(TARGETS structuracost RUNTIME DESTINATION ".")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/" DESTINATION "assets" PATTERN "*")

    # If there are prebuilt libsodium dlls in dependencies, include them in the installer root
    file(GLOB_RECURSE _WIN_SODIUM_DLLS "${CMAKE_SOURCE_DIR}/dependencies/libsodium/*.dll")
    foreach(_dll ${_WIN_SODIUM_DLLS})
        if(EXISTS "${_dll}")
            install(FILES "${_dll}" DESTINATION ".")
        endif()
    endforeach()

    # Add app icon to installer payload so installer can use it for shortcuts
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icons/app_icon.png")
        install(FILES "${CMAKE_SOURCE_DIR}/assets/icons/app_icon.png" DESTINATION ".")
    endif()
else()
    install(TARGETS structuracost RUNTIME DESTINATION bin)
    install(DIRECTORY assets/ DESTINATION assets)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/structuracost.sh "#!/bin/sh\ncd \"${APP_INSTALL_PREFIX}\" || exit 1\nexec ./bin/structuracost \"$@\"\n")
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/structuracost.sh DESTINATION /usr/bin RENAME structuracost)
    install(FILES ${CMAKE_SOURCE_DIR}/packaging/linux/structuracost.desktop DESTINATION share/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/assets/icons/business_logo.png DESTINATION share/icons/hicolor/256x256/apps RENAME structuracost.png)
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/packaging/linux/postinst.sh;")
    set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/packaging/linux/postinst.sh")

    # include any found libsodium shared objects in the package lib folder
    file(GLOB_RECURSE _LIN_SODIUM "${CMAKE_SOURCE_DIR}/dependencies/libsodium/*.so")
    foreach(_l ${_LIN_SODIUM})
        if(EXISTS "${_l}")
            install(FILES "${_l}" DESTINATION lib)
        endif()
    endforeach()
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
    # Add postinstall script for macOS packages
    set(CPACK_POSTFLIGHT_SCRIPT "${CMAKE_SOURCE_DIR}/packaging/macos/postinstall")
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
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\StructuraCost.lnk' '$INSTDIR\\\\structuracost.exe' '' '$INSTDIR\\\\structuracost.exe' 0")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\StructuraCost.lnk'")
    # Add PowerShell script to install directory for post-install libsodium download
    install(FILES "${CMAKE_SOURCE_DIR}/packaging/windows/install_libsodium.ps1" DESTINATION ".")
    # Add custom NSIS commands to run PowerShell script after installation
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        ExecWait 'powershell -ExecutionPolicy Bypass -File \\\"$INSTDIR\\\\install_libsodium.ps1\\\" -InstallDir \\\"$INSTDIR\\\"'
    ")
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
