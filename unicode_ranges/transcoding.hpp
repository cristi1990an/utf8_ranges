#if defined(UTF8_RANGES_UTF8_STRING_HPP) && defined(UTF8_RANGES_UTF16_STRING_HPP) && !defined(UTF8_RANGES_TRANSCODING_HPP)
#define UTF8_RANGES_TRANSCODING_HPP

namespace unicode_ranges
{
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>::basic_utf8_string(utf16_string_view view, const Allocator& alloc)
		: base_(alloc)
	{
		append_range(view.chars());
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::operator+=(utf16_string_view sv)
	{
		return append_range(sv.chars());
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>::basic_utf16_string(utf8_string_view view, const Allocator& alloc)
		: base_(alloc)
	{
		append_range(view.chars());
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::operator+=(utf8_string_view sv)
	{
		return append_range(sv.chars());
	}

	namespace details
	{
	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf8_string_crtp<Derived, View>::to_utf16(const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>(std::from_range, chars(), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf16_string_crtp<Derived, View>::to_utf8(const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>(std::from_range, chars(), alloc);
	}
	}

}

#endif // UTF8_RANGES_TRANSCODING_HPP
