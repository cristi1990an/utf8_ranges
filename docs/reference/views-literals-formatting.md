# Views, Literals, and Formatting

This page covers the helper view types in `unicode_ranges::views`, the compile-time validated literals in `unicode_ranges::literals`, and the shared formatting model used by the library-defined UTF-8 and UTF-16 types.

## `views::utf8_view` And `views::utf16_view`

### Synopsis

```cpp
class utf8_view : public std::ranges::view_interface<utf8_view> {
public:
    static constexpr utf8_view from_bytes_unchecked(std::u8string_view base) noexcept;
    constexpr std::u8string_view base() const noexcept;
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

class utf16_view : public std::ranges::view_interface<utf16_view> {
public:
    static constexpr utf16_view from_code_units_unchecked(std::u16string_view base) noexcept;
    constexpr std::u16string_view base() const noexcept;
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};
```

### Behavior

- These views adapt already-validated code-unit sequences into ranges of `utf8_char` or `utf16_char`.
- The views inherit [`std::ranges::view_interface`](https://en.cppreference.com/w/cpp/ranges/view_interface) and model lazy borrowed forward views.
- The views are cheap to copy.
- `reserve_hint()` reports the number of source code units, which is a safe upper bound for the number of yielded characters.

### Return value

Construction returns the helper view directly.

### Complexity

- Constructing the view is constant.
- Iterating the full view is linear in the source length.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

## `views::reversed_utf8_view` And `views::reversed_utf16_view`

### Synopsis

```cpp
class reversed_utf8_view : public std::ranges::view_interface<reversed_utf8_view> {
public:
    static constexpr reversed_utf8_view from_bytes_unchecked(std::u8string_view base) noexcept;
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

class reversed_utf16_view : public std::ranges::view_interface<reversed_utf16_view> {
public:
    static constexpr reversed_utf16_view from_code_units_unchecked(std::u16string_view base) noexcept;
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};
```

### Behavior

- These helper views inherit [`std::ranges::view_interface`](https://en.cppreference.com/w/cpp/ranges/view_interface).
- They are lazy borrowed forward views over the same underlying storage.
- They iterate validated characters from the end without first materializing a reversed string.

### Complexity

- Constructing the view is constant.
- Iterating the full view is linear in the source length.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

### Example

```cpp
--8<-- "examples/reference/helper-views.cpp"
```

## `views::grapheme_cluster_view<CharT>`

### Synopsis

```cpp
template <typename CharT>
class grapheme_cluster_view : public std::ranges::view_interface<grapheme_cluster_view<CharT>> {
public:
    using cluster_type = std::conditional_t<std::same_as<CharT, char8_t>, utf8_string_view, utf16_string_view>;

    static constexpr grapheme_cluster_view
    from_code_units_unchecked(std::basic_string_view<CharT> base) noexcept;

    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};
```

### Behavior

- `grapheme_cluster_view<char8_t>` yields `utf8_string_view` grapheme clusters.
- `grapheme_cluster_view<char16_t>` yields `utf16_string_view` grapheme clusters.
- The view inherits [`std::ranges::view_interface`](https://en.cppreference.com/w/cpp/ranges/view_interface).
- It is a lazy borrowed forward view and computes grapheme boundaries on demand during iteration.

### Complexity

- Constructing the view is constant.
- Iterating the full view is linear in the number of code units plus the segmentation work required by Unicode grapheme rules.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

## `views::lossy_utf8_view` And `views::lossy_utf16_view`

### Synopsis

```cpp
template <typename CharT>
class lossy_utf8_view : public std::ranges::view_interface<lossy_utf8_view<CharT>> {
public:
    lossy_utf8_view() = default;
    constexpr lossy_utf8_view(std::basic_string_view<CharT> base) noexcept;
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

template <typename CharT>
class lossy_utf16_view : public std::ranges::view_interface<lossy_utf16_view<CharT>> {
public:
    lossy_utf16_view() = default;
    constexpr lossy_utf16_view(std::basic_string_view<CharT> base) noexcept;
    constexpr iterator begin() const noexcept;
    constexpr std::default_sentinel_t end() const noexcept;
    constexpr std::size_t reserve_hint() const noexcept;
};

struct lossy_utf8_fn : std::ranges::range_adaptor_closure<lossy_utf8_fn> {
    template<lossy_utf8_viewable_range R>
    constexpr auto operator()(R&& range) const noexcept;
};

struct lossy_utf16_fn : std::ranges::range_adaptor_closure<lossy_utf16_fn> {
    template<lossy_utf16_viewable_range R>
    constexpr auto operator()(R&& range) const noexcept;
};

inline constexpr lossy_utf8_fn lossy_utf8{};
inline constexpr lossy_utf16_fn lossy_utf16{};
```

### Behavior

- Lossy views adapt possibly-invalid UTF input into a character range.
- Invalid units are replaced with `replacement_character`.
- Valid subsequences are yielded unchanged.
- The view types inherit [`std::ranges::view_interface`](https://en.cppreference.com/w/cpp/ranges/view_interface) and behave as lazy borrowed forward views.
- The closure objects are [`std::ranges::range_adaptor_closure`](https://en.cppreference.com/w/cpp/ranges/range_adaptor_closure)-style adapters, which makes the lossy views pipe-friendly.

### Complexity

Linear in the source length.

### Exceptions

None.

### `noexcept`

All listed members are `noexcept`.

## Compile-Time Validated Literals

### Synopsis

```cpp
using namespace unicode_ranges::literals;

consteval utf8_char operator ""_u8c();
consteval utf16_char operator ""_u16c();

consteval utf8_string_view operator ""_utf8_sv();
consteval utf16_string_view operator ""_utf16_sv();

constexpr utf8_string operator ""_utf8_s();
constexpr utf16_string operator ""_utf16_s();

consteval utf8_string_view operator ""_grapheme_utf8();
consteval utf16_string_view operator ""_grapheme_utf16();
```

### Behavior

- `_u8c` and `_u16c` require exactly one valid character in the corresponding encoding.
- `_utf8_sv` and `_utf16_sv` require fully valid UTF literals.
- `_utf8_s` and `_utf16_s` build owning strings from validated literals.
- `_grapheme_utf8` and `_grapheme_utf16` require exactly one grapheme cluster.

### Return value

Returns the corresponding validated character, view, or owning string.

### Exceptions

Invalid literal contents are rejected during constant evaluation.

### Example

```cpp
--8<-- "examples/reference/literals-and-formatting.cpp"
```

## Formatting And Printing

### Synopsis

```cpp
template<> struct std::formatter<utf8_char, char>;
template<> struct std::formatter<utf8_char, wchar_t>;
template<> struct std::formatter<utf16_char, char>;
template<> struct std::formatter<utf16_char, wchar_t>;
template<> struct std::formatter<utf8_string_view, char>;
template<> struct std::formatter<utf16_string_view, char>;
template<typename Allocator> struct std::formatter<basic_utf8_string<Allocator>, char>;
template<typename Allocator> struct std::formatter<basic_utf16_string<Allocator>, char>;
```

### Behavior

- Characters format as text by default.
- Character [`std::formatter`](https://en.cppreference.com/w/cpp/utility/format/formatter) specializations also support numeric presentations `d`, `b`, `B`, `o`, `x`, and `X`, which print `as_scalar()`.
- String and string-view formatters print textual content.
- Standard range formatting composes with the library formatters, which is why examples such as `std::println("{}", text.chars())` work directly.

Two practical printing rules used throughout this documentation:

- `std::println("{}", text.chars())` prints a range of validated characters.
- `std::println("{::s}", text.graphemes())` applies string formatting to each grapheme cluster, which is usually the cleanest textual representation.

### Complexity

Linear in the amount of formatted text.

### Exceptions

Formatter parsing may throw [`std::format_error`](https://en.cppreference.com/w/cpp/utility/format/format_error) for unsupported presentation types.

### `noexcept`

Not `noexcept`.

## Borrowed-Range Status

### Synopsis

```cpp
template <> inline constexpr bool std::ranges::enable_borrowed_range<views::utf8_view> = true;
template <> inline constexpr bool std::ranges::enable_borrowed_range<views::reversed_utf8_view> = true;
template <typename CharT> inline constexpr bool std::ranges::enable_borrowed_range<views::lossy_utf8_view<CharT>> = true;

template <> inline constexpr bool std::ranges::enable_borrowed_range<views::utf16_view> = true;
template <> inline constexpr bool std::ranges::enable_borrowed_range<views::reversed_utf16_view> = true;
template <typename CharT> inline constexpr bool std::ranges::enable_borrowed_range<views::lossy_utf16_view<CharT>> = true;

template <typename CharT> inline constexpr bool std::ranges::enable_borrowed_range<views::grapheme_cluster_view<CharT>> = true;
```

### Behavior

These specializations tell the ranges library that the helper views may safely borrow from the underlying storage instead of forcing owning semantics.

In other words:

- the helper adapters on this page are real range views, implemented as `std::ranges::view_interface` subclasses
- they are lazy rather than eagerly materialized
- and they remain borrowed ranges, so iterators and subviews may refer back to the original storage
