# CPack pre-build verification for Windows: fail packaging if the executable links debug CRT DLLs
# Expected to be invoked by CPack (CPACK_PRE_BUILD_SCRIPTS).

if(NOT WIN32)
    return()
endif()

# CPack runs this with CPACK_TOPLEVEL_DIRECTORY/CPACK_BUILD_CONFIG possibly set.
# Use CMAKE_BINARY_DIR if available, otherwise fall back to current script directory context.
set(_candidate_roots "")

if(DEFINED CMAKE_BINARY_DIR)
    list(APPEND _candidate_roots "${CMAKE_BINARY_DIR}")
endif()

# When invoked by cpack from the build dir, CMAKE_BINARY_DIR may not be set.
# CPack does set CPACK_TOPLEVEL_DIRECTORY to a staging directory but the binary is in the build tree.
# Try the current working directory as a fallback.
list(APPEND _candidate_roots "${CMAKE_CURRENT_LIST_DIR}/../..")
list(APPEND _candidate_roots "${CMAKE_CURRENT_BINARY_DIR}")
list(APPEND _candidate_roots "${CMAKE_CURRENT_LIST_DIR}")

# Find structuracost.exe
set(_exe "")
foreach(_root IN LISTS _candidate_roots)
    if(_exe)
        break()
    endif()

    if(NOT EXISTS "${_root}")
        continue()
    endif()

    file(GLOB_RECURSE _found_exes
        "${_root}/structuracost.exe"
        "${_root}/*/structuracost.exe"
        "${_root}/**/structuracost.exe"
    )
    list(LENGTH _found_exes _len)
    if(_len GREATER 0)
        list(GET _found_exes 0 _exe)
    endif()
endforeach()

if(NOT _exe OR NOT EXISTS "${_exe}")
    message(WARNING "[CPack preflight] Could not locate structuracost.exe to verify debug CRT dependencies. Packaging will continue.")
    return()
endif()

message(STATUS "[CPack preflight] Verifying Release runtime dependencies for: ${_exe}")

set(_verifier "${CMAKE_CURRENT_LIST_DIR}/verify_release_build.ps1")
if(NOT EXISTS "${_verifier}")
    message(WARNING "[CPack preflight] Verification script not found at ${_verifier}. Packaging will continue.")
    return()
endif()

# Execute the verifier. It exits non-zero if debug CRT DLLs are detected.
execute_process(
    COMMAND powershell -ExecutionPolicy Bypass -File "${_verifier}" -ExecutablePath "${_exe}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
)

# Always echo output to help diagnose failures in CI.
if(_out)
    string(REPLACE "\r\n" "\n" _out_nl "${_out}")
    message(STATUS "[CPack preflight] verifier output:\n${_out_nl}")
endif()
if(_err)
    string(REPLACE "\r\n" "\n" _err_nl "${_err}")
    message(STATUS "[CPack preflight] verifier stderr:\n${_err_nl}")
endif()

if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "[CPack preflight] Debug CRT dependencies detected (e.g., ucrtbased.dll / vcruntime140d.dll). Refusing to package. Ensure you built Release (/MD) and are packaging the Release artifact.")
endif()

message(STATUS "[CPack preflight] OK: no debug CRT DLL dependencies detected.")

