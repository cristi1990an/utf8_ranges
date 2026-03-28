#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto composed = u8"é"_utf8_sv;
	constexpr auto decomposed = u8"é"_utf8_sv;

	std::println("\"é\" is NFC: {}", composed.is_nfc());    // true
	std::println("\"é\" is NFC: {}", decomposed.is_nfc()); // false
	std::println("\"é\" is NFD: {}", decomposed.is_nfd()); // true
}
