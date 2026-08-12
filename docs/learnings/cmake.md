# CMake: multi-target projects

Notes from restructuring this repo from a single flat executable into a library
plus console apps. Written from a C#/.NET background, so the vocabulary map comes
first.

## Vocabulary, coming from .NET

The trap is assuming `CMakeLists.txt` is the equivalent of a `.csproj`. It isn't.

| .NET                                    | CMake                                         |                                    |
|-----------------------------------------|-----------------------------------------------|------------------------------------|
| `.sln`                                  | `project()`                                   | loose analogy — see below          |
| `.csproj`                               | **target** (`add_executable` / `add_library`) | one target = one build output      |
| —                                       | `CMakeLists.txt`                              | a build *script* for one directory |
| `ProjectReference`                      | `target_link_libraries()`                     |                                    |
| `PrivateAssets="all"`                   | `PRIVATE`                                     |                                    |
| default (transitive) `ProjectReference` | `PUBLIC`                                      |                                    |

### What a target is

A **target** is one named thing CMake knows how to build, created by
`add_executable(<name> ...)` or `add_library(<name> ...)`:

- an executable target → one binary you can run
- a library target → one thing other targets can link against

Each target carries its own sources, include directories, compile flags and
dependencies. That is exactly what the `target_*(<name> ...)` commands do:
attach a property to one named target. If the name isn't a target, the command
fails.

Targets are *declared in* a `CMakeLists.txt` but are not the same as it — one
file can declare none, one, or ten. In .NET a `.csproj` produces exactly one
assembly, so the file and the build output are effectively the same thing; in
CMake they are separate, and the target is the half that becomes a binary.

Target names live in **one global namespace across the whole build**, regardless
of which directory declared them, so two directories can't both define
`vec_demo`.

### Is `project()` a solution?

Roughly, but it differs in three ways that matter:

- A `.sln` **enumerates** its projects. `project()` enumerates nothing —
  membership comes from `add_subdirectory()` calls reaching into directories.
- A `.sln` produces nothing. `project()` **detects the compiler** and enables
  the language toolchain. This is its most important job and has no `.sln`
  equivalent. Omit it and CMake fakes one, with confusing downstream warnings.
- `project()` can nest. Calling it in a subdirectory opens a new project scope
  and rebinds `PROJECT_SOURCE_DIR`, which is almost never what you want inside
  one build.

Practical rule: **only the top-level `CMakeLists.txt` calls
`cmake_minimum_required()` and `project()`.** Subdirectory files declare targets
and nothing else. Settings like `CMAKE_CXX_STANDARD` set at the root are
inherited by every subdirectory automatically.

A `project()` name is **not** a target. `project(cpp_lab)` creates nothing called
`cpp_lab`, and `target_compile_options(cpp_lab ...)` is an error.

## PRIVATE / PUBLIC / INTERFACE

The concept with no real C# equivalent, and the one worth actually understanding.
These keywords appear on `target_link_libraries`, `target_include_directories`,
`target_compile_options` and friends. They answer: *who does this apply to?*

- **PRIVATE** — used when building this target, not passed to consumers.
- **INTERFACE** — not used building this target, passed to consumers only.
- **PUBLIC** — both.

"Consumers" means other targets that `target_link_libraries` against this one.
It is **per-property**, not per-target: a library can have private compile flags
and public include directories at the same time.

Rules of thumb:

- On an **executable**, essentially always `PRIVATE`. An executable has no
  consumers, so `INTERFACE` on one applies the setting to *nothing at all* —
  and it does so silently, with no error.
- **Include directories a header exposes** are `PUBLIC` (or `INTERFACE` on a
  header-only library). This is what lets a consumer `#include` without ever
  writing an include path itself.
- **Warning flags** are `PRIVATE`. You want warnings on your own code, not
  forced onto everything downstream.

### INTERFACE libraries

A header-only library has nothing to compile, so it is declared:

```cmake
add_library(lab_math INTERFACE)
target_include_directories(lab_math INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

It compiles nothing; it is a bundle of usage requirements that consumers absorb.
Closest .NET analogue is a props-only NuGet package with no assembly.

Consequence: an INTERFACE target accepts **only** the `INTERFACE` keyword.
`target_compile_options(lab_math PRIVATE ...)` is a hard error — there is no
private build to apply it to.

Once the library gains a `.cpp` it becomes `add_library(lab_math STATIC ...)`,
and the include directories switch from `INTERFACE` to `PUBLIC`.

## Layout

```
CMakeLists.txt                          # only file with cmake_minimum_required + project()
src/libs/lab_math/
    CMakeLists.txt                      # targets only
    include/lab_math/vec.h              # public headers
src/apps/vec_demo/
    CMakeLists.txt                      # targets only
    main.cpp
```

Every directory named by `add_subdirectory()` needs a `CMakeLists.txt`.
Intermediate directories do not — the root reaches straight down to
`src/libs/lab_math`, no `src/CMakeLists.txt` in between. Worth adding one only
when the root would otherwise list many subdirectories.

There is no globbing — every source is listed explicitly in `add_executable` /
`add_library`. (SDK-style `.csproj` globs by default; CMake does not, by design:
a glob can't tell the generator to re-run when a file appears.)

### Why `include/<libname>/`

The convention is `include/lab_math/vec.h`, giving `#include <lab_math/vec.h>`.
The doubled name does two unrelated jobs:

- `include/` **separates public from private.** Whatever directory is on the
  include path is reachable by every consumer. Private headers live beside the
  sources instead, where nothing outside can reach them.
- `lab_math/` **supplies the prefix.** `<vec.h>` is an unqualified name in a
  namespace shared with every library and system header on the path.
  `<lab_math/vec.h>` can't collide and names its origin at the include site.

Only the second job is visible today — `lab_math` is header-only, so there are
no private headers for `include/` to hide yet. It starts paying off as soon as
the library gains a `.cpp` with helpers that shouldn't be reachable from apps.

**Tempting shortcut worth avoiding:** leaving headers at `libs/lab_math/vec.h`
and putting `src/libs` on the include path gets you the prefix with no extra
directory — but it puts *every* library on the path at once. Any target could
then `#include <other_lib/foo.h>` without linking `other_lib`, and for
header-only libraries it would link fine too. Dependencies stop being enforced
by the build system, and it only becomes visible much later.

The language-level side of headers — angle brackets vs quotes, `#pragma once`
and guards, and what the ODR permits across translation units — is in
[`headers.md`](headers.md).

## Commands

`cmake_minimum_required(VERSION 4.3)` is satisfied only by the CLion-bundled
CMake (4.3.1). The `cmake` on `PATH` is 4.1.2 and fails to configure.

```bash
# configure / regenerate after editing any CMakeLists.txt
~/.local/share/JetBrains/Toolbox/apps/clion/bin/cmake/linux/x64/bin/cmake \
  -S . -B cmake-build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug

ninja -C cmake-build-debug              # build everything
ninja -C cmake-build-debug vec_demo     # build one target
./cmake-build-debug/src/apps/vec_demo/vec_demo
```

Binaries land in the build tree mirroring the source tree, so each app is at
`cmake-build-debug/src/apps/<name>/<name>`. Setting
`CMAKE_RUNTIME_OUTPUT_DIRECTORY` at root level would collect them in one place.

In CLion after big structural changes: Tools → CMake → Reset Cache and Reload
Project, not a plain reload.

## Gotchas hit along the way

**Build trees are not relocatable.** `CMakeCache.txt` and `build.ninja` contain
absolute paths baked in at generation time. Moving `cmake-build-debug` produces
errors naming the *old* location. Delete and regenerate; never move or copy.

**`ninja: error: loading 'build.ninja'` usually means configure failed.** CMake
writes `CMakeCache.txt` early, then errors out before generating build files, so
the directory exists and looks half-populated. Re-run the configure command and
read *its* output — ninja is only reporting the downstream symptom.

**CMake errors cascade.** The first message is often the only real information;
later ones are consequences. Fix one, re-run, read the next.

**Wrong keywords fail silently.** Once configure succeeds, mistakes stop
announcing themselves and show up as missing flags. `target_link_libraries(app
INTERFACE app)` — wrong target *and* wrong keyword — configured cleanly and
produced a compile with no `-I` and no `-Wall`. To check what a target actually
received:

```bash
ninja -C cmake-build-debug -v            # verbose compile lines
cat cmake-build-debug/compile_commands.json
```

**`.clang-tidy` goes at the repo root.** clang-tidy searches *upward* from each
source file, so a nested one would work but only covers its subtree. The upward
search is also the feature: a nested `.clang-tidy` overrides the root one for
that directory, which is how to relax a check in one place.

**`CMAKE_CXX_STANDARD 26` does not mean C++26 is available.** The installed GCC
silently compiles as `gnu++23` — visible in the compile line.
