This folder contains Conan package recipes for libraries used by the project that provide no packages themselves.
The 'clawmancat' remote in conanremotes.txt already provides these packages, but should it go down in the future,
the contents of this folder can be used to create new packages.
To create these packages, you will still need to acquire the sources yourself. Notably:  

- FastNoise2: https://github.com/Auburn/FastNoise2/
- Optick: https://github.com/bombomby/optick
- vulkan-sdk: You already need to have the SDK installed to use the package. Make sure to not copy the SDK into the package when you create it.

To create a package, run the following commands:
```shell
cd [PACKAGE_DIR]
conan create . [USERNAME]/stable -pr ../../profiles/[PROFILE].conanprofile -pr:b ../../profiles/[PROFILE].conanprofile
```