from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout

class CortanOrchestratorConan(ConanFile):
    name = "cortan-orchestrator"
    version = "3.0.0"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "with_ai": [True, False],
        "with_tests": [True, False],
        "with_benchmarks": [True, False]
    }
    default_options = {
        "shared": False,
        "with_ai": True,
        "with_tests": True,
        "with_benchmarks": True
    }

    def requirements(self):
        # Core dependencies
        self.requires("nlohmann_json/3.11.3")
        self.requires("spdlog/1.12.0")
        self.requires("libcurl/8.4.0")

        # Async I/O - using boost::asio via boost
        self.requires("boost/1.82.0")

        # Optional: Better allocators for performance
        self.requires("jemalloc/5.3.0", override=True)

        # Testing framework
        if self.options.with_tests:
            self.requires("gtest/1.14.0")

        # Benchmarking framework
        if self.options.with_benchmarks:
            self.requires("benchmark/1.8.3")

        # AI-specific dependencies
        if self.options.with_ai:
            self.requires("openssl/3.1.3")  # For secure connections

    def configure(self):
        # Configure boost to only include what we need
        self.options["boost"].without_python = True
        self.options["boost"].without_test = True
        self.options["boost"].without_fiber = True
        self.options["boost"].without_graph = True
        self.options["boost"].without_graph_parallel = True
        self.options["boost"].without_json = True
        self.options["boost"].without_locale = True
        self.options["boost"].without_log = True
        self.options["boost"].without_math = True
        self.options["boost"].without_mpi = True
        self.options["boost"].without_nowide = True
        # Keep random and regex as they are required by iostreams
        # self.options["boost"].without_random = True
        # self.options["boost"].without_regex = True
        self.options["boost"].without_serialization = True
        self.options["boost"].without_stacktrace = True
        self.options["boost"].without_timer = True
        self.options["boost"].without_type_erasure = True
        self.options["boost"].without_wave = True

        # Configure libcurl for minimal dependencies
        self.options["libcurl"].with_ssl = "openssl"
        self.options["libcurl"].with_zlib = True

        # Configure spdlog
        self.options["spdlog"].header_only = False

    def layout(self):
        cmake_layout(self)

    def generate(self):
        # Generate CMake toolchain
        tc = CMakeToolchain(self)
        tc.variables["USE_CONAN"] = True
        tc.variables["ENABLE_AI_FEATURES"] = self.options.with_ai
        tc.variables["BUILD_TESTS"] = self.options.with_tests
        tc.variables["BUILD_BENCHMARKS"] = self.options.with_benchmarks
        tc.generate()

        # Generate CMake dependencies
        deps = CMakeDeps(self)

    def build_requirements(self):
        # Build-time tools
        self.tool_requires("cmake/3.25.3")
        if self.options.with_tests:
            self.tool_requires("pkgconf/1.9.5")  # For finding system dependencies
