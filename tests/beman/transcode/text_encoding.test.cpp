// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg/text_encoding.hpp>
#include <beman/transcode/whatwg/text_encoding.hpp>
#include <beman/transcode/whatwg/encoding.hpp>

#include <tests/beman/transcode/test_utilities.hpp>

#include <catch2/catch_all.hpp>

#include <optional>
#include <text_encoding>
#include <version>

// This target is built as C++26, the only mode in which libstdc++ declares
// <text_encoding>.  CMake registers it only when a probe sees the macro, so a
// registered target that cannot see it is a configuration error, not a skip.
#if !defined(__cpp_lib_text_encoding)
    #error "text_encoding.test.cpp requires __cpp_lib_text_encoding (build as C++26)"
#endif

using namespace beman::transcoding;
using beman::transcoding::tests::constify;
using id = std::text_encoding::id;

namespace {

constexpr id mib(int n) { return static_cast<id>(n); }

struct mapping {
    whatwg::encoding  e;
    std::optional<id> expected;
    const char*       name;
};

// Every whatwg::encoding enumerator, with the id to_text_encoding_id gives it:
// the first label of the encoding, in label-table order, that names a
// registered encoding.  The mib comments name what each one is.  The ones that
// surprise: windows_1252 gives US-ASCII (the label "ansi_x3.4-1968" sorts
// first, and the Encoding Standard really does decode ASCII as windows-1252),
// iso_8859_8 gives ISO-8859-8-E, utf_16le gives ISO-10646-UCS-2 and gbk gives
// GB2312, all for the same reason.  The round trip below still holds for
// each, because the id is always a name that is a label of the encoding.
constexpr mapping all_encodings[] = {
    {whatwg::encoding::utf_8, mib(106), "utf_8"}, // UTF-8
    {whatwg::encoding::replacement, std::nullopt, "replacement"},
    {whatwg::encoding::x_user_defined, std::nullopt, "x_user_defined"},
    {whatwg::encoding::ibm866, mib(2086), "ibm866"},           // IBM866
    {whatwg::encoding::iso_8859_2, mib(5), "iso_8859_2"},      // ISO-8859-2
    {whatwg::encoding::iso_8859_3, mib(6), "iso_8859_3"},      // ISO-8859-3
    {whatwg::encoding::iso_8859_4, mib(7), "iso_8859_4"},      // ISO-8859-4
    {whatwg::encoding::iso_8859_5, mib(8), "iso_8859_5"},      // ISO-8859-5
    {whatwg::encoding::iso_8859_6, mib(9), "iso_8859_6"},      // ISO-8859-6
    {whatwg::encoding::iso_8859_7, mib(10), "iso_8859_7"},     // ISO-8859-7
    {whatwg::encoding::iso_8859_8, mib(84), "iso_8859_8"},     // ISO-8859-8-E
    {whatwg::encoding::iso_8859_8_i, mib(85), "iso_8859_8_i"}, // ISO-8859-8-I
    {whatwg::encoding::iso_8859_10, mib(13), "iso_8859_10"},   // ISO-8859-10
    {whatwg::encoding::iso_8859_13, mib(109), "iso_8859_13"},  // ISO-8859-13
    {whatwg::encoding::iso_8859_14, mib(110), "iso_8859_14"},  // ISO-8859-14
    {whatwg::encoding::iso_8859_15, mib(111), "iso_8859_15"},  // ISO-8859-15
    {whatwg::encoding::iso_8859_16, mib(112), "iso_8859_16"},  // ISO-8859-16
    {whatwg::encoding::koi8_r, mib(2084), "koi8_r"},           // KOI8-R
    {whatwg::encoding::koi8_u, mib(2088), "koi8_u"},           // KOI8-U
    {whatwg::encoding::macintosh, mib(2027), "macintosh"},     // macintosh
    {whatwg::encoding::windows_874, mib(2259), "windows_874"}, // windows-874
    {whatwg::encoding::windows_1250, mib(2250), "windows_1250"},
    {whatwg::encoding::windows_1251, mib(2251), "windows_1251"},
    {whatwg::encoding::windows_1252, mib(3), "windows_1252"}, // US-ASCII
    {whatwg::encoding::windows_1253, mib(2253), "windows_1253"},
    {whatwg::encoding::windows_1254, mib(12), "windows_1254"}, // ISO-8859-9
    {whatwg::encoding::windows_1255, mib(2255), "windows_1255"},
    {whatwg::encoding::windows_1256, mib(2256), "windows_1256"},
    {whatwg::encoding::windows_1257, mib(2257), "windows_1257"},
    {whatwg::encoding::windows_1258, mib(2258), "windows_1258"},
    {whatwg::encoding::x_mac_cyrillic, std::nullopt, "x_mac_cyrillic"},
    {whatwg::encoding::utf_16be, mib(1013), "utf_16be"},     // UTF-16BE
    {whatwg::encoding::utf_16le, mib(1000), "utf_16le"},     // ISO-10646-UCS-2, from the label "ucs-2"
    {whatwg::encoding::gbk, mib(57), "gbk"},                 // GB2312
    {whatwg::encoding::gb18030, mib(114), "gb18030"},        // GB18030
    {whatwg::encoding::big5, mib(2026), "big5"},             // Big5
    {whatwg::encoding::shift_jis, mib(17), "shift_jis"},     // Shift_JIS
    {whatwg::encoding::euc_jp, mib(18), "euc_jp"},           // EUC-JP
    {whatwg::encoding::iso_2022_jp, mib(39), "iso_2022_jp"}, // ISO-2022-JP
    {whatwg::encoding::euc_kr, mib(38), "euc_kr"},           // EUC-KR
};

} // namespace

TEST_CASE("get_encoding(text_encoding::id): registered ids", "[transcoding::text_encoding]") {
    CHECK(whatwg::get_encoding(id::UTF8) == whatwg::encoding::utf_8);
    CHECK(whatwg::get_encoding(id::ISOLatin1) == whatwg::encoding::windows_1252);
    CHECK(whatwg::get_encoding(id::windows1252) == whatwg::encoding::windows_1252);
    CHECK(whatwg::get_encoding(id::ShiftJIS) == whatwg::encoding::shift_jis);
    CHECK(whatwg::get_encoding(id::ASCII) == whatwg::encoding::windows_1252);
}

TEST_CASE("get_encoding(text_encoding::id): ids with no WHATWG label", "[transcoding::text_encoding]") {
    CHECK(whatwg::get_encoding(id::other) == std::nullopt);
    CHECK(whatwg::get_encoding(id::unknown) == std::nullopt);
    // A registered EBCDIC code page: not an Encoding Standard encoding.
    CHECK(whatwg::get_encoding(id::IBM037) == std::nullopt);
}

TEST_CASE("to_text_encoding_id: every whatwg::encoding", "[transcoding::text_encoding]") {
    for (const auto& m : all_encodings) {
        INFO(m.name);
        CHECK(whatwg::to_text_encoding_id(m.e) == m.expected);
    }
    // replacement and x_user_defined name no registered encoding.  So does
    // x_mac_cyrillic, whose labels are not IANA names.
    CHECK(whatwg::to_text_encoding_id(whatwg::encoding::replacement) == std::nullopt);
    CHECK(whatwg::to_text_encoding_id(whatwg::encoding::x_user_defined) == std::nullopt);
    CHECK(whatwg::to_text_encoding_id(whatwg::encoding::x_mac_cyrillic) == std::nullopt);
}

TEST_CASE("to_text_encoding_id: table covers every enumerator in order", "[transcoding::text_encoding]") {
    int i = 0;
    for (const auto& m : all_encodings) {
        CHECK(static_cast<int>(m.e) == i);
        ++i;
    }
    CHECK(i == static_cast<int>(whatwg::encoding::euc_kr) + 1);
}

TEST_CASE("text_encoding bridge: round trip", "[transcoding::text_encoding]") {
    for (const auto& m : all_encodings) {
        if (!m.expected)
            continue;
        INFO(m.name);
        CHECK(whatwg::get_encoding(*m.expected) == m.e);
    }
}

TEST_CASE("text_encoding bridge: constant evaluation", "[transcoding::text_encoding]") {
    CHECK(constify(whatwg::get_encoding(id::ISOLatin1)) == whatwg::encoding::windows_1252);
    CHECK(constify(whatwg::get_encoding(id::ShiftJIS)) == whatwg::encoding::shift_jis);
    CHECK(constify(whatwg::get_encoding(id::other)) == std::nullopt);
    CHECK(constify(whatwg::to_text_encoding_id(whatwg::encoding::shift_jis)) == std::optional<id>{id::ShiftJIS});
    CHECK(constify(whatwg::to_text_encoding_id(whatwg::encoding::utf_8)) == std::optional<id>{id::UTF8});
    CHECK(constify(whatwg::to_text_encoding_id(whatwg::encoding::replacement)) == std::nullopt);
}
