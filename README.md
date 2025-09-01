# 🤖 Cortan Orchestrator

**AI-Powered Multi-Model Orchestration Engine**

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![CMake](https://img.shields.io/badge/CMake-3.20+-green.svg)](https://cmake.org/)

---

## 📋 Overview

**Cortan Orchestrator** is a AI orchestration platform designed to seamlessly coordinate multiple AI models and services. Built with modern C++20, it provides a robust framework for AI model management, workflow coordination, and real-time processing capabilities.

This is a **prototype** implementation showcasing advanced AI orchestration patterns and modern C++ development practices, currently the cmake is being set up.

## 🎯 Key Features

- **🔄 Multi-Model Coordination**: Orchestrate multiple AI models simultaneously
- **⚡ High-Performance**: Built with C++20 coroutines and async I/O
- **🌐 Network Integration**: RESTful APIs and WebSocket support
- **🔧 Modular Architecture**: Clean separation of concerns with pluggable components
- **📊 Real-time Monitoring**: Built-in performance metrics and logging
- **🔒 Security-First**: Input validation and security management
- **🧪 Testing Framework**: Comprehensive unit testing with Google Test
- **📈 Benchmarking**: Performance analysis with Google Benchmark

## 🏗️ Architecture

```
Cortan Orchestrator
├── Core Engine          # Event system, workflow management
├── AI Layer            # Model management, conversation handling
├── Network Layer       # HTTP/WebSocket clients, connection pooling
├── Terminal Interface  # Interactive shell, command processing
└── Security Layer      # Input validation, access control
```

## 🚀 Quick Start

### Prerequisites

- **macOS**: 12.0+ (tested on macOS 14 Sonoma)
- **CMake**: 3.20+
- **C++ Compiler**: Apple Clang 14+ or GCC 11+
- **Conan**: 2.0+ (recommended for dependencies)

### Installation

```bash
# Clone the repository
git clone <repository-url>
cd cortan

# Build with Conan (recommended)
./cmake/build.sh

# Or build with system packages
./cmake/build.sh --no-conan
```

### Usage

```bash
# Run the orchestrator
./build/cortan

# Run with specific command
./build/cortan "process-model llama3:8b"

# Debug mode
./cmake/build.sh --debug
./build/cortan
```

## 📁 Project Structure

```
cortan/
├── cmake/                    # Build configuration
│   ├── CMakeLists.txt       # CMake build files
│   ├── conanfile.py         # Conan package management
│   └── build.sh             # Build automation script
├── src/                     # Source code
│   ├── main.cpp            # Application entry point
│   ├── core/               # Core orchestration components
│   ├── ai/                 # AI model management
│   ├── network/            # Network communication
│   └── terminal/           # User interface
├── include/cortan/         # Public API headers
├── tests/                  # Unit tests
├── benchmarks/             # Performance benchmarks
├── config/                 # Configuration files
└── scripts/                # Utility scripts
```

## 🛠️ Build System

### Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `--no-conan` | Use system packages instead of Conan | `false` |
| `--debug` | Build in Debug mode | `false` |
| `--no-ai` | Disable AI orchestration features | `false` |
| `--no-tests` | Don't build unit tests | `false` |
| `--no-benchmarks` | Don't build benchmarks | `false` |

### Dependencies

#### Conan Dependencies (Automatic)
- **nlohmann_json/3.11.3** - JSON processing
- **spdlog/1.12.0** - High-performance logging
- **boost/1.82.0** - ASIO networking
- **libcurl/8.4.0** - HTTP client
- **openssl/3.1.3** - SSL/TLS support

#### System Dependencies (Manual)
```bash
brew install cmake nlohmann-json spdlog curl boost
```

## 🔧 Development

### Code Style

The project follows modern C++ best practices:
- C++20 features with coroutines
- RAII resource management
- Exception safety
- Async programming patterns

### Testing

```bash
# Build with tests
./cmake/build.sh --tests

# Run test suite
cd build && make test_quick
```

### Benchmarking

```bash
# Build with benchmarks
./cmake/build.sh --benchmarks

# Run performance tests
cd build && make perf_check
```

## 📚 API Documentation

### Core Components

#### Event System
```cpp
#include <cortan/core/event_system.hpp>

cortan::core::EventBus bus;
bus.subscribe("ai.request", handler);
```

#### Model Manager
```cpp
#include <cortan/ai/model_manager.hpp>

cortan::ai::ModelManager manager;
manager.addModel(std::make_unique<OllamaModel>("llama3"));
```

#### HTTP Client
```cpp
#include <cortan/network/http_client.hpp>

cortan::network::HttpClient client;
auto response = client.get("https://api.example.com");
```

## 🔒 Security

- Input validation for all AI model interactions
- Rate limiting and access control
- Secure communication with TLS/SSL
- Audit logging for all operations

## 📊 Performance

- **Concurrent Processing**: Multi-threaded architecture
- **Memory Efficient**: Custom allocators and pooling
- **Network Optimized**: Connection pooling and keep-alive
- **Async I/O**: Non-blocking operations with Boost.ASIO

## 🤝 Contributing

We welcome contributions! Please see our contributing guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Write tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

### Development Setup

```bash
# Clone and setup
git clone <repository-url>
cd cortan

# Install development dependencies
./cmake/build.sh --debug --tests --benchmarks

# Run development server
./build/cortan --dev-mode
```

## 📄 License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**.

```
usCopyright (C) 2025 Space Labs AI
Copyright (C) 2025 Rishab Nuguru

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```

## 📚 Citation

If you use Cortan Orchestrator in your research or project, please cite it as follows:

### BibTeX
```bibtex
@software{nuguru_cortan_orchestrator_2025,
  author       = {Nuguru, Rishab},
  title        = {Cortan Orchestrator: AI-Powered Multi-Model Orchestration Engine},
  year         = 2025,
  publisher    = {Space Labs AI},
  url          = {https://github.com/rishabnuguru/cortan-orchestrator},
  license      = {AGPL-3.0},
  abstract     = {Cortan Orchestrator is a prototype implementation of a modern AI orchestration platform built with C++20, featuring coroutine-based architecture, async I/O, and modular design for coordinating multiple AI models and services.}
}
```

### APA Style
```
Nuguru, R. (2025). Cortan Orchestrator: AI-Powered Multi-Model Orchestration Engine [Computer software]. Space Labs AI. https://github.com/rishabnuguru/cortan-orchestrator
```

### Citation File Format (CFF)
A `CITATION.cff` file is included in the repository root for easy citation import into citation management tools.

## 👥 Authors & Acknowledgments

**Author**: Rishab Nuguru
**Company**: Space Labs AI

### Acknowledgments

- Built with modern C++20 features and best practices
- Inspired by production AI orchestration systems
- Thanks to the open-source community for amazing libraries

## 📞 Contact

**Space Labs AI**
- **Author**: Rishab Nuguru
- **Email**: [spacelabsai@gmail.com]
- **Website**: [company website]

## 🔄 Version History

### v0.0.1 (2025)
- Initial prototype release
- Multi-model orchestration framework
- C++20 coroutine support
- Async I/O with Boost.ASIO
- Comprehensive testing framework
- macOS optimization with Apple Clang
- Conan dependency management
- Modular architecture with clean separation

---

**⚠️ This is a prototype implementation for demonstration and development purposes.**

*Made with ❤️ by Space Labs AI* 🚀