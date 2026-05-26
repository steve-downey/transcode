// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_CODEC_CONCEPTS_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_CODEC_CONCEPTS_HPP

#include <beman/transcode/detail/codec_result.hpp>

#include <concepts>
#include <optional>

namespace beman::transcoding {

template <typename C>
concept decode_codec = std::semiregular<C> &&
                       requires(C& c, const unsigned char*& iter, const unsigned char* sentinel) {
                           { c.decode_one(iter, sentinel) } -> std::same_as<decode_result>;
                       };

template <typename C>
concept random_access_decode_codec_type =
    decode_codec<C> && requires(const C& c, unsigned char byte) {
                           { c.decode_byte(byte) } -> std::same_as<char32_t>;
                       };

template <typename C>
concept encode_codec = std::semiregular<C> && requires(C& c, char32_t cp) {
                                                  { c.encode_one(cp) } -> std::same_as<encode_result>;
                                              };

template <typename C>
concept flushable_decode_codec = decode_codec<C> && requires(C& c) {
                                                        { c.flush() } -> std::same_as<std::optional<decode_result>>;
                                                    };

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_CODEC_CONCEPTS_HPP
