// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// One std::text_encoding::id, two answers.
//
// std::text_encoding::id::ShiftJIS names a registry entry.  It does not say
// what the bytes 5C 7E decode to; the backend decides that.  The Encoding
// Standard's shift_jis maps the single-byte range as ASCII, so 5C 7E is
// U+005C U+007E.  glibc's iconv "Shift_JIS" maps it as JIS X 0201 Roman, so
// the same bytes are U+00A5 U+203E.  Neither is a bug: each backend was asked
// what the name means, and answered for itself.
//
// This is an example and not a test, because the iconv answer depends on the
// platform's iconv tables.  Needs <text_encoding>, so it is built as C++26.

#include <beman/transcode/transcode.hpp>

#include <array>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <optional>
#include <span>
#include <string_view>
#include <text_encoding>
#include <vector>

using namespace beman::transcoding;

namespace {

void print(const char* label, const std::vector<char32_t>& cps) {
    std::printf("%s:", label);
    for (char32_t cp : cps)
        std::printf(" U+%04X", static_cast<unsigned>(cp));
    std::printf("\n");
}

} // namespace

int main() {
    constexpr std::text_encoding::id id = std::text_encoding::id::ShiftJIS;
    constexpr std::array<char, 2>    bytes{'\x5C', '\x7E'};

    // WHATWG: the id goes through the Encoding Standard's label lookup.
    std::optional<whatwg::encoding> enc = whatwg::get_encoding(id);
    if (!enc || *enc != whatwg::encoding::shift_jis)
        return 1;
    std::vector<char32_t> from_whatwg;
    for (char32_t cp : bytes | whatwg::decode<whatwg::encoding::shift_jis>)
        from_whatwg.push_back(cp);
    print("WHATWG shift_jis", from_whatwg);

    // iconv: the id's registered name goes to the platform's iconv.
    const std::text_encoding registered{id};
    std::array<char, 256>    buffer{};
    std::vector<char>        raw;
    for (char b : bytes | iconv_transcode(registered.name(), "UTF-32LE", std::span(buffer)))
        raw.push_back(b);
    std::vector<char32_t> from_iconv;
    for (std::size_t i = 0; i + 4 <= raw.size(); i += 4) {
        std::uint32_t cp = 0;
        for (std::size_t k = 4; k-- > 0;)
            cp = (cp << 8) | static_cast<unsigned char>(raw[i + k]); // UTF-32LE
        from_iconv.push_back(static_cast<char32_t>(cp));
    }
    print("iconv Shift_JIS  ", from_iconv);

    // The two lists may differ, and that is the point.
    return 0;
}

// Output, run on Linux (WSL2) x86-64 with GCC 16.2 and glibc 2.39:
//
//   WHATWG shift_jis: U+005C U+007E
//   iconv Shift_JIS  : U+00A5 U+203E
