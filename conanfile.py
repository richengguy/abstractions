from conan import ConanFile
from conan.tools.cmake import cmake_layout


class AbstractionsRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self) -> None:
        ...

    def build_requirements(self) -> None:
        self.tool_requires("cmake/3.31.0")

    def layout(self) -> None:
        cmake_layout(self)

    def validate(self) -> None:
        # TODO: figure this out
        ...
