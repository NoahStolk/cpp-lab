# C++ headers

Language-level notes about headers: how `#include` actually behaves, guards, and
what the ODR does and doesn't allow. The build-system side — where headers live
on disk and how a target exposes them to consumers — is in
[`cmake.md`](cmake.md).

## `#include` is text substitution

There is no import mechanism involved. The preprocessor finds the named file and
pastes its contents in, then the compiler sees one long stream of text per
translation unit (one `.cpp` plus everything it pulled in, transitively).

Two consequences follow from that, and most header conventions exist to manage
them: the same file can easily arrive twice, and everything a header declares is
duplicated into every `.cpp` that includes it.

## Angle brackets vs quotes

- `#include <lab_math/vec.h>` — searched on the include path (the `-I`
  directories a target receives from its dependencies, plus the system paths).
- `#include "helper.h"` — searched relative to the including file *first*, then
  falls back to the include path.

Both would work in most cases, so the distinction is a signalling convention:
angle brackets mean "this arrives from a dependency", quotes mean "this is my
own header, sitting next to me". Keeping to it makes it visible at the include
site when a file is reaching outside its own target.

## Header guards

Every header needs protection against being included twice within one
translation unit — otherwise the second include re-runs `struct Vec3 { ... };`
and the compiler rejects the redefinition. That happens easily once headers
include other headers.

This repo uses **`#pragma once`** as the first line of every header. The
alternative is the traditional trio:

```cpp
#ifndef LAB_MATH_VEC_H
#define LAB_MATH_VEC_H
// ...
#endif
```

which is what CLion generates by default (configurable under Editor → Code Style
→ C/C++). Both work. `#pragma once` is not in the standard, though GCC, Clang and
MSVC all support it.

`#pragma once` was chosen here because guard macros share one global namespace
and their names conventionally encode the file's location, which creates two
problems:

- **Names go stale.** Moving a header leaves the old name behind — exactly what
  happened when `vec.h` moved out of the repo root still saying
  `CPP_LAB_VEC_H`.
- **Collisions fail silently.** Two headers picking the same macro name means
  the second one expands to *nothing*, and the error surfaces as an unrelated
  "unknown type name" somewhere else entirely.

`#pragma once` has no name to go stale or collide. The tradeoff is that it relies
on the compiler recognising a file it has already seen, which is a fuzzier notion
than a macro name — symlinks, hard links, or the same header reachable through
two mounted paths can defeat it. That's why some large codebases still prefer
guards.

C++20 modules eventually replace this whole mechanism, but GCC 13 compiling as
`gnu++23` isn't somewhere to rely on them yet.

## Guards are per translation unit, not program-wide

A guard stops a file being pasted in twice *within one `.cpp`*. It does nothing
across separate `.cpp` files — each one is compiled independently and gets its
own copy of everything the header declares.

That's usually fine, because the One Definition Rule explicitly permits a class
to be defined in multiple translation units as long as the definitions are
identical. Functions are stricter, and that's where it bites:

- Member functions defined **inside** the class body — as `Vec3::len()` is — are
  implicitly `inline`, so multiple copies are allowed and collapse at link time.
- A function defined in a header **outside** a class without `inline` is a
  single definition duplicated into every including `.cpp`, and the linker
  rejects it as a duplicate symbol as soon as a second `.cpp` includes that
  header.

`friend` functions defined in the class body count as inside it for this
purpose, and are implicitly `inline` too — see
[`operators.md`](operators.md), which also covers why `vec.h` defines its
stream operator that way.

Note the failure mode differs: the guard problem is a *compiler* error in one
file, this one is a *linker* error that only appears once a second `.cpp`
includes the header. Adding `inline` (or moving the definition into a `.cpp`) is
the fix.
