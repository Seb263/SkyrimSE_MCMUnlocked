# MCM Unlocked
*By Seb263*

Removes SkyUI's 128 MCM menu limit and loads all MCMs in just a few seconds thanks to a fast dynamic allocation system. Entries are only instantiated when needed, avoiding any fixed preallocation. Supports SE, AE, and VR via SKSE.

The mod can be downloaded here: [nexusmods.com](https://www.nexusmods.com/skyrimspecialedition/mods/180186)

## Requirements

- [CMake](https://cmake.org/)
  - Add this to your `PATH`
- [Vcpkg](https://github.com/microsoft/vcpkg)
  - Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
- [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
  - Desktop development with C++

## User Requirements

- [SkyUI (5.2 or 6.0+)](https://www.nexusmods.com/skyrimspecialedition/mods/12604)
  - Needed for any version
- [Address Library for SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
  - Needed for SSE/AE
- [VR Address Library for SKSEVR](https://www.nexusmods.com/skyrimspecialedition/mods/58101)
  - Needed for VR

## Register Visual Studio as a Generator

- Open `x64 Native Tools Command Prompt`
- Run `cmake`
- Close the cmd window

## Building

```
# to update submodules in /extern
git submodule update --init --recursive
# configure cmake
cmake --preset build-release-msvc-msvc
# build dll
cmake --build build --preset release-msvc-msvc
```