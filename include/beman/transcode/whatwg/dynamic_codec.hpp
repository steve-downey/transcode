// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_DYNAMIC_CODEC_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_DYNAMIC_CODEC_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec_result.hpp>
#include <beman/transcode/whatwg/decoder.hpp>
#include <beman/transcode/whatwg/encoder.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/detail/whatwg_dispatch.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <iterator>
    #include <optional>
    #include <variant>

#endif
namespace beman::transcoding::whatwg {

// \ref{transcode.whatwg.dynamic}, codecs for an encoding chosen at run time

//! \at transcode.whatwg.dynamic
//! \remarks `whatwg::dynamic_decoder` is a decoder, as a codec object, for an
//! encoding chosen at run time.  Each operation behaves as the same operation
//! of `whatwg::decoder<E>` does, where `E` is the encoding it was constructed
//! with.  It models `text_decoder`, `flushable_text_decoder` and
//! `bom_aware_text_decoder` \iref{transcode.codec.reqs} whatever the
//! encoding, so the decode adaptor over it \iref{transcode.codec.decode}
//! yields what `whatwg::decode<E>` does.  It does not model
//! `single_byte_text_decoder`.  It is equality comparable; two objects are
//! equal if they decode the same encoding and are in the same state.
class dynamic_decoder {
    //! \expos
    detail::decoder_variant decoder_;

  public:
    // \ref{transcode.whatwg.dynamic}, construction
    constexpr dynamic_decoder() noexcept;
    constexpr explicit dynamic_decoder(whatwg::encoding e) noexcept;

    constexpr whatwg::encoding encoding() const noexcept;

    // \ref{transcode.whatwg.dynamic}, decoding
    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_one(I& current, S end);

    template <std::input_iterator I, std::sentinel_for<I> S>
    constexpr decode_result decode_first(I& current, S end);

    constexpr std::optional<decode_result> flush();

    friend constexpr bool operator==(const dynamic_decoder&, const dynamic_decoder&) = default;
};

//! \at transcode.whatwg.dynamic
//! \remarks `whatwg::dynamic_encoder` is an encoder, as a codec object, for an
//! encoding chosen at run time.  Each operation behaves as the same operation
//! of `whatwg::encoder<E>` does, where `E` is the encoding it was constructed
//! with.  It models `text_encoder` and `flushable_text_encoder`
//! \iref{transcode.codec.reqs} whatever the encoding, so the encode adaptor
//! over it \iref{transcode.codec.encode} yields what `whatwg::encode<E>`
//! does.  It does not model `single_byte_text_encoder`.  It is equality
//! comparable.
class dynamic_encoder {
    //! \expos
    detail::encoder_variant encoder_;

  public:
    // \ref{transcode.whatwg.dynamic}, construction
    constexpr dynamic_encoder() noexcept;
    constexpr explicit dynamic_encoder(whatwg::encoding e) noexcept;

    constexpr whatwg::encoding encoding() const noexcept;

    // \ref{transcode.whatwg.dynamic}, encoding
    constexpr encode_result encode_one(char32_t scalar);

    constexpr std::optional<encode_result> flush();

    friend constexpr bool operator==(const dynamic_encoder&, const dynamic_encoder&) = default;
};

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg::dynamic_decoder
// ---------------------------------------------------------------------------

//! \effects Constructs a decoder for `encoding::utf_8`.
constexpr dynamic_decoder::dynamic_decoder() noexcept : decoder_() {}

//! \effects Constructs a decoder for `e`, in its initial state.
constexpr dynamic_decoder::dynamic_decoder(whatwg::encoding e) noexcept : decoder_(detail::make_decoder_variant(e)) {}

//! \returns The encoding this decoder decodes.
constexpr whatwg::encoding dynamic_decoder::encoding() const noexcept {
    return static_cast<whatwg::encoding>(decoder_.index());
}

//! \effects Equivalent to `d.decode_one(current, end)`, where `d` is the
//! `whatwg::decoder<E>` this object behaves as.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr decode_result dynamic_decoder::decode_one(I& current, S end) {
    return std::visit([&](auto& d) { return d.decode_one(current, end); }, decoder_);
}

//! \effects Equivalent to `d.decode_first(current, end)` if the
//! `whatwg::decoder<E>` this object behaves as has `decode_first`, and to
//! `d.decode_one(current, end)` otherwise.
template <std::input_iterator I, std::sentinel_for<I> S>
constexpr decode_result dynamic_decoder::decode_first(I& current, S end) {
    return std::visit(
        [&](auto& d) {
            if constexpr (requires { d.decode_first(current, end); })
                return d.decode_first(current, end);
            else
                return d.decode_one(current, end);
        },
        decoder_);
}

//! \effects Equivalent to `d.flush()` if the `whatwg::decoder<E>` this object
//! behaves as has `flush`.
//! \returns The result of `d.flush()` if there is one, and `nullopt` otherwise.
constexpr std::optional<decode_result> dynamic_decoder::flush() {
    return std::visit(
        [](auto& d) -> std::optional<decode_result> {
            if constexpr (requires { d.flush(); })
                return d.flush();
            else
                return std::nullopt;
        },
        decoder_);
}

// ---------------------------------------------------------------------------
// Out-of-line definitions: whatwg::dynamic_encoder
// ---------------------------------------------------------------------------

//! \effects Constructs an encoder for `encoding::utf_8`.
constexpr dynamic_encoder::dynamic_encoder() noexcept : encoder_() {}

//! \expects `whatwg::has_encoder(e)` is `true` \iref{transcode.string}.
//! \effects Constructs an encoder for `e`, in its initial state.
constexpr dynamic_encoder::dynamic_encoder(whatwg::encoding e) noexcept : encoder_(detail::make_encoder_variant(e)) {}

//! \returns The encoding this encoder encodes.
constexpr whatwg::encoding dynamic_encoder::encoding() const noexcept {
    return static_cast<whatwg::encoding>(encoder_.index());
}

//! \effects Equivalent to `e.encode_one(scalar)`, where `e` is the
//! `whatwg::encoder<E>` this object behaves as.
constexpr encode_result dynamic_encoder::encode_one(char32_t scalar) {
    return std::visit([&](auto& e) { return e.encode_one(scalar); }, encoder_);
}

//! \effects Equivalent to `e.flush()` if the `whatwg::encoder<E>` this object
//! behaves as has `flush`.
//! \returns The result of `e.flush()` if there is one, and `nullopt` otherwise.
constexpr std::optional<encode_result> dynamic_encoder::flush() {
    return std::visit(
        [](auto& e) -> std::optional<encode_result> {
            if constexpr (requires { e.flush(); })
                return e.flush();
            else
                return std::nullopt;
        },
        encoder_);
}

} // namespace beman::transcoding::whatwg

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_DYNAMIC_CODEC_HPP
