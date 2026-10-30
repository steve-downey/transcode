// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODER_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODER_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec_result.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/detail/whatwg_encode_select.hpp>
#include <beman/transcode/detail/reverse_index.hpp>
#include <beman/transcode/detail/single_byte.hpp>
#include <beman/transcode/detail/big5.hpp>
#include <beman/transcode/detail/euc_jp.hpp>
#include <beman/transcode/detail/euc_kr.hpp>
#include <beman/transcode/detail/gb18030.hpp>
#include <beman/transcode/detail/gbk.hpp>
#include <beman/transcode/detail/iso2022jp.hpp>
#include <beman/transcode/detail/shift_jis.hpp>
#include <beman/transcode/detail/utf8_encode.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <optional>

#endif
namespace beman::transcoding::whatwg {

// \ref{transcode.whatwg.encoders}, encoders

//! \at transcode.whatwg.encoders
//! \remarks `whatwg::encoder<C>` is the encoder, as a codec object, for the
//! encoding `C`.  `$whatwg-encode-codec$<C>` is `true` for every encoding but
//! `utf_16be`, `utf_16le`, `replacement` and `x_user_defined`, for which the
//! Encoding Standard defines no encoder, and `$random-access-encode-codec$<C>`
//! is `true` when `C` is one of the Encoding Standard's single-byte encodings
//! other than `x_user_defined`.  It models `text_encoder` \iref{transcode.codec.reqs} and the encode
//! adaptor over it \iref{transcode.codec.encode} yields what
//! `whatwg::encode<C>` does.  It models `flushable_text_encoder` when `C` is
//! `encoding::iso_2022_jp`, which owes an escape sequence returning to ASCII
//! after the last scalar value.  It models `single_byte_text_encoder` when
//! `C` names a single-byte encoding.  It is equality comparable.
template <encoding C>
    requires detail::whatwg_encode_codec<C>
class encoder {
    //! \expos
    [[no_unique_address]] detail::encode_state_t<C> state_{};

  public:
    // \ref{transcode.whatwg.encoders}, encoding
    constexpr encode_result encode_one(char32_t scalar);

    constexpr std::optional<unsigned char> encode_byte(char32_t scalar) const
        requires detail::random_access_encode_codec<C>;

    constexpr std::optional<encode_result> flush()
        requires(C == encoding::iso_2022_jp);

    friend constexpr bool operator==(const encoder&, const encoder&) = default;
};

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg::encoder
// ---------------------------------------------------------------------------

template <encoding C>
    requires detail::whatwg_encode_codec<C>
//! \effects Encodes `scalar`, which is a Unicode scalar value, as the encoding
//! `C` says, updating the encoder's state if `C` has one.
//! \returns The encoded bytes, or, if `C` has no representation for `scalar`,
//! a result with `is_error`.  An error from `iso_2022_jp` carries the bytes
//! that replace `scalar`: an escape sequence returning to ASCII if the output
//! was not in it, then `'?'`.
constexpr encode_result encoder<C>::encode_one(char32_t scalar) {
    // Each detail encoder builds the encode_result itself, and it is returned
    // as a prvalue: a copy here, or a whole-array copy anywhere on the way to
    // the view's buffer, makes GCC spill the result to the stack per element.
    if constexpr (C == encoding::utf_8) {
        return detail::utf8_encode_one(scalar);
    } else if constexpr (detail::random_access_encode_codec<C>) {
        const auto    r = detail::single_byte_encode_one<detail::random_access_encode_table<C>()>(scalar);
        encode_result out;
        out.bytes[0] = r.byte;
        out.count    = r.is_error ? 0 : 1;
        out.is_error = r.is_error;
        return out;
    } else if constexpr (C == encoding::gbk) {
        return detail::gbk_encode_one(scalar);
    } else if constexpr (C == encoding::gb18030) {
        return detail::gb18030_encode_one(scalar);
    } else if constexpr (C == encoding::big5) {
        return detail::big5_encode_one(scalar);
    } else if constexpr (C == encoding::shift_jis) {
        return detail::shift_jis_encode_one(scalar);
    } else if constexpr (C == encoding::euc_jp) {
        return detail::euc_jp_encode_one(scalar);
    } else if constexpr (C == encoding::iso_2022_jp) {
        return detail::iso2022jp_encode_one(state_, scalar);
    } else {
        static_assert(C == encoding::euc_kr);
        return detail::euc_kr_encode_one(scalar);
    }
}

template <encoding C>
    requires detail::whatwg_encode_codec<C>
//! \returns The byte `scalar` encodes to in the encoding `C`, or `nullopt` if
//! `C` has no representation for it.  Present for the single-byte encodings.
constexpr std::optional<unsigned char> encoder<C>::encode_byte(char32_t scalar) const
    requires detail::random_access_encode_codec<C>
{
    // Straight from the table into the optional: going through
    // single_byte_encode_one's {byte, is_error} pair costs GCC a second test
    // per element.
    if (scalar < 0x80)
        return static_cast<unsigned char>(scalar);
    const int pointer = detail::reverse_index_for<detail::random_access_encode_table<C>()>.find(scalar);
    if (pointer < 0)
        return std::nullopt;
    return static_cast<unsigned char>(0x80 + pointer);
}

template <encoding C>
    requires detail::whatwg_encode_codec<C>
//! \effects Returns the encoder to its initial state.
//! \returns `nullopt` if the output is already in ASCII.  Otherwise the escape
//! sequence that returns it there.
constexpr std::optional<encode_result> encoder<C>::flush()
    requires(C == encoding::iso_2022_jp)
{
    const auto r = detail::iso2022jp_encode_flush(state_);
    if (r.count == 0)
        return std::nullopt;
    return r;
}

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODER_HPP
