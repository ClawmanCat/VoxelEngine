from conans import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.layout import cmake_layout
import os


class FastNoise2(ConanFile):
    name            = 'fastnoise2'
    version         = '1.0.0'
    description     = 'Conan package for FastNoise2'
    settings        = ('os', 'compiler', 'build_type', 'arch')
    exports_sources = ('CmakeLists.txt', 'src/*', 'include/*', 'cmake/*')


    def configure_cmake(self):
        cmake = CMake(self)

        if not hasattr(cmake, 'definitions'):
            cmake.definitions = dict()

        cmake.definitions['FASTNOISE2_NOISETOOL'] = 'Off'
        cmake.definitions['FASTNOISE2_TESTS'] = 'Off'

        return cmake


    def layout(self):
        cmake_layout(self)


    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.generate()


    def build(self):
        cmake = self.configure_cmake()
        cmake.configure()
        cmake.build()


    def package(self):
        cmake = self.configure_cmake()
        cmake.install()


    def package_info(self):
        self.cpp_info.includedirs = [ os.path.join(self.package_folder, 'include') ]
        self.cpp_info.libdirs     = [ os.path.join(self.package_folder, 'lib') ]
        self.cpp_info.bindirs     = [ os.path.join(self.package_folder, 'bin') ]


        self.cpp_info.libs = []

        for libdir in self.cpp_info.libdirs:
            for lib in os.listdir(libdir):
                if os.path.splitext(lib)[1] == '.lib':
                    self.cpp_info.libs.append(
                        os.path.splitext(os.path.basename(lib))[0]
                    )