from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain
from conan.tools.build import check_min_cppstd


class ExampleRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    # Убираем "CMakeToolchain" из generators.
    # Conan сам сгенерирует его в методе generate.
    generators = "CMakeDeps" 

    def requirements(self):
        self.requires("gtest/1.16.0")

    def layout(self):
        cmake_layout(self)
        
    def validate(self):
        check_min_cppstd(self, "23")

    def generate(self):
        # Создаем и настраиваем CMakeToolchain только здесь
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = "ON"
        tc.generate()