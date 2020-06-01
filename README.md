# Description
This is a project for the course DAT205 Advanced Computer Graphics given at Chalmers University of Technology 2020.
The main part of the project is procedural architectural generation.

The code is based on code from the repository located at [https://gitlab.com/chalmerscg/tda362-simple-project](https://gitlab.com/chalmerscg/tda362-simple-project).

Has external dependency on CGAL. 
Easiest way to build on Windows is to install CGAL through [vcpkg](https://github.com/microsoft/vcpkg). Use ```.\vcpkg integrate install``` to access headers and build the first time (with cleared CMake cache if one exists) with ```"-DCMAKE_TOOLCHAIN_FILE=[path_to_vcpkg]/scripts/buildsystems/vcpkg.cmake"``` as an argument to CMake.
