cmake_minimum_required(VERSION 3.22) # minimum CMake version

# CRITICAL: Disable absolute path errors BEFORE project() for all platforms
# Use CACHE so the setting is visible to generated cmake_install.cmake used by CPack
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION OFF CACHE BOOL "Allow absolute install destinations during CPack staging" FORCE)
set(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION OFF CACHE BOOL "Suppress warnings for absolute install destinations during CPack staging" FORCE)

# Configure install prefix BEFORE project() to ensure early initialization
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    # Use relative path for CPack staging on all platforms
    # macOS: DMG will show /Applications as install target
    # Windows: NSIS will handle final install location
    set(CMAKE_INSTALL_PREFIX "." CACHE PATH "Install prefix for CPack staging" FORCE)
endif()

project(cpet_140_final_project LANGUAGES C CXX)

# Enforce Release build mode in CI environments
set(_IS_CI_BUILD OFF)
if(DEFINED ENV{GITHUB_ACTIONS} OR DEFINED ENV{CI} OR DEFINED ENV{FORCE_RELEASE_BUILD})
    set(_IS_CI_BUILD ON)
endif()

if(_IS_CI_BUILD)
    message(STATUS "CI detected: enforcing Release build configuration")

    # Force Release for both single-config and multi-config generators
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (forced by CI)" FORCE)
    if(CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "Configurations (restricted by CI)" FORCE)
    endif()

    # Strict validation
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        message(FATAL_ERROR "CI requires Release build; CMAKE_BUILD_TYPE is '${CMAKE_BUILD_TYPE}'")
    endif()
else()
    # Default to Release if not specified
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build type" FORCE)
    endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(FetchContent) # Fetch Hello ImGui (automatic download)
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

        # UI Modules
        src/ui/modules/loginUI.cpp
        src/ui/modules/loginUI.h

        src/ui/modules/accountUI.cpp
        src/ui/modules/accountUI.h

        src/ui/modules/summaryUI.cpp
        src/ui/modules/summaryUI.h

        src/ui/modules/payrollUI.cpp
        src/ui/modules/payrollUI.h

        src/ui/modules/monitorUI.cpp
        src/ui/modules/monitorUI.h

        src/ui/modules/testUI.cpp
        src/ui/modules/testUI.h


        # The Main Systems
        src/core/payroll.cpp
        src/core/payroll.h

        src/core/monitor.cpp
        src/core/monitor.h


        # Assets location
        ASSETS_LOCATION "assets/"
)

# Add include directory for SQLite3 headers
target_include_directories(structuracost PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/sqlite)

########################################################################################################################
#                                                                                                                      #
#                                                THIS AREA IS OFF LIMITS                                               #
#                                                                                                                      #
########################################################################################################################

# OVERVIEW: Logical sections for the remainder of this file
# - DEPENDENCIES: include dirs and small local dependency hints
# - CRYPTO: libsodium detection and linking
# - PACKAGING & INSTALL: platform-specific install rules (macOS / Windows / Linux)
# - CPACK CONFIGURATION: packaging/generator settings and metadata
#
# These headings are non-functional comments only; they help future maintainers
# quickly find the relevant blocks. The actual build/install behavior below
# remains unchanged.


# -------------------------------
# SECTION: CRYPTO (libsodium detection & linking)
# -------------------------------
# Use a small, cross-platform detection: prefer an installed libsodium (system or package manager)
# If found, add include dir and link the library. Otherwise emit a warning and continue.
# Support SODIUM_LIBRARY and SODIUM_INCLUDE_DIR hints from environment or CMake cache.
find_library(SODIUM_LIBRARY
    NAMES sodium libsodium
    HINTS ${SODIUM_LIBRARY} ENV SODIUM_LIBRARY_FILE
    PATHS
        ${CMAKE_SOURCE_DIR}/dependencies/libsodium
        ${CMAKE_SOURCE_DIR}/dependencies/libsodium/lib
        ${CMAKE_SOURCE_DIR}/dependencies/libsodium/x64/Release/v143/dynamic
        ${CMAKE_SOURCE_DIR}/dependencies/libsodium/Win32/Release/v143/dynamic
    PATH_SUFFIXES lib
)

find_path(SODIUM_INCLUDE_DIR
    NAMES sodium.h
    HINTS ${SODIUM_INCLUDE_DIR} ENV SODIUM_INCLUDE_DIR
    PATHS
        ${CMAKE_SOURCE_DIR}/dependencies/libsodium/include
    PATH_SUFFIXES include
)

if(SODIUM_LIBRARY AND SODIUM_INCLUDE_DIR)
    # libsodium found - configure targets
    target_include_directories(structuracost PRIVATE ${SODIUM_INCLUDE_DIR})
    target_link_libraries(structuracost PRIVATE ${SODIUM_LIBRARY})
else()
    message(FATAL_ERROR "libsodium not found - libsodium is required. Configure aborted.")
endif()


# -------------------------------
# SECTION: PACKAGING & INSTALL
# -------------------------------
# Install rules for packaging
# -------------------------------
if(APPLE)
    # macOS: Use relative install for CPack staging
    # CPack DragNDrop will create an Applications symlink in the DMG automatically
    set(CPACK_PACKAGING_INSTALL_PREFIX "/")
elseif(WIN32)
    # Windows: NSIS will handle final install location
    # Ensure CPack stages into a relative prefix to avoid absolute DESTINATION errors
    # (use a cache variable so CI can still override if desired)
    set(CPACK_PACKAGING_INSTALL_PREFIX "." CACHE PATH "Install prefix for CPack staging on Windows")
    # (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION is set unconditionally at top of file)
endif()


# -------------------------------
# ICONS: Application icons for different platforms
# -------------------------------
if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
    set(APP_ICON ${CMAKE_SOURCE_DIR}/assets/icons/AppIcon.icns)

    set_source_files_properties(
            ${APP_ICON}
            PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )

    target_sources(structuracost PRIVATE ${APP_ICON})

    # Ensure bundle id and version strings are available for Info.plist
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.structuracost.app" CACHE STRING "Bundle identifier for macOS app bundle")

    if(NOT CPACK_PACKAGE_VERSION OR CPACK_PACKAGE_VERSION STREQUAL "v0.0.0")
        # default when CPACK_PACKAGE_VERSION not set
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING "0.0.0")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "0.0.0")
    else()
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
    endif()

    # Configure File from template into the build directory
    # The template uses @VAR@ substitution (we use @ONLY in configure_file)
    configure_file("${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist.in"
                   "${CMAKE_BINARY_DIR}/Info.plist"
                   @ONLY)

    # Attach the Info.plist to the generated bundle and mark as a bundle
    set_target_properties(structuracost PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/Info.plist"
    )
    add_custom_command(TARGET structuracost POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/Info.plist"
            "$<TARGET_BUNDLE_DIR:structuracost>/Contents/Info.plist"
    )

endif()


# -------------------------------
# PACKAGING: macOS (app bundle) - install rules and resource copying
# -------------------------------
if(APPLE)
    install(TARGETS structuracost BUNDLE DESTINATION .)
    install(DIRECTORY assets/ DESTINATION "structuracost.app/Contents/Resources/assets")

    # Do not automatically bundle libsodium dylibs here. If a dylib from dependencies needs
    # to be bundled into the app, manage that explicitly in packaging scripts or by adding
    # the specific files to the install rules. This keeps the CMake configuration simple.

    # copy app icon into app resources (in addition to hello_imgui's conversion)
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/icons/app_icon.png")
        install(FILES "${CMAKE_SOURCE_DIR}/assets/icons/app_icon.png" DESTINATION "structuracost.app/Contents/Resources/")
    endif()

# -------------------------------
# PACKAGING: Windows (installer payload)
# -------------------------------
elseif(WIN32)
    # Install executable into root of package
    install(TARGETS structuracost
            RUNTIME
            DESTINATION "."
            COMPONENT Application)

    # Install assets with explicit component
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/"
            DESTINATION "assets"
            COMPONENT Application
            PATTERN "*"
            PATTERN ". git" EXCLUDE)

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

    set(APP_ICON_RC ${CMAKE_SOURCE_DIR}/assets/icons/app_icon.rc)
    target_sources(structuracost PRIVATE ${APP_ICON_RC})

    # Stage packaging resources into build directory to avoid absolute path issues
    set(PACKAGING_RES_DIR "${CMAKE_BINARY_DIR}/ci_packaging/resources/windows")
    file(MAKE_DIRECTORY "${PACKAGING_RES_DIR}")

    # Define packaging files to copy and install
    set(_PACKAGING_FILES
        "packaging/windows/install_dependencies.ps1:."
        "packaging/windows/install_libsodium.ps1:."
        "packaging/windows/README_WINDOWS.txt:."
        "doc/WINDOWS_ERROR_0xc000007b_FIX.md:doc"
    )

    # Copy files to staging directory and install them
    foreach(_file_spec ${_PACKAGING_FILES})
        string(REPLACE ":" ";" _parts "${_file_spec}")
        list(GET _parts 0 _src_rel)
        list(GET _parts 1 _dest)
        set(_src_path "${CMAKE_SOURCE_DIR}/${_src_rel}")
        get_filename_component(_filename "${_src_path}" NAME)

        if(EXISTS "${_src_path}")
            file(COPY "${_src_path}" DESTINATION "${PACKAGING_RES_DIR}/${_dest}")
            install(FILES "${PACKAGING_RES_DIR}/${_dest}/${_filename}" DESTINATION "${_dest}")
        endif()
    endforeach()

    # (Staging is performed above.)
    # Provide a concise count-only status message for CI logs.
    file(GLOB_RECURSE _CI_STAGED_FILES RELATIVE "${PACKAGING_RES_DIR}" "${PACKAGING_RES_DIR}/*")
    list(LENGTH _CI_STAGED_FILES _STAGED_COUNT)
    if(_STAGED_COUNT)
        message(STATUS "Staged ${_STAGED_COUNT} packaging files")
    endif()

    # NSIS install commands
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        DetailPrint 'Installing required dependencies...'
        ExecWait 'powershell -ExecutionPolicy Bypass -WindowStyle Normal -File \\\"$INSTDIR\\\\install_dependencies.ps1\\\" -InstallDir \\\"$INSTDIR\\\"'
    ")

# NOTE: Linux packaging removed by project policy — no installer will be produced for Linux/Unix.
# If a package for Linux is needed in the future, add a dedicated CI pipeline and packaging job.
endif()


# -------------------------------
# SECTION: CPACK CONFIGURATION (unified)
# -------------------------------
set(CPACK_PACKAGE_NAME "structuracost")
set(CPACK_PACKAGE_VENDOR "structuracost")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "StructuraCost - payroll and monitoring system")
if(NOT CPACK_PACKAGE_VERSION)
    set(CPACK_PACKAGE_VERSION "v0.0.0")
endif()
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt" CACHE FILEPATH "")

if(WIN32)
    # Prefer MSI via WiX if available; fall back to NSIS (exe) if WiX is not present.
    # Building MSI requires the WiX Toolset (candle.exe/light.exe) installed and on PATH.
    # CPack will pick the first working generator in the list.
    set(CPACK_GENERATOR "WIX;NSIS")
    target_link_libraries(structuracost PRIVATE Advapi32)

    # Keep existing NSIS configuration as a fallback
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


    # Keep NSIS extra install commands (executing dependency installer)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        DetailPrint 'Installing required dependencies...'
        ExecWait 'powershell -ExecutionPolicy Bypass -WindowStyle Normal -File \\\"$INSTDIR\\\\install_dependencies.ps1\\\" -InstallDir \\\"$INSTDIR\\\"'
    ")

elseif(APPLE)
    # Produce a DMG only (DragNDrop). To create DMGs for both arm64 and x86_64,
    # build separate binaries for each architecture (or a universal binary) and
    # package them separately — CPack/CPack DragNDrop will create a .dmg per build.
    set(CPACK_GENERATOR "DragNDrop")

    # Configure DMG to show Applications folder link (standard macOS DMG layout)
    set(CPACK_DMG_VOLUME_NAME "StructuraCost")
    set(CPACK_DMG_FORMAT "UDBZ")

    # Create symlink to /Applications in the DMG for drag-and-drop installation
    set(CPACK_DMG_CREATE_SYMLINK TRUE)

    # Link macOS Security framework if available (unchanged)
    find_library(SECURITY_FRAMEWORK Security)
    if(SECURITY_FRAMEWORK)
        target_link_libraries(structuracost PRIVATE ${SECURITY_FRAMEWORK})
    endif()
    # Add postinstall script for macOS packages (if any)
    set(CPACK_POSTFLIGHT_SCRIPT "${CMAKE_SOURCE_DIR}/packaging/macos/postinstall")

else()
    # Non-Windows, non-macOS platforms: disable installer creation.
    message(STATUS "CPack installer generation is disabled for this platform (Linux/Unix). No installer will be produced.")
    unset(CPACK_GENERATOR)
endif()

# Include CPack only when we're producing a package (Windows or macOS)
if(DEFINED CPACK_GENERATOR AND (WIN32 OR APPLE))
    include(CPack)
else()
    message(STATUS "Skipping include(CPack) — packaging disabled on this platform.")
endif()
