#ifndef UTF8_RANGES_UTF16_VIEWS_HPP
#define UTF8_RANGES_UTF16_VIEWS_HPP

#include "utf16_char.hpp"

namespace utf8_ranges
{

namespace views
{
	template<typename R>
	concept lossy_utf16_viewable_range =
		std::ranges::viewable_range<R> &&
		std::ranges::contiguous_range<R> &&
		std::ranges::sized_range<R> &&
		std::is_integral_v<std::ranges::range_value_t<R>> &&
		!std::is_same_v<std::remove_cv_t<std::ranges::range_value_t<R>>, bool>;

	template <typename CharT>
	class lossy_utf16_view : public std::ranges::view_interface<lossy_utf16_view<CharT>>
	{
	public:
		lossy_utf16_view() = default;
		constexpr lossy_utf16_view(std::basic_string_view<CharT> base) noexcept : base_(base) {}

		class iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = utf16_char;
			using difference_type = std::ptrdiff_t;
			using reference = utf16_char;
			using pointer = void;

			iterator() = default;

			constexpr iterator(const CharT* current, const CharT* end) noexcept
				: current_(current), end_(end)
			{
				load_current();
			}

			constexpr reference operator*() const noexcept
			{
				return current_valid_ ? current_char_ : utf16_char::replacement_character;
			}

			constexpr iterator& operator++() noexcept
			{
				current_ += static_cast<difference_type>(current_width_);
				load_current();
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
				return it.current_ == it.end_;
			}

			friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept
			{
				return lhs.current_ == rhs.current_;
			}

			friend constexpr bool operator==(std::default_sentinel_t, const iterator& it) noexcept
			{
				return it.current_ == it.end_;
			}

		private:
			constexpr void load_current() noexcept
			{
				current_width_ = 0;
				current_valid_ = false;

				if (current_ == end_) [[unlikely]]
				{
					return;
				}

				current_width_ = 1;
				const std::size_t remaining = static_cast<std::size_t>(end_ - current_);
				const auto first = static_cast<std::uint16_t>(*current_);
				if (first < 0xD800u || first > 0xDFFFu)
				{
					current_char_ = utf16_char::from_utf16_code_units_unchecked(current_, 1);
					current_valid_ = true;
					return;
				}

				if (first > 0xDBFFu || remaining < 2)
				{
					return;
				}

				const auto second = static_cast<std::uint16_t>(current_[1]);
				if (second < 0xDC00u || second > 0xDFFFu)
				{
					return;
				}

				current_char_ = utf16_char::from_utf16_code_units_unchecked(current_, 2);
				current_width_ = 2;
				current_valid_ = true;
			}

			const CharT* current_ = nullptr;
			const CharT* end_ = nullptr;
			utf16_char current_char_{};
			std::size_t current_width_ = 0;
			bool current_valid_ = false;
		};

		constexpr iterator begin() const noexcept
		{
			return iterator{ base_.data(), base_.data() + base_.size() };
		}

		constexpr std::default_sentinel_t end() const noexcept
		{
			return std::default_sentinel;
		}

	private:
		std::basic_string_view<CharT> base_;
	};

	struct lossy_utf16_fn : std::ranges::range_adaptor_closure<lossy_utf16_fn>
	{
		template<lossy_utf16_viewable_range R>
		constexpr auto operator()(R&& range) const noexcept
		{
			using char_type = std::remove_cv_t<std::ranges::range_value_t<R>>;
			return lossy_utf16_view<char_type>{
				std::basic_string_view<char_type>{
					std::ranges::data(range),
					static_cast<std::size_t>(std::ranges::size(range))
				}
			};
		}
	};

	inline constexpr lossy_utf16_fn lossy_utf16{};
}

}

#endif // UTF8_RANGES_UTF16_VIEWS_HPP
