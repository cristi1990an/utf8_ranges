# Changelog

All notable changes to this project will be documented in this file.

The format is based loosely on Keep a Changelog, with an `Unreleased` section
tracking local work before it is tagged or versioned.

## [Unreleased]

### Added

- `utf8_string::get_allocator()`
- `utf8_string::erase(index, count = npos)`
- `utf8_string::replace(pos, count, utf8_string_view)`
- `utf8_string::replace(pos, count, utf8_char)`
- `utf8_string::replace(pos, utf8_string_view)`
- `utf8_string::replace(pos, utf8_char)`
- `utf8_string::replace_with_range(pos, count, range)`
- `utf8_string::replace_with_range(pos, range)`
- `utf8_string_view::char_at_unchecked(index)`
- `std::uses_allocator` specialization for `utf8_string`

### Changed

- `utf8_string_view::char_at(index)` now interprets `index` as a byte index and returns `std::nullopt` when the index is out of range or not a UTF-8 character boundary
- library headers now use ordinary include guards instead of `#pragma once`
- tests and documentation now prefer direct Unicode literals instead of `u8`-prefixed literals where possible

### Documentation

- expanded the `utf8_string_view` reference for `char_at` and `char_at_unchecked`
- documented the `utf8_string` mutation APIs, including `erase`, `replace`, `replace_with_range`, and `get_allocator`
