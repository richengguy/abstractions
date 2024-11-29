from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake


class AbstractionsRecipe(ConanFile):
    settings = ["os", "compiler", "build_type", "arch"]
    generators = ["CMakeToolchain", "CMakeDeps"]

    tool_requires = ["cmake/3.31.0"]
    requires = [
        "blend2d/0.11.4",
        "cli11/2.4.2",
        "eigen/3.4.0",
        "fmt/11.0.2",
        "indicators/2.3",
        "nlohmann_json/3.11.3",
    ]

    def layout(self) -> None:
        cmake_layout(self)

    def validate(self) -> None:
        # TODO: figure this out
        ...

    def build(self) -> None:
        cmake = CMake(self)
        cmake.build()
