#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto composed = u8"é"_utf8_sv;
	constexpr auto decomposed = u8"é"_utf8_sv;

	std::println("NFD(é): {}", composed.to_nfd());      // é
	std::println("NFC(é): {}", decomposed.to_nfc());   // é
	std::println("NFKC(Ａ): {}", u8"Ａ"_utf8_sv.to_nfkc()); // A
}
