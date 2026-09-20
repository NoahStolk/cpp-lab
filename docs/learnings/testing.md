# Testing

Notes from putting the first test on `Vec3`. Most of the learning was not about
assertions — it was that C++ splits "the thing that runs tests" from "the thing
that writes them", and that a source file outside the build model lies to you.

The build-system vocabulary this leans on is in [`cmake.md`](cmake.md).

## Nothing is built in

.NET hands you a runner and a framework as one decision: `dotnet new xunit`,
`dotnet test`, done. C++ has neither by default, and they are two separate
choices:

| .NET                          | C++                                          |
|-------------------------------|----------------------------------------------|
| `xunit` / `nunit` — `[Fact]`  | GoogleTest / Catch2 / doctest — the framework |
| `dotnet test` / VSTest        | **CTest** — the runner, ships with CMake      |
| test discovery via reflection | static registration, or nothing at all        |

The runner half is already installed, because CTest comes with CMake. The
framework half is optional, and for a `constexpr`-heavy type like `Vec3` a
surprising amount of it isn't needed at all.

### What CTest actually considers a test

One executable whose **exit code is 0**. That is the entire contract. CTest does
not know what a test case is, doesn't parse output, and doesn't care what
library produced the binary. `add_test(NAME x COMMAND x)` says "run this thing,
check the exit code".

That is why adding a framework later changes nothing structurally — it changes
what happens *inside* the executable.

## `static_assert`: tests that run at compile time

The idea with no .NET equivalent. `Vec3` is almost entirely `constexpr`, and its
`operator==` is `= default`ed and `constexpr` too, so whole assertions can be
evaluated by the compiler:

```cpp
static_assert(Vec3{1, 2, 3} + Vec3{1, 1, 1} == Vec3{2, 3, 4});
```

No runner, no framework, no runtime cost — the expression is folded away and
nothing survives into the binary. A failure is a **build** failure:

```
error: static assertion failed
    2 | static_assert(Vec3{1, 2, 3} + Vec3{1, 1, 1} == Vec3{9, 9, 9});
      |               ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~
```

Consequences worth being explicit about:

- **The build succeeding *is* the pass.** By the time the binary exists, the
  assertions have already been checked. CTest running it afterwards is a
  formality that confirms nothing extra.
- **Execution stops at the first failure.** A framework keeps going and reports
  all failures; the compiler gives up on the translation unit. Fine early on,
  annoying later.
- **No values in the message.** GCC prints the source text, not
  `expected 2, got 1.99`. This is the single biggest thing a framework buys.

### What is eligible

Only what the compiler can evaluate. `len_squared()` is `constexpr`, so it
works. `len()` is not, because it calls `std::sqrt`:

```
error: non-constant condition for static assertion
    3 | static_assert(Vec3{3, 4, 0}.len() == 5.0f);
error: call to non-'constexpr' function 'float Vec3::len() const'
```

That is not a bug to fix — it is the boundary. Everything on the `constexpr`
side can be tested for free; `len()` and anything built on it needs a runtime
test, and that is the point where a framework starts earning its keep.

## Wiring the test target

A test is a normal executable target. Layout:

```
src/libs/lab_math/
    CMakeLists.txt
    include/lab_math/vec.h
    tests/
        CMakeLists.txt
        vec.tests.cpp
```

`tests/` sits beside `include/`, **not inside it**. Anything under `include/` is
on the public include path of every consumer of `lab_math` (see
[`cmake.md`](cmake.md) on `INTERFACE` include directories) — a test translation
unit is not part of a library's public surface.

Root `CMakeLists.txt` gains two lines:

```cmake
enable_testing()
add_subdirectory(src/libs/lab_math/tests)
```

`enable_testing()` must be called at the **top level**, and before the
`add_subdirectory()` that reaches a directory calling `add_test()`. Convention
is to put it directly after `project()` so that ordering can't be broken later.

`tests/CMakeLists.txt`, targets only as usual:

```cmake
add_executable(lab_math_tests vec.tests.cpp)

target_link_libraries(lab_math_tests PRIVATE lab_math)
target_compile_options(lab_math_tests PRIVATE -Wall -Wextra)

add_test(NAME lab_math_tests COMMAND lab_math_tests)
```

Linking `lab_math` is what supplies the include directory, so the test includes
the header the same way an app does — `<lab_math/vec.h>`, no relative path. If a
test file needs `../include/...` to find its own library's header, the target
wiring is wrong.

Warning flags are `PRIVATE` per target and there is no global setting, so a new
test target starts with no warnings until it asks for them.

## Running

```bash
# after editing any CMakeLists.txt
~/.local/share/JetBrains/Toolbox/apps/clion/bin/cmake/linux/x64/bin/cmake \
  -S . -B cmake-build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug

ninja -C cmake-build-debug lab_math_tests
ctest --test-dir cmake-build-debug
ctest --test-dir cmake-build-debug --output-on-failure   # show stdout when a test fails
```

## Gotchas hit along the way

**A file in no target gets no flags, and the IDE invents some.** The first
version of `vec.tests.cpp` wasn't listed in any `add_executable`, so it had no
entry in `compile_commands.json` at all. CLion fell back to a default
configuration and reported a *false* error on the `static_assert`:

```
Expression did not evaluate to a constant:
invalid body of constexpr Vec3::operator==(const Vec3& vec) const
```

GCC compiled the identical file clean. The mechanism: `operator== = default` is
a **C++20** feature, `CMAKE_CXX_STANDARD 26` reaches a file only *through its
target*, and under a pre-C++20 fallback the defaulted body never becomes a valid
constant expression. GCC at `-std=gnu++17` says the same thing in its own words:

```
error: defaulted 'constexpr bool A::operator==(const A&) const' only available with '-std=c++20'
error: non-constant condition for static assertion
```

This is the C++ trap with no `.csproj` analogue — an SDK-style project globs the
folder, CMake does not, so an unlisted file is invisible to the build and the
IDE is guessing. **`compile_commands.json` is the ground truth**; when an error
looks impossible, check the file is in there and read the flags it records:

```bash
grep -o '"file": "[^"]*"' cmake-build-debug/compile_commands.json
```

**A `static_assert`-only file still needs `main()`.** Moving the assertion out of
`main` to namespace scope — which reads better, and makes clear it's
compile-time — left the file with no entry point:

```
/usr/bin/ld: (.text+0x1b): undefined reference to `main'
```

The compile step had already succeeded. `add_executable` produces a program and
a program needs an entry point, so `int main() {}` goes back in, empty. It does
nothing, and returning 0 is exactly the "pass" CTest wants. A framework's
`gtest_main` / Catch2 main replaces it later.

Note where each error surfaced: the false one as an inline squiggle at compile
time, the real one in build output at link time. Worth checking *which* is being
reported before assuming they're the same problem.

**Exact float comparison is a real hazard, and it hides.** `Vec3::operator==` is
`= default`ed, so it compares floats with `==`. Two cases that fail while
printing as though they passed:

```
sum of 0.1f x10 == Vec3{1,1,1}? 0  (1, 1, 1)
normalized len  == 1.0f?        0  (1)
```

Both print `1` and both compare false — `operator<<` rounds for display, `==`
does not. Simple constant-folded cases happen to round-trip exactly
(`Vec3{1,2,3} / 3.0f * 3.0f == Vec3{1,2,3}` is true), which makes this worse,
not better: early tests pass and the hazard only appears once `len()` and
normalization get involved.

Keeping `operator==` exact is the right call for a value type — the tolerance
belongs in the test, not the type. That means runtime approximate comparison,
which is another thing pointing at a framework (`EXPECT_FLOAT_EQ`,
`EXPECT_NEAR`) rather than at changing `Vec3`.

**CTest sees one test, not N.** `ctest` currently reports `1/1 Test #1:
lab_math_tests`, however many assertions are inside. Per-case reporting is a
framework feature, not a CTest one.

## When a framework goes in

Not yet — `static_assert` covers the `constexpr` surface, which is most of
`Vec3`. The trigger will be wanting any of: named cases that keep running past a
failure, actual/expected values in the message, approximate float comparison, or
fixtures.

At that point, sketched but **not yet tried in this repo**:

- **GoogleTest** is the common default (and what `TEST(Suite, Case)` comes
  from), **Catch2** has nicer assertion syntax, **doctest** compiles fastest.
- Dependencies come in via CMake's `FetchContent`, which clones and builds from
  source at *configure* time — the rough `PackageReference` analogue, except it
  needs network on first configure and compiles into the build tree. It defines
  `GTest::gtest` / `GTest::gtest_main` to link against.
- `include(GoogleTest)` + `gtest_discover_tests(<target>)` runs the built binary
  with `--gtest_list_tests` and registers each case with CTest individually,
  which is what turns `1/1` into per-case results.
- `TEST(Suite, Case)` expands to a class deriving from `::testing::Test` with the
  braces as its `TestBody()`, plus a namespace-scope static whose constructor
  registers it in a global registry before `main` runs. The mechanism is
  **static initialisation, not reflection** — C# scans assembly metadata for
  `[Fact]`, C++ has no metadata, so the macro has to physically emit
  registration code. A practical consequence: tests inside a static library can
  be dropped by the linker as unreferenced, which is why test targets are
  normally executables directly.
- Watch the standard — the root sets `CMAKE_CXX_STANDARD 26` and subdirectories
  inherit it, so a fetched dependency gets compiled at `gnu++23` too. First
  place to look if it won't build.
