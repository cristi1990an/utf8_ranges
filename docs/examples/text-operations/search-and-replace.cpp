#include "unicode_ranges.hpp"

#include <print>

using namespace unicode_ranges;
using namespace unicode_ranges::literals;

int main()
{
	constexpr auto view = u8"café café"_utf8_sv;
	auto owned = u8"été en été"_utf8_s;

	std::println("view: {}", view);                            // café café
	std::println("find('é'): {}", view.find(u8"é"_u8c));      // 3
	std::println("rfind('é'): {}", view.rfind(u8"é"_u8c));    // 9

	std::println("replace_all('é', 'e'): {}",
		owned.replace_all(u8"é"_u8c, u8"e"_u8c));               // ete en ete
}
