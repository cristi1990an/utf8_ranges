use std::char;

fn collect_ranges<F: Fn(char) -> bool>(pred: F) -> Vec<(u32, u32)> {
    let mut ranges = Vec::new();
    let mut start: Option<u32> = None;
    let mut prev = 0u32;

    for scalar in 0u32..=0x10FFFF {
        if (0xD800..=0xDFFF).contains(&scalar) {
            continue;
        }

        let ch = char::from_u32(scalar).unwrap();
        let matches = pred(ch);
        match (start, matches) {
            (None, true) => {
                start = Some(scalar);
                prev = scalar;
            }
            (Some(_), true) if scalar == prev + 1 => {
                prev = scalar;
            }
            (Some(s), true) => {
                ranges.push((s, prev));
                start = Some(scalar);
                prev = scalar;
            }
            (Some(s), false) => {
                ranges.push((s, prev));
                start = None;
            }
            (None, false) => {}
        }
    }

    if let Some(s) = start {
        ranges.push((s, prev));
    }

    ranges
}

fn emit_ranges(name: &str, ranges: &[(u32, u32)]) {
    println!("inline constexpr std::array<unicode_range, {}> {}{{{{", ranges.len(), name);
    for &(first, last) in ranges {
        println!("    {{ 0x{first:04X}u, 0x{last:04X}u }},");
    }
    println!("}}}};");
    println!();
}

fn main() {
    let version = char::UNICODE_VERSION;

    println!("#pragma once");
    println!();
    println!("namespace details::unicode");
    println!("{{");
    println!("struct unicode_range");
    println!("{{");
    println!("    std::uint32_t first;");
    println!("    std::uint32_t last;");
    println!("}};");
    println!();
    println!(
        "inline constexpr std::tuple<std::size_t, std::size_t, std::size_t> unicode_version{{ {}, {}, {} }};",
        version.0,
        version.1,
        version.2
    );
    println!();
    println!("template <std::size_t N>");
    println!("constexpr bool in_ranges(std::uint32_t scalar, const std::array<unicode_range, N>& ranges) noexcept");
    println!("{{");
    println!("    std::size_t left = 0;");
    println!("    std::size_t right = N;");
    println!("    while (left < right)");
    println!("    {{");
    println!("        const std::size_t mid = left + (right - left) / 2;");
    println!("        const unicode_range range = ranges[mid];");
    println!("        if (scalar < range.first)");
    println!("        {{");
    println!("            right = mid;");
    println!("        }}");
    println!("        else if (scalar > range.last)");
    println!("        {{");
    println!("            left = mid + 1;");
    println!("        }}");
    println!("        else");
    println!("        {{");
    println!("            return true;");
    println!("        }}");
    println!("    }}");
    println!("    return false;");
    println!("}}");
    println!();

    emit_ranges("alphabetic_ranges", &collect_ranges(|ch| ch.is_alphabetic()));
    emit_ranges("lowercase_ranges", &collect_ranges(|ch| ch.is_lowercase()));
    emit_ranges("uppercase_ranges", &collect_ranges(|ch| ch.is_uppercase()));
    emit_ranges("whitespace_ranges", &collect_ranges(|ch| ch.is_whitespace()));
    emit_ranges("control_ranges", &collect_ranges(|ch| ch.is_control()));
    emit_ranges("numeric_ranges", &collect_ranges(|ch| ch.is_numeric()));
    emit_ranges("digit_ranges", &collect_ranges(|ch| ch.is_digit(10)));

    println!("constexpr bool is_alphabetic(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return in_ranges(scalar, alphabetic_ranges);");
    println!("}}");
    println!();
    println!("constexpr bool is_lowercase(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return in_ranges(scalar, lowercase_ranges);");
    println!("}}");
    println!();
    println!("constexpr bool is_uppercase(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return in_ranges(scalar, uppercase_ranges);");
    println!("}}");
    println!();
    println!("constexpr bool is_whitespace(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return in_ranges(scalar, whitespace_ranges);");
    println!("}}");
    println!();
    println!("constexpr bool is_control(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return in_ranges(scalar, control_ranges);");
    println!("}}");
    println!();
    println!("constexpr bool is_numeric(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return in_ranges(scalar, numeric_ranges);");
    println!("}}");
    println!();
    println!("constexpr bool is_digit(std::uint32_t scalar) noexcept");
    println!("{{");
    println!("    return in_ranges(scalar, digit_ranges);");
    println!("}}");
    println!("}}");
}
