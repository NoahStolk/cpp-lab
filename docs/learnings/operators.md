# Operator overloading

Notes from making `Vec3` printable with `std::cout`. The interesting part turned
out to be not the operator itself but `friend`, which does something quite
different from what its name suggests.

The ODR and `inline` background this leans on is in [`headers.md`](headers.md).

## Operators are just function calls

`std::cout << pos` is spelled as an operator but it is an ordinary function call
in disguise:

```cpp
operator<<(std::cout, pos)
```

Nothing about `std::cout` knows what a `Vec3` is. The standard library ships
overloads of `operator<<` for `float`, `const char*`, `int` and so on, and the
compiler picks one by ordinary overload resolution. A type becomes printable by
supplying one more overload for the set to choose from.

There is no `ToString()` being overridden here, and no common base class
involved. That's the main mental shift coming from .NET: printing is not a
virtual method on the object, it's an external function that happens to accept
the object.

## Member or non-member: the left operand decides

An operator overload can be a member function or a free function, and the choice
isn't stylistic — it follows from which operand ends up on the left.

For a member, the object it's called on **is** the left operand:

```cpp
a + b   ->   a.operator+(b)
```

So `Vec3::operator+` works as a member: `this` is the left `Vec3`, and the one
declared parameter is the right one. A binary operator as a member therefore
takes *one* explicit parameter.

`operator<<` can't work this way. Its left operand must be the stream, so a
member version would have to be a member of `std::ostream`, and that class isn't
ours to extend. It has to be a non-member taking both operands explicitly:

```cpp
std::ostream& operator<<(std::ostream& os, const Vec3& v)
```

The stream is taken and returned **by reference**: returning it is what makes
chaining work, since `std::cout << a << b` parses as `(std::cout << a) << b` and
the first call has to hand the stream on to the second.

### The error this produces

Pasting that free-function signature into the class body without `friend` gets:

```
Overloaded 'operator<<' must be a binary operator (has 3 parameters)
```

The three are `this`, `os` and `v` — the compiler read it as a member and added
the implicit `this`. Writing `const` after the parameter list is the same
mistake seen from the other side, because `const` there means "does not modify
`*this*`", and a free function has no `*this*` to qualify.

## `friend` is two features sharing one keyword

The fix is to add `friend`:

```cpp
friend std::ostream& operator<<(std::ostream& os, const Vec3& v)
{
    return os << v.x << ", " << v.y << ", " << v.z;
}
```

`friend` is usually introduced as "grants access to private and protected
members", and it does do that. But that's not what's doing the work here —
`Vec3` is a `struct` with public members, so the access grant is a no-op. What
`friend` provides is the *other* half of its meaning:

> This is **not** a member function. It is a free function at the enclosing
> namespace scope, which merely happens to be declared (and here, defined)
> inside the class body.

That's why the parameter count comes out right: no implicit `this` is added,
so the two declared parameters are the whole signature.

A friend declaration is also unaffected by `public:` / `private:` sections — it
can sit anywhere in the class body and means the same thing.

### The "it's like `static`" intuition

It reads like a `static` method in C#, and that intuition is half right. Both
lack an implicit `this` and are called without an object. But C++ *also* has
`static` member functions, which are the literal counterpart of C# `static`, and
a friend is not one of them:

|                        | C# `static` method | C++ `static` member fn | C++ `friend` in class body |
| ---------------------- | ------------------ | ---------------------- | -------------------------- |
| implicit `this`        | no                 | no                     | no                         |
| **is a member**        | yes                | yes                    | **no**                     |
| called as              | `Type.M(x)`        | `Type::m(x)`           | `m(x)`                     |
| access to private      | yes                | yes                    | yes                        |

The row that matters is "is a member". A `static` member function is still
scoped to the class: its name is `Vec3::m`, it appears in the class's interface,
and access specifiers apply to it. A friend is a namespace-scope function that
borrowed the class body as a place to be written down. `Vec3::operator<<` does
not exist and does not compile.

So: `static` removes `this` while staying inside the class; `friend` puts the
function outside the class entirely, and the missing `this` is a consequence of
that rather than the point of it.

Worth keeping the access-granting meaning in mind anyway — if `Vec3` ever gains
private members, the same declaration keeps compiling precisely because it *is*
a friend, where a plain free function outside the class would stop.

## Hidden friends and ADL

Defining the operator inside the class body rather than after it makes it a
**hidden friend**: the name is not visible to ordinary unqualified lookup, and
can only be found by argument-dependent lookup — that is, by having a `Vec3`
among the arguments at the call site.

This costs nothing today, because `Vec3` sits in the global namespace and
everything is findable anyway. It starts to matter once the type moves into a
`lab_math` namespace: ADL is what lets `main.cpp` write `std::cout << pos`
without a `using` declaration, since the compiler looks for `operator<<` in the
namespaces of its argument types. It also keeps the overload out of consideration
for calls that have nothing to do with `Vec3`, which shortens overload
resolution and removes a class of surprising ambiguity errors.

The alternative — declaring it at namespace scope in the header, after the
closing `};` — works identically at the call site but needs an explicit
`inline`. Both forms are fine; the hidden friend is the more idiomatic modern
choice.

What does *not* work well is mixing them: a `friend` declaration inside the
class does not introduce the name into the enclosing namespace, so a friend
declared in the class and defined out-of-line in a `.cpp` still needs a separate
namespace-scope declaration to be callable. Pick one form.

## Why nothing here needed `inline`

Every function in `vec.h` is defined inside the class body, and definitions in a
class body — members and friends alike — are **implicitly `inline`**. That's the
ODR exemption from [`headers.md`](headers.md): multiple identical copies across
translation units are permitted and collapse at link time.

This is why `operator+`, `len()` and the constructor have never needed the
keyword, and why the friend didn't either. The moment a definition moves
*outside* the class body but stays in the header, `inline` becomes mandatory and
its absence shows up as a duplicate-symbol **linker** error — only once a second
`.cpp` includes the header, which can be a long time after the mistake.

## `<ostream>`, not `<iostream>` — and self-contained headers

Using `std::ostream` in a header means the header needs to include something
that declares it. `<iostream>` would work, but `<ostream>` is the right choice:
`<iostream>` additionally defines the global `std::cin`/`std::cout` objects and
their static initialiser, which every translation unit including `vec.h` would
then pay for. The library only needs the type; the app already includes
`<iostream>` for the streams themselves.

There's a trap here worth naming. `vec.h` currently compiles only because
`main.cpp` includes `<iostream>` on line 1, *before* `<lab_math/vec.h>` on line
3 — so by the time the preprocessor pastes `vec.h` in, `std::ostream` is already
declared. `#include` being plain text substitution means include order is
load-bearing. Compiling a translation unit that includes `vec.h` first fails:

```
error: 'ostream' in namespace 'std' does not name a type
note: 'std::ostream' is defined in header '<ostream>'
```

The rule that avoids this class of bug: **every header must be self-contained**,
compiling on its own without depending on what its includer happened to include
first. A cheap way to check is a throwaway `.cpp` that includes the header and
nothing else.

## Which operators must be which

- **Must be members:** `=`, `[]`, `()`, `->`. The language requires it.
- **Must be non-members:** `<<` and `>>` for streams, and anything else whose
  left operand is a type you don't own.
- **Free choice:** the arithmetic and comparison operators. Convention leans
  towards non-member for symmetric binary ones, because a member only allows
  implicit conversion on the *right* operand — `v + something_convertible`
  compiles where `something_convertible + v` doesn't. `Vec3` isn't affected yet,
  since its only constructor takes three arguments and so defines no implicit
  conversion, but a one-argument constructor added later would expose the
  asymmetry.

## The other way to print: `std::formatter`

`std::format` (C++20) uses a separate extension mechanism — a specialisation of
`std::formatter<Vec3>` — rather than `operator<<`. It's more machinery, and it's
what `std::format` and `std::println` consume. GCC 13 has `<format>`, so
`std::cout << std::format("{}", v)` is reachable; `<print>` only arrived in GCC
14. `operator<<` is the smaller step and worth having first.
