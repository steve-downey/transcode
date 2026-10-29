// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DECODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_DECODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/concepts.hpp>
#include <beman/transcode/detail/range_traits.hpp>
#include <beman/transcode/whatwg/error.hpp>
#include <beman/transcode/error.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <concepts>
    #include <expected>
    #include <iterator>
    #include <ranges>
    #include <type_traits>

#endif
namespace beman::transcoding {

// ---------------------------------------------------------------------------
// random_access_decode_view
// ---------------------------------------------------------------------------

//! \omit
template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E = error_mode::replacement>
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
            std::conditional_t<E == error_mode::report, std::expected<char32_t, whatwg::error>, char32_t>;

        static constexpr value_type error_result(whatwg::error e) {
            if constexpr (E == error_mode::report)
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
// decode_view — decodes bytes to expected<char32_t, whatwg::error>
// ---------------------------------------------------------------------------

// \ref{transcode.codec.decode}, class template decode_view

//! \at transcode.codec.decode
//! \remarks `decode_view<Codec, R, E>` presents the bytes of `R` as the
//! Unicode scalar values `Codec` decodes them to, and decodes lazily.  When
//! `E` is `error_mode::replacement`, its value type is `char32_t` and a
//! decoding error yields U+FFFD.  When `E` is `error_mode::report`, its value
//! type is `expected<char32_t, whatwg::error>` and a decoding error yields an
//! element holding the error.  The view does not
//! validate a successful `code_point`; the program-supplied `Codec` is
//! responsible for returning Unicode scalar values as required by
//! `text_decoder`.
//!
//! The view models `random_access_range` when `Codec` models
//! `single_byte_text_decoder` and `R` models `random_access_range`.
//! When it does, a byte whose value is `0x80` or above and for which
//! `Codec::decode_byte` returns U+FFFD is reported as
//! `whatwg::error::invalid_byte` in report mode and yields U+FFFD in
//! replacement mode.  This follows from the U+FFFD reservation required by
//! `single_byte_text_decoder` \iref{transcode.codec.reqs}.
template <text_decoder Codec, std::ranges::input_range R, error_mode E = error_mode::replacement>
    requires legacy_byte_range<R>
class decode_view : public std::ranges::view_interface<decode_view<Codec, R, E>> {
    //! \expos
    R base_;
    //! \expos
    [[no_unique_address]] Codec codec_;

    //! \expos
    //! \seebelow
    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;
        using base_sent = detail::compatible_sentinel_t<R>;
        using result_t = std::conditional_t<E == error_mode::report, std::expected<char32_t, whatwg::error>, char32_t>;

        //! \expos
        static constexpr result_t error_result(whatwg::error e) {
            if constexpr (E == error_mode::report)
                return result_t(std::unexpect, e);
            else
                return U'\xFFFD';
        }

        //! \expos
        base_iter current_{};
        //! \expos
        base_sent end_{};
        //! \expos
        [[no_unique_address]] Codec codec_{};
        //! \expos
        result_t value_{};
        //! \expos
        bool done_{false};

        //! \expos
        constexpr void load();
        //! \expos
        constexpr void load_first();

      public:
        using iterator_concept =
            std::conditional_t<std::ranges::forward_range<R>, std::forward_iterator_tag, std::input_iterator_tag>;
        using iterator_category = iterator_concept;
        using value_type        = result_t;
        using difference_type   = std::ptrdiff_t;
        using reference         = result_t;

        constexpr iterator() = default;

        //! \expos
        static constexpr iterator terminal()
            requires std::ranges::forward_range<R>;

        // \ref{transcode.codec.decode.iterator}, iterator operations
        constexpr iterator(base_iter current, base_sent end, Codec codec);

        //! \returns The iterator into `R` this iterator reads from, positioned
        //! at the first byte of the element it has not yet decoded.
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
            if (!(lhs.current_ == rhs.current_ && lhs.value_ == rhs.value_))
                return false;
            if constexpr (std::equality_comparable<Codec>)
                return lhs.codec_ == rhs.codec_;
            else
                return true;
        }

        constexpr friend bool operator==(const iterator& it, std::default_sentinel_t) { return it.done_; }
    };

  public:
    // \ref{transcode.codec.decode}, construction and access
    constexpr explicit decode_view(R base, Codec codec = {});

    //! \returns-expr
    constexpr const R& base() const& noexcept { return base_; }
    //! \returns-expr
    constexpr R base() && { return std::move(base_); }

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

//! \omit
template <text_decoder Codec, error_mode E = error_mode::replacement>
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
                      "transcode: raw arrays are not valid input to decode; "
                      "use beman::transcoding::views::null_term to get a range from a "
                      "null-terminated string");
    }
};

//! \seebelow
//! \returns A range adaptor object over `codec`.  Given a subexpression `E`
//! that models `legacy_byte_range`, `decode(codec)(E)` and `E | decode(codec)`
//! are each expression-equivalent to a
//! `decode_view<Codec, views::all_t<decltype((E))>>` over `E` and a copy of
//! `codec`.  An `E` of array type is ill-formed.
template <text_decoder Codec>
constexpr auto decode(Codec codec = {}) {
    return decode_closure<Codec>{codec};
}

//! \seebelow
//! \returns `decode(codec)` with `error_mode::report`: the view it
//! adapts to has value type `expected<char32_t, whatwg::error>`, and a decoding
//! error is the error rather than U+FFFD.
template <text_decoder Codec>
constexpr auto decode_or_error(Codec codec = {}) {
    return decode_closure<Codec, error_mode::report>{codec};
}

// ===========================================================================
// Out-of-line definitions: random_access_decode_view
// ===========================================================================

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr random_access_decode_view<Codec, R, E>::random_access_decode_view(R base, Codec codec)
    : base_(std::move(base)), codec_(codec) {}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr random_access_decode_view<Codec, R, E>::iterator::iterator(base_iter current, Codec codec)
    : current_(current), codec_(codec) {}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator*() const -> value_type {
    const auto byte = static_cast<unsigned char>(*current_);
    char32_t   cp   = codec_.decode_byte(byte);
    if (cp == U'\xFFFD' && byte >= 0x80)
        return error_result(whatwg::error::invalid_byte);
    return cp;
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator[](difference_type n) const -> value_type {
    return *(*this + n);
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator++() -> iterator& {
    ++current_;
    return *this;
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator++(int) -> iterator {
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator--() -> iterator& {
    --current_;
    return *this;
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator--(int) -> iterator {
    auto tmp = *this;
    --*this;
    return tmp;
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator+=(difference_type n) -> iterator& {
    current_ += n;
    return *this;
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator-=(difference_type n) -> iterator& {
    current_ -= n;
    return *this;
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator+(difference_type n) const -> iterator {
    auto tmp = *this;
    return tmp += n;
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator-(difference_type n) const -> iterator {
    auto tmp = *this;
    return tmp -= n;
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::iterator::operator-(const iterator& other) const
    -> difference_type {
    return current_ - other.current_;
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), codec_);
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
//! \omit
constexpr auto random_access_decode_view<Codec, R, E>::begin() const -> iterator
    requires detail::const_iterator_compatible_range<R>
{
    return iterator(std::ranges::begin(base_), codec_);
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr auto random_access_decode_view<Codec, R, E>::end() -> iterator {
    return iterator(std::ranges::end(base_), codec_);
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
//! \omit
constexpr auto random_access_decode_view<Codec, R, E>::end() const -> iterator
    requires detail::const_iterator_compatible_range<R> && std::ranges::range<const R>
{
    return iterator(std::ranges::end(base_), codec_);
}

template <single_byte_text_decoder Codec, std::ranges::random_access_range R, error_mode E>
    requires legacy_byte_range<R>
//! \omit
constexpr auto random_access_decode_view<Codec, R, E>::size() const
    requires std::ranges::sized_range<R>
{
    return std::ranges::size(base_);
}

// ===========================================================================
// Out-of-line definitions: decode_view
// ===========================================================================

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \effects Initializes `$base$` with `std::move(base)` and `$codec$` with
//! `codec`.
constexpr decode_view<Codec, R, E>::decode_view(R base, Codec codec) : base_(std::move(base)), codec_(codec) {}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \effects Initializes `$current$` with `std::move(current)`, `$end$` with
//! `std::move(end)` and the iterator's own copy of the codec with `codec`,
//! then decodes the first element.  When `Codec` models
//! `bom_aware_text_decoder` and `current != end`, that element is
//! decoded by `decode_first`, and, if its result has `is_empty`, decoding goes
//! on as `operator++` does.
constexpr decode_view<Codec, R, E>::iterator::iterator(base_iter current, base_sent end, Codec codec)
    : current_(current), end_(end), codec_(codec) {
    load_first();
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \omit
constexpr auto decode_view<Codec, R, E>::iterator::terminal() -> iterator
    requires std::ranges::forward_range<R>
{
    iterator it;
    it.done_ = true;
    return it;
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr void decode_view<Codec, R, E>::iterator::load_first() {
    if constexpr (bom_aware_text_decoder<Codec>) {
        if (current_ != end_) {
            auto r = codec_.decode_first(current_, end_);
            if (!r.is_empty) {
                if (r.is_error)
                    value_ = error_result(r.error);
                else
                    value_ = r.code_point;
                return;
            }
        }
    }
    load();
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
constexpr void decode_view<Codec, R, E>::iterator::load() {
    while (true) {
        if (current_ == end_) {
            if constexpr (flushable_text_decoder<Codec>) {
                auto flushed = codec_.flush();
                if (flushed && !flushed->is_empty) {
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
        if (r.is_empty)
            continue;
        if (r.is_error)
            value_ = error_result(r.error);
        else
            value_ = r.code_point;
        return;
    }
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \returns-expr
constexpr auto decode_view<Codec, R, E>::iterator::operator*() const -> result_t {
    return value_;
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \effects Decodes the next element of the base range, or, if none remains,
//! makes `*this` equal to `end()`.  A `decode_one` result with `is_empty`
//! yields no element and decoding goes on.  When `Codec` models
//! `flushable_text_decoder`, exhausting the base range calls `flush()`, and
//! calls it again after each engaged result, yielding what it returns until
//! it returns `nullopt` or a result with `is_empty`.
//! \returns `*this`.
constexpr auto decode_view<Codec, R, E>::iterator::operator++() -> iterator& {
    load();
    return *this;
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \effects-equiv
constexpr auto decode_view<Codec, R, E>::iterator::operator++(int) -> iterator
    requires std::ranges::forward_range<R>
{
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \effects-equiv
constexpr void decode_view<Codec, R, E>::iterator::operator++(int)
    requires(!std::ranges::forward_range<R>)
{
    ++*this;
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \returns An `$iterator$` over `$base$` and a copy of `$codec$`, positioned
//! at its first decoded element.
constexpr auto decode_view<Codec, R, E>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_), codec_);
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \returns An `$iterator$` over `$base$` and a copy of `$codec$`, positioned
//! at its first decoded element.
constexpr auto decode_view<Codec, R, E>::begin() const -> iterator
    requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator(std::ranges::begin(base_), std::ranges::end(base_), codec_);
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \returns An `$iterator$` that compares equal to an iterator that has
//! decoded every element of `$base$`.
constexpr auto decode_view<Codec, R, E>::end() -> iterator
    requires std::ranges::forward_range<R> && std::ranges::common_range<R>
{
    return iterator::terminal();
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \returns An `$iterator$` that compares equal to an iterator that has
//! decoded every element of `$base$`.
constexpr auto decode_view<Codec, R, E>::end() const -> iterator
    requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
             detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator::terminal();
}

template <text_decoder Codec, std::ranges::input_range R, error_mode E>
    requires legacy_byte_range<R>
//! \returns `default_sentinel`.
constexpr std::default_sentinel_t decode_view<Codec, R, E>::end() const {
    return {};
}

// ===========================================================================
// Out-of-line definitions: decode_closure
// ===========================================================================

template <text_decoder Codec, error_mode E>
template <legacy_byte_range R>
constexpr auto decode_closure<Codec, E>::operator()(R&& r) const {
    using view_t = std::views::all_t<R>;
    auto all     = std::views::all(std::forward<R>(r));
    if constexpr (single_byte_text_decoder<Codec> && std::ranges::random_access_range<view_t>)
        return random_access_decode_view<Codec, view_t, E>(std::move(all), codec_);
    else
        return decode_view<Codec, view_t, E>(std::move(all), codec_);
}

} // namespace beman::transcoding

namespace std::ranges {

//! \omit
template <beman::transcoding::single_byte_text_decoder Codec, random_access_range R, beman::transcoding::error_mode E>
    requires beman::transcoding::legacy_byte_range<R>
inline constexpr bool enable_borrowed_range<beman::transcoding::random_access_decode_view<Codec, R, E>> =
    borrowed_range<R>;

template <beman::transcoding::text_decoder Codec, input_range R, beman::transcoding::error_mode E>
    requires beman::transcoding::legacy_byte_range<R>
inline constexpr bool enable_borrowed_range<beman::transcoding::decode_view<Codec, R, E>> = borrowed_range<R>;

} // namespace std::ranges

#endif // INCLUDE_BEMAN_TRANSCODE_DECODE_VIEW_HPP
