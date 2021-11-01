from conans import ConanFile
from conans.errors import ConanInvalidConfiguration
import os


# This Conanfile finds the Vulkan SDK on your computer by looking for the VK_SDK_DIR and VULKAN_SDK environment variables.
# It then exports the include, binary and library directories of the SDK as a package.
class VulkanSDK(ConanFile):
    name        = 'vulkan-sdk'
    version     = '1.0.0'
    description = 'Conan package for using an already installed Vulkan SDK'
    settings    = ('os', 'arch')
    exports     = '*'
    sdk_dir     = None
    
    
    def get_sdk_dir(self, name, arch_specific = True):
        if arch_specific and self.settings.arch == 'x86': 
            dirname = name + '32'
        else: 
            dirname = name
        
        return os.path.join(str(self.sdk_dir), dirname)


    def configure(self):
        if self.settings.arch not in ['x86', 'x86_64']:
            raise ConanInvalidConfiguration(f'Unsupported platform {str(self.settings.arch)}!')
        
    
        for env_var in ['VK_SDK_DIR', 'VULKAN_SDK']:
            if env_var in os.environ:
                self.sdk_dir = os.environ[env_var]
                break
        
        
        if self.sdk_dir is None:
            raise ConanInvalidConfiguration('Failed to find Vulkan SDK.')


    def build(self):
        pass


    def package(self):
        pass
    
    
    def package_info(self):
        self.cpp_info.includedirs = [ self.get_sdk_dir('Include', False) ]
        self.cpp_info.libdirs     = [ self.get_sdk_dir('Lib') ]
        self.cpp_info.bindirs     = [ self.get_sdk_dir('Bin') ]
        
        
        self.cpp_info.libs = []
        
        for lib in os.listdir(self.get_sdk_dir('Lib')):
            if os.path.splitext(lib)[1] == '.lib':
                self.cpp_info.libs.append(
                    os.path.splitext(os.path.basename(lib))[0]
                )