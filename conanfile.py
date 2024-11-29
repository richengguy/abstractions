from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake, CMakeDeps, CMakeToolchain


class AbstractionsRecipe(ConanFile):
    settings = ["os", "compiler", "build_type", "arch"]

    tool_requires = ["cmake/3.31.0"]
    requires = [
        "blend2d/0.11.4",
        "cli11/2.4.2",
        "eigen/3.4.0",
        "fmt/11.0.2",
        "indicators/2.3",
        "nlohmann_json/3.11.3",
    ]

    options = {
        "build_tests": [True, False]
    }

    default_options = {
        "build_tests": False
    }

    def layout(self) -> None:
        cmake_layout(self)

    def validate(self) -> None:
        # TODO: figure this out
        ...

    def generate(self) -> None:
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.cache_variables["ABSTRACTIONS_BUILD_TESTS"] = self.options.build_tests  # type: ignore
        tc.generate()

    def build(self) -> None:
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
