#!/bin/bash
# macOS Build Script for DoomRPG-RE (Apple Silicon M1/M2/M3)

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${GREEN}================================${NC}"
    echo -e "${GREEN}$1${NC}"
    echo -e "${GREEN}================================${NC}"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check if running on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    print_error "This script is for macOS only!"
    print_info "For Linux builds, use the Dockerfile or native build"
    exit 1
fi

# Check if running on Apple Silicon
ARCH=$(uname -m)
if [[ "$ARCH" != "arm64" ]]; then
    print_warning "This script is optimized for Apple Silicon (M1/M2/M3)"
    print_info "Detected architecture: $ARCH"
    print_info "Build will continue but may not be optimized"
fi

print_header "DoomRPG-RE macOS Build Script"
echo "Architecture: $ARCH"
echo "macOS Version: $(sw_vers -productVersion)"
echo ""

# Function to check if Homebrew is installed
check_homebrew() {
    if ! command -v brew &> /dev/null; then
        print_error "Homebrew not found!"
        print_info "Install Homebrew from: https://brew.sh"
        print_info "Run: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi
    print_success "Homebrew found: $(brew --version | head -1)"
}

# Function to install dependencies
install_dependencies() {
    print_header "Installing Dependencies"

    local packages=("cmake" "sdl2" "sdl2_mixer" "fluid-synth" "pkg-config")
    local missing_packages=()

    # Check which packages are missing
    for package in "${packages[@]}"; do
        if ! brew list "$package" &>/dev/null; then
            missing_packages+=("$package")
        else
            print_info "$package is already installed"
        fi
    done

    # Install missing packages
    if [ ${#missing_packages[@]} -eq 0 ]; then
        print_success "All dependencies already installed!"
    else
        print_info "Installing missing packages: ${missing_packages[*]}"
        brew install "${missing_packages[@]}"
        print_success "Dependencies installed successfully!"
    fi

    # Show installed versions
    echo ""
    print_info "Installed versions:"
    cmake --version | head -1
    pkg-config --version
    brew list --versions sdl2 sdl2_mixer fluid-synth
}

# Function to build the project
build_project() {
    print_header "Building DoomRPG-RE"

    # Create build directory
    if [ -d "build" ]; then
        print_warning "Build directory exists. Cleaning..."
        rm -rf build
    fi
    mkdir -p build
    cd build

    print_info "Running CMake configuration..."
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES=arm64 \
        -DCMAKE_PREFIX_PATH="$(brew --prefix)" \
        -DSDL2_INCLUDE_DIR="$(brew --prefix sdl2)/include/SDL2" \
        -DSDL2_LIBRARY="$(brew --prefix sdl2)/lib/libSDL2.dylib" \
        -DSDL2_MIXER_INCLUDE_DIR="$(brew --prefix sdl2_mixer)/include/SDL2" \
        -DSDL2_MIXER_LIBRARY="$(brew --prefix sdl2_mixer)/lib/libSDL2_mixer.dylib"

    print_info "Compiling (using $(sysctl -n hw.ncpu) cores)..."
    cmake --build . --config Release -j$(sysctl -n hw.ncpu)

    cd ..

    if [ -f "build/src/DoomRPG" ]; then
        print_success "Build completed successfully!"
        echo ""
        print_info "Binary location: $(pwd)/build/src/DoomRPG"
        print_info "Binary size: $(du -h build/src/DoomRPG | cut -f1)"
        print_info "Architecture: $(file build/src/DoomRPG | cut -d: -f2)"

        # Check dependencies
        echo ""
        print_info "Dynamic library dependencies:"
        otool -L build/src/DoomRPG | grep -v "$(pwd)" | tail -n +2
    else
        print_error "Build failed! Binary not found."
        exit 1
    fi
}

# Function to clean build artifacts
clean() {
    print_header "Cleaning Build Artifacts"
    if [ -d "build" ]; then
        rm -rf build
        print_success "Build directory removed"
    fi
    if [ -d "build-output" ]; then
        rm -rf build-output
        print_success "Build output directory removed"
    fi
    print_success "Clean complete!"
}

# Function to show help
show_help() {
    cat << EOF
DoomRPG-RE macOS Build Script (Apple Silicon)

Usage: $0 [COMMAND]

Commands:
    deps        Install dependencies via Homebrew
    build       Build the project
    clean       Remove build artifacts
    rebuild     Clean and rebuild
    all         Install deps, build, and create bundle
    help        Show this help message

Examples:
    $0 all        # Complete setup: deps + build + bundle
    $0 build      # Just build the project
    # $0 run        # Build and run the game
    # $0 bundle     # Create .app bundle

Requirements:
    - macOS 11.0 or later (Big Sur+)
    - Homebrew package manager
    - Apple Silicon (M1/M2/M3) recommended
    - Xcode Command Line Tools

Installation:
    # Install Homebrew (if not installed)
    /bin/bash -c "\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

    # Install Xcode Command Line Tools
    xcode-select --install

EOF
}

# Main script logic
case "${1:-build}" in
    deps|dependencies)
        check_homebrew
        install_dependencies
        ;;
    build)
        check_homebrew
        install_dependencies
        build_project
        ;;
    clean)
        clean
        ;;
    rebuild)
        clean
        check_homebrew
        install_dependencies
        build_project
        ;;
    all)
        check_homebrew
        install_dependencies
        build_project
        # create_app_bundle
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        print_error "Unknown command: ${1}"
        echo ""
        show_help
        exit 1
        ;;
esac
