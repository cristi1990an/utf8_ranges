#if defined(UTF8_RANGES_UTF8_STRING_HPP) && defined(UTF8_RANGES_UTF16_STRING_HPP) && !defined(UTF8_RANGES_TRANSCODING_HPP)
#define UTF8_RANGES_TRANSCODING_HPP

namespace unicode_ranges
{
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::append_range(views::utf8_view rg)
	{
		return append_bytes(rg.base());
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::append_range(views::utf16_view rg)
	{
		const auto code_units = rg.base();
		const auto original_size = base_.size();
		base_.resize_and_overwrite(original_size + code_units.size() * details::encoding_constants::three_code_unit_count,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = original_size;
				std::size_t read_index = 0;
				while (read_index < code_units.size())
				{
					const auto remaining = std::u16string_view{ code_units.data() + read_index, code_units.size() - read_index };
					const auto ascii_run = details::ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						details::copy_ascii_utf16_to_utf8(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto first = static_cast<std::uint16_t>(code_units[read_index]);
					const auto count = details::is_utf16_high_surrogate(first)
						? details::encoding_constants::utf16_surrogate_code_unit_count
						: details::encoding_constants::single_code_unit_count;
					const auto scalar = details::decode_valid_utf16_char(code_units.data() + read_index, count);
					write_index += details::encode_unicode_scalar_utf8_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});

		return *this;
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::assign_range(views::utf8_view rg)
	{
		const auto bytes = rg.base();
		if (overlaps_base(bytes))
		{
			base_type replacement{ bytes, base_.get_allocator() };
			base_ = std::move(replacement);
		}
		else
		{
			base_.assign(bytes);
		}

		return *this;
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator>& basic_utf8_string<Allocator>::assign_range(views::utf16_view rg)
	{
		base_type replacement{ base_.get_allocator() };
		const auto code_units = rg.base();
		replacement.resize_and_overwrite(code_units.size() * details::encoding_constants::three_code_unit_count,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < code_units.size())
				{
					const auto remaining = std::u16string_view{ code_units.data() + read_index, code_units.size() - read_index };
					const auto ascii_run = details::ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						details::copy_ascii_utf16_to_utf8(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto first = static_cast<std::uint16_t>(code_units[read_index]);
					const auto count = details::is_utf16_high_surrogate(first)
						? details::encoding_constants::utf16_surrogate_code_unit_count
						: details::encoding_constants::single_code_unit_count;
					const auto scalar = details::decode_valid_utf16_char(code_units.data() + read_index, count);
					write_index += details::encode_unicode_scalar_utf8_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});
		base_ = std::move(replacement);
		return *this;
	}

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
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::append_range(views::utf16_view rg)
	{
		return append_code_units(rg.base());
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::append_range(views::utf8_view rg)
	{
		const auto bytes = rg.base();
		const auto original_size = base_.size();
		base_.resize_and_overwrite(original_size + bytes.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = original_size;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto remaining = std::u8string_view{ bytes.data() + read_index, bytes.size() - read_index };
					const auto ascii_run = details::ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						details::copy_ascii_utf8_to_utf16(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					const auto scalar = details::decode_valid_utf8_char(bytes.data() + read_index, count);
					write_index += details::encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});

		return *this;
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::assign_range(views::utf16_view rg)
	{
		const auto code_units = rg.base();
		if (overlaps_base(code_units))
		{
			base_type replacement{ code_units, base_.get_allocator() };
			base_ = std::move(replacement);
		}
		else
		{
			base_.assign(code_units);
		}

		return *this;
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator>& basic_utf16_string<Allocator>::assign_range(views::utf8_view rg)
	{
		base_type replacement{ base_.get_allocator() };
		const auto bytes = rg.base();
		replacement.resize_and_overwrite(bytes.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				std::size_t read_index = 0;
				while (read_index < bytes.size())
				{
					const auto remaining = std::u8string_view{ bytes.data() + read_index, bytes.size() - read_index };
					const auto ascii_run = details::ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						details::copy_ascii_utf8_to_utf16(buffer + write_index, remaining.substr(0, ascii_run));
						write_index += ascii_run;
						read_index += ascii_run;
						continue;
					}

					const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read_index]));
					const auto scalar = details::decode_valid_utf8_char(bytes.data() + read_index, count);
					write_index += details::encode_unicode_scalar_utf16_unchecked(scalar, buffer + write_index);
					read_index += count;
				}

				return write_index;
			});
		base_ = std::move(replacement);
		return *this;
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
	inline constexpr std::size_t unicode_scalar_utf8_size(std::uint32_t scalar) noexcept
	{
		if (scalar <= encoding_constants::ascii_scalar_max) [[likely]]
		{
			return encoding_constants::single_code_unit_count;
		}

		if (scalar <= encoding_constants::two_byte_scalar_max)
		{
			return encoding_constants::two_code_unit_count;
		}

		if (scalar <= encoding_constants::bmp_scalar_max)
		{
			return encoding_constants::three_code_unit_count;
		}

		return encoding_constants::max_utf8_code_units;
	}

	inline constexpr std::size_t unicode_scalar_utf16_size(std::uint32_t scalar) noexcept
	{
		return scalar <= encoding_constants::bmp_scalar_max
			? encoding_constants::single_code_unit_count
			: encoding_constants::utf16_surrogate_code_unit_count;
	}

	inline constexpr std::uint32_t lowercase_ascii_scalar(std::uint32_t scalar) noexcept
	{
		return (scalar >= static_cast<std::uint32_t>('A') && scalar <= static_cast<std::uint32_t>('Z'))
			? (scalar + (static_cast<std::uint32_t>('a') - static_cast<std::uint32_t>('A')))
			: scalar;
	}

	inline constexpr std::uint32_t uppercase_ascii_scalar(std::uint32_t scalar) noexcept
	{
		return (scalar >= static_cast<std::uint32_t>('a') && scalar <= static_cast<std::uint32_t>('z'))
			? (scalar - (static_cast<std::uint32_t>('a') - static_cast<std::uint32_t>('A')))
			: scalar;
	}

	template <bool Lowercase>
	inline constexpr std::uint32_t ascii_case_scalar(std::uint32_t scalar) noexcept
	{
		if constexpr (Lowercase)
		{
			return lowercase_ascii_scalar(scalar);
		}
		else
		{
			return uppercase_ascii_scalar(scalar);
		}
	}

	template <bool Lowercase, typename BaseType>
	constexpr bool try_case_map_utf8_same_size(
		std::u8string_view bytes,
		BaseType& result,
		const unicode::unicode_case_mapping* (*lookup)(std::uint32_t) noexcept)
	{
		bool same_size = true;
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t index = 0; index < bytes.size();)
				{
					const auto remaining = std::u8string_view{ bytes.data() + index, bytes.size() - index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						if constexpr (Lowercase)
						{
							ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}
						else
						{
							ascii_uppercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}

						write_index += ascii_run;
						index += ascii_run;
						continue;
					}

					const auto decoded = decode_next_scalar(bytes, index);
					const auto input_size = decoded.next_index - index;
					if (const auto* mapping = lookup(decoded.scalar); mapping != nullptr)
					{
						if (mapping->count != 1u)
						{
							same_size = false;
							return write_index;
						}

						const auto mapped = mapping->mapped[0];
						if (unicode_scalar_utf8_size(mapped) != input_size)
						{
							same_size = false;
							return write_index;
						}

						write_index += encode_unicode_scalar_utf8_unchecked(mapped, buffer + write_index);
					}
					else
					{
						std::char_traits<char8_t>::copy(buffer + write_index, bytes.data() + index, input_size);
						write_index += input_size;
					}

					index = decoded.next_index;
				}

				return write_index;
			});

		return same_size;
	}

	template <bool Lowercase, typename BaseType>
	constexpr bool try_case_map_utf16_same_size(
		std::u16string_view code_units,
		BaseType& result,
		const unicode::unicode_case_mapping* (*lookup)(std::uint32_t) noexcept)
	{
		bool same_size = true;
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t index = 0; index < code_units.size();)
				{
					const auto remaining = std::u16string_view{ code_units.data() + index, code_units.size() - index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						if constexpr (Lowercase)
						{
							ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}
						else
						{
							ascii_uppercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}

						write_index += ascii_run;
						index += ascii_run;
						continue;
					}

					const auto decoded = decode_next_scalar(code_units, index);
					const auto input_size = decoded.next_index - index;
					if (const auto* mapping = lookup(decoded.scalar); mapping != nullptr)
					{
						if (mapping->count != 1u)
						{
							same_size = false;
							return write_index;
						}

						const auto mapped = mapping->mapped[0];
						if (unicode_scalar_utf16_size(mapped) != input_size)
						{
							same_size = false;
							return write_index;
						}

						write_index += encode_unicode_scalar_utf16_unchecked(mapped, buffer + write_index);
					}
					else
					{
						std::char_traits<char16_t>::copy(buffer + write_index, code_units.data() + index, input_size);
						write_index += input_size;
					}

					index = decoded.next_index;
				}

				return write_index;
			});

		return same_size;
	}

	template <bool Lowercase, typename Allocator>
	constexpr basic_utf8_string<Allocator> case_map_utf8_copy(
		std::u8string_view bytes,
		const Allocator& alloc,
		const unicode::unicode_case_mapping* (*lookup)(std::uint32_t) noexcept)
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;

		if (is_ascii_only(bytes))
		{
			bool changed = false;
			base_type result{ alloc };
			result.resize_and_overwrite(bytes.size(),
				[&](char8_t* buffer, std::size_t) noexcept
				{
					if constexpr (Lowercase)
					{
						changed = ascii_lowercase_copy(buffer, bytes);
					}
					else
					{
						changed = ascii_uppercase_copy(buffer, bytes);
					}

					return bytes.size();
				});

			if (!changed)
			{
				return basic_utf8_string<Allocator>::from_bytes_unchecked(base_type{ bytes, alloc });
			}

			return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
		}

		base_type same_size_result{ alloc };
		if (try_case_map_utf8_same_size<Lowercase>(bytes, same_size_result, lookup))
		{
			return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(same_size_result));
		}

		bool changed = false;
		std::size_t output_size = 0;
		for (std::size_t index = 0; index < bytes.size();)
		{
			const auto remaining = std::u8string_view{ bytes.data() + index, bytes.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				output_size += ascii_run;
				for (std::size_t ascii_index = 0; ascii_index != ascii_run; ++ascii_index)
				{
					const auto scalar = static_cast<std::uint8_t>(remaining[ascii_index]);
					changed = changed || (ascii_case_scalar<Lowercase>(scalar) != scalar);
				}

				index += ascii_run;
				if (index == bytes.size())
				{
					break;
				}
			}

			const auto decoded = decode_next_scalar(bytes, index);
			if (const auto* mapping = lookup(decoded.scalar); mapping != nullptr)
			{
				changed = true;
				for (std::size_t mapped_index = 0; mapped_index != mapping->count; ++mapped_index)
				{
					output_size += unicode_scalar_utf8_size(mapping->mapped[mapped_index]);
				}
			}
			else
			{
				output_size += decoded.next_index - index;
			}

			index = decoded.next_index;
		}

		if (!changed)
		{
			return basic_utf8_string<Allocator>::from_bytes_unchecked(base_type{ bytes, alloc });
		}

		base_type result{ alloc };
		result.resize_and_overwrite(output_size,
			[&](char8_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t index = 0; index < bytes.size();)
				{
					const auto remaining = std::u8string_view{ bytes.data() + index, bytes.size() - index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						if constexpr (Lowercase)
						{
							ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}
						else
						{
							ascii_uppercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}

						write_index += ascii_run;
						index += ascii_run;
						continue;
					}

					const auto decoded = decode_next_scalar(bytes, index);
					if (const auto* mapping = lookup(decoded.scalar); mapping != nullptr)
					{
						for (std::size_t mapped_index = 0; mapped_index != mapping->count; ++mapped_index)
						{
							write_index += encode_unicode_scalar_utf8_unchecked(
								mapping->mapped[mapped_index],
								buffer + write_index);
						}
					}
					else
					{
						std::char_traits<char8_t>::copy(
							buffer + write_index,
							bytes.data() + index,
							decoded.next_index - index);
						write_index += decoded.next_index - index;
					}

					index = decoded.next_index;
				}

				return write_index;
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <bool Lowercase, typename Allocator>
	constexpr basic_utf16_string<Allocator> case_map_utf16_copy(
		std::u16string_view code_units,
		const Allocator& alloc,
		const unicode::unicode_case_mapping* (*lookup)(std::uint32_t) noexcept)
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;

		if (is_ascii_only(code_units))
		{
			bool changed = false;
			base_type result{ alloc };
			result.resize_and_overwrite(code_units.size(),
				[&](char16_t* buffer, std::size_t) noexcept
				{
					if constexpr (Lowercase)
					{
						changed = ascii_lowercase_copy(buffer, code_units);
					}
					else
					{
						changed = ascii_uppercase_copy(buffer, code_units);
					}

					return code_units.size();
				});

			if (!changed)
			{
				return basic_utf16_string<Allocator>::from_code_units_unchecked(base_type{ code_units, alloc });
			}

			return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
		}

		base_type same_size_result{ alloc };
		if (try_case_map_utf16_same_size<Lowercase>(code_units, same_size_result, lookup))
		{
			return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(same_size_result));
		}

		bool changed = false;
		std::size_t output_size = 0;
		for (std::size_t index = 0; index < code_units.size();)
		{
			const auto remaining = std::u16string_view{ code_units.data() + index, code_units.size() - index };
			const auto ascii_run = ascii_prefix_length(remaining);
			if (ascii_run != 0)
			{
				output_size += ascii_run;
				for (std::size_t ascii_index = 0; ascii_index != ascii_run; ++ascii_index)
				{
					const auto scalar = static_cast<std::uint16_t>(remaining[ascii_index]);
					changed = changed || (ascii_case_scalar<Lowercase>(scalar) != scalar);
				}

				index += ascii_run;
				if (index == code_units.size())
				{
					break;
				}
			}

			const auto decoded = decode_next_scalar(code_units, index);
			if (const auto* mapping = lookup(decoded.scalar); mapping != nullptr)
			{
				changed = true;
				for (std::size_t mapped_index = 0; mapped_index != mapping->count; ++mapped_index)
				{
					output_size += unicode_scalar_utf16_size(mapping->mapped[mapped_index]);
				}
			}
			else
			{
				output_size += decoded.next_index - index;
			}

			index = decoded.next_index;
		}

		if (!changed)
		{
			return basic_utf16_string<Allocator>::from_code_units_unchecked(base_type{ code_units, alloc });
		}

		base_type result{ alloc };
		result.resize_and_overwrite(output_size,
			[&](char16_t* buffer, std::size_t) noexcept
			{
				std::size_t write_index = 0;
				for (std::size_t index = 0; index < code_units.size();)
				{
					const auto remaining = std::u16string_view{ code_units.data() + index, code_units.size() - index };
					const auto ascii_run = ascii_prefix_length(remaining);
					if (ascii_run != 0)
					{
						if constexpr (Lowercase)
						{
							ascii_lowercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}
						else
						{
							ascii_uppercase_copy(buffer + write_index, remaining.substr(0, ascii_run));
						}

						write_index += ascii_run;
						index += ascii_run;
						continue;
					}

					const auto decoded = decode_next_scalar(code_units, index);
					if (const auto* mapping = lookup(decoded.scalar); mapping != nullptr)
					{
						for (std::size_t mapped_index = 0; mapped_index != mapping->count; ++mapped_index)
						{
							write_index += encode_unicode_scalar_utf16_unchecked(
								mapping->mapped[mapped_index],
								buffer + write_index);
						}
					}
					else
					{
						std::char_traits<char16_t>::copy(
							buffer + write_index,
							code_units.data() + index,
							decoded.next_index - index);
						write_index += decoded.next_index - index;
					}

					index = decoded.next_index;
				}

				return write_index;
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_utf8_owned(const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>{ View::from_bytes_unchecked(byte_view()), alloc };
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_ascii_lowercase(const Allocator& alloc) const
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		const auto bytes = byte_view();
		base_type result{ alloc };
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				ascii_lowercase_copy(buffer, bytes);
				return bytes.size();
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_ascii_uppercase(const Allocator& alloc) const
	{
		using base_type = typename basic_utf8_string<Allocator>::base_type;
		const auto bytes = byte_view();
		base_type result{ alloc };
		result.resize_and_overwrite(bytes.size(),
			[&](char8_t* buffer, std::size_t) noexcept
			{
				ascii_uppercase_copy(buffer, bytes);
				return bytes.size();
			});

		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_lowercase(const Allocator& alloc) const
	{
		return case_map_utf8_copy<true>(byte_view(), alloc, &unicode::lowercase_mapping);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::to_uppercase(const Allocator& alloc) const
	{
		return case_map_utf8_copy<false>(byte_view(), alloc, &unicode::uppercase_mapping);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf8_string_crtp<Derived, View>::to_utf16(const Allocator& alloc) const
	{
		basic_utf16_string<Allocator> result{ alloc };
		result.append_range(chars());
		return result;
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(utf8_char from, utf8_char to) const
	{
		return replace_all(
			View::from_bytes_unchecked(details::utf8_char_view(from)),
			View::from_bytes_unchecked(details::utf8_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(utf8_char from, View to) const
	{
		return replace_all(View::from_bytes_unchecked(details::utf8_char_view(from)), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(View from, utf8_char to) const
	{
		return replace_all(from, View::from_bytes_unchecked(details::utf8_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(View from, View to) const
	{
		return replace_all(from, to, std::allocator<char8_t>{});
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(utf8_char from, utf8_char to, const Allocator& alloc) const
	{
		return replace_all(
			View::from_bytes_unchecked(details::utf8_char_view(from)),
			View::from_bytes_unchecked(details::utf8_char_view(to)),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(utf8_char from, View to, const Allocator& alloc) const
	{
		return replace_all(View::from_bytes_unchecked(details::utf8_char_view(from)), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(View from, utf8_char to, const Allocator& alloc) const
	{
		return replace_all(from, View::from_bytes_unchecked(details::utf8_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(View from, View to, const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>::from_bytes_unchecked(details::replace_utf8_bytes_copy(
			byte_view(),
			from.base(),
			to.base(),
			npos,
			alloc));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(std::span<const utf8_char> from, utf8_char to) const
	{
		if (from.empty())
		{
			return to_utf8_owned();
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to);
		}

		return replace_all(details::utf8_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(std::span<const utf8_char> from, View to) const
	{
		if (from.empty())
		{
			return to_utf8_owned();
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to);
		}

		return replace_all(details::utf8_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(std::span<const utf8_char> from, utf8_char to, const Allocator& alloc) const
	{
		if (from.empty())
		{
			return to_utf8_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to, alloc);
		}

		return replace_all(details::utf8_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(std::span<const utf8_char> from, View to, const Allocator& alloc) const
	{
		if (from.empty())
		{
			return to_utf8_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to, alloc);
		}

		return replace_all(details::utf8_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, utf8_char to) const
	{
		return replace_n(
			count,
			View::from_bytes_unchecked(details::utf8_char_view(from)),
			View::from_bytes_unchecked(details::utf8_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, View to) const
	{
		return replace_n(count, View::from_bytes_unchecked(details::utf8_char_view(from)), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, View from, utf8_char to) const
	{
		return replace_n(count, from, View::from_bytes_unchecked(details::utf8_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, View from, View to) const
	{
		return replace_n(count, from, to, std::allocator<char8_t>{});
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, utf8_char to, const Allocator& alloc) const
	{
		return replace_n(
			count,
			View::from_bytes_unchecked(details::utf8_char_view(from)),
			View::from_bytes_unchecked(details::utf8_char_view(to)),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, utf8_char from, View to, const Allocator& alloc) const
	{
		return replace_n(count, View::from_bytes_unchecked(details::utf8_char_view(from)), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, View from, utf8_char to, const Allocator& alloc) const
	{
		return replace_n(count, from, View::from_bytes_unchecked(details::utf8_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, View from, View to, const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>::from_bytes_unchecked(details::replace_utf8_bytes_copy(
			byte_view(),
			from.base(),
			to.base(),
			count,
			alloc));
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf8_char> from, utf8_char to) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf8_owned();
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to);
		}

		return replace_n(count, details::utf8_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf8_char> from, View to) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf8_owned();
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to);
		}

		return replace_n(count, details::utf8_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf8_char> from, utf8_char to, const Allocator& alloc) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf8_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to, alloc);
		}

		return replace_n(count, details::utf8_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf8_char> from, View to, const Allocator& alloc) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf8_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to, alloc);
		}

		return replace_n(count, details::utf8_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(Pred pred, utf8_char to) const
	{
		return replace_all(std::move(pred), View::from_bytes_unchecked(details::utf8_char_view(to)));
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_all(Pred pred, View to) const
	{
		return replace_all(std::move(pred), to, std::allocator<char8_t>{});
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred, typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(Pred pred, utf8_char to, const Allocator& alloc) const
	{
		return replace_all(std::move(pred), View::from_bytes_unchecked(details::utf8_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred, typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_all(Pred pred, View to, const Allocator& alloc) const
	{
		return replace_n(npos, std::move(pred), to, alloc);
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, utf8_char to) const
	{
		return replace_n(count, std::move(pred), View::from_bytes_unchecked(details::utf8_char_view(to)));
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred>
	constexpr basic_utf8_string<> utf8_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, View to) const
	{
		return replace_n(count, std::move(pred), to, std::allocator<char8_t>{});
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred, typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, utf8_char to, const Allocator& alloc) const
	{
		return replace_n(count, std::move(pred), View::from_bytes_unchecked(details::utf8_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <details::utf8_char_predicate Pred, typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, View to, const Allocator& alloc) const
	{
		if (count == 0)
		{
			return to_utf8_owned(alloc);
		}

		const auto bytes = byte_view();
		const auto replacement = to.base();
		size_type replacements = 0;
		size_type replaced_size = 0;
		for (size_type pos = 0; pos < bytes.size() && replacements != count; )
		{
			const auto char_size = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[pos]));
			const auto ch = utf8_char::from_utf8_bytes_unchecked(bytes.data() + pos, char_size);
			if (static_cast<bool>(std::invoke(pred, ch)))
			{
				++replacements;
				replaced_size += char_size;
			}
			pos += char_size;
		}

		if (replacements == 0)
		{
			return to_utf8_owned(alloc);
		}

		using result_base = typename basic_utf8_string<Allocator>::base_type;
		result_base result{ alloc };
		const auto result_size = bytes.size() - replaced_size + replacements * replacement.size();
		result.resize_and_overwrite(result_size,
			[&](char8_t* buffer, std::size_t) constexpr
			{
				size_type read = 0;
				size_type write = 0;
				size_type replaced = 0;
				while (read < bytes.size())
				{
					const auto char_size = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(bytes[read]));
					const auto ch = utf8_char::from_utf8_bytes_unchecked(bytes.data() + read, char_size);
					if (replaced != count && static_cast<bool>(std::invoke(pred, ch)))
					{
						if (!replacement.empty())
						{
							std::char_traits<char8_t>::copy(buffer + write, replacement.data(), replacement.size());
						}
						write += replacement.size();
						++replaced;
					}
					else
					{
						std::char_traits<char8_t>::copy(buffer + write, bytes.data() + read, char_size);
						write += char_size;
					}

					read += char_size;
				}

				return write;
			});
		return basic_utf8_string<Allocator>::from_bytes_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_utf16_owned(const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>{ View::from_code_units_unchecked(code_unit_view()), alloc };
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_ascii_lowercase(const Allocator& alloc) const
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		const auto code_units = code_unit_view();
		base_type result{ alloc };
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				ascii_lowercase_copy(buffer, code_units);
				return code_units.size();
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_ascii_uppercase(const Allocator& alloc) const
	{
		using base_type = typename basic_utf16_string<Allocator>::base_type;
		const auto code_units = code_unit_view();
		base_type result{ alloc };
		result.resize_and_overwrite(code_units.size(),
			[&](char16_t* buffer, std::size_t) noexcept
			{
				ascii_uppercase_copy(buffer, code_units);
				return code_units.size();
			});

		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_lowercase(const Allocator& alloc) const
	{
		return case_map_utf16_copy<true>(code_unit_view(), alloc, &unicode::lowercase_mapping);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::to_uppercase(const Allocator& alloc) const
	{
		return case_map_utf16_copy<false>(code_unit_view(), alloc, &unicode::uppercase_mapping);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf16_string_crtp<Derived, View>::to_utf8(const Allocator& alloc) const
	{
		basic_utf8_string<Allocator> result{ alloc };
		result.append_range(chars());
		return result;
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(utf16_char from, utf16_char to) const
	{
		return replace_all(
			View::from_code_units_unchecked(details::utf16_char_view(from)),
			View::from_code_units_unchecked(details::utf16_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(utf16_char from, View to) const
	{
		return replace_all(View::from_code_units_unchecked(details::utf16_char_view(from)), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(View from, utf16_char to) const
	{
		return replace_all(from, View::from_code_units_unchecked(details::utf16_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(View from, View to) const
	{
		return replace_all(from, to, std::allocator<char16_t>{});
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(utf16_char from, utf16_char to, const Allocator& alloc) const
	{
		return replace_all(
			View::from_code_units_unchecked(details::utf16_char_view(from)),
			View::from_code_units_unchecked(details::utf16_char_view(to)),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(utf16_char from, View to, const Allocator& alloc) const
	{
		return replace_all(View::from_code_units_unchecked(details::utf16_char_view(from)), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(View from, utf16_char to, const Allocator& alloc) const
	{
		return replace_all(from, View::from_code_units_unchecked(details::utf16_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(View from, View to, const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>::from_code_units_unchecked(details::replace_utf16_code_units_copy(
			code_unit_view(),
			from.base(),
			to.base(),
			npos,
			alloc));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(std::span<const utf16_char> from, utf16_char to) const
	{
		if (from.empty())
		{
			return to_utf16_owned();
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to);
		}

		return replace_all(details::utf16_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(std::span<const utf16_char> from, View to) const
	{
		if (from.empty())
		{
			return to_utf16_owned();
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to);
		}

		return replace_all(details::utf16_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(std::span<const utf16_char> from, utf16_char to, const Allocator& alloc) const
	{
		if (from.empty())
		{
			return to_utf16_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to, alloc);
		}

		return replace_all(details::utf16_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(std::span<const utf16_char> from, View to, const Allocator& alloc) const
	{
		if (from.empty())
		{
			return to_utf16_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_all(from.front(), to, alloc);
		}

		return replace_all(details::utf16_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, utf16_char to) const
	{
		return replace_n(
			count,
			View::from_code_units_unchecked(details::utf16_char_view(from)),
			View::from_code_units_unchecked(details::utf16_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, View to) const
	{
		return replace_n(count, View::from_code_units_unchecked(details::utf16_char_view(from)), to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, View from, utf16_char to) const
	{
		return replace_n(count, from, View::from_code_units_unchecked(details::utf16_char_view(to)));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, View from, View to) const
	{
		return replace_n(count, from, to, std::allocator<char16_t>{});
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, utf16_char to, const Allocator& alloc) const
	{
		return replace_n(
			count,
			View::from_code_units_unchecked(details::utf16_char_view(from)),
			View::from_code_units_unchecked(details::utf16_char_view(to)),
			alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, utf16_char from, View to, const Allocator& alloc) const
	{
		return replace_n(count, View::from_code_units_unchecked(details::utf16_char_view(from)), to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, View from, utf16_char to, const Allocator& alloc) const
	{
		return replace_n(count, from, View::from_code_units_unchecked(details::utf16_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, View from, View to, const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>::from_code_units_unchecked(details::replace_utf16_code_units_copy(
			code_unit_view(),
			from.base(),
			to.base(),
			count,
			alloc));
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf16_char> from, utf16_char to) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf16_owned();
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to);
		}

		return replace_n(count, details::utf16_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf16_char> from, View to) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf16_owned();
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to);
		}

		return replace_n(count, details::utf16_char_span_matcher{ from }, to);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf16_char> from, utf16_char to, const Allocator& alloc) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf16_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to, alloc);
		}

		return replace_n(count, details::utf16_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, std::span<const utf16_char> from, View to, const Allocator& alloc) const
	{
		if (count == 0 || from.empty())
		{
			return to_utf16_owned(alloc);
		}

		if (from.size() == 1)
		{
			return replace_n(count, from.front(), to, alloc);
		}

		return replace_n(count, details::utf16_char_span_matcher{ from }, to, alloc);
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(Pred pred, utf16_char to) const
	{
		return replace_all(std::move(pred), View::from_code_units_unchecked(details::utf16_char_view(to)));
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_all(Pred pred, View to) const
	{
		return replace_all(std::move(pred), to, std::allocator<char16_t>{});
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred, typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(Pred pred, utf16_char to, const Allocator& alloc) const
	{
		return replace_all(std::move(pred), View::from_code_units_unchecked(details::utf16_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred, typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_all(Pred pred, View to, const Allocator& alloc) const
	{
		return replace_n(npos, std::move(pred), to, alloc);
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, utf16_char to) const
	{
		return replace_n(count, std::move(pred), View::from_code_units_unchecked(details::utf16_char_view(to)));
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred>
	constexpr basic_utf16_string<> utf16_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, View to) const
	{
		return replace_n(count, std::move(pred), to, std::allocator<char16_t>{});
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred, typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, utf16_char to, const Allocator& alloc) const
	{
		return replace_n(count, std::move(pred), View::from_code_units_unchecked(details::utf16_char_view(to)), alloc);
	}

	template <typename Derived, typename View>
	template <details::utf16_char_predicate Pred, typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_string_crtp<Derived, View>::replace_n(size_type count, Pred pred, View to, const Allocator& alloc) const
	{
		if (count == 0)
		{
			return to_utf16_owned(alloc);
		}

		const auto code_units = code_unit_view();
		const auto replacement = to.base();
		size_type replacements = 0;
		size_type replaced_size = 0;
		for (size_type pos = 0; pos < code_units.size() && replacements != count; )
		{
			const auto first = static_cast<std::uint16_t>(code_units[pos]);
			const auto char_size = details::is_utf16_high_surrogate(first) ? 2u : 1u;
			const auto ch = utf16_char::from_utf16_code_units_unchecked(code_units.data() + pos, char_size);
			if (static_cast<bool>(std::invoke(pred, ch)))
			{
				++replacements;
				replaced_size += char_size;
			}
			pos += char_size;
		}

		if (replacements == 0)
		{
			return to_utf16_owned(alloc);
		}

		using result_base = typename basic_utf16_string<Allocator>::base_type;
		result_base result{ alloc };
		const auto result_size = code_units.size() - replaced_size + replacements * replacement.size();
		result.resize_and_overwrite(result_size,
			[&](char16_t* buffer, std::size_t) constexpr
			{
				size_type read = 0;
				size_type write = 0;
				size_type replaced = 0;
				while (read < code_units.size())
				{
					const auto first = static_cast<std::uint16_t>(code_units[read]);
					const auto char_size = details::is_utf16_high_surrogate(first) ? 2u : 1u;
					const auto ch = utf16_char::from_utf16_code_units_unchecked(code_units.data() + read, char_size);
					if (replaced != count && static_cast<bool>(std::invoke(pred, ch)))
					{
						if (!replacement.empty())
						{
							std::char_traits<char16_t>::copy(buffer + write, replacement.data(), replacement.size());
						}
						write += replacement.size();
						++replaced;
					}
					else
					{
						std::char_traits<char16_t>::copy(buffer + write, code_units.data() + read, char_size);
						write += char_size;
					}

					read += char_size;
				}

				return write;
			});
		return basic_utf16_string<Allocator>::from_code_units_unchecked(std::move(result));
	}
	}

	template <typename Allocator>
	constexpr basic_utf8_string<Allocator> utf8_char::to_utf8_owned(const Allocator& alloc) const
	{
		return basic_utf8_string<Allocator>{
			utf8_string_view::from_bytes_unchecked(details::utf8_char_view(*this)),
			alloc
		};
	}

	template <typename Allocator>
	constexpr basic_utf16_string<Allocator> utf16_char::to_utf16_owned(const Allocator& alloc) const
	{
		return basic_utf16_string<Allocator>{
			utf16_string_view::from_code_units_unchecked(details::utf16_char_view(*this)),
			alloc
		};
	}

}

#endif // UTF8_RANGES_TRANSCODING_HPP
