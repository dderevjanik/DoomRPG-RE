#!/bin/bash
# Web Build Script for DoomRPG-RE (Emscripten/WebAssembly)

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

# Check if Emscripten is installed
check_emscripten() {
    if ! command -v emcc &> /dev/null; then
        print_error "Emscripten not found!"
        print_info "Install Emscripten SDK from: https://emscripten.org/docs/getting_started/downloads.html"
        echo ""
        print_info "Quick install:"
        echo "  git clone https://github.com/emscripten-core/emsdk.git"
        echo "  cd emsdk"
        echo "  ./emsdk install latest"
        echo "  ./emsdk activate latest"
        echo "  source ./emsdk_env.sh"
        exit 1
    fi
    print_success "Emscripten found: $(emcc --version | head -1)"
}

# Function to build the project
build_project() {
    print_header "Building DoomRPG-RE for Web (WASM)"

    # Create build directory
    if [ -d "build-web" ]; then
        print_warning "Build directory exists. Cleaning..."
        rm -rf build-web
    fi
    mkdir -p build-web
    cd build-web

    print_info "Running CMake configuration with Emscripten..."
    emcmake cmake .. \
        -DCMAKE_BUILD_TYPE=Release

    print_info "Compiling to WebAssembly..."
    emmake make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

    cd ..

    if [ -f "build-web/src/DoomRPG.html" ]; then
        print_success "Build completed successfully!"
        echo ""
        print_info "Output files:"
        ls -lh build-web/src/DoomRPG.*
        echo ""
        print_info "To run locally, start a web server:"
        echo "  cd build-web/src"
        echo "  python3 -m http.server 8000"
        echo "  # Then open http://localhost:8000/DoomRPG.html"
    else
        print_error "Build failed! Output files not found."
        exit 1
    fi
}

# Function to run local web server
serve() {
    if [ ! -f "build-web/src/DoomRPG.html" ]; then
        print_error "Build not found! Build the project first."
        exit 1
    fi

    print_header "Starting Local Web Server"
    print_info "Open http://localhost:8000/DoomRPG.html in your browser"
    print_info "Press Ctrl+C to stop the server"
    echo ""
    cd build-web/src
    python3 -m http.server 8000
}

# Function to clean build artifacts
clean() {
    print_header "Cleaning Build Artifacts"
    if [ -d "build-web" ]; then
        rm -rf build-web
        print_success "Build directory removed"
    fi
    print_success "Clean complete!"
}

# Function to show help
show_help() {
    cat << EOF
DoomRPG-RE Web Build Script (Emscripten/WebAssembly)

Usage: $0 [COMMAND]

Commands:
    build       Build the project for web browsers
    serve       Start a local web server to test the build
    clean       Remove build artifacts
    rebuild     Clean and rebuild
    help        Show this help message

Examples:
    $0 build      # Build for web
    $0 serve      # Start web server
    $0 rebuild    # Clean and rebuild

Requirements:
    - Emscripten SDK (latest version)
    - CMake 3.25+
    - Python 3 (for local web server)

Installation:
    # Install Emscripten SDK
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh

EOF
}

# Main script logic
case "${1:-build}" in
    build)
        check_emscripten
        build_project
        ;;
    serve|run)
        serve
        ;;
    clean)
        clean
        ;;
    rebuild)
        clean
        check_emscripten
        build_project
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
