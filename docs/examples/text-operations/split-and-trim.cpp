#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto line = u8" café | thé | apă "_utf8_sv;
	constexpr auto framed = u8"***café***"_utf8_sv;

	for (auto part : line.split_trimmed(u8"|"_utf8_sv))
	{
		std::println("[{}]", part);
	}
	// [café]
	// [thé]
	// [apă]

	std::println("trim('*'): {}", framed.trim_matches(u8"*"_u8c)); // café
}
