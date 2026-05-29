// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ENCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_ENCODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/codec_concepts.hpp>
#include <beman/transcode/detail/concepts.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <array>
    #include <expected>
    #include <iterator>
    #include <ranges>
    #include <type_traits>

#endif
namespace beman::transcoding {

// ---------------------------------------------------------------------------
// encode_view — encodes char32_t scalars to bytes using a pluggable codec
// ---------------------------------------------------------------------------

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
class encode_view : public std::ranges::view_interface<encode_view<Codec, R>> {
    R     base_;
    Codec codec_;

    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;
        using base_sent = detail::compatible_sentinel_t<R>;

        base_iter                    current_{};
        base_sent                    end_{};
        Codec                        codec_{};
        std::array<unsigned char, 8> buf_{};
        int                          len_{0};
        int                          pos_{0};
        bool                         done_{false};

        constexpr void load();

      public:
        using iterator_concept =
            std::conditional_t<std::ranges::forward_range<R>, std::forward_iterator_tag, std::input_iterator_tag>;
        using iterator_category = iterator_concept;
        using value_type        = char;
        using difference_type   = std::ptrdiff_t;
        using reference         = char;

        constexpr iterator() = default;

        static constexpr iterator terminal()
            requires std::ranges::forward_range<R>;

        constexpr iterator(base_iter current, base_sent end, Codec codec);

        constexpr char      operator*() const;
        constexpr iterator& operator++();
        constexpr iterator  operator++(int)
            requires std::ranges::forward_range<R>;
        constexpr void operator++(int)
            requires(!std::ranges::forward_range<R>);

        constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
            requires std::ranges::forward_range<R>
        {
            if (lhs.done_ || rhs.done_)
                return lhs.done_ == rhs.done_;
            return lhs.current_ == rhs.current_ && lhs.pos_ == rhs.pos_;
        }

        constexpr friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit encode_view(R base, Codec codec = {});

    constexpr const R& base() const& noexcept { return base_; }
    constexpr R        base() && { return std::move(base_); }

    constexpr iterator begin();
    constexpr iterator begin() const
        requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>;
    constexpr iterator end()
        requires std::ranges::forward_range<R> && std::ranges::common_range<R>;
    constexpr iterator end() const
        requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
                 detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>;
    constexpr std::default_sentinel_t end() const;
};

// ---------------------------------------------------------------------------
// encode_or_error_view — encodes char32_t scalars to expected<char, decode_error>
// ---------------------------------------------------------------------------

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
class encode_or_error_view : public std::ranges::view_interface<encode_or_error_view<Codec, R>> {
    R     base_;
    Codec codec_;

    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;
        using base_sent = detail::compatible_sentinel_t<R>;
        using result_t  = std::expected<char, decode_error>;

        base_iter                    current_{};
        base_sent                    end_{};
        Codec                        codec_{};
        std::array<unsigned char, 8> buf_{};
        int                          len_{0};
        int                          pos_{0};
        bool                         is_error_{false};
        bool                         done_{false};

        constexpr void load();

      public:
        using iterator_concept =
            std::conditional_t<std::ranges::forward_range<R>, std::forward_iterator_tag, std::input_iterator_tag>;
        using iterator_category = iterator_concept;
        using value_type        = result_t;
        using difference_type   = std::ptrdiff_t;
        using reference         = result_t;

        constexpr iterator() = default;

        static constexpr iterator terminal()
            requires std::ranges::forward_range<R>;

        constexpr iterator(base_iter current, base_sent end, Codec codec);

        constexpr result_t  operator*() const;
        constexpr iterator& operator++();
        constexpr iterator  operator++(int)
            requires std::ranges::forward_range<R>;
        constexpr void operator++(int)
            requires(!std::ranges::forward_range<R>);

        constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
            requires std::ranges::forward_range<R>
        {
            if (lhs.done_ || rhs.done_)
                return lhs.done_ == rhs.done_;
            return lhs.current_ == rhs.current_ && lhs.pos_ == rhs.pos_ && lhs.is_error_ == rhs.is_error_;
        }

        constexpr friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit encode_or_error_view(R base, Codec codec = {});

    constexpr const R& base() const& noexcept { return base_; }
    constexpr R        base() && { return std::move(base_); }

    constexpr iterator begin();
    constexpr iterator begin() const
        requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>;
    constexpr iterator end()
        requires std::ranges::forward_range<R> && std::ranges::common_range<R>;
    constexpr iterator end() const
        requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
                 detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>;
    constexpr std::default_sentinel_t end() const;
};

// ---------------------------------------------------------------------------
// encode_closure — pipe adaptor for encode_view
// ---------------------------------------------------------------------------

template <encode_codec Codec>
struct encode_closure {
    Codec codec_;

    template <unicode_scalar_range R>
    constexpr auto operator()(R&& r) const;

    template <unicode_scalar_range R>
    constexpr friend auto operator|(R&& r, const encode_closure& self) {
        return self(std::forward<R>(r));
    }

    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const encode_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "transcode: raw arrays are not valid input to encode");
    }
};

template <encode_codec Codec>
struct encode_or_error_closure {
    Codec codec_;

    template <unicode_scalar_range R>
    constexpr auto operator()(R&& r) const;

    template <unicode_scalar_range R>
    constexpr friend auto operator|(R&& r, const encode_or_error_closure& self) {
        return self(std::forward<R>(r));
    }

    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const encode_or_error_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "transcode: raw arrays are not valid input to encode_or_error");
    }
};

template <encode_codec Codec>
constexpr encode_closure<Codec> encode(Codec codec = {}) {
    return {codec};
}

template <encode_codec Codec>
constexpr encode_or_error_closure<Codec> encode_or_error(Codec codec = {}) {
    return {codec};
}

// ===========================================================================
// Out-of-line definitions: encode_view
// ===========================================================================

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr encode_view<Codec, R>::encode_view(R base, Codec codec) : base_(std::move(base)), codec_(codec) {}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr encode_view<Codec, R>::iterator::iterator(base_iter current, base_sent end, Codec codec)
    : current_(current), end_(end), codec_(codec) {
    load();
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_view<Codec, R>::iterator::terminal() -> iterator
    requires std::ranges::forward_range<R>
{
    iterator it;
    it.done_ = true;
    return it;
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void encode_view<Codec, R>::iterator::load() {
    if (current_ == end_) {
        done_ = true;
        return;
    }
    auto r = codec_.encode_one(*current_++);
    if (r.is_error) {
        buf_[0] = static_cast<unsigned char>('?');
        len_    = 1;
    } else {
        for (int i = 0; i < r.count; ++i)
            buf_[static_cast<std::size_t>(i)] = r.bytes[static_cast<std::size_t>(i)];
        len_ = r.count;
    }
    pos_ = 0;
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr char encode_view<Codec, R>::iterator::operator*() const {
    return static_cast<char>(buf_[static_cast<std::size_t>(pos_)]);
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_view<Codec, R>::iterator::operator++() -> iterator& {
    ++pos_;
    if (pos_ == len_)
        load();
    return *this;
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_view<Codec, R>::iterator::operator++(int) -> iterator
    requires std::ranges::forward_range<R>
{
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void encode_view<Codec, R>::iterator::operator++(int)
    requires(!std::ranges::forward_range<R>)
{
    ++*this;
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_view<Codec, R>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_), codec_);
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_view<Codec, R>::begin() const -> iterator
    requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator(std::ranges::begin(base_), std::ranges::end(base_), codec_);
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_view<Codec, R>::end() -> iterator
    requires std::ranges::forward_range<R> && std::ranges::common_range<R>
{
    return iterator::terminal();
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_view<Codec, R>::end() const -> iterator
    requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
             detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator::terminal();
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr std::default_sentinel_t encode_view<Codec, R>::end() const {
    return {};
}

// ===========================================================================
// Out-of-line definitions: encode_or_error_view
// ===========================================================================

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr encode_or_error_view<Codec, R>::encode_or_error_view(R base, Codec codec)
    : base_(std::move(base)), codec_(codec) {}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr encode_or_error_view<Codec, R>::iterator::iterator(base_iter current, base_sent end, Codec codec)
    : current_(current), end_(end), codec_(codec) {
    load();
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_or_error_view<Codec, R>::iterator::terminal() -> iterator
    requires std::ranges::forward_range<R>
{
    iterator it;
    it.done_ = true;
    return it;
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void encode_or_error_view<Codec, R>::iterator::load() {
    if (current_ == end_) {
        done_ = true;
        return;
    }
    auto r = codec_.encode_one(*current_++);
    if (r.is_error) {
        is_error_ = true;
        len_      = 1;
    } else {
        is_error_ = false;
        for (int i = 0; i < r.count; ++i)
            buf_[static_cast<std::size_t>(i)] = r.bytes[static_cast<std::size_t>(i)];
        len_ = r.count;
    }
    pos_ = 0;
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_or_error_view<Codec, R>::iterator::operator*() const -> result_t {
    if (is_error_)
        return std::unexpected(decode_error::unmapped_codepoint);
    return static_cast<char>(buf_[static_cast<std::size_t>(pos_)]);
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_or_error_view<Codec, R>::iterator::operator++() -> iterator& {
    if (is_error_) {
        load();
    } else {
        ++pos_;
        if (pos_ == len_)
            load();
    }
    return *this;
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_or_error_view<Codec, R>::iterator::operator++(int) -> iterator
    requires std::ranges::forward_range<R>
{
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr void encode_or_error_view<Codec, R>::iterator::operator++(int)
    requires(!std::ranges::forward_range<R>)
{
    ++*this;
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_or_error_view<Codec, R>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_), codec_);
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_or_error_view<Codec, R>::begin() const -> iterator
    requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator(std::ranges::begin(base_), std::ranges::end(base_), codec_);
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_or_error_view<Codec, R>::end() -> iterator
    requires std::ranges::forward_range<R> && std::ranges::common_range<R>
{
    return iterator::terminal();
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr auto encode_or_error_view<Codec, R>::end() const -> iterator
    requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
             detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator::terminal();
}

template <encode_codec Codec, std::ranges::input_range R>
    requires unicode_scalar_range<R>
constexpr std::default_sentinel_t encode_or_error_view<Codec, R>::end() const {
    return {};
}

// ===========================================================================
// Out-of-line definitions: encode_closure, encode_or_error_closure
// ===========================================================================

template <encode_codec Codec>
template <unicode_scalar_range R>
constexpr auto encode_closure<Codec>::operator()(R&& r) const {
    using view_t = std::views::all_t<R>;
    auto all     = std::views::all(std::forward<R>(r));
    return encode_view<Codec, view_t>(std::move(all), codec_);
}

template <encode_codec Codec>
template <unicode_scalar_range R>
constexpr auto encode_or_error_closure<Codec>::operator()(R&& r) const {
    using view_t = std::views::all_t<R>;
    auto all     = std::views::all(std::forward<R>(r));
    return encode_or_error_view<Codec, view_t>(std::move(all), codec_);
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_ENCODE_VIEW_HPP
