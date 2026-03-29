#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto utf8_text = "😄🇷🇴✨"_utf8_sv;
	constexpr auto utf16_text = u"😄🇷🇴✨"_utf16_sv;

	const auto utf8_chars = utf8_text.chars();
	const auto utf8_reversed = utf8_text.reversed_chars();
	const auto utf8_graphemes = utf8_text.graphemes();

	const auto utf16_chars = utf16_text.chars();
	const auto utf16_reversed = utf16_text.reversed_chars();
	const auto utf16_graphemes = utf16_text.graphemes();

	std::println("{}", utf8_chars);            // [😄, 🇷, 🇴, ✨]
	std::println("{}", utf8_reversed);         // [✨, 🇴, 🇷, 😄]
	std::println("{::s}", utf8_graphemes);     // [😄, 🇷🇴, ✨]

	std::println("{}", utf16_chars);           // [😄, 🇷, 🇴, ✨]
	std::println("{}", utf16_reversed);        // [✨, 🇴, 🇷, 😄]
	std::println("{::s}", utf16_graphemes);    // [😄, 🇷🇴, ✨]

	std::println("{}", "✨"_u8c);               // ✨
	std::println("{}", "🇷🇴"_grapheme_utf8);    // 🇷🇴
}
