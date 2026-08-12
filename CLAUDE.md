# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Ground rule: do not write the code

Per README.md, this is a personal learning repository: **code will *not* be AI generated, but AI can be used to assist in learning.**

Treat this as binding. Default to explaining, reviewing, answering questions about C++ semantics, pointing at the relevant standard/library behaviour, and suggesting directions — not producing implementations. If the user explicitly asks for code to be written, honour the request, but keep it minimal and explain what it does rather than handing over a finished feature.

## Build

CLion generated the `cmake-build-debug/` tree with Ninja; reuse it rather than making a new build dir.

```bash
ninja -C cmake-build-debug          # build
./cmake-build-debug/cpp_lab         # run

# regenerate after editing CMakeLists.txt — must be the CLion-bundled cmake
~/.local/share/JetBrains/Toolbox/apps/clion/bin/cmake/linux/x64/bin/cmake \
  -S . -B cmake-build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

`cmake_minimum_required(VERSION 4.3)` is satisfied only by the CLion-bundled CMake (4.3.1) at the path above. The `cmake` on `PATH` (`~/.cmake-deps/…`) is 4.1.2 and **fails to configure**. The compiler is `/usr/bin/c++` (GCC 13); the target builds with `-Wall -Wextra`.

There is no test framework or formatter configured. Lint config lives in `.clang-tidy`; `.editorconfig` sets 4-space indent, UTF-8, trimmed trailing whitespace, final newline.

Running clang-tidy from the command line needs GCC's internal include dir, or it dies on `'stddef.h' file not found` (the `compile_commands.json` records GCC flags but clang-tidy parses with a clang frontend):

```bash
~/.local/share/JetBrains/Toolbox/apps/clion/bin/clang/linux/x64/bin/clang-tidy \
  -p cmake-build-debug --extra-arg=-I/usr/lib/gcc/x86_64-linux-gnu/13/include main.cpp
```

CLion's built-in clang-tidy does not hit this. IDE inspection severities are version-controlled in `.idea/editor.xml` (not gitignored) — change them through the IDE, which rewrites that file, rather than by hand while CLion is running.

## Structure

Single executable target `cpp_lab` built from sources listed directly in `CMakeLists.txt` — **every new `.cpp`/`.h` must be added to the `add_executable(...)` list**, there is no glob. Everything lives flat in the repo root: `main.cpp` is the scratch driver, headers like `vec.h` hold the experiments.

`CMAKE_CXX_STANDARD` is set to 26, but the installed GCC silently compiles as `gnu++23` — don't assume C++26 features are actually available.

## Current state

`vec.h` holds a `Vec3` aggregate exercised from `main.cpp`. Expect half-finished experiments here — commented-out code and unused members are deliberate learning scratch, not bugs to fix unprompted.
