# GitHub Actions Release Build Fix - COMPREHENSIVE SOLUTION

## Problem
The GitHub Actions workflow was building Windows executables that referenced **debug runtime libraries** (`ucrtbased.dll` and `vcruntime140d.dll` - note the `d` suffix indicating debug versions) even when building in Release mode. This caused the error:

```
Error 0xc000007b: The application was unable to start correctly
```

This happened because dependencies (particularly hello_imgui and its FetchContent dependencies) were being built with debug runtime libraries, causing the final executable to link against them.

## Root Cause
1. CMakeLists.txt did not set `CMAKE_MSVC_RUNTIME_LIBRARY` **before** `project()` was called
2. Runtime library settings were only applied to the main target, not to FetchContent dependencies
3. No verification was in place to catch debug runtime library usage
4. CMake's default behavior can link debug libraries even in Release mode if dependencies use them

## Comprehensive Solution Implemented

### 1. Pre-Project Runtime Library Configuration

Set the MSVC runtime library **before** `project()` is called to ensure it applies to all subsequent targets:

```cmake
cmake_minimum_required(VERSION 3.22)

# CRITICAL: Set MSVC runtime library BEFORE project() to ensure all dependencies use correct runtime
if(WIN32 AND MSVC)
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC runtime library" FORCE)
endif()

project(cpet_140_final_project)
```

### 2. Global Compiler Flag Cleanup

Remove debug flags and force release flags globally:

```cmake
if(MSVC)
    # Remove any existing /MDd or /MTd flags and replace with /MD for Release
    foreach(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/MDd" "/MD" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MTd" "/MT" ${flag_var} "${${flag_var}}")
        string(REPLACE "/D_DEBUG" "" ${flag_var} "${${flag_var}}")
        
        if(flag_var MATCHES "RELEASE|MINSIZEREL|RELWITHDEBINFO")
            if(NOT ${flag_var} MATCHES "/MD")
                set(${flag_var} "${${flag_var}} /MD")
            endif()
        endif()
    endforeach()
    
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /DNDEBUG")
endif()
```

### 3. FetchContent Dependency Configuration

Ensure all FetchContent dependencies use the same runtime library:

```cmake
include(FetchContent)

if(MSVC)
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)
endif()

FetchContent_Declare(hello_imgui ...)
```

### 4. Target-Specific Enforcement

Double-check the main target's runtime library settings:

```cmake
if(WIN32 AND MSVC)
    set_property(TARGET structuracost PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    target_compile_options(structuracost PRIVATE
        $<$<CONFIG:Release>:/MD /DNDEBUG>
        $<$<CONFIG:Debug>:/MDd /D_DEBUG>
    )
    
    target_compile_definitions(structuracost PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    )
endif()
```

### 5. Post-Build Verification

Created verification scripts that run after build to detect debug DLLs:

**`packaging/windows/verify_release_build.ps1`**:
- Uses `dumpbin` to inspect executable dependencies
- Detects `ucrtbased.dll`, `vcruntime140d.dll`, `msvcp140d.dll`
- **Fails the build** if debug DLLs are found
- Provides detailed error messages

**GitHub Actions Integration**:
```yaml
- name: Verify Release Build (Check for Debug DLLs)
  shell: pwsh
  run: |
    $exe = Get-ChildItem -Recurse build -Filter "structuracost.exe" | Select-Object -First 1
    & powershell -ExecutionPolicy Bypass -File "packaging/windows/verify_release_build.ps1" -ExecutablePath $exe.FullName
    if ($LASTEXITCODE -ne 0) {
      Write-Host "CRITICAL ERROR: Build contains DEBUG runtime dependencies!"
      exit 1
    }
```

## Multi-Layer Protection Strategy

1. **Layer 1: Pre-Project** - Set runtime library before any targets exist
2. **Layer 2: Global Flags** - Clean and enforce flags across all build configurations
3. **Layer 3: FetchContent** - Apply settings to all dependencies
4. **Layer 4: Target-Specific** - Double-enforce on main executable
5. **Layer 5: Verification** - Validate final output catches any mistakes

## Result

✅ **Release builds now correctly link against release runtime libraries**
- `ucrtbase.dll` (without `d`)
- `vcruntime140.dll` (without `d`)
- `msvcp140.dll` (without `d`)

✅ **Builds fail early if debug libraries are detected**
- Prevents shipping broken builds
- Clear error messages guide fixes

✅ **All dependencies use consistent runtime library**
- hello_imgui
- GLFW
- FreeType
- All other FetchContent dependencies

## Testing & Verification

### Local Testing
```bash
# Windows (PowerShell)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
powershell -ExecutionPolicy Bypass -File packaging/windows/verify_release_build.ps1 -ExecutablePath build/structuracost.exe
```

### Manual Verification
```bash
# Check dependencies manually
dumpbin /dependents build/structuracost.exe

# Should show:
#   ucrtbase.dll       ✓ (not ucrtbased.dll)
#   vcruntime140.dll   ✓ (not vcruntime140d.dll)
```

### GitHub Actions
The workflow now automatically:
1. Builds in Release mode
2. Runs verification script
3. **Fails if debug DLLs detected**
4. Outputs detailed dependency information

## References
- [CMake Policy CMP0091](https://cmake.org/cmake/help/latest/policy/CMP0091.html)
- [MSVC Runtime Library Property](https://cmake.org/cmake/help/latest/prop_tgt/MSVC_RUNTIME_LIBRARY.html)
- [Microsoft: /MD, /MT, /LD](https://docs.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library)
- [dumpbin Reference](https://docs.microsoft.com/en-us/cpp/build/reference/dumpbin-reference)

## Files Modified
- `CMakeLists.txt` - Multi-layer runtime library configuration
- `.github/workflows/release.yml` - Added verification step
- `.github/workflows/build-windows.yml` - Added verification step
- `packaging/windows/verify_release_build.ps1` - Verification script
- `packaging/windows/verify_release_build.bat` - Batch verification script
- `CHANGELOG.md` - Documented changes

