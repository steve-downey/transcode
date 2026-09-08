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

// \ref{transcode.custom.reqs}, codec requirements

//! \remarks A type models `decode_codec` if it can decode one Unicode scalar
//! value from a byte sequence.  `c.decode_one(iter, sentinel)` reads from
//! `iter` up to but not past `sentinel`, advances `iter` by the bytes it
//! consumed, and returns what it decoded or the error that stopped it.  It is
//! called only with `iter != sentinel`.  A codec that consumes no byte does
//! not terminate the view that drives it.
//!
//! A codec is a value: `semiregular` because a view owns one and copies it,
//! and a codec that decodes a sequence with state keeps that state in itself,
//! not in the range.
template <typename C>
concept decode_codec =
    std::semiregular<C> && requires(C& c, const unsigned char*& iter, const unsigned char* sentinel) {
        { c.decode_one(iter, sentinel) } -> std::same_as<decode_result>;
    };

//! \remarks A type models `random_access_decode_codec_type` if each byte
//! decodes to one scalar value independently of every other:
//! `c.decode_byte(byte)` returns what `byte` alone decodes to, and U+FFFD if
//! it decodes to nothing.  A view over such a codec is a
//! `random_access_range`, since the *n*th element is the *n*th byte.
template <typename C>
concept random_access_decode_codec_type = decode_codec<C> && requires(const C& c, unsigned char byte) {
    { c.decode_byte(byte) } -> std::same_as<char32_t>;
};

//! \remarks A type models `encode_codec` if it can encode one Unicode scalar
//! value to bytes.  `c.encode_one(cp)` returns the encoded form of `cp`, or an
//! error if the encoding has no representation for it.  `cp` is required to be
//! a Unicode scalar value.
template <typename C>
concept encode_codec = std::semiregular<C> && requires(C& c, char32_t cp) {
    { c.encode_one(cp) } -> std::same_as<encode_result>;
};

//! \remarks A type models `flushable_decode_codec` if it can owe output after
//! the last byte.  `c.flush()` is called once, when the input is exhausted,
//! and returns what the codec still owes -- a truncation error for a sequence
//! that ended mid-way, say -- or `nullopt` if it owes nothing.  A codec that
//! does not model this concept is asked for nothing at the end of the input.
template <typename C>
concept flushable_decode_codec = decode_codec<C> && requires(C& c) {
    { c.flush() } -> std::same_as<std::optional<decode_result>>;
};

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_CODEC_CONCEPTS_HPP
