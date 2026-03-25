#ifndef UTF8_RANGES_UTF8_STRING_CRTP_HPP
#define UTF8_RANGES_UTF8_STRING_CRTP_HPP

#include "utf8_views.hpp"

namespace unicode_ranges
{

namespace details
{

class utf8_char_indices_view : public std::ranges::view_interface<utf8_char_indices_view>
{
public:
	static constexpr utf8_char_indices_view from_bytes_unchecked(std::u8string_view base) noexcept
	{
		return utf8_char_indices_view{ base };
	}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = std::pair<std::size_t, utf8_char>;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(std::u8string_view base, std::size_t current) noexcept
			: base_(base), current_(current)
		{}

		constexpr reference operator*() const noexcept
		{
			const auto count = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base_[current_]));
			return { current_, utf8_char::from_utf8_bytes_unchecked(base_.data() + current_, count) };
		}

		constexpr iterator& operator++() noexcept
		{
			current_ += details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base_[current_]));
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == it.base_.size();
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == it.base_.size();
		}

	private:
		std::u8string_view base_{};
		std::size_t current_ = 0;
	};

	constexpr iterator begin() const noexcept
	{
		return iterator{ base_, 0 };
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit utf8_char_indices_view(std::u8string_view base) noexcept
		: base_(base)
	{}

	std::u8string_view base_{};
};

template <typename View>
class utf8_grapheme_indices_view : public std::ranges::view_interface<utf8_grapheme_indices_view<View>>
{
public:
	static constexpr utf8_grapheme_indices_view from_bytes_unchecked(std::u8string_view base) noexcept
	{
		return utf8_grapheme_indices_view{ base };
	}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = std::pair<std::size_t, View>;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(std::u8string_view base, std::size_t current, std::size_t next) noexcept
			: base_(base), current_(current), next_(next)
		{}

		constexpr reference operator*() const noexcept
		{
			return value_type{
				current_,
				View::from_bytes_unchecked(base_.substr(current_, next_ - current_))
			};
		}

		constexpr iterator& operator++() noexcept
		{
			current_ = next_;
			if (current_ != base_.size())
			{
				next_ = details::next_grapheme_boundary(base_, current_);
			}
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == it.base_.size();
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == it.base_.size();
		}

	private:
		std::u8string_view base_{};
		std::size_t current_ = 0;
		std::size_t next_ = 0;
	};

	constexpr iterator begin() const noexcept
	{
		if (base_.empty())
		{
			return iterator{ base_, base_.size(), base_.size() };
		}

		return iterator{ base_, 0, details::next_grapheme_boundary(base_, 0) };
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit utf8_grapheme_indices_view(std::u8string_view base) noexcept
		: base_(base)
	{}

	std::u8string_view base_{};
};

inline constexpr std::size_t find_utf8_split_delimiter(
	std::u8string_view base,
	std::u8string_view delimiter,
	std::size_t pos) noexcept
{
	if (delimiter.empty())
	{
		return std::u8string_view::npos;
	}

	if (pos > base.size() || delimiter.size() > base.size() - pos)
	{
		return std::u8string_view::npos;
	}

	if consteval
	{
		for (std::size_t index = pos; index + delimiter.size() <= base.size(); ++index)
		{
			bool matches = true;
			for (std::size_t delimiter_index = 0; delimiter_index != delimiter.size(); ++delimiter_index)
			{
				if (base[index + delimiter_index] != delimiter[delimiter_index])
				{
					matches = false;
					break;
				}
			}

			if (matches)
			{
				return index;
			}
		}

		return std::u8string_view::npos;
	}
	else
	{
		return base.find(delimiter, pos);
	}
}

inline constexpr std::size_t rfind_utf8_split_delimiter(
	std::u8string_view base,
	std::u8string_view delimiter,
	std::size_t end_exclusive) noexcept
{
	if (delimiter.empty() || delimiter.size() > base.size() || end_exclusive < delimiter.size())
	{
		return std::u8string_view::npos;
	}

	const auto max_start = (std::min)(base.size() - delimiter.size(), end_exclusive - delimiter.size());
	if consteval
	{
		for (std::size_t index = max_start + 1; index != 0; --index)
		{
			const auto candidate = index - 1;
			bool matches = true;
			for (std::size_t delimiter_index = 0; delimiter_index != delimiter.size(); ++delimiter_index)
			{
				if (base[candidate + delimiter_index] != delimiter[delimiter_index])
				{
					matches = false;
					break;
				}
			}

			if (matches)
			{
				return candidate;
			}
		}

		return std::u8string_view::npos;
	}
	else
	{
		return base.rfind(delimiter, max_start);
	}
}

inline constexpr bool utf8_split_input_ends_with_delimiter(
	std::u8string_view base,
	std::u8string_view delimiter) noexcept
{
	return !delimiter.empty()
		&& base.size() >= delimiter.size()
		&& base.substr(base.size() - delimiter.size()) == delimiter;
}

template <typename View>
class utf8_split_view : public std::ranges::view_interface<utf8_split_view<View>>
{
public:
	static constexpr utf8_split_view from_bytes_unchecked(
		std::u8string_view base,
		std::u8string_view delimiter,
		bool drop_trailing_empty = false) noexcept
	{
		return utf8_split_view{ base, delimiter, drop_trailing_empty };
	}

	static constexpr utf8_split_view from_utf8_char_unchecked(
		std::u8string_view base,
		utf8_char delimiter,
		bool drop_trailing_empty = false) noexcept
	{
		return utf8_split_view{ base, delimiter, drop_trailing_empty };
	}

	class iterator
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using iterator_concept = std::bidirectional_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			std::u8string_view delimiter,
			bool drop_trailing_empty,
			std::size_t current,
			std::size_t next_delimiter) noexcept
			: base_(base),
			  delimiter_(delimiter),
			  drop_trailing_empty_(drop_trailing_empty),
			  current_(current),
			  next_delimiter_(next_delimiter)
		{}

		constexpr reference operator*() const noexcept
		{
			const auto segment_end = next_delimiter_ == std::u8string_view::npos
				? base_.size()
				: next_delimiter_;
			return View::from_bytes_unchecked(base_.substr(current_, segment_end - current_));
		}

		constexpr iterator& operator++() noexcept
		{
			if (next_delimiter_ == std::u8string_view::npos)
			{
				current_ = std::u8string_view::npos;
				return *this;
			}

			const auto next_current = next_delimiter_ + delimiter_.size();
			if (drop_trailing_empty_ && next_current == base_.size())
			{
				current_ = std::u8string_view::npos;
				next_delimiter_ = std::u8string_view::npos;
				return *this;
			}

			current_ = next_current;
			next_delimiter_ = details::find_utf8_split_delimiter(base_, delimiter_, current_);
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		constexpr iterator& operator--() noexcept
		{
			const auto old_current = current_;
			const auto current_for_search = old_current == std::u8string_view::npos
				&& drop_trailing_empty_
				&& details::utf8_split_input_ends_with_delimiter(base_, delimiter_)
				? base_.size()
				: old_current;
			current_ = utf8_split_view::find_previous_segment_start(base_, delimiter_, current_for_search);
			next_delimiter_ = old_current == std::u8string_view::npos
				? details::find_utf8_split_delimiter(base_, delimiter_, current_)
				: old_current - delimiter_.size();
			return *this;
		}

		constexpr iterator operator--(int) noexcept
		{
			iterator old = *this;
			--(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept
		{
			return lhs.base_.data() == rhs.base_.data()
				&& lhs.base_.size() == rhs.base_.size()
				&& lhs.delimiter_.data() == rhs.delimiter_.data()
				&& lhs.delimiter_.size() == rhs.delimiter_.size()
				&& lhs.drop_trailing_empty_ == rhs.drop_trailing_empty_
				&& lhs.current_ == rhs.current_
				&& lhs.next_delimiter_ == rhs.next_delimiter_;
		}

	private:
		std::u8string_view base_{};
		std::u8string_view delimiter_{};
		bool drop_trailing_empty_ = false;
		std::size_t current_ = 0;
		std::size_t next_delimiter_ = std::u8string_view::npos;
	};

	constexpr iterator begin() const noexcept
	{
		const auto delimiter = delimiter_view();
		return iterator{
			base_,
			delimiter,
			drop_trailing_empty_,
			0,
			details::find_utf8_split_delimiter(base_, delimiter, 0)
		};
	}

	constexpr iterator end() const noexcept
	{
		const auto delimiter = delimiter_view();
		return iterator{
			base_,
			delimiter,
			drop_trailing_empty_,
			std::u8string_view::npos,
			std::u8string_view::npos
		};
	}

private:
	constexpr explicit utf8_split_view(
		std::u8string_view base,
		std::u8string_view delimiter,
		bool drop_trailing_empty) noexcept
		: base_(base), delimiter_(delimiter), drop_trailing_empty_(drop_trailing_empty)
	{}

	constexpr explicit utf8_split_view(
		std::u8string_view base,
		utf8_char delimiter,
		bool drop_trailing_empty) noexcept
		: base_(base), delimiter_owned_size_(delimiter.as_view().size()), owns_delimiter_(true), drop_trailing_empty_(drop_trailing_empty)
	{
		const auto delimiter_view = delimiter.as_view();
		for (std::size_t i = 0; i != delimiter_view.size(); ++i)
		{
			delimiter_owned_[i] = delimiter_view[i];
		}
	}

	constexpr std::u8string_view delimiter_view() const noexcept
	{
		return owns_delimiter_
			? std::u8string_view{ delimiter_owned_.data(), delimiter_owned_size_ }
			: delimiter_;
	}

	static constexpr std::size_t find_previous_segment_start(
		std::u8string_view base,
		std::u8string_view delimiter,
		std::size_t current) noexcept
	{
		std::size_t previous_start = 0;
		std::size_t next = details::find_utf8_split_delimiter(base, delimiter, 0);
		while (next != std::u8string_view::npos
			&& next + delimiter.size() < current)
		{
			previous_start = next + delimiter.size();
			next = details::find_utf8_split_delimiter(base, delimiter, previous_start);
		}

		if (current == std::u8string_view::npos)
		{
			while (next != std::u8string_view::npos)
			{
				previous_start = next + delimiter.size();
				next = details::find_utf8_split_delimiter(base, delimiter, previous_start);
			}
		}

		return previous_start;
	}

	std::u8string_view base_{};
	std::u8string_view delimiter_{};
	std::array<char8_t, 4> delimiter_owned_{};
	std::size_t delimiter_owned_size_ = 0;
	bool owns_delimiter_ = false;
	bool drop_trailing_empty_ = false;
};

template <typename View, bool Reverse>
class utf8_splitn_view : public std::ranges::view_interface<utf8_splitn_view<View, Reverse>>
{
public:
	static constexpr utf8_splitn_view from_bytes_unchecked(
		std::u8string_view base,
		std::u8string_view delimiter,
		std::size_t count) noexcept
	{
		return utf8_splitn_view{ base, delimiter, count };
	}

	static constexpr utf8_splitn_view from_utf8_char_unchecked(
		std::u8string_view base,
		utf8_char delimiter,
		std::size_t count) noexcept
	{
		return utf8_splitn_view{ base, delimiter, count };
	}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			std::u8string_view delimiter,
			std::size_t current,
			std::size_t delimiter_pos,
			std::size_t remaining) noexcept
			: base_(base),
			  delimiter_(delimiter),
			  current_(current),
			  delimiter_pos_(delimiter_pos),
			  remaining_(remaining)
		{}

		constexpr reference operator*() const noexcept
		{
			if constexpr (Reverse)
			{
				if (remaining_ == 1 || delimiter_pos_ == std::u8string_view::npos)
				{
					return View::from_bytes_unchecked(base_.substr(0, current_));
				}

				const auto segment_start = delimiter_pos_ + delimiter_.size();
				return View::from_bytes_unchecked(base_.substr(segment_start, current_ - segment_start));
			}
			else
			{
				if (remaining_ == 1 || delimiter_pos_ == std::u8string_view::npos)
				{
					return View::from_bytes_unchecked(base_.substr(current_));
				}

				return View::from_bytes_unchecked(base_.substr(current_, delimiter_pos_ - current_));
			}
		}

		constexpr iterator& operator++() noexcept
		{
			if (remaining_ == 1 || delimiter_pos_ == std::u8string_view::npos)
			{
				current_ = std::u8string_view::npos;
				delimiter_pos_ = std::u8string_view::npos;
				remaining_ = 0;
				return *this;
			}

			--remaining_;
			if constexpr (Reverse)
			{
				current_ = delimiter_pos_;
				delimiter_pos_ = details::rfind_utf8_split_delimiter(base_, delimiter_, current_);
			}
			else
			{
				current_ = delimiter_pos_ + delimiter_.size();
				delimiter_pos_ = details::find_utf8_split_delimiter(base_, delimiter_, current_);
			}
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.remaining_ == 0;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.remaining_ == 0;
		}

	private:
		std::u8string_view base_{};
		std::u8string_view delimiter_{};
		std::size_t current_ = std::u8string_view::npos;
		std::size_t delimiter_pos_ = std::u8string_view::npos;
		std::size_t remaining_ = 0;
	};

	constexpr iterator begin() const noexcept
	{
		if (count_ == 0)
		{
			return iterator{};
		}

		const auto delimiter = delimiter_view();
		if constexpr (Reverse)
		{
			return iterator{
				base_,
				delimiter,
				base_.size(),
				details::rfind_utf8_split_delimiter(base_, delimiter, base_.size()),
				count_
			};
		}
		else
		{
			return iterator{
				base_,
				delimiter,
				0,
				details::find_utf8_split_delimiter(base_, delimiter, 0),
				count_
			};
		}
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit utf8_splitn_view(
		std::u8string_view base,
		std::u8string_view delimiter,
		std::size_t count) noexcept
		: base_(base), delimiter_(delimiter), count_(count)
	{}

	constexpr explicit utf8_splitn_view(
		std::u8string_view base,
		utf8_char delimiter,
		std::size_t count) noexcept
		: base_(base), delimiter_owned_size_(delimiter.as_view().size()), owns_delimiter_(true), count_(count)
	{
		const auto delimiter_view = delimiter.as_view();
		for (std::size_t i = 0; i != delimiter_view.size(); ++i)
		{
			delimiter_owned_[i] = delimiter_view[i];
		}
	}

	constexpr std::u8string_view delimiter_view() const noexcept
	{
		return owns_delimiter_
			? std::u8string_view{ delimiter_owned_.data(), delimiter_owned_size_ }
			: delimiter_;
	}

	std::u8string_view base_{};
	std::u8string_view delimiter_{};
	std::array<char8_t, 4> delimiter_owned_{};
	std::size_t delimiter_owned_size_ = 0;
	bool owns_delimiter_ = false;
	std::size_t count_ = 0;
};

inline constexpr bool utf8_char_is_whitespace_at(
	std::u8string_view base,
	std::size_t pos,
	bool ascii_only) noexcept
{
	const auto len = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base[pos]));
	const auto ch = utf8_char::from_utf8_bytes_unchecked(base.data() + pos, len);
	return ascii_only ? ch.is_ascii_whitespace() : ch.is_whitespace();
}

inline constexpr std::size_t next_utf8_scalar_boundary(
	std::u8string_view base,
	std::size_t pos) noexcept
{
	return pos + details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(base[pos]));
}

inline constexpr std::size_t find_utf8_non_whitespace_boundary(
	std::u8string_view base,
	std::size_t pos,
	bool ascii_only) noexcept
{
	while (pos < base.size())
	{
		if (!details::utf8_char_is_whitespace_at(base, pos, ascii_only))
		{
			return pos;
		}

		pos = details::next_utf8_scalar_boundary(base, pos);
	}

	return std::u8string_view::npos;
}

inline constexpr std::size_t find_utf8_whitespace_boundary(
	std::u8string_view base,
	std::size_t pos,
	bool ascii_only) noexcept
{
	while (pos < base.size())
	{
		if (details::utf8_char_is_whitespace_at(base, pos, ascii_only))
		{
			return pos;
		}

		pos = details::next_utf8_scalar_boundary(base, pos);
	}

	return std::u8string_view::npos;
}

inline constexpr std::size_t utf8_trim_end_boundary(
	std::u8string_view base,
	bool ascii_only) noexcept
{
	std::size_t end = 0;
	for (std::size_t pos = 0; pos < base.size();)
	{
		const auto next = details::next_utf8_scalar_boundary(base, pos);
		if (!details::utf8_char_is_whitespace_at(base, pos, ascii_only))
		{
			end = next;
		}

		pos = next;
	}

	return end;
}

template <typename View, bool AsciiOnly>
class utf8_whitespace_split_view : public std::ranges::view_interface<utf8_whitespace_split_view<View, AsciiOnly>>
{
private:
	static constexpr utf8_whitespace_split_view from_bytes_unchecked(std::u8string_view base) noexcept
	{
		return utf8_whitespace_split_view{ base };
	}

	template <typename, typename>
	friend class utf8_string_crtp;

public:

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using iterator_concept = std::forward_iterator_tag;
		using value_type = View;
		using difference_type = std::ptrdiff_t;
		using reference = value_type;
		using pointer = void;

		iterator() = default;

		constexpr iterator(
			std::u8string_view base,
			std::size_t current,
			std::size_t next_whitespace) noexcept
			: base_(base), current_(current), next_whitespace_(next_whitespace)
		{}

		constexpr reference operator*() const noexcept
		{
			const auto end = next_whitespace_ == std::u8string_view::npos
				? base_.size()
				: next_whitespace_;
			return View::from_bytes_unchecked(base_.substr(current_, end - current_));
		}

		constexpr iterator& operator++() noexcept
		{
			current_ = details::find_utf8_non_whitespace_boundary(
				base_,
				next_whitespace_ == std::u8string_view::npos ? base_.size() : next_whitespace_,
				AsciiOnly);
			if (current_ == std::u8string_view::npos)
			{
				next_whitespace_ = std::u8string_view::npos;
				return *this;
			}

			next_whitespace_ = details::find_utf8_whitespace_boundary(base_, current_, AsciiOnly);
			return *this;
		}

		constexpr iterator operator++(int) noexcept
		{
			iterator old = *this;
			++(*this);
			return old;
		}

		friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

		friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
		{
			return it.current_ == std::u8string_view::npos;
		}

	private:
		std::u8string_view base_{};
		std::size_t current_ = std::u8string_view::npos;
		std::size_t next_whitespace_ = std::u8string_view::npos;
	};

	constexpr iterator begin() const noexcept
	{
		const auto current = details::find_utf8_non_whitespace_boundary(base_, 0, AsciiOnly);
		if (current == std::u8string_view::npos)
		{
			return iterator{};
		}

		return iterator{
			base_,
			current,
			details::find_utf8_whitespace_boundary(base_, current, AsciiOnly)
		};
	}

	constexpr std::default_sentinel_t end() const noexcept
	{
		return std::default_sentinel;
	}

private:
	constexpr explicit utf8_whitespace_split_view(std::u8string_view base) noexcept
		: base_(base)
	{}

	std::u8string_view base_{};
};

template <typename Derived, typename View>
class utf8_string_crtp
{
public:
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	static constexpr size_type npos = static_cast<size_type>(-1);

	constexpr auto chars() const noexcept
	{
		return views::utf8_view::from_bytes_unchecked(byte_view());
	}

	constexpr auto reversed_chars() const noexcept
	{
		return views::reversed_utf8_view::from_bytes_unchecked(byte_view());
	}

	constexpr auto graphemes() const noexcept -> views::grapheme_cluster_view<char8_t>;
	constexpr auto reversed_graphemes() const noexcept -> views::reversed_grapheme_cluster_view<char8_t>;
	template <typename Allocator = std::allocator<char8_t>>
	constexpr basic_utf8_string<Allocator> to_utf8_owned(const Allocator& alloc = Allocator()) const;
	template <typename Allocator = std::allocator<char16_t>>
	constexpr basic_utf16_string<Allocator> to_utf16(const Allocator& alloc = Allocator()) const;

	constexpr size_type size() const noexcept
	{
		return byte_view().size();
	}

	constexpr bool empty() const noexcept
	{
		return byte_view().empty();
	}

	constexpr bool is_ascii() const noexcept
	{
		return std::ranges::all_of(byte_view(),
			[](char8_t byte) noexcept
			{
				return static_cast<std::uint8_t>(byte) <= details::encoding_constants::ascii_scalar_max;
			});
	}

	constexpr auto char_indices() const noexcept
	{
		return utf8_char_indices_view::from_bytes_unchecked(byte_view());
	}

	constexpr auto grapheme_indices() const noexcept
	{
		return utf8_grapheme_indices_view<View>::from_bytes_unchecked(byte_view());
	}

	constexpr bool is_grapheme_boundary(size_type index) const noexcept
	{
		return details::is_grapheme_boundary(byte_view(), index);
	}

	constexpr bool contains(utf8_char ch) const noexcept
	{
		return find(ch) != npos;
	}

	constexpr bool contains(View sv) const noexcept
	{
		return find(sv) != npos;
	}

	constexpr bool contains_grapheme(utf8_char ch) const noexcept
	{
		return find_grapheme(ch) != npos;
	}

	constexpr bool contains_grapheme(View sv) const noexcept
	{
		return find_grapheme(sv) != npos;
	}

	constexpr size_type find(char8_t ch, size_type pos = 0) const noexcept
	{
		pos = (std::min)(size(), pos);
		if consteval
		{
			for (size_type index = pos; index != size(); ++index)
			{
				if (byte_view()[index] == ch)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().find(ch, pos);
		}
	}

	constexpr size_type find(utf8_char ch, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		std::array<char8_t, 4> bytes{};
		const auto needle_size = ch.encode_utf8<char8_t>(bytes.begin());
		if consteval
		{
			if (needle_size > size() - pos)
			{
				return npos;
			}

			for (size_type index = pos; index + needle_size <= size(); ++index)
			{
				bool matches = true;
				for (size_type needle_index = 0; needle_index != needle_size; ++needle_index)
				{
					if (byte_view()[index + needle_index] != bytes[needle_index])
					{
						matches = false;
						break;
					}
				}

				if (matches)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().find(std::u8string_view{ bytes.data(), needle_size }, pos);
		}
	}

	constexpr size_type find(View sv, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		const auto needle = sv.base();
		if (needle.empty())
		{
			return pos;
		}

		if consteval
		{
			if (needle.size() > size() - pos)
			{
				return npos;
			}

			for (size_type index = pos; index + needle.size() <= size(); ++index)
			{
				bool matches = true;
				for (size_type needle_index = 0; needle_index != needle.size(); ++needle_index)
				{
					if (byte_view()[index + needle_index] != needle[needle_index])
					{
						matches = false;
						break;
					}
				}

				if (matches)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().find(needle, pos);
		}
	}

	constexpr size_type find_grapheme(utf8_char ch, size_type pos = 0) const noexcept
	{
		return details::find_grapheme(byte_view(), ch.as_view(), pos);
	}

	constexpr size_type find_grapheme(View sv, size_type pos = 0) const noexcept
	{
		return details::find_grapheme(byte_view(), sv.base(), pos);
	}

	constexpr size_type find_first_of(char8_t ch, size_type pos = 0) const noexcept
	{
		return find(ch, pos);
	}

	constexpr size_type find_first_of(utf8_char ch, size_type pos = 0) const noexcept
	{
		return find(ch, pos);
	}

	constexpr size_type find_first_of(View sv, size_type pos = 0) const noexcept
	{
		if (sv.empty())
		{
			return npos;
		}

		pos = ceil_char_boundary((std::min)(size(), pos));
		auto indices = char_indices();
		const auto it = std::ranges::find_if(indices, [&](const auto& entry)
		{
			const auto [index, current] = entry;
			return index >= pos && sv.contains(current);
		});

		return it == indices.end() ? npos : (*it).first;
	}

	constexpr size_type find_first_not_of(char8_t ch, size_type pos = 0) const noexcept
	{
		pos = (std::min)(size(), pos);
		for (size_type index = pos; index != size(); ++index)
		{
			if (byte_view()[index] != ch)
			{
				return index;
			}
		}

		return npos;
	}

	constexpr size_type find_first_not_of(utf8_char ch, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		auto indices = char_indices();
		const auto it = std::ranges::find_if(indices, [&](const auto& entry)
		{
			const auto [index, current] = entry;
			return index >= pos && current != ch;
		});

		return it == indices.end() ? npos : (*it).first;
	}

	constexpr size_type find_first_not_of(View sv, size_type pos = 0) const noexcept
	{
		pos = ceil_char_boundary((std::min)(size(), pos));
		auto indices = char_indices();
		const auto it = std::ranges::find_if(indices, [&](const auto& entry)
		{
			const auto [index, current] = entry;
			return index >= pos && !sv.contains(current);
		});

		return it == indices.end() ? npos : (*it).first;
	}

	constexpr size_type rfind(char8_t ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = (std::min)(size() - 1, pos);
		if consteval
		{
			for (size_type index = pos + 1; index != 0;)
			{
				--index;
				if (byte_view()[index] == ch)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().rfind(ch, pos);
		}
	}

	constexpr size_type rfind(utf8_char ch, size_type pos = npos) const noexcept
	{
		std::array<char8_t, 4> bytes{};
		const auto needle_size = ch.encode_utf8<char8_t>(bytes.begin());
		if (needle_size > size())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		pos = floor_char_boundary((std::min)(pos, size() - needle_size));
		if consteval
		{
			for (size_type index = pos + 1; index != 0;)
			{
				--index;
				bool matches = true;
				for (size_type needle_index = 0; needle_index != needle_size; ++needle_index)
				{
					if (byte_view()[index + needle_index] != bytes[needle_index])
					{
						matches = false;
						break;
					}
				}

				if (matches)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().rfind(std::u8string_view{ bytes.data(), needle_size }, pos);
		}
	}

	constexpr size_type rfind(View sv, size_type pos = npos) const noexcept
	{
		const auto needle = sv.base();
		pos = floor_char_boundary((std::min)(size(), pos));
		if (needle.empty())
		{
			return pos;
		}

		if (needle.size() > size())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(pos, size() - needle.size()));
		if consteval
		{
			for (size_type index = pos + 1; index != 0;)
			{
				--index;
				bool matches = true;
				for (size_type needle_index = 0; needle_index != needle.size(); ++needle_index)
				{
					if (byte_view()[index + needle_index] != needle[needle_index])
					{
						matches = false;
						break;
					}
				}

				if (matches)
				{
					return index;
				}
			}

			return npos;
		}
		else
		{
			return byte_view().rfind(needle, pos);
		}
	}

	constexpr size_type rfind_grapheme(utf8_char ch, size_type pos = npos) const noexcept
	{
		return details::rfind_grapheme(byte_view(), ch.as_view(), pos);
	}

	constexpr size_type rfind_grapheme(View sv, size_type pos = npos) const noexcept
	{
		return details::rfind_grapheme(byte_view(), sv.base(), pos);
	}

	constexpr size_type find_last_of(char8_t ch, size_type pos = npos) const noexcept
	{
		return rfind(ch, pos);
	}

	constexpr size_type find_last_of(utf8_char ch, size_type pos = npos) const noexcept
	{
		return rfind(ch, pos);
	}

	constexpr size_type find_last_of(View sv, size_type pos = npos) const noexcept
	{
		if (empty() || sv.empty())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		if (pos == size())
		{
			pos = floor_char_boundary(size() - 1);
		}
		for (size_type index = pos;;)
		{
			if (sv.contains(char_at_unchecked(index)))
			{
				return index;
			}

			if (index == 0)
			{
				return npos;
			}

			index = floor_char_boundary(index - 1);
		}
	}

	constexpr size_type find_last_not_of(char8_t ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = (std::min)(size() - 1, pos);
		for (size_type index = pos + 1; index != 0;)
		{
			--index;
			if (byte_view()[index] != ch)
			{
				return index;
			}
		}

		return npos;
	}

	constexpr size_type find_last_not_of(utf8_char ch, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		size_type result = npos;
		for (const auto [index, current] : char_indices())
		{
			if (index > pos)
			{
				break;
			}

			if (current != ch)
			{
				result = index;
			}
		}

		return result;
	}

	constexpr size_type find_last_not_of(View sv, size_type pos = npos) const noexcept
	{
		if (empty())
		{
			return npos;
		}

		pos = floor_char_boundary((std::min)(size(), pos));
		if (pos == size())
		{
			pos = floor_char_boundary(size() - 1);
		}
		for (size_type index = pos;;)
		{
			if (!sv.contains(char_at_unchecked(index)))
			{
				return index;
			}

			if (index == 0)
			{
				return npos;
			}

			index = floor_char_boundary(index - 1);
		}
	}

	constexpr bool is_char_boundary(size_type index) const noexcept
	{
		if (index > size()) [[unlikely]]
		{
			return false;
		}

		if (index == 0 || index == size()) [[unlikely]]
		{
			return true;
		}

		return details::is_utf8_lead_byte(static_cast<std::uint8_t>(byte_view()[index]));
	}

	constexpr size_type char_count() const noexcept
	{
		return static_cast<size_type>(std::ranges::distance(chars()));
	}

	constexpr size_type grapheme_count() const noexcept
	{
		return details::grapheme_count(byte_view());
	}

	constexpr auto split(utf8_char ch) const noexcept
	{
		return utf8_split_view<View>::from_utf8_char_unchecked(byte_view(), ch);
	}

	constexpr auto split(View sv) const noexcept
	{
		return utf8_split_view<View>::from_bytes_unchecked(byte_view(), sv.base());
	}

	constexpr auto split_whitespace() const noexcept
	{
		return utf8_whitespace_split_view<View, false>::from_bytes_unchecked(byte_view());
	}

	constexpr auto split_ascii_whitespace() const noexcept
	{
		return utf8_whitespace_split_view<View, true>::from_bytes_unchecked(byte_view());
	}

	constexpr auto rsplit(utf8_char ch) const noexcept
	{
		return rsplit(View::from_bytes_unchecked(ch.as_view()));
	}

	constexpr auto rsplit(View sv) const noexcept
	{
		return std::views::reverse(split(sv));
	}

	constexpr auto split_terminator(utf8_char ch) const noexcept
	{
		return utf8_split_view<View>::from_utf8_char_unchecked(byte_view(), ch, true);
	}

	constexpr auto split_terminator(View sv) const noexcept
	{
		return utf8_split_view<View>::from_bytes_unchecked(byte_view(), sv.base(), true);
	}

	constexpr auto rsplit_terminator(utf8_char ch) const noexcept
	{
		return rsplit_terminator(View::from_bytes_unchecked(ch.as_view()));
	}

	constexpr auto rsplit_terminator(View sv) const noexcept
	{
		return std::views::reverse(split_terminator(sv));
	}

	constexpr auto splitn(size_type count, utf8_char ch) const noexcept
	{
		return utf8_splitn_view<View, false>::from_utf8_char_unchecked(byte_view(), ch, count);
	}

	constexpr auto splitn(size_type count, View sv) const noexcept
	{
		return utf8_splitn_view<View, false>::from_bytes_unchecked(byte_view(), sv.base(), count);
	}

	constexpr auto rsplitn(size_type count, utf8_char ch) const noexcept
	{
		return utf8_splitn_view<View, true>::from_utf8_char_unchecked(byte_view(), ch, count);
	}

	constexpr auto rsplitn(size_type count, View sv) const noexcept
	{
		return utf8_splitn_view<View, true>::from_bytes_unchecked(byte_view(), sv.base(), count);
	}

	constexpr std::optional<std::pair<View, View>> split_once(utf8_char ch) const noexcept
	{
		return split_once(View::from_bytes_unchecked(ch.as_view()));
	}

	constexpr std::optional<std::pair<View, View>> split_once(View sv) const noexcept
	{
		const auto delimiter = sv.base();
		const auto pos = details::find_utf8_split_delimiter(byte_view(), delimiter, 0);
		if (pos == npos)
		{
			return std::nullopt;
		}

		const auto bytes = byte_view();
		return std::pair{
			View::from_bytes_unchecked(bytes.substr(0, pos)),
			View::from_bytes_unchecked(bytes.substr(pos + delimiter.size()))
		};
	}

	constexpr std::optional<std::pair<View, View>> rsplit_once(utf8_char ch) const noexcept
	{
		return rsplit_once(View::from_bytes_unchecked(ch.as_view()));
	}

	constexpr std::optional<std::pair<View, View>> rsplit_once(View sv) const noexcept
	{
		const auto delimiter = sv.base();
		const auto pos = details::rfind_utf8_split_delimiter(byte_view(), delimiter, byte_view().size());
		if (pos == npos)
		{
			return std::nullopt;
		}

		const auto bytes = byte_view();
		return std::pair{
			View::from_bytes_unchecked(bytes.substr(0, pos)),
			View::from_bytes_unchecked(bytes.substr(pos + delimiter.size()))
		};
	}

	constexpr std::optional<std::pair<View, View>> split_once_at(size_type delim) const noexcept
	{
		if (!is_char_boundary(delim)) [[unlikely]]
		{
			return std::nullopt;
		}

		return split_once_at_unchecked(delim);
	}

	constexpr std::pair<View, View> split_once_at_unchecked(size_type delim) const noexcept
	{
		const auto bytes = byte_view();
		return {
			View::from_bytes_unchecked(bytes.substr(0, delim)),
			View::from_bytes_unchecked(bytes.substr(delim))
		};
	}

	constexpr std::optional<View> strip_prefix(utf8_char ch) const noexcept
	{
		return strip_prefix(View::from_bytes_unchecked(ch.as_view()));
	}

	constexpr std::optional<View> strip_prefix(View sv) const noexcept
	{
		if (!starts_with(sv))
		{
			return std::nullopt;
		}

		return View::from_bytes_unchecked(byte_view().substr(sv.base().size()));
	}

	constexpr std::optional<View> strip_suffix(utf8_char ch) const noexcept
	{
		return strip_suffix(View::from_bytes_unchecked(ch.as_view()));
	}

	constexpr std::optional<View> strip_suffix(View sv) const noexcept
	{
		if (!ends_with(sv))
		{
			return std::nullopt;
		}

		return View::from_bytes_unchecked(byte_view().substr(0, size() - sv.base().size()));
	}

	constexpr std::optional<View> strip_circumfix(utf8_char prefix, utf8_char suffix) const noexcept
	{
		return strip_circumfix(
			View::from_bytes_unchecked(prefix.as_view()),
			View::from_bytes_unchecked(suffix.as_view()));
	}

	constexpr std::optional<View> strip_circumfix(View prefix, View suffix) const noexcept
	{
		const auto stripped = strip_prefix(prefix);
		if (!stripped.has_value())
		{
			return std::nullopt;
		}

		return stripped->strip_suffix(suffix);
	}

	constexpr View trim_prefix(utf8_char ch) const noexcept
	{
		return trim_prefix(View::from_bytes_unchecked(ch.as_view()));
	}

	constexpr View trim_prefix(View sv) const noexcept
	{
		return strip_prefix(sv).value_or(view_from_whole_string());
	}

	constexpr View trim_suffix(utf8_char ch) const noexcept
	{
		return trim_suffix(View::from_bytes_unchecked(ch.as_view()));
	}

	constexpr View trim_suffix(View sv) const noexcept
	{
		return strip_suffix(sv).value_or(view_from_whole_string());
	}

	constexpr View trim_start_matches(utf8_char ch) const noexcept
	{
		const auto pos = find_first_not_of(ch);
		return pos == npos
			? empty_view()
			: View::from_bytes_unchecked(byte_view().substr(pos));
	}

	constexpr View trim_start_matches(View sv) const noexcept
	{
		auto result = byte_view();
		const auto needle = sv.base();
		if (needle.empty())
		{
			return view_from_whole_string();
		}

		while (result.starts_with(needle))
		{
			result.remove_prefix(needle.size());
		}

		return View::from_bytes_unchecked(result);
	}

	constexpr View trim_end_matches(utf8_char ch) const noexcept
	{
		const auto pos = find_last_not_of(ch);
		if (pos == npos)
		{
			return empty_view();
		}

		return View::from_bytes_unchecked(byte_view().substr(0, pos + char_at_unchecked(pos).code_unit_count()));
	}

	constexpr View trim_end_matches(View sv) const noexcept
	{
		auto result = byte_view();
		const auto needle = sv.base();
		if (needle.empty())
		{
			return view_from_whole_string();
		}

		while (result.ends_with(needle))
		{
			result.remove_suffix(needle.size());
		}

		return View::from_bytes_unchecked(result);
	}

	constexpr View trim_matches(utf8_char ch) const noexcept
	{
		return trim_start_matches(ch).trim_end_matches(ch);
	}

	constexpr View trim_matches(View sv) const noexcept
	{
		return trim_start_matches(sv).trim_end_matches(sv);
	}

	constexpr View trim_start() const noexcept
	{
		const auto pos = details::find_utf8_non_whitespace_boundary(byte_view(), 0, false);
		return pos == npos
			? empty_view()
			: View::from_bytes_unchecked(byte_view().substr(pos));
	}

	constexpr View trim_end() const noexcept
	{
		return View::from_bytes_unchecked(byte_view().substr(0, details::utf8_trim_end_boundary(byte_view(), false)));
	}

	constexpr View trim() const noexcept
	{
		return trim_start().trim_end();
	}

	constexpr View trim_ascii_start() const noexcept
	{
		const auto pos = details::find_utf8_non_whitespace_boundary(byte_view(), 0, true);
		return pos == npos
			? empty_view()
			: View::from_bytes_unchecked(byte_view().substr(pos));
	}

	constexpr View trim_ascii_end() const noexcept
	{
		return View::from_bytes_unchecked(byte_view().substr(0, details::utf8_trim_end_boundary(byte_view(), true)));
	}

	constexpr View trim_ascii() const noexcept
	{
		return trim_ascii_start().trim_ascii_end();
	}

	constexpr std::optional<utf8_char> char_at(size_type index) const noexcept
	{
		if (index >= size() || !is_char_boundary(index)) [[unlikely]]
		{
			return std::nullopt;
		}

		return char_at_unchecked(index);
	}

	constexpr utf8_char char_at_unchecked(size_type index) const noexcept
	{
		const auto len = details::utf8_byte_count_from_lead(static_cast<std::uint8_t>(byte_view()[index]));
		return utf8_char::from_utf8_bytes_unchecked(byte_view().data() + index, len);
	}

	constexpr std::optional<View> grapheme_at(size_type index) const noexcept
	{
		if (index >= size() || !is_grapheme_boundary(index)) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto end = details::next_grapheme_boundary(byte_view(), index);
		return View::from_bytes_unchecked(byte_view().substr(index, end - index));
	}

	constexpr std::optional<View> substr(size_type pos, size_type count = npos) const noexcept
	{
		if (!is_char_boundary(pos)) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto end = (count == npos)
			? size()
			: (std::min)(size(), pos + count);

		if (!is_char_boundary(end)) [[unlikely]]
		{
			return std::nullopt;
		}

		return View::from_bytes_unchecked(byte_view().substr(pos, end - pos));
	}

	constexpr std::optional<View> grapheme_substr(size_type pos, size_type count = npos) const noexcept
	{
		if (!is_grapheme_boundary(pos)) [[unlikely]]
		{
			return std::nullopt;
		}

		const auto end = (count == npos)
			? size()
			: (std::min)(size(), pos + count);

		if (!is_grapheme_boundary(end)) [[unlikely]]
		{
			return std::nullopt;
		}

		return View::from_bytes_unchecked(byte_view().substr(pos, end - pos));
	}

	constexpr std::optional<utf8_char> front() const noexcept
	{
		if (empty()) [[unlikely]]
		{
			return std::nullopt;
		}

		return front_unchecked();
	}

	constexpr utf8_char front_unchecked() const noexcept
	{
		return *chars().begin();
	}

	constexpr std::optional<utf8_char> back() const noexcept
	{
		if (empty()) [[unlikely]]
		{
			return std::nullopt;
		}

		return back_unchecked();
	}

	constexpr utf8_char back_unchecked() const noexcept
	{
		return *reversed_chars().begin();
	}

	constexpr bool starts_with(char ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	constexpr bool starts_with(char8_t ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	constexpr bool starts_with(utf8_char ch) const noexcept
	{
		return !empty() && (front_unchecked() == ch);
	}

	constexpr bool starts_with(View sv) const noexcept
	{
		return byte_view().starts_with(sv.base());
	}

	constexpr bool ends_with(char ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
	}

	constexpr bool ends_with(char8_t ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
	}

	constexpr bool ends_with(utf8_char ch) const noexcept
	{
		return !empty() && (back_unchecked() == ch);
	}

	constexpr bool ends_with(View sv) const noexcept
	{
		return byte_view().ends_with(sv.base());
	}

	constexpr size_type ceil_char_boundary(size_type pos) const noexcept
	{
		pos = (std::min)(size(), pos);
		while (pos != size() && !is_char_boundary(pos))
		{
			++pos;
		}

		return pos;
	}

	constexpr size_type floor_char_boundary(size_type pos) const noexcept
	{
		pos = (std::min)(size(), pos);
		while (pos != 0 && !is_char_boundary(pos))
		{
			--pos;
		}

		return pos;
	}

	constexpr size_type ceil_grapheme_boundary(size_type pos) const noexcept
	{
		return details::ceil_grapheme_boundary(byte_view(), pos);
	}

	constexpr size_type floor_grapheme_boundary(size_type pos) const noexcept
	{
		return details::floor_grapheme_boundary(byte_view(), pos);
	}

protected:
	constexpr const Derived& self() const noexcept
	{
		return static_cast<const Derived&>(*this);
	}

	constexpr View view_from_whole_string() const noexcept
	{
		return View::from_bytes_unchecked(byte_view());
	}

	constexpr View empty_view() const noexcept
	{
		return View::from_bytes_unchecked(byte_view().substr(0, 0));
	}

	constexpr std::u8string_view byte_view() const noexcept
	{
		return std::u8string_view{ self().base() };
	}
};

}

}

#endif // UTF8_RANGES_UTF8_STRING_CRTP_HPP
