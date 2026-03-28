# Getting Started

## Requirements

`unicode_ranges` requires a compiler and standard library with strong C++23 support.

Minimum toolchains currently exercised in CI:

- MSVC with the MSVC STL: Visual Studio 2022 toolset `v143` or newer
- Clang-cl with the MSVC STL: current Visual Studio 2022 `ClangCL`
- GCC with libstdc++: GCC 14 / libstdc++ 14 or newer
- Clang with libc++: Clang 22 / libc++ 22 or newer

The checked-in Unicode data currently tracks Unicode `17.0.0`.

## Include the library

```cpp
#include "unicode_ranges.hpp"
```

## Choose the right entry point

- Use `_utf8_sv` / `_utf16_sv` for validated compile-time views.
- Use `utf8_string::from_bytes(...)` / `utf16_string::from_code_units(...)` when input arrives as raw runtime data.
- Use `_utf8_s` / `_utf16_s` when you want an owning validated string immediately.

## A first validated view

This is the style the docs will use going forward: visible Unicode text, runnable code, `std::println`, and comments showing what to expect.

```cpp
--8<-- "examples/getting-started/validated-view.cpp"
```

## Runtime validation

When text arrives at runtime as raw bytes, validate it once and keep the validated type:

```cpp
--8<-- "examples/getting-started/runtime-validation.cpp"
```

## Formatting and printing

Library-defined UTF-8 and UTF-16 types support formatting and printing directly. Borrowed views such as `chars()` and `graphemes()` are easy to inspect too. For grapheme views, the examples use `"{::s}"` so the printed range stays visually uniform with the underlying text:

```cpp
--8<-- "examples/getting-started/formatting.cpp"
```

## Views versus owning strings

- `utf8_string_view` / `utf16_string_view` borrow existing storage.
- `utf8_string` / `utf16_string` own and mutate storage.
- `chars()`, `graphemes()`, `char_indices()`, and `grapheme_indices()` are borrowing range views.

Do not keep borrowed ranges alive after the source storage dies or after the owning string mutates.

## Counting and indexing

The library intentionally distinguishes:

- code units: `size()`
- Unicode scalar values: `char_count()`
- grapheme clusters: `grapheme_count()`

UTF-8 view/string search APIs generally return byte offsets. UTF-16 view/string search APIs generally return code-unit offsets. Character-oriented APIs are named explicitly, such as `char_at`, `is_char_boundary`, and `ceil_char_boundary`.

## Example sanity checks

The examples under `docs/examples/` are compiled in CI so the docs do not silently drift away from the library surface.

## Where to go next

- [Design](design.md)
- [Text Operations](text-operations.md)
- [Casing and Normalization](casing-and-normalization.md)
- [Reference](reference/index.md)
