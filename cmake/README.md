# Cortan Orchestrator - CMake Build System

This directory contains the CMake build configuration for the Cortan Orchestrator project.

## Quick Start

### Prerequisites

1. **CMake 3.20+**
   ```bash
   brew install cmake
   ```

2. **Conan (recommended)**
   ```bash
   pip install conan
   conan profile detect
   ```

3. **Alternative: System packages**
   ```bash
   brew install nlohmann-json spdlog curl boost googletest google-benchmark
   ```

### Build with Conan (Recommended)

```bash
# Full release build with all features
./cmake/build.sh

# Debug build
./cmake/build.sh --debug

# Build without AI features
./cmake/build.sh --no-ai

# Build without tests and benchmarks
./cmake/build.sh --no-tests --no-benchmarks
```

### Build with System Packages

```bash
# Build without Conan
./cmake/build.sh --no-conan

# Debug build without Conan
./cmake/build.sh --no-conan --debug
```

## Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `--no-conan` | Use system packages instead of Conan | `false` |
| `--debug` | Build in Debug mode | `false` (Release) |
| `--no-ai` | Disable AI orchestration features | `false` |
| `--no-tests` | Don't build unit tests | `false` |
| `--no-benchmarks` | Don't build benchmarks | `false` |
| `--clean` | Clean build directory first | `false` |
| `--verbose` | Enable verbose CMake output | `false` |

## Project Structure

After building, you'll find:

```
build/
├── cortan                    # Main executable
├── cortan_tests             # Test executable (if enabled)
├── cortan_benchmarks        # Benchmark executable (if enabled)
├── compile_commands.json    # For IDE integration
└── CMakeCache.txt          # CMake cache
```

## Development Targets

### Testing
```bash
cd build
make test_quick          # Run all tests
```

### Benchmarking
```bash
cd build
make perf_check          # Run performance benchmarks
```

### Code Quality
```bash
cd build
make format             # Format code with clang-format
make analyze            # Run static analysis with clang-tidy
```

## Dependencies

### Conan Dependencies (automatic)
- **nlohmann_json/3.11.3** - JSON processing
- **spdlog/1.12.0** - Logging
- **libcurl/8.4.0** - HTTP client
- **boost/1.82.0** - ASIO for async I/O
- **gtest/1.14.0** - Testing framework
- **benchmark/1.8.3** - Benchmarking framework
- **openssl/3.1.3** - SSL/TLS support
- **jemalloc/5.3.0** - High-performance allocator

### System Dependencies (manual install)
- **nlohmann-json** - JSON processing
- **spdlog** - Logging
- **curl** - HTTP client
- **boost** - ASIO for async I/O
- **googletest** - Testing framework (optional)
- **google-benchmark** - Benchmarking framework (optional)

## Configuration

The build system is highly configurable through CMake options:

- `ENABLE_AI_FEATURES` - Enable AI orchestration (default: ON)
- `BUILD_TESTS` - Build unit tests (default: ON)
- `BUILD_BENCHMARKS` - Build benchmarks (default: ON)
- `USE_CONAN` - Use Conan for dependencies (default: ON)
- `CMAKE_BUILD_TYPE` - Build type: Debug/Release (default: Release)

## macOS-Specific Optimizations

This build system includes macOS-specific optimizations:

- **Architecture**: Optimized for x86_64
- **Compiler**: Uses clang with Apple-specific flags
- **Parallel Builds**: Automatically uses all 8 CPU cores
- **Memory**: Optimized for 8GB RAM systems

## Troubleshooting

### Conan Issues
```bash
# Clear Conan cache
conan remove "*" -c

# Update Conan
pip install --upgrade conan

# Detect profile again
conan profile detect
```

### CMake Issues
```bash
# Clean rebuild
./cmake/build.sh --clean

# Verbose output
./cmake/build.sh --verbose
```

### Missing Dependencies
```bash
# Update Homebrew
brew update

# Install missing packages
brew install cmake nlohmann-json spdlog curl boost
```

## IDE Integration

### CLion/CLion
- Import as CMake project
- Build directory: `build/`
- Compile commands: `build/compile_commands.json`

### VS Code
- Install "CMake Tools" extension
- Configure with `cmake.configureSettings` in `.vscode/settings.json`

### Xcode
```bash
# Generate Xcode project
cd build
cmake .. -G Xcode
```

## Contributing

1. Ensure all tests pass: `make test_quick`
2. Run benchmarks: `make perf_check`
3. Format code: `make format`
4. Run static analysis: `make analyze`

## License

This build system is part of the Cortan Orchestrator project.
