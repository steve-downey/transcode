// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODER_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODER_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec_result.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/whatwg/error.hpp>
#include <beman/transcode/detail/whatwg_decode_select.hpp>
#include <beman/transcode/detail/single_byte.hpp>
#include <beman/transcode/detail/big5.hpp>
#include <beman/transcode/detail/euc_jp.hpp>
#include <beman/transcode/detail/euc_kr.hpp>
#include <beman/transcode/detail/gb18030.hpp>
#include <beman/transcode/detail/iso2022jp.hpp>
#include <beman/transcode/detail/shift_jis.hpp>
#include <beman/transcode/detail/utf8.hpp>
#include <beman/transcode/detail/utf16.hpp>
#include <beman/transcode/detail/x_user_defined.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <concepts>
    #include <iterator>
    #include <optional>
    #include <utility>

#endif
namespace beman::transcoding::whatwg {

// \ref{transcode.whatwg.decoders}, decoders

//! \remarks `whatwg::decoder<C>` is the decoder, as a codec object, for the
//! encoding `C` the Encoding Standard defines.  It models `text_decoder`
//! \iref{transcode.custom.reqs} for every encoding, and the decode adaptor
//! over it \iref{transcode.custom.decode} yields what `whatwg::decode<C>`
//! does.  It models `single_byte_text_decoder` when `C` names an encoding that
//! decodes one byte to one scalar value, `flushable_text_decoder` when
//! `C` names an encoding whose decoding can leave a result owed after the last
//! byte, and `bom_aware_text_decoder` when `C` names a UTF encoding: its
//! `decode_first` discards one leading U+FEFF BYTE ORDER MARK from the text it
//! decodes.  It is equality comparable.
template <encoding C>
class decoder {
    //! \expos
    [[no_unique_address]] detail::decode_state_t<C> state_{};

  public:
    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, S end);

    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_first(I& current, S end)
        requires detail::strips_leading_bom<C>;

    constexpr char32_t decode_byte(unsigned char byte) const
        requires detail::random_access_decode_codec<C>;

    constexpr std::optional<decode_result> flush()
        requires(!std::same_as<detail::decode_state_t<C>, detail::no_decode_state>);

    friend constexpr bool operator==(const decoder&, const decoder&) = default;
};

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg::decoder
// ---------------------------------------------------------------------------

template <encoding C>
template <std::input_iterator I, std::sentinel_for<I> S>
//! \effects Decodes one scalar value from `[current, end)` as the encoding
//! `C` says, advancing `current` past the bytes it consumed.
//! \returns The scalar value or the error, or, for an escape sequence that
//! produced no scalar value, a result with `is_empty`.
constexpr decode_result decoder<C>::decode_one(I& current, S end) {
    const auto failure = [](whatwg::error e) { return decode_result{U'\xFFFD', e, true}; };
    const auto success = [](char32_t cp) { return decode_result{cp, {}, false}; };
    if constexpr (C == encoding::big5) {
        if (state_.has_pending) {
            state_.has_pending = false;
            return success(state_.code_point);
        }
    }
    if constexpr (C == encoding::replacement) {
        while (current != end)
            ++current;
        return failure(whatwg::error::invalid_byte);
    } else if constexpr (C == encoding::utf_8) {
        auto r = detail::utf8_decode_one(current, end);
        return r.is_error ? failure(r.error) : success(r.code_point);
    } else if constexpr (C == encoding::x_user_defined) {
        return success(detail::x_user_defined_decode_one(current, end));
    } else if constexpr (C == encoding::windows_1252) {
        auto r = detail::single_byte_decode_one(current, end, detail::tables::windows_1252);
        if (r.is_error)
            std::unreachable();
        return success(r.code_point);
    } else if constexpr (detail::single_byte_table_codec<C>) {
        auto r = detail::single_byte_decode_one(current, end, detail::single_byte_decode_table<C>());
        return r.is_error ? failure(r.error) : success(r.code_point);
    } else if constexpr (C == encoding::utf_16be || C == encoding::utf_16le) {
        auto r = detail::utf16_decode_one<C == encoding::utf_16be>(state_, current, end);
        return r.is_error ? failure(r.error) : success(r.code_point);
    } else if constexpr (C == encoding::gbk || C == encoding::gb18030) {
        auto r = detail::gb18030_decode_one(state_, current, end);
        return r.is_error ? failure(r.error) : success(r.code_point);
    } else if constexpr (C == encoding::big5) {
        auto r = detail::big5_decode_one(current, end);
        if (r.code_point2 != 0) {
            state_.code_point  = r.code_point2;
            state_.has_pending = true;
        }
        return r.is_error ? failure(r.error) : success(r.code_point);
    } else if constexpr (C == encoding::shift_jis) {
        auto r = detail::shift_jis_decode_one(current, end);
        return r.is_error ? failure(r.error) : success(r.code_point);
    } else if constexpr (C == encoding::euc_jp) {
        auto r = detail::euc_jp_decode_one(current, end);
        return r.is_error ? failure(r.error) : success(r.code_point);
    } else if constexpr (C == encoding::iso_2022_jp) {
        auto r = detail::iso2022jp_decode_one(state_, current, end);
        if (r.done)
            return decode_result{.is_empty = true};
        return r.is_error ? failure(r.error) : success(r.code_point);
    } else {
        static_assert(C == encoding::euc_kr);
        auto r = detail::euc_kr_decode_one(current, end);
        return r.is_error ? failure(r.error) : success(r.code_point);
    }
}

template <encoding C>
template <std::input_iterator I, std::sentinel_for<I> S>
//! \effects Decodes one scalar value as `decode_one(current, end)` does.  If
//! that is U+FEFF, it is discarded and, unless the input is exhausted, the next
//! scalar value decoded in its place.
//! \returns The scalar value or the error, or, for a U+FEFF discarded with no
//! input after it, a result with `is_empty`.
constexpr decode_result decoder<C>::decode_first(I& current, S end)
    requires detail::strips_leading_bom<C>
{
    auto r = decode_one(current, end);
    if (!r.is_error && r.code_point == U'\xFEFF') {
        if (current == end && state_.at_end())
            return decode_result{.is_empty = true};
        return decode_one(current, end);
    }
    return r;
}

template <encoding C>
//! \returns `0xFFFD` if `byte` decodes to nothing in `C`, otherwise the
//! scalar value `byte` decodes to.
constexpr char32_t decoder<C>::decode_byte(unsigned char byte) const
    requires detail::random_access_decode_codec<C>
{
    if constexpr (C == encoding::x_user_defined)
        return detail::decode_x_user_defined_code_unit(byte);
    else
        return detail::decode_random_access_code_unit(byte, detail::random_access_decode_table<C>());
}

template <encoding C>
//! \effects Decodes what the end of the input leaves owed, from an empty
//! range.
//! \returns `nullopt` if nothing is owed.  Otherwise a scalar value still
//! pending, or the error for a sequence the input ended in the middle of.
constexpr std::optional<decode_result> decoder<C>::flush()
    requires(!std::same_as<detail::decode_state_t<C>, detail::no_decode_state>)
{
    if (state_.at_end())
        return std::nullopt;
    const unsigned char* none = nullptr;
    auto                 r    = decode_one(none, none);
    if (r.is_empty)
        return std::nullopt;
    return r;
}

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_DECODER_HPP
