// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ENCODE_VIEW_HPP
#define INCLUDE_BEMAN_TRANSCODE_ENCODE_VIEW_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec_concepts.hpp>
#include <beman/transcode/concepts.hpp>
#include <beman/transcode/detail/range_traits.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <array>
    #include <expected>
    #include <iterator>
    #include <ranges>
    #include <type_traits>

#endif
namespace beman::transcoding {

// ---------------------------------------------------------------------------
// encode_view — encodes char32_t scalars to expected<char, whatwg_error>
// ---------------------------------------------------------------------------

// \ref{transcode.custom.encode}, class template encode_view

//! \remarks `encode_view<Codec, R, E>` is `whatwg_encode_view`
//! \iref{transcode.whatwg.encode} with the codec supplied as a value rather
//! than named by an enumerator: it presents the Unicode scalar values of `R`
//! as the bytes `Codec` encodes them to, reports an encoding error as `E`
//! says, and encodes lazily.  Each element of `R` is required to be a Unicode
//! scalar value, which is a precondition and not a constraint.
template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E = transcode_error_kind::replacement>
    requires unicode_scalar_range<R>
class encode_view : public std::ranges::view_interface<encode_view<Codec, R, E>> {
    //! \expos
    R base_;
    //! \expos
    Codec codec_;

    //! \expos
    //! \seebelow
    class iterator {
        using base_iter = detail::compatible_iterator_t<R>;
        using base_sent = detail::compatible_sentinel_t<R>;
        using result_t =
            std::conditional_t<E == transcode_error_kind::expected, std::expected<char, whatwg_error>, char>;

        //! \expos
        base_iter current_{};
        //! \expos
        base_sent end_{};
        //! \expos
        Codec codec_{};
        //! \expos
        std::array<unsigned char, 8> buf_{};
        //! \expos
        int len_{0};
        //! \expos
        int pos_{0};
        //! \expos
        bool is_error_{false};
        //! \expos
        bool done_{false};

        //! \expos
        constexpr void load();

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

        // \ref{transcode.custom.encode.iterator}, iterator operations
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
    // \ref{transcode.custom.encode}, construction and access
    constexpr explicit encode_view(R base, Codec codec = {});

    //! \returns-equiv
    constexpr const R& base() const& noexcept { return base_; }
    //! \returns-equiv
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
// encode_closure — pipe adaptor for encode_view
// ---------------------------------------------------------------------------

//! \omit
template <encode_codec Codec, transcode_error_kind E = transcode_error_kind::replacement>
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
        static_assert(!std::is_array_v<std::remove_cvref_t<R>>, "transcode: raw arrays are not valid input to encode");
    }
};

//! \seebelow
//! \returns A range adaptor object over `codec`.  Given a subexpression `E`
//! that models `unicode_scalar_range`, `encode(codec)(E)` and
//! `E | encode(codec)` are each expression-equivalent to an
//! `encode_view<Codec, views::all_t<decltype((E))>>` over `E` and a copy of
//! `codec`.
template <encode_codec Codec>
constexpr encode_closure<Codec> encode(Codec codec = {}) {
    return {codec};
}

//! \seebelow
//! \returns `encode(codec)` with `transcode_error_kind::expected`: the view it
//! adapts to has value type `expected<char, whatwg_error>`, and an encoding
//! error is the error rather than `'?'`.
template <encode_codec Codec>
constexpr encode_closure<Codec, transcode_error_kind::expected> encode_or_error(Codec codec = {}) {
    return {codec};
}

// ===========================================================================
// Out-of-line definitions: encode_view
// ===========================================================================

// ===========================================================================
// Out-of-line definitions: encode_view
// ===========================================================================

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
//! \effects Initializes `$base$` with `std::move(base)` and `$codec$` with
//! `codec`.
constexpr encode_view<Codec, R, E>::encode_view(R base, Codec codec) : base_(std::move(base)), codec_(codec) {}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
//! \effects Initializes `$current$` with `std::move(current)`, `$end$` with
//! `std::move(end)` and the iterator's own copy of the codec with `codec`,
//! then encodes the first element.
constexpr encode_view<Codec, R, E>::iterator::iterator(base_iter current, base_sent end, Codec codec)
    : current_(current), end_(end), codec_(codec) {
    load();
}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
constexpr auto encode_view<Codec, R, E>::iterator::terminal() -> iterator
    requires std::ranges::forward_range<R>
{
    iterator it;
    it.done_ = true;
    return it;
}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
constexpr void encode_view<Codec, R, E>::iterator::load() {
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

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
//! \returns The byte at the current position of the encoded element, or the
//! error the encoding of that element produced.
constexpr auto encode_view<Codec, R, E>::iterator::operator*() const -> result_t {
    if (is_error_) {
        // The lossy encoder substitutes '?'; the expected one reports why.
        if constexpr (E == transcode_error_kind::expected)
            return result_t(std::unexpect, whatwg_error::unmapped_codepoint);
        else
            return '?';
    }
    return static_cast<char>(buf_[static_cast<std::size_t>(pos_)]);
}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
//! \effects Advances to the next byte of the encoded element, encoding the
//! next element of the base range when the current one is exhausted; if no
//! element remains, makes `*this` equal to `end()`.
//! \returns `*this`.
constexpr auto encode_view<Codec, R, E>::iterator::operator++() -> iterator& {
    if (is_error_) {
        load();
    } else {
        ++pos_;
        if (pos_ == len_)
            load();
    }
    return *this;
}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
constexpr auto encode_view<Codec, R, E>::iterator::operator++(int) -> iterator
    requires std::ranges::forward_range<R>
{
    auto tmp = *this;
    ++*this;
    return tmp;
}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
constexpr void encode_view<Codec, R, E>::iterator::operator++(int)
    requires(!std::ranges::forward_range<R>)
{
    ++*this;
}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
//! \returns An `$iterator$` over `$base$` and a copy of `$codec$`, positioned
//! at the first byte of its first encoded element.
constexpr auto encode_view<Codec, R, E>::begin() -> iterator {
    return iterator(std::ranges::begin(base_), std::ranges::end(base_), codec_);
}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
//! \returns An `$iterator$` over `$base$` and a copy of `$codec$`, positioned
//! at the first byte of its first encoded element.
constexpr auto encode_view<Codec, R, E>::begin() const -> iterator
    requires detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator(std::ranges::begin(base_), std::ranges::end(base_), codec_);
}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
//! \returns An `$iterator$` that compares equal to an iterator that has
//! encoded every element of `$base$`.
constexpr auto encode_view<Codec, R, E>::end() -> iterator
    requires std::ranges::forward_range<R> && std::ranges::common_range<R>
{
    return iterator::terminal();
}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
//! \returns An `$iterator$` that compares equal to an iterator that has
//! encoded every element of `$base$`.
constexpr auto encode_view<Codec, R, E>::end() const -> iterator
    requires std::ranges::forward_range<const R> && std::ranges::common_range<const R> &&
             detail::const_iterator_compatible_range<R> && detail::const_sentinel_compatible_range<R>
{
    return iterator::terminal();
}

template <encode_codec Codec, std::ranges::input_range R, transcode_error_kind E>
    requires unicode_scalar_range<R>
//! \returns `default_sentinel`.
constexpr std::default_sentinel_t encode_view<Codec, R, E>::end() const {
    return {};
}

// ===========================================================================
// Out-of-line definitions: encode_closure, encode_closure
// ===========================================================================

template <encode_codec Codec, transcode_error_kind E>
template <unicode_scalar_range R>
constexpr auto encode_closure<Codec, E>::operator()(R&& r) const {
    using view_t = std::views::all_t<R>;
    auto all     = std::views::all(std::forward<R>(r));
    return encode_view<Codec, view_t, E>(std::move(all), codec_);
}

// The names the paired views had before they were unified.
template <encode_codec Codec, std::ranges::input_range R>
using encode_or_error_view = encode_view<Codec, R, transcode_error_kind::expected>;

template <encode_codec Codec>
using encode_or_error_closure = encode_closure<Codec, transcode_error_kind::expected>;

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_ENCODE_VIEW_HPP
