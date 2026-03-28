#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto text = u8"straße café"_utf8_sv;

	std::println("ASCII upper: {}", text.to_ascii_uppercase()); // STRAßE CAFé
	std::println("Unicode upper: {}", text.to_uppercase());     // STRASSE CAFÉ
	std::println("Unicode lower: {}", u8"CAFÉ Ω"_utf8_sv.to_lowercase()); // café ω
}
