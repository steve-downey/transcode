// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODER_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODER_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec_result.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/detail/whatwg_encode_select.hpp>
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

//! \remarks `whatwg::encoder<C>` is the encoder, as a codec object, for the
//! encoding `C`, which is required to be an encoding the Encoding Standard
//! defines an encoder for.  It defines none for `utf_16be`, `utf_16le`,
//! `replacement` or `x_user_defined`, and `whatwg::encoder` is not defined for
//! them.  It models `text_encoder` \iref{transcode.custom.reqs} and the encode
//! adaptor over it \iref{transcode.custom.encode} yields what
//! `whatwg::encode<C>` does.  It models `flushable_text_encoder` when `C` is
//! `encoding::iso_2022_jp`, which owes an escape sequence returning to ASCII
//! after the last scalar value.  It is equality comparable.
template <encoding C>
    requires detail::whatwg_encode_codec<C>
class encoder {
    //! \expos
    [[no_unique_address]] detail::encode_state_t<C> state_{};

  public:
    constexpr encode_result encode_one(char32_t scalar);

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
    const auto failure = encode_result{.is_error = true};
    const auto copy    = [](const auto& r) {
        encode_result out;
        for (int i = 0; i < r.count; ++i)
            out.bytes[i] = static_cast<unsigned char>(r.bytes[i]);
        out.count = r.count;
        return out;
    };
    if constexpr (C == encoding::utf_8) {
        return copy(detail::utf8_encode_one(scalar));
    } else if constexpr (detail::random_access_encode_codec<C>) {
        const auto r = detail::single_byte_encode_one<detail::random_access_encode_table<C>()>(scalar);
        if (r.is_error)
            return failure;
        encode_result out;
        out.bytes[0] = r.byte;
        out.count    = 1;
        return out;
    } else if constexpr (C == encoding::gbk) {
        const auto r = detail::gbk_encode_one(scalar);
        return r.is_error ? failure : copy(r);
    } else if constexpr (C == encoding::gb18030) {
        const auto r = detail::gb18030_encode_one(scalar);
        return r.is_error ? failure : copy(r);
    } else if constexpr (C == encoding::big5) {
        const auto r = detail::big5_encode_one(scalar);
        return r.is_error ? failure : copy(r);
    } else if constexpr (C == encoding::shift_jis) {
        const auto r = detail::shift_jis_encode_one(scalar);
        return r.is_error ? failure : copy(r);
    } else if constexpr (C == encoding::euc_jp) {
        const auto r = detail::euc_jp_encode_one(scalar);
        return r.is_error ? failure : copy(r);
    } else if constexpr (C == encoding::iso_2022_jp) {
        const auto r   = detail::iso2022jp_encode_one(state_, scalar);
        auto       out = copy(r);
        out.is_error   = r.is_error;
        return out;
    } else {
        static_assert(C == encoding::euc_kr);
        const auto r = detail::euc_kr_encode_one(scalar);
        return r.is_error ? failure : copy(r);
    }
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
    encode_result out;
    for (int i = 0; i < r.count; ++i)
        out.bytes[i] = static_cast<unsigned char>(r.bytes[i]);
    out.count = r.count;
    return out;
}

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_ENCODER_HPP
