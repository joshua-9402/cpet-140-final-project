# StructuraCost - Windows libsodium Auto-Installer
# PowerShell script to download and install libsodium

param(
    [string]$InstallDir = "$env:ProgramFiles\StructuraCost"
)

$ErrorActionPreference = "Stop"
$ProgressPreference = 'SilentlyContinue'

Write-Host "StructuraCost: Checking for libsodium..." -ForegroundColor Cyan

# Function to check if libsodium.dll exists
function Test-Libsodium {
    # Check bundled DLL
    if (Test-Path "$InstallDir\libsodium.dll") {
        Write-Host "✓ libsodium found in installation directory" -ForegroundColor Green
        return $true
    }

    # Check system PATH
    $found = Get-Command libsodium.dll -ErrorAction SilentlyContinue
    if ($found) {
        Write-Host "✓ libsodium found in system PATH" -ForegroundColor Green
        return $true
    }

    return $false
}

# Function to download and install libsodium
function Install-Libsodium {
    Write-Host "libsodium not found - downloading..." -ForegroundColor Yellow

    # Detect architecture
    $arch = if ([Environment]::Is64BitOperatingSystem) { "x64" } else { "x86" }
    Write-Host "Detected architecture: $arch"

    $version = "1.0.20"
    $url = "https://download.libsodium.org/libsodium/releases/libsodium-$version-stable-msvc.zip"
    $tempDir = Join-Path $env:TEMP "libsodium_install_$(Get-Random)"
    $zipFile = Join-Path $tempDir "libsodium.zip"

    try {
        # Create temp directory
        New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

        # Download
        Write-Host "Downloading libsodium $version from official repository..."
        Invoke-WebRequest -Uri $url -OutFile $zipFile -UseBasicParsing

        # Extract
        Write-Host "Extracting..."
        Expand-Archive -Path $zipFile -DestinationPath $tempDir -Force

        # Find the correct DLL path
        $dllPath = if ($arch -eq "x64") {
            Get-ChildItem -Path $tempDir -Filter "libsodium.dll" -Recurse |
                Where-Object { $_.FullName -like "*x64*Release*dynamic*" } |
                Select-Object -First 1
        } else {
            Get-ChildItem -Path $tempDir -Filter "libsodium.dll" -Recurse |
                Where-Object { $_.FullName -like "*Win32*Release*dynamic*" } |
                Select-Object -First 1
        }

        if (-not $dllPath) {
            # Fallback: try to find any libsodium.dll
            $dllPath = Get-ChildItem -Path $tempDir -Filter "libsodium.dll" -Recurse |
                Select-Object -First 1
        }

        if ($dllPath) {
            # Copy to installation directory
            Write-Host "Installing to $InstallDir..."
            Copy-Item -Path $dllPath.FullName -Destination "$InstallDir\libsodium.dll" -Force
            Write-Host "✓ libsodium installed successfully" -ForegroundColor Green
        } else {
            throw "Could not find libsodium.dll in downloaded archive"
        }

    } catch {
        Write-Host ""
        Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Red
        Write-Host "WARNING: Failed to install libsodium automatically" -ForegroundColor Red
        Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Red
        Write-Host ""
        Write-Host "Error: $_" -ForegroundColor Red
        Write-Host ""
        Write-Host "Please download libsodium manually from:"
        Write-Host "https://download.libsodium.org/libsodium/releases/"
        Write-Host ""
        Write-Host "Extract and copy libsodium.dll to:"
        Write-Host "$InstallDir"
        Write-Host ""
        exit 1
    } finally {
        # Cleanup
        if (Test-Path $tempDir) {
            Remove-Item -Path $tempDir -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
}

# Main logic
if (-not (Test-Libsodium)) {
    Install-Libsodium
} else {
    Write-Host "libsodium is already available - skipping installation" -ForegroundColor Green
}

Write-Host ""
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "✓ StructuraCost installation complete!" -ForegroundColor Green
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

exit 0

