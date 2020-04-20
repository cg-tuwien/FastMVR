# Fast Multi-View Rendering for Real-Time Applications

Accompanying source code to "Fast Multi-View Rendering for Real-Time Applications" (EGPGV 2020 Paper)

## Setup

1. Clone the repository
2. Pull all the submodules (a.k.a. "dependencies"): `git submodule update --init`. The dependencies will emerge in the folder `dependencies/`.
3. Create a folder `dependencies/libs` and within that, two subfolders: `dependencies/libs/release` and `dependencies/libs/debug`
4. Create a folder `dependencies/dlls` and within that, two subfolders: `dependencies/dlls/release` and `dependencies/dlls/debug`
5. Use CMake to build GLEW (`dependencies/glew/build/cmake/CMakeLists.txt`)
    * Copy the resulting debug and release libs (`glew32d.lib`/`glew32.lib`) into the appropriate subfolders of `dependencies/libs`
    * Copy the resulting debug and release dlls (`glew32d.dll`/`glew32.dll`) into the appropriate subfolders of `dependencies/dlls`
    * If you have problems building GLEW due to errors related to "Runtime Checks" or "RTC", set the option "Basic Runtime Checks" to "Default" in the Visual Studio project `glew` (Project properties -> C/C++ -> All Options -> Basic Runtime Checks)
6. Use CMake to build GLFW (`dependencies/glfw/CMakeLists.txt`) 
    * Copy the resulting debug and release libraries (`glfw3.lib`/`glfw3.lib`) into the appropriate subfolders of `dependencies/libs`
7. Open `FastMVR.sln` in the project properties of `FastMVR`, ass the path to a scene via command line argument (In Visual Studio: Project properties -> Debugging -> Command Arguments)
8. Build and run `FastMVR`
    * A custom build step will copy the required DLLs from `dependencies/dlls/release` or `dependencies/dlls/debug` into the target folder.
	


