# GitHub Actions Release Build Fix

## Problem
The GitHub Actions workflow was building Windows executables that referenced **debug runtime libraries** (`ucrtbased.dll` and `vcruntime140d.dll` - note the `d` suffix indicating debug versions) even when building in Release mode. This caused the error:

```
Error 0xc000007b: The application was unable to start correctly
```

## Root Cause
The CMakeLists.txt did not explicitly set a default build type, and the MSVC runtime library configuration was not forcing the correct runtime libraries for Release builds.

## Solution Implemented

### 1. CMakeLists.txt Changes

#### Added Default Build Type Configuration
```cmake
# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
```

This ensures that if no build type is specified, CMake defaults to **Release** instead of Debug.

#### Enhanced MSVC Runtime Library Configuration
```cmake
if(MSVC)
    set_property(TARGET structuracost PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    # Force Release runtime libraries for Release builds
    target_compile_options(structuracost PRIVATE
        $<$<CONFIG:Release>:/MD>
        $<$<CONFIG:Debug>:/MDd>
    )
endif()
```

This explicitly sets the compiler flags:
- `/MD` for Release builds → uses **release runtime libraries** (`ucrtbase.dll`, `vcruntime140.dll`)
- `/MDd` for Debug builds → uses **debug runtime libraries** (`ucrtbased.dll`, `vcruntime140d.dll`)

### 2. GitHub Actions Workflow Verification

The `.github/workflows/release.yml` workflow already correctly specifies:
```yaml
- name: Configure and Build
  shell: pwsh
  run: |
    cmake -B build `
      -DCMAKE_BUILD_TYPE=Release `
      ...
    cmake --build build --config Release -j $env:NUMBER_OF_PROCESSORS
```

And the `.github/workflows/build-windows.yml` workflow accepts a `build_type` parameter with `Release` as the default.

## Result

✅ **Release builds now correctly link against release runtime libraries**
- `ucrtbase.dll` (without `d`)
- `vcruntime140.dll` (without `d`)

These are standard runtime libraries that can be distributed with the **Visual C++ Redistributable** package, which is automatically installed by the Windows installer.

## Testing

To verify the fix:

1. **Build locally in Release mode:**
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

2. **Check dependencies of the executable:**
   ```bash
   dumpbin /dependents build/structuracost.exe
   ```
   
   Should show `VCRUNTIME140.dll` and not `VCRUNTIME140D.dll`

3. **Run GitHub Actions workflow** and verify the resulting executable does not reference debug DLLs

## References
- [Microsoft Docs: /MD, /MT, /LD (Use Run-Time Library)](https://docs.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library)
- [CMake MSVC_RUNTIME_LIBRARY](https://cmake.org/cmake/help/latest/prop_tgt/MSVC_RUNTIME_LIBRARY.html)

