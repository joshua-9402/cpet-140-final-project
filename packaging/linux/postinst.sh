#!/bin/bash
# StructuraCost Post-Installation Script for Linux
# Automatically downloads and installs libsodium if not present

set -e

INSTALL_PREFIX="/opt/structuracost"
LIBSODIUM_VERSION="1.0.20"
LIBSODIUM_URL="https://download.libsodium.org/libsodium/releases/libsodium-${LIBSODIUM_VERSION}.tar.gz"
BUILD_DIR="/tmp/libsodium-build-$$"

echo "StructuraCost: Checking for libsodium..."

# Function to check if libsodium is installed
check_libsodium() {
    # Check if libsodium is in standard system paths
    if ldconfig -p | grep -q libsodium.so; then
        echo "✓ libsodium found in system libraries"
        return 0
    fi

    # Check bundled library
    if [ -f "${INSTALL_PREFIX}/lib/libsodium.so" ]; then
        echo "✓ libsodium found in bundled libraries"
        return 0
    fi

    return 1
}

# Function to install libsodium from package manager
install_from_package_manager() {
    echo "Attempting to install libsodium from package manager..."

    # Detect package manager and install
    if command -v apt-get &> /dev/null; then
        echo "Using apt-get..."
        apt-get update -qq
        apt-get install -y libsodium-dev libsodium23 2>/dev/null || apt-get install -y libsodium-dev
        return $?
    elif command -v dnf &> /dev/null; then
        echo "Using dnf..."
        dnf install -y libsodium libsodium-devel
        return $?
    elif command -v yum &> /dev/null; then
        echo "Using yum..."
        yum install -y libsodium libsodium-devel
        return $?
    elif command -v zypper &> /dev/null; then
        echo "Using zypper..."
        zypper install -y libsodium libsodium-devel
        return $?
    elif command -v pacman &> /dev/null; then
        echo "Using pacman..."
        pacman -S --noconfirm libsodium
        return $?
    fi

    return 1
}

# Function to build libsodium from source
build_from_source() {
    echo "Building libsodium from source..."

    # Check for required build tools
    if ! command -v gcc &> /dev/null || ! command -v make &> /dev/null; then
        echo "ERROR: gcc and make are required to build libsodium"
        echo "Please install build-essential (Debian/Ubuntu) or Development Tools (RedHat/Fedora)"
        return 1
    fi

    # Create build directory
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"

    # Download libsodium
    echo "Downloading libsodium ${LIBSODIUM_VERSION}..."
    if command -v wget &> /dev/null; then
        wget -q "${LIBSODIUM_URL}" -O libsodium.tar.gz
    elif command -v curl &> /dev/null; then
        curl -sL "${LIBSODIUM_URL}" -o libsodium.tar.gz
    else
        echo "ERROR: wget or curl is required to download libsodium"
        return 1
    fi

    # Extract
    echo "Extracting..."
    tar xzf libsodium.tar.gz
    cd "libsodium-${LIBSODIUM_VERSION}"

    # Configure, build, and install to bundled location
    echo "Configuring..."
    ./configure --prefix="${INSTALL_PREFIX}" --libdir="${INSTALL_PREFIX}/lib" --quiet

    echo "Building... (this may take a few minutes)"
    make -j$(nproc) > /dev/null 2>&1

    echo "Installing to ${INSTALL_PREFIX}..."
    make install > /dev/null 2>&1

    # Update library cache
    if [ -f "${INSTALL_PREFIX}/lib/libsodium.so" ]; then
        echo "${INSTALL_PREFIX}/lib" > /etc/ld.so.conf.d/structuracost.conf
        ldconfig
    fi

    # Cleanup
    cd /
    rm -rf "${BUILD_DIR}"

    echo "✓ libsodium built and installed successfully"
    return 0
}

# Main installation logic
if check_libsodium; then
    echo "libsodium is already available - skipping installation"
else
    echo "libsodium not found - installing..."

    # Try package manager first (faster and cleaner)
    if install_from_package_manager; then
        echo "✓ libsodium installed from package manager"
    else
        echo "Package manager installation failed - building from source..."
        if build_from_source; then
            echo "✓ libsodium built from source successfully"
        else
            echo ""
            echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
            echo "WARNING: Failed to install libsodium automatically"
            echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
            echo ""
            echo "StructuraCost requires libsodium to run. Please install it manually:"
            echo ""
            echo "  Debian/Ubuntu:  sudo apt-get install libsodium23"
            echo "  Fedora/RHEL:    sudo dnf install libsodium"
            echo "  Arch Linux:     sudo pacman -S libsodium"
            echo ""
            echo "Or build from source:"
            echo "  wget https://download.libsodium.org/libsodium/releases/libsodium-${LIBSODIUM_VERSION}.tar.gz"
            echo "  tar xzf libsodium-${LIBSODIUM_VERSION}.tar.gz"
            echo "  cd libsodium-${LIBSODIUM_VERSION}"
            echo "  ./configure && make && sudo make install"
            echo ""
            exit 1
        fi
    fi
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✓ StructuraCost installation complete!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "To launch: structuracost"
echo ""

exit 0

