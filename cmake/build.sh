#!/bin/bash

# Cortan Orchestrator Build Script
# Supports both Conan and direct CMake builds

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
USE_CONAN=true
BUILD_TYPE="Release"
ENABLE_AI=true
BUILD_TESTS=true
BUILD_BENCHMARKS=true
CLEAN_BUILD=false
VERBOSE=false

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

# Function to show usage
usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --no-conan           Build without Conan (use system packages)"
    echo "  --debug              Build in Debug mode"
    echo "  --no-ai              Disable AI features"
    echo "  --no-tests           Don't build tests"
    echo "  --no-benchmarks      Don't build benchmarks"
    echo "  --clean              Clean build directory before building"
    echo "  --verbose            Enable verbose output"
    echo "  --help               Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                    # Full build with Conan (default)"
    echo "  $0 --debug --no-ai    # Debug build without AI features"
    echo "  $0 --no-conan         # Build with system packages only"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --no-conan)
            USE_CONAN=false
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --no-ai)
            ENABLE_AI=false
            shift
            ;;
        --no-tests)
            BUILD_TESTS=false
            shift
            ;;
        --no-benchmarks)
            BUILD_BENCHMARKS=false
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

print_header "Cortan Orchestrator Build Script"
print_status "Build Type: $BUILD_TYPE"
print_status "Use Conan: $USE_CONAN"
print_status "AI Features: $ENABLE_AI"
print_status "Build Tests: $BUILD_TESTS"
print_status "Build Benchmarks: $BUILD_BENCHMARKS"
print_status "Project Root: $PROJECT_ROOT"

# Check if we're in the right directory
if [[ ! -f "$PROJECT_ROOT/cmake/CMakeLists.txt" ]]; then
    print_error "CMakeLists.txt not found in $PROJECT_ROOT/cmake/"
    print_error "Please run this script from the project root or ensure CMakeLists.txt exists."
    exit 1
fi

# Create build directory
BUILD_DIR="$PROJECT_ROOT/build"
if [[ "$CLEAN_BUILD" == true ]]; then
    print_status "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Set CMake arguments
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DENABLE_AI_FEATURES="$ENABLE_AI"
    -DBUILD_TESTS="$BUILD_TESTS"
    -DBUILD_BENCHMARKS="$BUILD_BENCHMARKS"
    -DUSE_CONAN="$USE_CONAN"
)

if [[ "$VERBOSE" == true ]]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

if [[ "$USE_CONAN" == true ]]; then
    print_header "Building with Conan"

    # Check if Conan is installed
    if ! command -v conan &> /dev/null; then
        print_error "Conan is not installed. Please install it first:"
        echo "  pip install conan"
        echo "  conan profile detect"
        exit 1
    fi

    print_status "Installing dependencies with Conan..."
    conan install "$SCRIPT_DIR" \
        --build=missing \
        --settings=build_type="$BUILD_TYPE"

    print_status "Configuring with CMake..."
    cmake "$PROJECT_ROOT" "${CMAKE_ARGS[@]}"

else
    print_header "Building with System Packages"
    print_warning "Make sure you have the required dependencies installed:"
    echo "  - nlohmann_json"
    echo "  - spdlog"
    echo "  - libcurl"
    echo "  - boost (for ASIO)"
    echo "  - googletest (optional)"
    echo "  - google-benchmark (optional)"

    print_status "Configuring with CMake..."
    cmake "$PROJECT_ROOT" "${CMAKE_ARGS[@]}"
fi

print_status "Building project..."
make -j$(sysctl -n hw.ncpu)

print_header "Build Complete!"
print_status "Binary location: $BUILD_DIR/cortan"

if [[ "$BUILD_TESTS" == true ]]; then
    print_status "Run tests with: cd $BUILD_DIR && make test_quick"
fi

if [[ "$BUILD_BENCHMARKS" == true ]]; then
    print_status "Run benchmarks with: cd $BUILD_DIR && make perf_check"
fi

print_status "To install: cd $BUILD_DIR && sudo make install"
