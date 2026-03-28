#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = u8"😄🇷🇴✨"_utf8_sv;
	constexpr auto sparkle = u8"✨"_u8c;
	constexpr auto flag = u8"🇷🇴"_grapheme_utf8;
	auto owned = u8"😄✨"_utf8_s;

	std::println("text: {}", text);                                // 😄🇷🇴✨
	std::println("find('✨'): {}", text.find(sparkle));            // 12
	std::println("chars(): {}", text.chars());                     // [😄, 🇷, 🇴, ✨]
	std::println("graphemes(): {::s}", text.graphemes());          // [😄, 🇷🇴, ✨]
	std::println("grapheme literal: {}", flag);                    // 🇷🇴
	std::println("replace('✨', '🔥'): {}", owned.replace_all(u8"✨"_u8c, u8"🔥"_u8c)); // 😄🔥
}
