from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class AbstractionsRecipe(ConanFile):
    settings = ["os", "compiler", "build_type", "arch"]

    requires = [
        "blend2d/0.11.4",
        "cli11/2.4.2",
        "doctest/2.4.11",
        "eigen/3.4.0",
        "fmt/11.0.2",
        "gperftools/2.16",
        "indicators/2.3",
        "nlohmann_json/3.11.3",
    ]

    options = {
        "build_docs": [True, False],
        "build_tests": [True, False],
    }

    default_options = {"build_docs": False, "build_tests": True}

    def layout(self) -> None:
        cmake_layout(self)

    def configure(self) -> None:
        self.options["gperftools/*"].build_cpu_profiler = True  # type: ignore
        self.options["gperftools/*"].build_heap_profiler = True  # type: ignore
        self.options["gperftools/*"].enable_libunwind = self.settings.os != "Macos"  # type: ignore

    def validate(self) -> None:
        # TODO: figure this out
        ...

    def generate(self) -> None:
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.cache_variables["ABSTRACTIONS_BUILD_DOCS"] = self.options.build_docs  # type: ignore
        tc.cache_variables["ABSTRACTIONS_BUILD_TESTS"] = self.options.build_tests  # type: ignore
        tc.generate()

    def build(self) -> None:
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
