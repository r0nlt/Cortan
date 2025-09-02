cmake_minimum_required(VERSION 3.20)

# ===============================
# Project Setup
# ===============================
project(CortanOrchestrator
    VERSION 0.0.1
    DESCRIPTION "AI Orchestrational Engine with Multi-Model Coordination"
    LANGUAGES CXX
)

# ===============================
# Modern C++ Configuration
# ===============================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ===============================
# Build Configuration
# ===============================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# ===============================
# Compiler Setup
# ===============================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wconversion -Wsign-conversion
        -Wunused -Wuninitialized
        -Woverloaded-virtual
    )

    # Enable coroutines
    add_compile_options(-fcoroutines)

    # Debug/Release specific flags
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-O0 -g3)
        add_compile_definitions(DEBUG=1)
    else()
        add_compile_options(-O3 -DNDEBUG -march=native)
        add_compile_definitions(NDEBUG=1)
    endif()

elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /permissive-)
    add_compile_definitions(_WIN32_WINNT=0x0A00)

    # Enable coroutines for MSVC
    add_compile_options(/await:strict)

    # Parallel compilation
    add_compile_options(/MP)
endif()

# ===============================
# Options
# ===============================
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(ENABLE_AI_FEATURES "Enable AI orchestration" ON)
option(USE_CONAN "Use Conan for dependencies" ON)

# ===============================
# Directory Structure
# ===============================
set(CORTAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CORTAN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ===============================
# Conan Integration
# ===============================
if(USE_CONAN)
    if(EXISTS "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
        include("${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
    endif()

    if(EXISTS "${CMAKE_BINARY_DIR}/conan_deps.cmake")
        include("${CMAKE_BINARY_DIR}/conan_deps.cmake")
    endif()
endif()

# ===============================
# Core Dependencies
# ===============================
find_package(Threads REQUIRED)

# JSON library
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Logging library
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(spdlog)
endif()

# HTTP client library
find_package(CURL REQUIRED)

# Async I/O library
find_package(asio QUIET)
if(NOT asio_FOUND)
    FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-28-0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(asio)

    # Create asio target
    add_library(asio INTERFACE)
    target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
    target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
    add_library(asio::asio ALIAS asio)
endif()

# Testing framework
if(BUILD_TESTS)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(googletest)
    endif()
endif()

# Benchmarking framework
if(BUILD_BENCHMARKS)
    find_package(benchmark QUIET)
    if(NOT benchmark_FOUND)
        FetchContent_Declare(
            googlebenchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.3
            GIT_SHALLOW TRUE
        )
        # Disable Google Benchmark's internal Google Test build to avoid CMake compatibility issues
        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googlebenchmark)
    endif()
endif()

# ===============================
# Core Orchestrator Library
# ===============================
add_library(cortan_core
    # Core orchestration
    src/core/event_system.cpp
    src/core/workflow_engine.cpp
    src/core/resource_manager.cpp
    src/core/thread_pool.cpp

    # Memory management
    src/core/memory_pool.cpp
    src/core/allocator.cpp

    # Utilities
    src/core/logger.cpp
    src/core/config.cpp
)

target_include_directories(cortan_core
    PUBLIC
        $<BUILD_INTERFACE:${CORTAN_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CORTAN_SOURCE_DIR}
)

target_compile_features(cortan_core PUBLIC cxx_std_20)

target_link_libraries(cortan_core
    PUBLIC
        Threads::Threads
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)

# ===============================
# AI Orchestration Library
# ===============================
if(ENABLE_AI_FEATURES)
    add_library(cortan_ai
        # Model management
        src/ai/model_manager.cpp
        src/ai/model_interface.cpp
        src/ai/ollama_client.cpp

        # Conversation handling
        src/ai/conversation_manager.cpp
        src/ai/context_manager.cpp

        # Task coordination
        src/ai/task_dispatcher.cpp
        src/ai/response_aggregator.cpp
        src/ai/workflow_coordinator.cpp

        # Security
        src/ai/input_validator.cpp
        src/ai/security_manager.cpp
    )

    target_include_directories(cortan_ai
        PUBLIC
            $<BUILD_INTERFACE:${CORTAN_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CORTAN_SOURCE_DIR}
    )

    target_compile_features(cortan_ai PUBLIC cxx_std_20)

    target_link_libraries(cortan_ai
        PUBLIC
            cortan_core
        PRIVATE
            CURL::libcurl
    )
endif()

# ===============================
# Network Library
# ===============================
add_library(cortan_network
    src/network/http_client.cpp
    src/network/websocket_client.cpp
    src/network/connection_pool.cpp
    src/network/request_handler.cpp
)

target_include_directories(cortan_network
    PUBLIC
        $<BUILD_INTERFACE:${CORTAN_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CORTAN_SOURCE_DIR}
)

target_compile_features(cortan_network PUBLIC cxx_std_20)

target_link_libraries(cortan_network
    PUBLIC
        cortan_core
        asio::asio
    PRIVATE
        CURL::libcurl
)

# ===============================
# Terminal Interface Library
# ===============================
add_library(cortan_terminal
    src/terminal/command_processor.cpp
    src/terminal/interactive_shell.cpp
    src/terminal/streaming_display.cpp
    src/terminal/history_manager.cpp
    src/terminal/completion_engine.cpp
)

target_include_directories(cortan_terminal
    PUBLIC
        $<BUILD_INTERFACE:${CORTAN_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CORTAN_SOURCE_DIR}
)

target_compile_features(cortan_terminal PUBLIC cxx_std_20)

target_link_libraries(cortan_terminal
    PUBLIC
        cortan_core
        cortan_network
)

if(ENABLE_AI_FEATURES)
    target_link_libraries(cortan_terminal PUBLIC cortan_ai)
endif()

# ===============================
# Main Executable
# ===============================
add_executable(cortan src/main.cpp)

target_link_libraries(cortan
    PRIVATE
        cortan_core
        cortan_network
        cortan_terminal
)

if(ENABLE_AI_FEATURES)
    target_link_libraries(cortan PRIVATE cortan_ai)
    target_compile_definitions(cortan PRIVATE CORTAN_AI_ENABLED=1)
endif()

target_compile_features(cortan PRIVATE cxx_std_20)

# ===============================
# Testing
# ===============================
if(BUILD_TESTS)
    enable_testing()

    add_executable(cortan_tests
        # Core tests
        tests/core/test_event_system.cpp
        # TODO: Create missing test files
        # tests/core/test_workflow_engine.cpp
        # tests/core/test_resource_manager.cpp
        # tests/core/test_thread_pool.cpp
        # tests/core/test_memory_pool.cpp

        # Network tests
        # TODO: Create missing test files
        # tests/network/test_http_client.cpp
        # tests/network/test_connection_pool.cpp

        # Terminal tests
        # TODO: Create missing test files
        # tests/terminal/test_command_processor.cpp
        # tests/terminal/test_interactive_shell.cpp

        # Main test runner
        tests/main.cpp
    )

    if(ENABLE_AI_FEATURES)
        # TODO: Create missing AI test files
        # target_sources(cortan_tests PRIVATE
        #     tests/ai/test_model_manager.cpp
        #     tests/ai/test_ollama_client.cpp
        #     tests/ai/test_conversation_manager.cpp
        #     tests/ai/test_task_dispatcher.cpp
        #     tests/ai/test_security_manager.cpp
        # )
    endif()

    target_link_libraries(cortan_tests
        PRIVATE
            cortan_core
            cortan_network
            cortan_terminal
            GTest::gtest
            GTest::gtest_main
    )

    if(ENABLE_AI_FEATURES)
        target_link_libraries(cortan_tests PRIVATE cortan_ai)
    endif()

    target_compile_features(cortan_tests PRIVATE cxx_std_20)

    # Discover and add tests
    include(GoogleTest)
    gtest_discover_tests(cortan_tests)
endif()

# ===============================
# Benchmarks
# ===============================
if(BUILD_BENCHMARKS)
    add_executable(cortan_benchmarks
        benchmarks/core_benchmarks.cpp
        benchmarks/memory_benchmarks.cpp
        benchmarks/network_benchmarks.cpp
    )

    if(ENABLE_AI_FEATURES)
        target_sources(cortan_benchmarks PRIVATE
            benchmarks/ai_benchmarks.cpp
            benchmarks/model_benchmarks.cpp
        )
    endif()

    target_link_libraries(cortan_benchmarks
        PRIVATE
            cortan_core
            cortan_network
            cortan_terminal
            benchmark::benchmark
            benchmark::benchmark_main
    )

    if(ENABLE_AI_FEATURES)
        target_link_libraries(cortan_benchmarks PRIVATE cortan_ai)
    endif()

    target_compile_features(cortan_benchmarks PRIVATE cxx_std_20)
endif()

# ===============================
# Development Tools
# ===============================

# Format target
find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    add_custom_target(format
        COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/tests -name "*.cpp" -o -name "*.hpp" | xargs ${CLANG_FORMAT_EXE} -i
        COMMENT "Formatting source code"
    )
endif()

# Static analysis target
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if(CLANG_TIDY_EXE)
    add_custom_target(analyze
        COMMAND ${CLANG_TIDY_EXE}
        -p ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp
        COMMENT "Running static analysis"
    )
endif()

# ===============================
# Custom Targets
# ===============================

# Build everything
add_custom_target(all_targets ALL
    DEPENDS cortan
)

if(BUILD_TESTS)
    add_dependencies(all_targets cortan_tests)
endif()

if(BUILD_BENCHMARKS)
    add_dependencies(all_targets cortan_benchmarks)
endif()

# Quick test run
if(BUILD_TESTS)
    add_custom_target(test_quick
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel
        DEPENDS cortan_tests
        COMMENT "Running all tests"
    )
endif()

# Performance check
if(BUILD_BENCHMARKS)
    add_custom_target(perf_check
        COMMAND cortan_benchmarks --benchmark_min_time=0.1
        DEPENDS cortan_benchmarks
        COMMENT "Running quick performance benchmarks"
    )
endif()

# ===============================
# Installation
# ===============================
install(TARGETS cortan
    RUNTIME DESTINATION bin
)

install(TARGETS cortan_core cortan_network cortan_terminal
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

if(ENABLE_AI_FEATURES)
    install(TARGETS cortan_ai
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

install(DIRECTORY ${CORTAN_INCLUDE_DIR}/
    DESTINATION include
)

# ===============================
# Status Summary
# ===============================
message(STATUS "")
message(STATUS "🚀 Cortan Orchestrator Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  AI Features: ${ENABLE_AI_FEATURES}")
message(STATUS "  Tests: ${BUILD_TESTS}")
message(STATUS "  Benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")
