// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <array>
#include <cstddef>
#include <numeric>
#include <span>
#include <string>
#include <string_view>

namespace {

void append_fraction(std::u32string& result, unsigned numerator, unsigned denominator) {
    unsigned divisor = std::gcd(numerator, denominator);
    numerator /= divisor;
    denominator /= divisor;

    auto append_unsigned = [&](unsigned value) {
        std::string digits = std::to_string(value);
        for (char digit : digits)
            result.push_back(static_cast<char32_t>(digit));
    };

    append_unsigned(numerator);
    result.push_back(U'/');
    append_unsigned(denominator);
}

void append_custom_code_point(std::u32string& result, unsigned char byte) {
    switch (byte) {
    case 0x00: result += U"\u00C7"; return;
    case 0x01: result += U"\u00FC"; return;
    case 0x02: result += U"\u00E9"; return;
    case 0x03: result += U"\u00E2"; return;
    case 0x04: result += U"\u00E4"; return;
    case 0x05: result += U"\u00E0"; return;
    case 0x06: result += U"\u00E5"; return;
    case 0x07: result += U"\u00E7"; return;
    case 0x08: result += U"\u00EA"; return;
    case 0x09: result += U"\u00EB"; return;
    case 0x0A: result += U"\u00E8"; return;
    case 0x0B: result += U"\u00EF"; return;
    case 0x0C: result += U"\u00EE"; return;
    case 0x0D: result += U"\u00EC"; return;
    case 0x0E: result += U"\u00C4"; return;
    case 0x0F: result += U"\u00C5"; return;
    case 0x10: result += U"\u00C9"; return;
    case 0x11: result += U"\u00C8"; return;
    case 0x12: result += U"\u00CC"; return;
    case 0x13: result += U"\u00F4"; return;
    case 0x14: result += U"\u00F6"; return;
    case 0x15: result += U"\u00F2"; return;
    case 0x16: result += U"\u00FB"; return;
    case 0x17: result += U"\u00F9"; return;
    case 0x18: result += U"\u00FF"; return;
    case 0x19: result += U"\u00D6"; return;
    case 0x1A: result += U"\u00DC"; return;
    case 0x1B: result += U"\u00E1"; return;
    case 0x1C: result += U"\u00ED"; return;
    case 0x1D: result += U"\u00F3"; return;
    case 0x1E: result += U"\u00FA"; return;
    case 0x1F: result += U"\u00F1"; return;
    case 0x7F: result += U"\u20AC"; return;
    case 0xBF: result += U"\u00D7"; return;
    case 0xC0: result += U"0)"; return;
    case 0xC1: result += U"1)"; return;
    case 0xC2: result += U"2)"; return;
    case 0xC3: result += U"3)"; return;
    case 0xC4: result += U"4)"; return;
    case 0xC5: result += U"5)"; return;
    case 0xC6: result += U"6)"; return;
    case 0xC7: result += U"7)"; return;
    case 0xC8: result += U"8)"; return;
    case 0xC9: result += U"9)"; return;
    case 0xCA: result += U"0"; return;
    case 0xCB: result += U"1"; return;
    case 0xCC: result += U"2"; return;
    case 0xCD: result += U"3"; return;
    case 0xCE: result += U"4"; return;
    case 0xCF: result += U"5"; return;
    case 0xD0: result += U"6"; return;
    case 0xD1: result += U"7"; return;
    case 0xD2: result += U"8"; return;
    case 0xD3: result += U"9"; return;
    case 0xD4: result += U"Up"; return;
    case 0xD5: result += U"Dn"; return;
    case 0xD6: result += U"\uE0D6"; return;
    case 0xD7: result += U"\uE0D7"; return;
    case 0xD8: result += U"\uE0D8"; return;
    case 0xD9: result += U"\uE0D9"; return;
    case 0xDA: result += U"\uE0DA"; return;
    case 0xDB: result += U"\uE0DB"; return;
    case 0xDC: result += U"\uE0DC"; return;
    case 0xDD: result += U"\uE0DD"; return;
    case 0xDE: result += U"\uE0DE"; return;
    case 0xDF: result += U"\uE0DF"; return;
    case 0xE0: result += U"\u00A3"; return;
    case 0xE1: result += U"\u00A5"; return;
    case 0xE2: result += U"\uE0E2"; return;
    case 0xE3: result += U"\u00D2"; return;
    case 0xE4: result += U"\u00D9"; return;
    case 0xE5: result += U"\u00B1"; return;
    case 0xE6: result += U"\uE0E6"; return;
    case 0xE7: result += U"\uE0E7"; return;
    case 0xE8: result += U"\uE0E8"; return;
    case 0xE9: result += U"\uE0E9"; return;
    case 0xEA: result += U"\u00D5"; return;
    case 0xEB: result += U"\u00C1"; return;
    case 0xEC: result += U"\u00CD"; return;
    case 0xED: result += U"\u0099"; return;
    case 0xEE: result += U"\u00A9"; return;
    case 0xEF: result += U"\u00AE"; return;
    case 0xF0: result += U"\u00D4"; return;
    case 0xF1: result += U"\uE0F1"; return;
    case 0xF2: result += U"\u00D3"; return;
    case 0xF3: result += U"\u00DA"; return;
    case 0xF4: result += U"\u00C2"; return;
    case 0xF5: result += U"\u00CA"; return;
    case 0xF6: result += U"\u00F5"; return;
    case 0xF7: result += U"\u00C0"; return;
    case 0xF8: result += U"\u00D1"; return;
    case 0xF9: result += U"\u00BF"; return;
    case 0xFA: result += U"\u00A1"; return;
    case 0xFB: result += U"\u00AB"; return;
    case 0xFC: result += U"\u00BB"; return;
    case 0xFD: result += U"\u00E3"; return;
    case 0xFE: result += U"\u00C3"; return;
    case 0xFF: result += U"\u00DF"; return;
    default: break;
    }

    if (byte >= 0x20 && byte <= 0x7E) {
        result.push_back(static_cast<char32_t>(byte));
        return;
    }

    if (byte >= 0x80 && byte <= 0xBE) {
        append_fraction(result, static_cast<unsigned>(byte) - 0x80U + 1U, 64U);
        return;
    }
}

std::u32string decode_custom_single_byte(std::span<const std::byte> input) {
    std::u32string result;
    for (std::byte byte : input)
        append_custom_code_point(result, static_cast<unsigned char>(byte));
    return result;
}

bool matches_expected(const std::u32string& decoded) {
    return decoded == U"\u00C7A1/641/2\u00D7Up\u00A3\u00DF";
}

} // namespace

int main() {
    constexpr std::array<std::byte, 8> input = {
        std::byte{0x00},
        std::byte{0x41},
        std::byte{0x80},
        std::byte{0x9F},
        std::byte{0xBF},
        std::byte{0xD4},
        std::byte{0xE0},
        std::byte{0xFF},
    };

    return matches_expected(decode_custom_single_byte(input)) ? 0 : 1;
}
