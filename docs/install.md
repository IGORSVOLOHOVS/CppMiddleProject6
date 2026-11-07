# Install

The dispatcher needs no installing: put `include` on your include path, compile the sources of `src` with your project, and build with C++23 and threads.

To build and test it here:

1. Install mingw g++ 13 or newer with POSIX threads, CMake 3.28 or newer, Ninja, and LLVM for `clang-tidy` and `clang-format`.
2. Clone this repository.
3. Run `.\recipe.ps1 test`. The first run downloads GoogleTest, whose archive `CMakeLists.txt` pins by hash.

`recipe.ps1` looks for g++ in `C:\ProgramData\mingw64\mingw64\bin` and for LLVM in `C:\Program Files\LLVM\bin`, and adds those folders to its own PATH.

`.\recipe.ps1 build` compiles the core, the demo and the tests into `build\`. `.\recipe.ps1 install` does nothing: there is no installer yet.

`conanfile.py`, `CMakePresets.json`, `scripts/build_windows.ps1`, `.devcontainer` and `.vscode` are what the course and the old GitHub route left. No gate runs them.
