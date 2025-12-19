cmake_minimum_required(VERSION 3.22) # minimum CMake version

# CRITICAL: Set MSVC runtime library BEFORE project() to ensure all dependencies use correct runtime
# This MUST come before project() to apply to all targets including dependencies
if(WIN32 AND MSVC)
    # Use CMAKE_POLICY to set MSVC runtime library policy
    cmake_policy(SET CMP0091 NEW)
    # Force MultiThreadedDLL for Release, MultiThreadedDebugDLL for Debug
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC runtime library" FORCE)
endif()

project(cpet_140_final_project) # project name

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 20) # set C++ standard to C++20 (AppleClang doesn't support C++26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Force Release runtime libraries for MSVC - apply globally to all targets
if(MSVC)
    # Remove any existing /MDd or /MTd flags and replace with /MD for Release
    foreach(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/MDd" "/MD" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MTd" "/MT" ${flag_var} "${${flag_var}}")
        string(REPLACE "/D_DEBUG" "" ${flag_var} "${${flag_var}}")
        # Ensure /MD is set for release builds
        if(flag_var MATCHES "RELEASE|MINSIZEREL|RELWITHDEBINFO")
            if(NOT ${flag_var} MATCHES "/MD")
                set(${flag_var} "${${flag_var}} /MD")
            endif()
        endif()
    endforeach()

    # Explicitly set for all configurations
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /DNDEBUG")

    message(STATUS "MSVC Runtime: Using /MD for Release builds (MultiThreadedDLL)")
endif()

include(FetchContent) # Fetch Hello ImGui (automatic download)

# ============================================================================
# ULTRA-AGGRESSIVE RELEASE MODE ENFORCEMENT FOR DEPENDENCIES
# ============================================================================
# ABSOLUTELY NO DEBUG DLLS ALLOWED - NOT IN MAIN EXECUTABLE, NOT IN ANY DEPENDENCY!
if(MSVC)
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)

    # FORCE ALL DEPENDENCIES TO BUILD IN RELEASE MODE ONLY
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "🚨 ENFORCING RELEASE MODE FOR ALL DEPENDENCIES 🚨")
        message(STATUS "🚫 DEBUG DLLS ARE ABSOLUTELY FORBIDDEN 🚫")

        # Override FetchContent to always use Release for dependencies
        set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)

        # Force all subprojects to use Release runtime
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)
    endif()
endif()

FetchContent_Declare(
        hello_imgui
        GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
)
set(HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED ON CACHE BOOL "Download FreeType if needed" FORCE)
set(HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED ON CACHE BOOL "Download GLFW if needed" FORCE)
set(HELLOIMGUI_ADD_APP_WITH_INSTALL OFF CACHE BOOL "Disable automatic asset install" FORCE)

# FORCE hello_imgui and ALL its dependencies to use Release runtime
if(MSVC AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_BUILD_TYPE_INIT "Release" CACHE STRING "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(hello_imgui)

# ============================================================================
# POST-FETCH: FORCE ALL DEPENDENCY TARGETS TO USE RELEASE RUNTIME
# ============================================================================
if(MSVC AND CMAKE_BUILD_TYPE STREQUAL "Release")
    # Get ALL targets including dependencies
    function(force_release_runtime_on_all_targets)
        # Get all targets recursively (only non-imported targets)
        get_directory_property(all_targets DIRECTORY ${CMAKE_SOURCE_DIR} BUILDSYSTEM_TARGETS)

        foreach(target ${all_targets})
            if(TARGET ${target})
                # Check if target is IMPORTED (cannot modify IMPORTED targets)
                get_target_property(is_imported ${target} IMPORTED)
                if(is_imported)
                    continue()
                endif()

                get_target_property(target_type ${target} TYPE)

                # Apply to executable and library targets (skip INTERFACE_LIBRARY)
                if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
                    message(STATUS "🔧 Forcing Release runtime on: ${target}")

                    # Set MSVC runtime library
                    set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

                    # Remove debug flags
                    target_compile_options(${target} PRIVATE
                        $<$<CONFIG:Release>:/MD>
                        $<$<CONFIG:Release>:/DNDEBUG>
                    )

                    # Remove _DEBUG definition
                    get_target_property(compile_defs ${target} COMPILE_DEFINITIONS)
                    if(compile_defs)
                        list(REMOVE_ITEM compile_defs "_DEBUG")
                        set_property(TARGET ${target} PROPERTY COMPILE_DEFINITIONS ${compile_defs})
                    endif()
                endif()
            endif()
        endforeach()
    endfunction()

    # Apply to all targets
    force_release_runtime_on_all_targets()

    message(STATUS "✅ ALL TARGETS FORCED TO RELEASE RUNTIME")
endif()

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

# Windows-specific configuration to fix 0xc000007b error
if(WIN32)
    # Ensure we're building for the correct architecture
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_GENERATOR_PLATFORM x64)
        add_definitions(-D_WIN64)
    else()
        set(CMAKE_GENERATOR_PLATFORM Win32)
    endif()

    # Additional enforcement of Release runtime for the main target
    # (Global settings should already handle this, but double-check)
    if(MSVC)
        set_property(TARGET structuracost PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

        # Explicitly force /MD for Release configurations
        target_compile_options(structuracost PRIVATE
            $<$<CONFIG:Release>:/MD /DNDEBUG>
            $<$<CONFIG:Debug>:/MDd /D_DEBUG>
        )

        # Remove any lingering debug flags from Release builds
        target_compile_definitions(structuracost PRIVATE
            $<$<CONFIG:Release>:NDEBUG>
            $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
        )
    endif()

    # Copy required DLLs to output directory after build
    add_custom_command(TARGET structuracost POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Checking for required DLLs..."
        COMMENT "Post-build DLL check for Windows"
    )

    # If libsodium.dll exists in dependencies, copy it
    file(GLOB_RECURSE SODIUM_DLLS "${CMAKE_SOURCE_DIR}/dependencies/libsodium/*.dll")
    if(SODIUM_DLLS)
        # Filter for correct architecture
        foreach(DLL_PATH ${SODIUM_DLLS})
            string(TOUPPER "${DLL_PATH}" DLL_PATH_UPPER)
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                # 64-bit build - look for x64/Win64/AMD64 DLLs
                if(DLL_PATH_UPPER MATCHES "X64" OR DLL_PATH_UPPER MATCHES "WIN64" OR DLL_PATH_UPPER MATCHES "AMD64")
                    add_custom_command(TARGET structuracost POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${DLL_PATH}"
                        "$<TARGET_FILE_DIR:structuracost>/libsodium.dll"
                        COMMENT "Copying libsodium.dll (x64) to output directory"
                    )
                    break()
                endif()
            else()
                # 32-bit build - look for Win32/x86 DLLs
                if(DLL_PATH_UPPER MATCHES "WIN32" OR DLL_PATH_UPPER MATCHES "\\bX86\\b")
                    add_custom_command(TARGET structuracost POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${DLL_PATH}"
                        "$<TARGET_FILE_DIR:structuracost>/libsodium.dll"
                        COMMENT "Copying libsodium.dll (Win32) to output directory"
                    )
                    break()
                endif()
            endif()
        endforeach()
    endif()
endif()


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
        message(WARNING "libsodium headers not found at ${LOCAL_LIBSODIUM_ROOT}/include/sodium.h - will attempt to use system libsodium")
    else()

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
    endif()  # end of else block for libsodium header check
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
    message(WARNING "libsodium not found - some security features may not be available. The build will continue but may fail at link time if libsodium symbols are required.")
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

    # Fail packaging if the built executable links against debug CRT DLLs
    # (ucrtbased.dll, vcruntime140d.dll, msvcp140d.dll). These are not redistributable.
    if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/windows/cmake_verify_release_runtime.cmake")
        set(CPACK_PRE_BUILD_SCRIPTS "${CMAKE_SOURCE_DIR}/packaging/windows/cmake_verify_release_runtime.cmake")
    endif()

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
    # Add dependency installer scripts
    install(FILES "${CMAKE_SOURCE_DIR}/packaging/windows/install_dependencies.ps1" DESTINATION ".")
    install(FILES "${CMAKE_SOURCE_DIR}/packaging/windows/install_libsodium.ps1" DESTINATION ".")
    # Add Windows README and troubleshooting guide
    install(FILES "${CMAKE_SOURCE_DIR}/packaging/windows/README_WINDOWS.txt" DESTINATION ".")
    install(FILES "${CMAKE_SOURCE_DIR}/doc/WINDOWS_ERROR_0xc000007b_FIX.md" DESTINATION "doc")
    # Add custom NSIS commands to run comprehensive dependency installer after installation
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        DetailPrint 'Installing required dependencies...'
        ExecWait 'powershell -ExecutionPolicy Bypass -WindowStyle Normal -File \\\"$INSTDIR\\\\install_dependencies.ps1\\\" -InstallDir \\\"$INSTDIR\\\"'
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
