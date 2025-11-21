### What’s going on
CMake can’t see the command `hello_imgui_add_app` because that macro isn’t in scope in your top-level `CMakeLists.txt`. With HelloImGui, the macro becomes available only if you include the helper CMake module after making the dependency available.

Right now you have:

```
include(FetchContent)
FetchContent_Declare(
        hello_imgui
        GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
)
FetchContent_MakeAvailable(hello_imgui)

# (then) hello_imgui_add_app(...)
```

But you never `include(hello_imgui_add_app)`, nor do you `find_package(hello_imgui ...)` (which would bring the module in via the package config).

### Two ways to fix it
Pick one of the approaches below.

#### Option A — Keep FetchContent and include the macro
Add a single `include` right after `FetchContent_MakeAvailable(hello_imgui)`:

```
cmake_minimum_required(VERSION 3.20)
project(cpet_140_final_project)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
    hello_imgui
    GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
)
FetchContent_MakeAvailable(hello_imgui)

# Bring the macro into scope (uses the dependency’s source dir variable set by FetchContent)
include(${hello_imgui_SOURCE_DIR}/hello_imgui_cmake/hello_imgui_add_app.cmake)

hello_imgui_add_app(
    pos-and-inventory-system
    dependencies/sqlite/sqlite3.c
    dependencies/sqlite/sqlite3.h
    src/main.cpp
    src/lib/db.cpp
    src/lib/db.h
    src/ui/UI.cpp
    src/ui/UI.h
    src/lib/payroll.cpp
    src/lib/payroll.h
    src/lib/monitor.cpp
    src/lib/monitor.h
    ASSETS_LOCATION "assets/"
)
```

Notes:
- `hello_imgui_SOURCE_DIR` is defined by `FetchContent_MakeAvailable`.
- I also recommend realistic versions: `cmake_minimum_required(VERSION 3.20)` and `set(CMAKE_CXX_STANDARD 20)` — adjust if you truly require newer C++.

#### Option B — Use find_package instead of FetchContent
If you prefer a preinstalled package (vcpkg, system, or a prior build/install of HelloImGui):

```
cmake_minimum_required(VERSION 3.20)
project(cpet_140_final_project)
set(CMAKE_CXX_STANDARD 20)

find_package(hello_imgui CONFIG REQUIRED)
include(hello_imgui_add_app) # provided by the package config

hello_imgui_add_app(
    pos-and-inventory-system
    # ...same sources as above...
    ASSETS_LOCATION "assets/"
)
```

### After editing, clean and re-configure
Because the error happened during configure, clear your build cache to avoid stale state:
- In CLion: File → Reload CMake Project (or click the reload icon), and if that fails, delete the `cmake-build-*` directory and re-run CMake.
- Or from the terminal: delete your build directory (e.g., `cmake-build-cpet-140-final-project-win-unix`) and configure again.

### Why you saw references under build/_deps
You noticed `build-noqt/_deps/hello_imgui-src/...` contains `hello_imgui_add_app.cmake`. That’s inside the dependency’s source tree. It doesn’t automatically make the macro globally visible; you must explicitly include it from your top-level CMake with one of the approaches above.

### Bonus checks
- Ensure your network allows `FetchContent` to clone the repo.
- Don’t mix multiple build dirs (e.g., `build-noqt` and `cmake-build-*`) for the same source tree without cleaning when you change dependency mechanisms.
- `ASSETS_LOCATION "assets/"` is fine; HelloImGui will embed or copy accordingly.

If you paste your current `CMakeLists.txt` after trying these changes (and the CMake version you’re using), I can tailor it precisely and help resolve any follow-up errors.