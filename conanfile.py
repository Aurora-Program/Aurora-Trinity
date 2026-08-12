from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class TrinityConan(ConanFile):
    name = "trinity"
    version = "0.1.0"
    package_type = "library"

    license = "Apache-2.0 AND CC-BY-4.0"
    homepage = "https://github.com/Aurora-Program/Aurora-Trinity"
    url = "https://github.com/Aurora-Program/Aurora-Trinity"
    description = "C11 execution library for the Aurora model"
    topics = ("c", "cmake", "aurora", "trinity", "ternary")

    settings = "os", "arch", "compiler", "build_type"
    exports_sources = "CMakeLists.txt", "cmake/*", "include/*", "src/*", "genesis/*"

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["genesis", "trinity"]