# Existing Document Content

---

## Repository sync note
Updated to reflect current UI calling convention and selector behavior (2025-11-29). If UI buttons revert, ensure state is stored in the global UI state (`ui::g_position`, `ui::g_userName`) and that `auth` functions don't mutate UI state directly.

# UI module (`ui.h` / `ui.cpp`)

## Purpose

Provides the immediate-mode UI for the application using HelloImGui/ImGui. This module is responsible for:
- constructing and running the HelloImGui runner
- loading assets and a custom font
- maintaining a registry of named UIs (left navigation and right panel mapping)
- implementing the login flow, account panel, selector/navigation panel, and several sample panels (summary, payroll, monitor, test)

The UI module is strictly presentation — business logic and persistence live in `handler/*` and `security/*` modules.

## Files

- `src/ui/ui.h` — declares the `ui` class, global UI strings, and `constructUI(...)`
- `src/ui/ui.cpp` — implementation of UI rendering, registry, and helper functions

## Public API (summary)

- `static std::string ui::g_failedMessage` — holds a message displayed by the failed modal
- `static std::string ui::g_userName` — current user name shown in account panel
- `static std::string ui::g_position` — current user position shown in account panel
- `static void ui::constructUI(const std::string &a_title, const std::string &a_fontLocation, int a_widthPx, int a_heightPx, const std::string &a_window)`
  - Initializes HelloImGui runner, sets assets folder and font callback, populates `g_uiMap`, and starts the UI loop.

Note: the module uses several internal static variables and functions (UI registry `g_uiMap`, `g_rightUI`, `g_currentUI`, `g_failedPopupOpen`, helper UI functions like `loginUI`, `selectorUI`, `mainUI`, `testUI`, etc.). Those are implementation details but important for understanding behavior.

## Data shapes

- UI state is stored in globals (`std::string`) and local static buffers inside UI functions (e.g., char arrays for Login inputs).
- The registry maps string keys to `std::function<void()>` handlers for named UIs.

## Behavior details

### `constructUI(...)`
- Sets HelloImGui assets folder by scanning parent paths for an `assets` directory.
- Configures HelloImGui windowing to use a full-screen root window.
- Populates `g_uiMap` with keys: `auth`, `main`, `summary`, `payroll`, `monitor`, `test`, `failed`.
- Loads a custom font (limited glyph ranges) via `params.callbacks.LoadAdditionalFonts` using the provided `a_fontLocation`.
- Uses the `a_window` argument (lowercased) to choose the starting UI/window. Recognized values: `auth`, `payroll`, `monitor`, `test`, `summary`, `main`.
- Registers `params.callbacks.ShowGui` to call the active `g_currentUI` each frame.

### Login flow (`loginUI`)
- Uses fixed-size char arrays for `username` and `password` inputs.
- Validates that both username and password are non-empty; if empty, sets `ui::g_failedMessage` and opens a modal popup named `Failed##login`.
- Calls `auth::testAuth` and `auth::testDeployAuth` to authenticate.
  - On success of either, sets `appConfig::g_auth = true`, sets `appConfig::g_testMode` appropriately, clears input buffers, and calls `system::appShutdown()` (app flow expects shutdown-to-restart behavior).
  - On failure, sets appropriate failure message and opens failed popup.
- The failed popup is modal and controlled by the `g_failedPopupOpen` flag; it must be rendered every frame to stay visible.

### Selector and Main layout (`selectorUI`, `mainUI`)
- `mainUI` implements a two-column layout: left navigation (selector) and right active panel.
- On first call `mainUI` sets the default right panel to `testUI` if `appConfig::g_testMode` is true, otherwise `summaryUI`.
- `selectorUI` displays greeting and `ui::g_userName`, and renders navigation buttons. For each navigation button click, it sets `g_rightUI` to the corresponding function from `g_uiMap` (if present), e.g. `g_uiMap["payroll"]`.
- The right pane simply calls `g_rightUI()` to render the currently selected panel each frame.

### Account panel (`accountUI`)
- Shows `ui::g_userName` and `ui::g_position` and a `Log Out` button.
- `Log Out` clears `appConfig::g_auth`, `appConfig::g_testMode`, resets `ui::g_userName` and `ui::g_position`, and calls `system::appShutdown()`.

### Test UI (`testUI`)
- Provides interactive testing controls (key generation, show hashed key, encrypt/decrypt files).
- Key generation flow:
  - `keySizeBits` user input (int)
  - `cryptography::generateKey(keySizeBits)` produces a `std::vector<unsigned char>` key stored in `storedKeyRaw`
  - `cryptography::toHex(storedKeyRaw)` and `cryptography::saltKey(...)` are used to display hashed and salted representations
- Encryption / Decryption buttons call `cryptography::encryptFile(location, storedKeyRaw)` and `cryptography::decryptFile(location + ".enc", storedKeyRaw)` respectively. On successful encryption the unencrypted file is deleted; on successful decryption the `.enc` file is deleted.

## Error modes and return conventions

- UI functions are void and operate via side-effects. Errors are surfaced to the user via `ui::g_failedMessage` and modal popups.
- Many UI flows call into other modules which return booleans or strings; callers should check those return values when integrating.

## Security considerations

- Do not keep plaintext passwords in memory longer than necessary. The current implementation uses fixed-size char buffers in `loginUI` and clears them after use; keep in mind these are not zeroed explicitly in all paths.
- The `testUI` display of hex and salted keys is for debugging only; avoid showing real keys or secrets in debug UIs.

## Build / runtime dependencies

- HelloImGui (and dependencies: ImGui, Plutovg, GLFW), plus `cryptography`, `auth`, `system`, and `config` modules.
- Assets folder (icons, fonts) must be present under an `assets` directory found by `constructUI` scanning parent paths.

## Usage examples

- Typical call from `main.cpp`:

```cpp
ui::constructUI(appConfig::g_appTitle, appConfig::g_fontName, appConfig::g_defaultWidth, appConfig::g_defaultHeight, "main");
```

- To start directly in the login screen:

```cpp
ui::constructUI("My App", "fonts/OpenSans-Regular.ttf", 1024, 768, "auth");
```

## Troubleshooting & debugging tips

- If selector buttons appear to revert immediately back to `summary`:
  - Confirm `g_rightUI` is being set in `selectorUI` (click handlers set `g_rightUI = g_uiMap["key"]`).
  - Ensure no other code overwrites `g_rightUI` each frame. `mainUI` only sets the default `g_rightUI` on first call (static `initialized` flag). If you still see reversion, search for places that assign `g_rightUI` or call `g_uiMap.clear()`.
  - Make sure `g_uiMap` contains the named key; if the map doesn't contain a key the code uses fallback static functions.

- If login modal doesn't appear for empty credentials:
  - Login code sets `g_failedPopupOpen = true` and calls `ImGui::OpenPopup("Failed##login")`. Ensure that `loginUI`'s popup rendering block runs every frame (it must be executed each frame to keep the popup open).

- If UI globals (`ui::g_userName`, `ui::g_position`) appear undefined or not updating after successful auth checks, verify the `auth` functions set them and that the `ui` globals are correctly linked (no duplicate symbol definitions).

- If fonts or icons do not load, `constructUI` attempts to find the `assets` directory by scanning paths relative to the source; ensure the `assets` folder exists in the expected location.

---

## Repository sync note

Updated to reflect the real implementation in `src/ui/ui.h` and `src/ui/ui.cpp` (sync date: 2025-11-29). This doc now maps actual globals, `constructUI` behavior, registry keys, and the login/test flows. If you want, I can also add a small sequence diagram to show the UI switching lifecycle.
