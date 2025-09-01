#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "🚀 Building Cortan Orchestrator..."

# Create build directory
mkdir -p "${PROJECT_ROOT}/build"
cd "${PROJECT_ROOT}/build"

# Install dependencies with Conan
if command -v conan &> /dev/null; then
    echo "📦 Installing dependencies with Conan..."
    conan install .. --build=missing -s build_type=Release
fi

# Configure with CMake
echo "⚙️  Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_CONAN=ON

# Build
echo "🔨 Building..."
cmake --build . --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "✅ Build complete!"
echo "Run './build/cortan' to start the orchestrator"
