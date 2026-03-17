#include <windows.h>
#include <clocale>

#include "utf8_ranges.hpp"
#include "utf8_ranges_tests.hpp"

#include <print>
#include <iostream>

using namespace literals;

namespace
{
	void enable_utf8_console()
	{
		SetConsoleOutputCP(CP_UTF8);
		SetConsoleCP(CP_UTF8);
		std::setlocale(LC_ALL, ".UTF-8");
	}
}

int main()
{
	enable_utf8_console();

	constexpr auto text = "café €"_utf8_sv;

	static_assert(text.as_view() == u8"café €");

	// `size()` counts UTF-8 code units.
	static_assert(text.size() == 9);
	static_assert(text.as_view() == std::u8string{ 0x63,0x61,0x66,0xC3,0xA9,0x20,0xE2,0x82,0xAC });

	// `char_count()` counts Unicode scalar values.
	static_assert(text.char_count() == 6);
	static_assert(std::ranges::equal(text.chars(), std::array{ "c"_u8c, "a"_u8c, "f"_u8c, "é"_u8c, " "_u8c, "€"_u8c }));

	static_assert(text.front() == 'c');
	static_assert(text.back() == "€"_u8c);

	// `find()` returns a byte offset.
	static_assert(text.find("€"_u8c) == 6);

}
