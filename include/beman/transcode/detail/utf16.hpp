// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF16_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF16_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/error.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <iterator>

#endif
namespace beman::transcoding::detail {

// The decoder's state between calls.  A high surrogate not followed by a low
// one leaves the two bytes that were read in its place: WHATWG re-processes
// them as a fresh code unit, and an input iterator cannot back up to do that.
struct utf16_decode_state {
    unsigned char pending[2]{};
    int           pending_count{0};

    // Whether an exhausted input leaves nothing to emit.
    constexpr bool at_end() const { return pending_count == 0; }

    friend constexpr bool operator==(const utf16_decode_state& lhs, const utf16_decode_state& rhs) {
        return lhs.pending_count == rhs.pending_count && lhs.pending[0] == rhs.pending[0] &&
               lhs.pending[1] == rhs.pending[1];
    }
};

struct utf16_decode_result {
    char32_t     code_point{0xFFFD};
    whatwg_error error{};
    bool         is_error{false};
};

struct utf16_encode_result {
    char bytes[4]{};
    int  count{0};
    bool is_error{false};
};

// Decode one code point, advancing `current` and updating `st`.
//
// A high surrogate not followed by a low one is an error, and the two bytes
// that were read in the low surrogate's place are *not* consumed with it: they
// go to `st` and are read again as a fresh code unit, which is WHATWG's
// "prepend to stream" and the reason this decode carries state at all.
template <bool BigEndian, typename I, typename S>
constexpr utf16_decode_result utf16_decode_one(utf16_decode_state& st, I& current, S end);

constexpr utf16_encode_result utf16be_encode_one(char32_t cp);
constexpr utf16_encode_result utf16le_encode_one(char32_t cp);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <bool BigEndian>
constexpr char16_t utf16_code_unit(unsigned char first, unsigned char second) {
    if constexpr (BigEndian)
        return static_cast<char16_t>((static_cast<unsigned>(first) << 8) | second);
    else
        return static_cast<char16_t>((static_cast<unsigned>(second) << 8) | first);
}

template <bool BigEndian, typename I, typename S>
constexpr utf16_decode_result utf16_decode_one(utf16_decode_state& st, I& current, S end) {
    unsigned char b0;
    unsigned char b1;
    if (st.pending_count > 0) {
        b0               = st.pending[0];
        b1               = st.pending[1];
        st.pending_count = 0;
    } else {
        b0 = static_cast<unsigned char>(*current);
        ++current;
        if (current == end)
            return {0xFFFD, whatwg_error::truncated_sequence, true};
        b1 = static_cast<unsigned char>(*current);
        ++current;
    }

    const char16_t unit = utf16_code_unit<BigEndian>(b0, b1);

    if (unit >= 0xD800 && unit <= 0xDBFF) {
        if (current == end)
            return {0xFFFD, whatwg_error::truncated_sequence, true};
        auto b2 = static_cast<unsigned char>(*current);
        ++current;
        if (current == end)
            return {0xFFFD, whatwg_error::truncated_sequence, true};
        auto b3 = static_cast<unsigned char>(*current);
        ++current;

        const char16_t low = utf16_code_unit<BigEndian>(b2, b3);
        if (low >= 0xDC00 && low <= 0xDFFF)
            return {0x10000 + ((static_cast<char32_t>(unit - 0xD800) << 10) | (low - 0xDC00)), {}, false};

        st.pending[0]    = b2;
        st.pending[1]    = b3;
        st.pending_count = 2;
        return {0xFFFD, whatwg_error::surrogate_code_point, true};
    }

    if (unit >= 0xDC00 && unit <= 0xDFFF)
        return {0xFFFD, whatwg_error::surrogate_code_point, true};

    return {static_cast<char32_t>(unit), {}, false};
}

constexpr utf16_encode_result utf16be_encode_one(char32_t cp) {
    if (cp >= 0xD800 && cp <= 0xDFFF)
        return {{}, 0, true};
    if (cp <= 0xFFFF) {
        utf16_encode_result r{};
        r.bytes[0] = static_cast<char>((cp >> 8) & 0xFF);
        r.bytes[1] = static_cast<char>(cp & 0xFF);
        r.count    = 2;
        return r;
    }
    // Supplementary plane
    char32_t            offset = cp - 0x10000;
    auto                high   = static_cast<char16_t>(0xD800 + (offset >> 10));
    auto                low    = static_cast<char16_t>(0xDC00 + (offset & 0x3FF));
    utf16_encode_result r{};
    r.bytes[0] = static_cast<char>((high >> 8) & 0xFF);
    r.bytes[1] = static_cast<char>(high & 0xFF);
    r.bytes[2] = static_cast<char>((low >> 8) & 0xFF);
    r.bytes[3] = static_cast<char>(low & 0xFF);
    r.count    = 4;
    return r;
}

constexpr utf16_encode_result utf16le_encode_one(char32_t cp) {
    if (cp >= 0xD800 && cp <= 0xDFFF)
        return {{}, 0, true};
    if (cp <= 0xFFFF) {
        utf16_encode_result r{};
        r.bytes[0] = static_cast<char>(cp & 0xFF);
        r.bytes[1] = static_cast<char>((cp >> 8) & 0xFF);
        r.count    = 2;
        return r;
    }
    char32_t            offset = cp - 0x10000;
    auto                high   = static_cast<char16_t>(0xD800 + (offset >> 10));
    auto                low    = static_cast<char16_t>(0xDC00 + (offset & 0x3FF));
    utf16_encode_result r{};
    r.bytes[0] = static_cast<char>(high & 0xFF);
    r.bytes[1] = static_cast<char>((high >> 8) & 0xFF);
    r.bytes[2] = static_cast<char>(low & 0xFF);
    r.bytes[3] = static_cast<char>((low >> 8) & 0xFF);
    r.count    = 4;
    return r;
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_UTF16_HPP
