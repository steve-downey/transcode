// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_CODEC_CONCEPTS_HPP
#define INCLUDE_BEMAN_TRANSCODE_CODEC_CONCEPTS_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec_result.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <concepts>
    #include <optional>

#endif
namespace beman::transcoding {

// \ref{transcode.codec.reqs}, codec requirements

//! \remarks A type models `text_decoder` if it can decode one Unicode scalar
//! value from a byte sequence.  `c.decode_one(iter, sentinel)` reads from
//! `iter` up to but not past `sentinel`, advances `iter` by the bytes it
//! consumed, and returns what it decoded or the error that stopped it.  It is
//! called only with `iter != sentinel`.  A codec that consumes no byte does
//! not terminate the view that drives it.
//!
//! A codec that decodes a sequence with state keeps that state in itself, not
//! in the range.
template <typename C>
concept text_decoder =
    std::semiregular<C> && requires(C& c, const unsigned char*& iter, const unsigned char* sentinel) {
        { c.decode_one(iter, sentinel) } -> std::same_as<decode_result>;
    };

//! \remarks A type models `single_byte_text_decoder` if each byte
//! decodes to one scalar value independently of every other:
//! `c.decode_byte(byte)` returns what `byte` alone decodes to, and U+FFFD if
//! it decodes to nothing.  For byte values `0x80` and above, U+FFFD is
//! reserved as that "decodes to nothing" signal; the author of such a codec
//! cannot map one of those bytes to U+FFFD itself.  A view over such a codec
//! is a `random_access_range`, since the *n*th element is the *n*th byte.
template <typename C>
concept single_byte_text_decoder = text_decoder<C> && requires(const C& c, unsigned char byte) {
    { c.decode_byte(byte) } -> std::same_as<char32_t>;
};

//! \remarks A type models `text_encoder` if it can encode one Unicode scalar
//! value to bytes.  `c.encode_one(cp)` returns the encoded form of `cp`, or an
//! error if the encoding has no representation for it.  An encode view
//! validates its UTF-32 input before calling this function, so `cp` is always a
//! Unicode scalar value.
template <typename C>
concept text_encoder = std::semiregular<C> && requires(C& c, char32_t cp) {
    { c.encode_one(cp) } -> std::same_as<encode_result>;
};

//! \remarks A type models `single_byte_text_encoder` if each Unicode scalar
//! value encodes to exactly one byte, independently of every other, or to
//! nothing: `c.encode_byte(scalar)` returns the byte `scalar` alone encodes
//! to, or `nullopt` if the encoding has no representation for it.  Its
//! `encode_one` agrees with `encode_byte`, never produces more than one byte,
//! and carries no state, so a type that also models `flushable_text_encoder`
//! must not model this concept.  A view over such an encoder is a
//! `random_access_range`, since the *n*th byte is the encoding of the *n*th
//! scalar value.
template <typename C>
concept single_byte_text_encoder = text_encoder<C> && requires(const C& c, char32_t scalar) {
    { c.encode_byte(scalar) } -> std::same_as<std::optional<unsigned char>>;
};

//! \remarks A type models `flushable_text_decoder` if it can owe output after
//! the last byte.  `c.flush()` is called when the input is exhausted, and
//! again after each engaged result, until it returns `nullopt`.  It returns
//! what the codec still owes -- a truncation error for a sequence that ended
//! mid-way, say -- or `nullopt` if it owes nothing.  A codec that does not
//! model this concept is asked for nothing at the end of the input.
template <typename C>
concept flushable_text_decoder = text_decoder<C> && requires(C& c) {
    { c.flush() } -> std::same_as<std::optional<decode_result>>;
};

//! \remarks A type models `bom_aware_text_decoder` if it decodes the
//! start of a text differently from the rest, as a decoder that discards a
//! leading byte order mark does.  `c.decode_first(iter, sentinel)` is called
//! instead of `decode_one` for the first element of a non-empty input, once,
//! and has the same contract.  A codec that does not model this concept has
//! `decode_one` called for every element.
template <typename C>
concept bom_aware_text_decoder =
    text_decoder<C> && requires(C& c, const unsigned char*& iter, const unsigned char* sentinel) {
        { c.decode_first(iter, sentinel) } -> std::same_as<decode_result>;
    };

//! \remarks A type models `flushable_text_encoder` if it can owe output after
//! the last scalar value, as an encoding with shift states must return to its
//! initial state.  `c.flush()` is called when the input is exhausted, and
//! again after each engaged result, until it returns `nullopt`.  It returns
//! the bytes the codec still owes, or `nullopt` if it owes nothing.  A codec
//! that does not model this concept is asked for nothing at the end of the
//! input.
template <typename C>
concept flushable_text_encoder = text_encoder<C> && requires(C& c) {
    { c.flush() } -> std::same_as<std::optional<encode_result>>;
};

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_CODEC_CONCEPTS_HPP
