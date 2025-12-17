; StructuraCost NSIS Installer Script - libsodium Auto-Install
; Automatically downloads and installs libsodium if not present on Windows

!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

; Custom page to install libsodium
Page custom CheckLibsodium

Var LIBSODIUM_NEEDED
Var DOWNLOAD_URL
Var TEMP_DIR

Function CheckLibsodium
    ; Check if libsodium.dll is bundled
    IfFileExists "$INSTDIR\libsodium.dll" LibsodiumFound

    ; Check if libsodium is in system PATH
    nsExec::ExecToStack 'where libsodium.dll'
    Pop $0
    ${If} $0 == 0
        Goto LibsodiumFound
    ${EndIf}

    ; libsodium not found - download it
    DetailPrint "libsodium not found - downloading..."
    StrCpy $LIBSODIUM_NEEDED "1"
    Call DownloadLibsodium
    Goto Done

    LibsodiumFound:
        DetailPrint "libsodium already available"
        StrCpy $LIBSODIUM_NEEDED "0"

    Done:
FunctionEnd

Function DownloadLibsodium
    ; Detect architecture
    ${If} ${RunningX64}
        StrCpy $DOWNLOAD_URL "https://download.libsodium.org/libsodium/releases/libsodium-1.0.20-stable-msvc.zip"
    ${Else}
        StrCpy $DOWNLOAD_URL "https://download.libsodium.org/libsodium/releases/libsodium-1.0.20-stable-msvc.zip"
    ${EndIf}

    StrCpy $TEMP_DIR "$TEMP\libsodium_install"
    CreateDirectory "$TEMP_DIR"

    DetailPrint "Downloading libsodium from official repository..."
    inetc::get /CAPTION "Downloading libsodium..." /CANCELTEXT "Cancel" $DOWNLOAD_URL "$TEMP_DIR\libsodium.zip" /END
    Pop $0

    ${If} $0 != "OK"
        DetailPrint "Download failed: $0"
        MessageBox MB_ICONEXCLAMATION "Failed to download libsodium. The application may not work correctly."
        Return
    ${EndIf}

    DetailPrint "Extracting libsodium..."
    nsisunz::Unzip "$TEMP_DIR\libsodium.zip" "$TEMP_DIR"
    Pop $0

    ${If} $0 != "success"
        DetailPrint "Extraction failed"
        MessageBox MB_ICONEXCLAMATION "Failed to extract libsodium. The application may not work correctly."
        Return
    ${EndIf}

    ; Copy the appropriate DLL to installation directory
    ${If} ${RunningX64}
        CopyFiles /SILENT "$TEMP_DIR\libsodium\x64\Release\v143\dynamic\libsodium.dll" "$INSTDIR\libsodium.dll"
    ${Else}
        CopyFiles /SILENT "$TEMP_DIR\libsodium\Win32\Release\v143\dynamic\libsodium.dll" "$INSTDIR\libsodium.dll"
    ${EndIf}

    ; Cleanup
    RMDir /r "$TEMP_DIR"

    DetailPrint "libsodium installed successfully"
FunctionEnd

; Cleanup function for uninstaller
Function un.RemoveLibsodium
    ; Only remove if we installed it
    ${If} $LIBSODIUM_NEEDED == "1"
        Delete "$INSTDIR\libsodium.dll"
    ${EndIf}
FunctionEnd

