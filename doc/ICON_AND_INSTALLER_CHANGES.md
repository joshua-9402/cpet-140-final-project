# Application Icon and Installer Enhancement Summary

## Changes Made (December 17, 2025)

### 1. Application Icon Implementation

**Files Modified/Created:**
- `src/ui/set_app_icon_macos.mm` (new) - macOS dock icon setter using Cocoa APIs
- `src/ui/ui.cpp` - Added calls to set application icon on all platforms
- `CMakeLists.txt` - Configured icon asset copying and platform-specific builds

**Functionality:**
- **Windows (GLFW)**: Sets window icon using `glfwSetWindowIcon()` with app_icon.png
- **macOS (GLFW/SDL)**: Sets both window icon AND dock icon using NSApp's `setApplicationIconImage`
- **Linux (SDL)**: Sets window icon using `SDL_SetWindowIcon()`
- Icon loaded from `assets/icons/app_icon.png`, fallback to `business_logo.png`

**How It Works:**
1. During `PostInit` callback, the icon path is resolved via HelloImGui asset system
2. Icon is loaded using stb_image (RGBA, 4 channels)
3. Platform-specific APIs set the icon:
   - GLFW: Creates GLFWimage struct and calls glfwSetWindowIcon
   - macOS: Calls extern C function `setAppIcon()` which uses Cocoa NSImage
   - SDL: Creates SDL_Surface and calls SDL_SetWindowIcon

---

### 2. Windows Desktop Shortcut

**Files Modified:**
- `CMakeLists.txt` - Added NSIS installer configuration for desktop shortcuts

**Functionality:**
- Windows installers now automatically create a desktop shortcut during installation
- Shortcut points to `$INSTDIR\structuracost.exe`
- Icon embedded in the executable is used for the shortcut

**NSIS Configuration Added:**
```cmake
set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$DESKTOP\\\\StructuraCost.lnk' '$INSTDIR\\\\structuracost.exe' '' '$INSTDIR\\\\structuracost.exe' 0")
set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$DESKTOP\\\\StructuraCost.lnk'")
```

---

### 3. Prebuilt Libsodium Bundling

**Files Modified:**
- `CMakeLists.txt` - Added install rules for libsodium runtime libraries on all platforms
- `doc/LIBSODIUM_PREBUILT_GUIDE.md` (new) - Documentation for adding prebuilt libraries

**Functionality:**

#### Windows
- Searches `dependencies/libsodium/**/*.dll` for runtime libraries
- Bundles all found .dll files into installer root directory
- Auto-detects architecture (x64 vs ARM64) and links appropriate .lib file

#### macOS
- Searches `dependencies/libsodium/**/*libsodium*.dylib` for dynamic libraries
- Bundles .dylib files into `structuracost.app/Contents/Frameworks`
- Ensures runtime dependencies are included in the app bundle

#### Linux
- Searches `dependencies/libsodium/**/*.so` for shared objects
- Bundles .so files into package `lib/` directory
- Works with macOS (.dmg/.pkg) and Windows (MSI/NSIS) installers; Linux packaging (.deb/.rpm) is no longer produced

**Developer Workflow:**
1. Download/build libsodium for your target platform
2. Place prebuilt files in `dependencies/libsodium/` following documented structure
3. CMake automatically detects and bundles them during packaging
4. No code changes needed - fully automatic

---

### 4. HelloImGui Icon Integration

**Files Modified:**
- `CMakeLists.txt` - Copies app_icon.png to build assets/app_settings/icon.png

**Functionality:**
- HelloImGui's macOS packaging looks for `assets/app_settings/icon.png`
- Automatically converts PNG to ICNS for macOS .app bundles
- Ensures consistent icon across all packaging methods

---

## Testing Recommendations

### Test Application Icon Display:
1. **macOS**: Check window title bar icon AND dock icon when app is running
2. **Windows**: Check window title bar icon and taskbar icon
3. **Linux**: Check window icon and system tray (if applicable)

### Test Windows Installer:
1. Build Windows installer: `cd build && cpack`
2. Install the .exe on a Windows machine
3. Verify desktop shortcut is created
4. Verify shortcut has proper icon
5. Verify app launches from shortcut

### Test Libsodium Bundling:
1. Place prebuilt libsodium in `dependencies/libsodium/`
2. Configure and build
3. Create package: `cpack`
4. Install package on clean system (no system libsodium)
5. Verify app runs without "missing library" errors

---

## Migration Notes

**No Breaking Changes**: All changes are backward compatible
- If `dependencies/libsodium/` doesn't exist, CMake falls back to system libraries
- If app_icon.png is missing, falls back to business_logo.png
- Desktop shortcuts are Windows-only (no impact on other platforms)

**For CI/CD Pipelines:**
- GitHub Actions can pre-download libsodium and place in dependencies/ before build
- Ensures consistent packaging across all platforms
- No external dependencies needed at build time

---

## File Summary

**New Files:**
- `src/ui/set_app_icon_macos.mm` - macOS icon helper (Objective-C++)
- `doc/LIBSODIUM_PREBUILT_GUIDE.md` - Libsodium setup documentation

**Modified Files:**
- `CMakeLists.txt` - Icon setup, libsodium bundling, NSIS shortcuts
- `src/ui/ui.cpp` - Icon loading and setting in PostInit

**Build System Changes:**
- macOS: Added .mm file compilation support
- All platforms: Added icon asset copying during configure
- NSIS: Added desktop shortcut creation scripts
