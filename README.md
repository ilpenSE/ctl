# CTL - C Template Library

- CTL is an application for C++ STL in C.
- It adds high level concepts with a pleasant API.
- Tested on x86_64 GNU/Linux environment with GCC/Clang compiler.
  (C++ not tested because this is obselete in C++, use STL instead.)
- Note that this APIs have no proper documentation for now. (I'll add one)

## Usage

- Bootstrap the build system by running this: `cc -o buic build.c -lm`
- Then run `buic` executable in the project root.
- This will create `.build` also `lib` and `include` in it.
- You can directly use stb-style single-header which is located in project root.
- Or include the header then link it with static or dynamic library which is located in `build/lib`.

### Usage of STB-Style Single Headers

- First, you gotta define `CTL_IMPLEMENTATION` in a translation unit (C source file)
- Then include the `ctl.h` header.
- In other translation units, you don't have to define implementation macro, just including the header brings all the macros and functions
- Because all functions definitions/implementations are included in the first TU that you define the macro

## About

- Developer: ilpeN
- License: GPLv3
