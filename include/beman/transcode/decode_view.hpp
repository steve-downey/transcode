// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DECODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_DECODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/concepts.hpp>
#include <beman/transcode/detail/range_traits.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <expected>
    #include <iterator>
    #include <ranges>
    #include <type_traits>

#endif
namespace beman::transcoding {

// ---------------------------------------------------------------------------
// random_access_decode_view
// ---------------------------------------------------------------------------

template <random_access_decode_codec_type  Codec,
          std::ranges::random_access_range R,
          transcode_error_kind             E = transcode_error_kind::replacement>
    requires legacy_byte_range<R>
class random_access_decode_view : public std::ranges::view_interface<random_access_decode_view<Codec, R, E>> {
    R     base_;
    Codec codec_;

    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;

        base_iter current_{};
        Codec     codec_{};

      public:
        using value_type =
            std::conditional_t<E == transcode_error_kind::expected, std::expected<char32_t, whatwg_error>, char32_t>;

        static constexpr value_type error_result(whatwg_error e) {
            if constexpr (E == transcode_error_kind::expected)
                return value_type(std::unexpect, e);
            else
                return U'\xFFFD';
        }
        using iterator_concept  = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using reference         = value_type;

        constexpr iterator() = default;
        constexpr iterator(base_iter current, Codec codec);

        constexpr const base_iter& base() const noexcept { return current_; }

        constexpr value_type operator*() const;
        constexpr value_type operator[](difference_type n) const;

        constexpr iterator&       operator++();
        constexpr iterator        operator++(int);
        constexpr iterator&       operator--();
        constexpr iterator        operator--(int);
        constexpr iterator&       operator+=(difference_type n);
        constexpr iterator&       operator-=(difference_type n);
        constexpr iterator        operator+(difference_type n) const;
        constexpr iterator        operator-(difference_type n) const;
        constexpr difference_type operator-(const iterator& other) const;

        constexpr friend iterator operator+(difference_type n, iterator it) { return it += n; }

        constexpr friend bool operator==(const iterator& lhs, const iterator& rhs) {
            return lhs.current_ == rhs.current_;
        }

        constexpr friend auto operator<=>(const iterator& lhs, const iterator& rhs) {
            return lhs.current_ <=> rhs.current_;
        }
    };

  public:
    constexpr random_access_decode_view(R base, Codec codec);

    constexpr const R& base() const& noexcept { return base_; }
    constexpr R        base() && { return std::move(base_); }

    constexpr iterator begin();
    constexpr iterator begin() const
        requires detail::const_iterator_compatible_range<R>;
    constexpr iterator end();
    constexpr iterator end() const
        requires detail::const_iterator_compatible_range<R> && std::ranges::range<const R>;
    constexpr auto size() const
        requires std::ranges::sized_range<R>;
};

// ---------------------------------------------------------------------------
// decode_view — decodes bytes to expected<char32_t, whatwg_error>
// ---------------------------------------------------------------------------

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E = transcode_error_kind::replacement>
    requires legacy_byte_range<R>
class decode_view : public std::ranges::view_interface<decode_view<Codec, R, E>> {
    R     base_;
    Codec codec_;

    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;
        using base_sent = detail::compatible_sentinel_t<R>;
        using result_t =
            std::conditional_t<E == transcode_error_kind::expected, std::expected<char32_t, whatwg_error>, char32_t>;

        static constexpr result_t error_result(whatwg_error e) {
            if constexpr (E == transcode_error_kind::expected)
                return result_t(std::unexpect, e);
            else
                return U'\xFFFD';
        }

        base_iter current_{};
        base_sent end_{};
        Codec     codec_;
        result_t  value_;
        bool      done_{false};

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

        constexpr const base_iter& base() const noexcept { return current_; }

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
            return lhs.current_ == rhs.current_ && lhs.done_ == rhs.done_;
        }

        constexpr friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    constexpr explicit decode_view(R base, Codec codec = {});

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
// decode_closure — pipe adapter
// ---------------------------------------------------------------------------

template <decode_codec Codec, transcode_error_kind E = transcode_error_kind::replacement>
struct decode_closure {
    Codec codec_;

    template <legacy_byte_range R>
    constexpr auto operator()(R&& r) const;

    template <legacy_byte_range R>
    constexpr friend auto operator|(R&& r, const decode_closure& self) {
        return self(std::forward<R>(r));
    }

    template <typename R>
        requires std::is_array_v<std::remove_cvref_t<R>>
    friend auto operator|(R&&, const decode_closure&) {
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>,
                      "transcode: raw arrays are not valid input to decode_or_error; "
                      "use beman::transcoding::views::null_term to get a range from a "
                      "null-terminated string");
    }
};

template <decode_codec Codec>
constexpr decode_closure<Codec> decode(Codec codec = {}) {
    return {codec};
}

template <decode_codec Codec>
constexpr decode_closure<Codec, transcode_error_kind::expected> decode_or_error(Codec codec = {}) {
    return {codec};
}

// ===========================================================================
// Out-of-line definitions: random_access_decode_view
// ===========================================================================

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr random_access_decode_view<Codec, R, E>::random_access_decode_view(R base, Codec codec)
    : base_(std::move(base)), codec_(codec) {}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr random_access_decode_view<Codec, R, E>::iterator::iterator(base_iter current, Codec codec)
    : current_(current), codec_(codec) {}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator*() const -> value_type {
    const auto byte = static_cast<unsigned char>(*current_);
    char32_t   cp   = codec_.decode_byte(byte);
    if (cp == U'\xFFFD' && byte >= 0x80)
        return error_result(whatwg_error::invalid_byte);
    return cp;
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator[](difference_type n) const -> value_type {
    return *(*this + n);
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator++() -> iterator& {
    ++current_;
    return *this;
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator++(int) -> iterator {
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator--() -> iterator& {
    --current_;
    return *this;
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator--(int) -> iterator {
    auto tmp = *this;
    --*this;
    return tmp;
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator+=(difference_type n) -> iterator& {
    current_ += n;
    return *this;
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator-=(difference_type n) -> iterator& {
    current_ -= n;
    return *this;
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator+(difference_type n) const -> iterator {
    auto tmp = *this;
    return tmp += n;
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator-(difference_type n) const -> iterator {
    auto tmp = *this;
    return tmp -= n;
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator-(const iterator& other) const
    -> difference_type {
    return current_ - other.current_;
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), codec_);
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::begin() const -> iterator
    requires detail::const_iterator_compatible_range<R>
{
    return iterator(std::ranges::begin(base_), codec_);
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::end() -> iterator {
    return iterator(std::ranges::end(base_), codec_);
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::end() const -> iterator
    requires detail::const_iterator_compatible_range<R> && std::ranges::range<const R>
{
    return iterator(std::ranges::end(base_), codec_);
}

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::size() const
    requires std::ranges::sized_range<R>
{
    return std::ranges::size(base_);
}

// ===========================================================================
// Out-of-line definitions: decode_view
// ===========================================================================

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr decode_view<Codec, R, E>::decode_view(R base, Codec codec) : base_(std::move(base)), codec_(codec) {}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr decode_view<Codec, R, E>::iterator::iterator(base_iter current, base_sent end, Codec codec)
    : current_(current), end_(end), codec_(codec) {
    load();
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto decode_view<Codec, R, E>::iterator::terminal() -> iterator
    requires std::ranges::forward_range<R>
{
    iterator it;
    it.done_ = true;
    return it;
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr void decode_view<Codec, R, E>::iterator::load() {
    if (current_ == end_) {
        if constexpr (flushable_decode_codec<Codec>) {
            auto flushed = codec_.flush();
            if (flushed) {
                if (flushed->is_error)
                    value_ = error_result(flushed->error);
                else
                    value_ = flushed->code_point;
                return;
            }
        }
        done_ = true;
        return;
    }
    auto r = codec_.decode_one(current_, end_);
    if (r.is_error)
        value_ = error_result(r.error);
    else
        value_ = r.code_point;
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto decode_view<Codec, R, E>::iterator::operator*() const -> result_t {
    return value_;
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto decode_view<Codec, R, E>::iterator::operator++() -> iterator& {
    load();
    return *this;
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto decode_view<Codec, R, E>::iterator::operator++(int) -> iterator
    requires std::ranges::forward_range<R>
{
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr void decode_view<Codec, R, E>::iterator::operator++(int)
    requires(!std::ranges::forward_range<R>)
{
    ++*this;
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto decode_view<Codec, R, E>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_), codec_);
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto decode_view<Codec, R, E>::begin() const -> iterator
    requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator(std::ranges::begin(base_), std::ranges::end(base_), codec_);
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto decode_view<Codec, R, E>::end() -> iterator
    requires std::ranges::forward_range<R> && std::ranges::common_range<R>
{
    return iterator::terminal();
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr auto decode_view<Codec, R, E>::end() const -> iterator
    requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
             detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator::terminal();
}

template <decode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires legacy_byte_range<R>
constexpr std::default_sentinel_t decode_view<Codec, R, E>::end() const {
    return {};
}

// ===========================================================================
// Out-of-line definitions: decode_closure
// ===========================================================================

template <decode_codec Codec, transcode_error_kind E>
template <legacy_byte_range R>
constexpr auto decode_closure<Codec, E>::operator()(R&& r) const {
    using view_t = std::views::all_t<R>;
    auto all     = std::views::all(std::forward<R>(r));
    if constexpr (random_access_decode_codec_type<Codec> && std::ranges::random_access_range<view_t>)
        return random_access_decode_view<Codec, view_t, E>(std::move(all), codec_);
    else
        return decode_view<Codec, view_t, E>(std::move(all), codec_);
}

// The names the paired views had before they were unified.
template <decode_codec Codec, std::ranges::input_range R>
using decode_or_error_view = decode_view<Codec, R, transcode_error_kind::expected>;

template <random_access_decode_codec_type Codec, std::ranges::random_access_range R>
using random_access_decode_or_error_view = random_access_decode_view<Codec, R, transcode_error_kind::expected>;

template <decode_codec Codec>
using decode_or_error_closure = decode_closure<Codec, transcode_error_kind::expected>;

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DECODE_VIEW_HPP
