// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ISO-2022-JP, the one WHATWG codec whose decoder is a state machine rather
// than a function of the bytes in front of it: an escape sequence switches the
// stream between ASCII, Roman, Katakana and JIS X 0208 modes, and a decoder
// that has read half a sequence has to remember that across calls.  The state
// lives in iso2022jp_decode_state, which the view's iterator holds and
// compares, so the iterator itself keeps no codec-specific fields.

#ifndef INCLUDE_BEMAN_TRANSCODE_DETAIL_ISO2022JP_HPP
#define INCLUDE_BEMAN_TRANSCODE_DETAIL_ISO2022JP_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/error.hpp>
#include <beman/transcode/detail/tables/shift_jis.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <iterator>

#endif
namespace beman::transcoding::detail {

// The decoder's state between calls, per WHATWG §12.2 "ISO-2022-JP decoder".
//
// `state` is the machine's current mode and `output_state` the mode to return
// to when a sequence turns out not to be an escape after all.  `pending` holds
// the bytes an abandoned escape sequence has to re-process -- the WHATWG
// "prepend to stream" step, which an input iterator cannot do by backing up.
struct iso2022jp_decode_state {
    // 0=ASCII, 1=Roman, 2=Katakana, 3=Lead byte, 4=Trail byte,
    // 5=Escape start, 6=Escape
    int           state{0};
    int           output_state{0};
    bool          output_flag{false};
    unsigned char lead{0};
    unsigned char pending[2]{};
    int           pending_count{0};

    // Whether an exhausted input leaves nothing to emit.  States 4, 5 and 6
    // are mid-sequence: each still owes a truncation error.
    constexpr bool at_end() const { return pending_count == 0 && state <= 3; }

    friend constexpr bool operator==(const iso2022jp_decode_state& lhs, const iso2022jp_decode_state& rhs) {
        return lhs.state == rhs.state && lhs.output_state == rhs.output_state && lhs.output_flag == rhs.output_flag &&
               lhs.lead == rhs.lead && lhs.pending_count == rhs.pending_count && lhs.pending[0] == rhs.pending[0] &&
               lhs.pending[1] == rhs.pending[1];
    }
};

struct iso2022jp_decode_result {
    char32_t     code_point{0xFFFD};
    whatwg_error error{};
    bool         is_error{false};
    // The input is exhausted and the state machine has nothing left to emit.
    bool done{false};
};

// Decode one code point, advancing `current` and updating `st`.  Loops over
// escape sequences and mode switches, which produce no output of their own,
// and returns at the first byte that does.
template <typename I, typename S>
constexpr iso2022jp_decode_result iso2022jp_decode_one(iso2022jp_decode_state& st, I& current, S end);

// The encoder's state between calls: the mode the output stream is currently
// in, which decides whether a code point needs an escape sequence in front of
// it.  0=ASCII, 1=Roman, 2=JIS X 0208.
struct iso2022jp_encode_state {
    int state{0};

    friend constexpr bool operator==(const iso2022jp_encode_state& lhs, const iso2022jp_encode_state& rhs) {
        return lhs.state == rhs.state;
    }
};

// Up to an escape sequence and the two bytes it introduces.  On an unmapped
// code point `bytes` is what a lossy encoder writes instead, which is why the
// error carries bytes at all: returning to ASCII is part of the replacement.
struct iso2022jp_encode_result {
    char bytes[5]{};
    int  count{0};
    bool is_error{false};
};

// Encode one code point, updating `st`.
constexpr iso2022jp_encode_result iso2022jp_encode_one(iso2022jp_encode_state& st, char32_t cp);

// The escape back to ASCII that an exhausted input owes, and `count == 0` when
// the stream is already there.  WHATWG's encoder ends in ASCII mode.
constexpr iso2022jp_encode_result iso2022jp_encode_flush(iso2022jp_encode_state& st);

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

template <typename I, typename S>
constexpr iso2022jp_decode_result iso2022jp_decode_one(iso2022jp_decode_state& st, I& current, S end) {
    const auto error = [&st](whatwg_error e) {
        st.state       = st.output_state;
        st.output_flag = false;
        return iso2022jp_decode_result{0xFFFD, e, true, false};
    };

    while (true) {
        unsigned char byte;
        if (st.pending_count > 0) {
            byte = st.pending[0];
            if (st.pending_count > 1)
                st.pending[0] = st.pending[1];
            --st.pending_count;
        } else if (current != end) {
            byte = static_cast<unsigned char>(*current++);
        } else {
            switch (st.state) {
            default: // 0=ASCII, 1=Roman, 2=Katakana, 3=Lead_Byte
                return {0xFFFD, {}, false, true};
            case 4:
            case 5:
                return error(whatwg_error::truncated_sequence);
            case 6:
                st.pending[0]    = st.lead;
                st.pending_count = 1;
                st.lead          = 0;
                return error(whatwg_error::truncated_sequence);
            }
        }

        switch (st.state) {
        case 5:
            if (byte == 0x24 || byte == 0x28) {
                st.lead  = byte;
                st.state = 6;
                continue;
            }
            st.pending[0]    = byte;
            st.pending_count = 1;
            return error(whatwg_error::invalid_byte);

        case 6: {
            auto lead     = st.lead;
            st.lead       = 0;
            int new_state = -1;
            if (lead == 0x28) {
                if (byte == 0x42)
                    new_state = 0;
                else if (byte == 0x4A)
                    new_state = 1;
                else if (byte == 0x49)
                    new_state = 2;
            } else {
                if (byte == 0x40 || byte == 0x42)
                    new_state = 3;
            }
            if (new_state < 0) {
                st.pending[0]    = lead;
                st.pending[1]    = byte;
                st.pending_count = 2;
                return error(whatwg_error::invalid_byte);
            }
            st.output_state = new_state;
            st.state        = new_state;
            if (st.output_flag) {
                st.output_flag = false;
                return {0xFFFD, whatwg_error::invalid_byte, true, false};
            }
            st.output_flag = true;
            continue;
        }

        case 3:
            if (byte == 0x1B) {
                st.state = 5;
                continue;
            }
            if (byte >= 0x21 && byte <= 0x7E) {
                st.lead  = byte;
                st.state = 4;
                continue;
            }
            return error(whatwg_error::invalid_byte);

        case 4: {
            if (byte == 0x1B) {
                st.state = 5;
                continue;
            }
            st.state       = st.output_state;
            st.output_flag = false;
            if (byte >= 0x21 && byte <= 0x7E) {
                int  pointer = ((static_cast<int>(st.lead) - 0x21) * 94) + (static_cast<int>(byte) - 0x21);
                auto cp      = tables::shift_jis[pointer];
                if (cp != 0)
                    return {cp, {}, false, false};
            }
            return {0xFFFD, whatwg_error::invalid_byte, true, false};
        }

        default: // 0=ASCII, 1=Roman, 2=Katakana
            if (byte == 0x1B) {
                st.state = 5;
                continue;
            }
            if (byte == 0x0E || byte == 0x0F) {
                st.output_flag = false;
                return {0xFFFD, whatwg_error::invalid_byte, true, false};
            }
            st.output_flag = false;
            if (st.state == 0) {
                if (byte <= 0x7F)
                    return {static_cast<char32_t>(byte), {}, false, false};
                return {0xFFFD, whatwg_error::invalid_byte, true, false};
            }
            if (st.state == 1) {
                if (byte == 0x5C)
                    return {U'\x00A5', {}, false, false};
                if (byte == 0x7E)
                    return {U'\x203E', {}, false, false};
                if (byte <= 0x7F)
                    return {static_cast<char32_t>(byte), {}, false, false};
                return {0xFFFD, whatwg_error::invalid_byte, true, false};
            }
            // Katakana: 0x21-0x5F → U+FF61-U+FF9F
            if (byte >= 0x21 && byte <= 0x5F)
                return {static_cast<char32_t>(0xFF61 + byte - 0x21), {}, false, false};
            return {0xFFFD, whatwg_error::invalid_byte, true, false};
        }
    }
}

constexpr iso2022jp_encode_result iso2022jp_encode_one(iso2022jp_encode_state& st, char32_t cp) {
    // Roman state: U+00A5 (YEN SIGN) and U+203E (OVERLINE) are the two code
    // points ASCII spells differently.
    if (cp == 0x00A5 || cp == 0x203E) {
        const char ascii_byte = (cp == 0x00A5) ? '\x5C' : '\x7E';
        if (st.state != 1) {
            st.state = 1;
            return {{'\x1B', '\x28', '\x4A', ascii_byte}, 4, false};
        }
        return {{ascii_byte}, 1, false};
    }
    if (cp < 0x80) {
        const char ascii_byte = static_cast<char>(cp);
        if (st.state != 0) {
            st.state = 0;
            return {{'\x1B', '\x28', '\x42', ascii_byte}, 4, false};
        }
        return {{ascii_byte}, 1, false};
    }
    for (int i = 0; i < 8836; ++i) {
        if (tables::shift_jis[i] == cp) {
            const auto lead  = static_cast<char>((i / 94) + 0x21);
            const auto trail = static_cast<char>((i % 94) + 0x21);
            if (st.state != 2) {
                st.state = 2;
                return {{'\x1B', '\x24', '\x42', lead, trail}, 5, false};
            }
            return {{lead, trail}, 2, false};
        }
    }
    // Unmapped.  The lossy encoder returns to ASCII before substituting, so the
    // replacement carries the escape sequence with it.
    if (st.state != 0) {
        st.state = 0;
        return {{'\x1B', '\x28', '\x42', '?'}, 4, true};
    }
    return {{'?'}, 1, true};
}

constexpr iso2022jp_encode_result iso2022jp_encode_flush(iso2022jp_encode_state& st) {
    if (st.state == 0)
        return {};
    st.state = 0;
    return {{'\x1B', '\x28', '\x42'}, 3, false};
}

} // namespace beman::transcoding::detail

#endif // INCLUDE_BEMAN_TRANSCODE_DETAIL_ISO2022JP_HPP
