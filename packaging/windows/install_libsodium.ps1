<#
install_libsodium.ps1

Purpose:
- Minimal helper that installs libsodium runtime DLLs into the application's install directory.
- Intended to be invoked from `install_dependencies.ps1` during installer execution or run manually.

Behavior:
- Detect target architecture (x64 or ARM64) via -Arch parameter or environment.
- If a bundled libsodium DLL exists next to this script, use that.
- Otherwise download the official libsodium MSVC release zip (fallback URL provided), extract, find matching DLLs, and copy them into $InstallDir.
- Exit code: 0 on success, non-zero on failure.

Usage:
  powershell -ExecutionPolicy Bypass -File install_libsodium.ps1 -InstallDir "C:\Program Files\StructuraCost"

Parameters:
-InstallDir  : destination directory where DLLs will be copied (default: .)
-[Arch]      : optional override architecture: x64 or arm64
-[Quiet]     : suppress verbose output
#>
param(
    [Parameter(Mandatory=$false)][string]$InstallDir = ".",
    [Parameter(Mandatory=$false)][ValidateSet('x64','arm64')][string]$Arch = $null,
    [switch]$Quiet,
    [switch]$SkipDownload
)

function Write-Info {
    param(
        [Parameter(ValueFromRemainingArguments=$true)]
        [object[]]$Message
    )
    if (-not $Quiet) { Write-Host ($Message -join ' ') }
}
function Write-Warn {
    param(
        [Parameter(ValueFromRemainingArguments=$true)]
        [object[]]$Message
    )
    Write-Warning ($Message -join ' ')
}
function Write-Err {
    param(
        [Parameter(ValueFromRemainingArguments=$true)]
        [object[]]$Message
    )
    Write-Error ($Message -join ' ')
}

Write-Info "install_libsodium.ps1: invoked. InstallDir='$InstallDir' Arch='$Arch' SkipDownload=$SkipDownload"

# Resolve the script dir
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition

# helper: ensure destination exists
try {
    if (-not (Test-Path -Path $InstallDir)) {
        New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
        Write-Info "Created install directory: $InstallDir"
    }
} catch {
    Write-Err ("Failed to ensure InstallDir exists: {0}" -f $_.Exception.Message)
    exit 2
}

# Determine architecture if not specified
if (-not $Arch) {
    $procArch = $env:PROCESSOR_ARCHITECTURE
    switch ($procArch.ToLower()) {
        'amd64' { $Arch = 'x64' }
        'arm64' { $Arch = 'arm64' }
        default {
            # fallback to x64
            Write-Warn "Unknown PROCESSOR_ARCHITECTURE='$procArch' - defaulting to x64"
            $Arch = 'x64'
        }
    }
}
Write-Info "Target architecture: $Arch"

# First prefer bundled DLLs shipped alongside the installer/script
$bundledCandidates = Get-ChildItem -Path $ScriptDir -Filter "*libsodium*.dll" -Recurse -ErrorAction SilentlyContinue | Where-Object { $_.Name -match 'libsodium' }
if ($bundledCandidates -and $bundledCandidates.Count -gt 0) {
    Write-Info "Found bundled libsodium DLL(s); copying to $InstallDir"
    foreach ($f in $bundledCandidates) {
        try {
            Copy-Item -Path $f.FullName -Destination $InstallDir -Force
            Write-Info "Copied $($f.Name) -> $InstallDir"
        } catch {
            Write-Warn ("Failed to copy {0}: {1}" -f $f.FullName, $_.Exception.Message)
        }
    }
    Write-Info "Bundled copy complete"
    exit 0
}

if ($SkipDownload) {
    Write-Warn "SkipDownload set and no bundled DLLs found. Exiting with failure."
    exit 3
}

# Otherwise download libsodium MSVC ZIP from official source.
# Use a known stable release as fallback. You can update URL/version if desired.
$libsodiumVersion = 'libsodium-1.0.20-stable-msvc'  # conservative known-good release
$zipName = "$libsodiumVersion.zip"
$downloadUrl = "https://download.libsodium.org/libsodium/releases/$zipName"

$temp = [System.IO.Path]::GetTempPath()
$zipPath = Join-Path $temp $zipName
$extractDir = Join-Path $temp ([System.IO.Path]::GetRandomFileName())

Write-Info "Downloading libsodium from: $downloadUrl"
try {
    Invoke-WebRequest -Uri $downloadUrl -OutFile $zipPath -UseBasicParsing -ErrorAction Stop
    Write-Info "Downloaded to $zipPath"
} catch {
    Write-Warn ("Failed to download from {0}: {1}. Will attempt to continue if local candidates exist." -f $downloadUrl, $_)
}

if (-not (Test-Path $zipPath)) {
    Write-Err "Download failed and no local zip present. Aborting."
    exit 4
}

# Extract
try {
    Expand-Archive -LiteralPath $zipPath -DestinationPath $extractDir -Force
    Write-Info "Extracted libsodium to $extractDir"
} catch {
    Write-Err ("Failed to extract {0}: {1}" -f $zipPath, $_.Exception.Message)
    exit 5
}

# Find suitable DLL(s) for the requested architecture
# Common layouts in the MSVC zip include folders like x64/Release, libsodium\bin\x64 etc. We'll search recursively.
$pattern = 'libsodium*.dll'
# Use a simple recursive dll search and then filter in PowerShell to avoid parser issues with -Include
$allDlls = @(Get-ChildItem -Path $extractDir -Recurse -Filter '*.dll' -File -ErrorAction SilentlyContinue)

# Filter by architecture hints in path or name
$matching = @()
foreach ($d in $allDlls) {
    $pathLower = $d.DirectoryName.ToLower()
    if ($Arch -eq 'x64') {
        if ($pathLower -match 'x64' -or $pathLower -match 'win64' -or $d.Name -match 'x64' -or $d.Name -match 'amd64') {
            $matching += $d
        }
    } elseif ($Arch -eq 'arm64') {
        if ($pathLower -match 'arm64' -or $d.Name -match 'arm64') {
            $matching += $d
        }
    }
}

# If none matched by arch, fall back to DLLs that mention 'libsodium' explicitly
if ($matching.Count -eq 0) {
    foreach ($d in $allDlls) {
        if ($d.Name -match 'libsodium') {
            $matching += $d
        }
    }
}

if ($matching.Count -eq 0) {
    Write-Err "No libsodium DLLs found for architecture '$Arch' in extracted archive. Search path: $extractDir"
    # clean up
    Remove-Item -Recurse -Force $extractDir -ErrorAction SilentlyContinue
    exit 6
}

# Copy the selected DLL(s) to InstallDir
$copied = 0
foreach ($f in ($matching | Select-Object -Unique)) {
    try {
        Copy-Item -Path $f.FullName -Destination $InstallDir -Force
        Write-Info "Copied $($f.FullName) -> $InstallDir"
        $copied++
    } catch {
        Write-Warn ("Failed to copy {0}: {1}" -f $f.FullName, $_.Exception.Message)
    }
}

# Cleanup extracted content & zip
try { Remove-Item -Recurse -Force $extractDir -ErrorAction SilentlyContinue } catch {}
try { Remove-Item -Force $zipPath -ErrorAction SilentlyContinue } catch {}

if ($copied -gt 0) {
    Write-Info "libsodium installation complete (copied $copied file(s))."
    exit 0
} else {
    Write-Err "Failed to copy any libsodium DLLs to $InstallDir"
    exit 7
}
