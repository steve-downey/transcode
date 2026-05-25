// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_fatal_single_byte_vectors.hpp>

#include <algorithm>
#include <array>
#include <cstdint>

using namespace beman::transcoding;
using beman::transcoding::tests::wpt::WptFatalSingleByteCase;
using beman::transcoding::tests::wpt::wpt_fatal_single_byte_cases;

namespace {

template <codec C>
void check_fatal_single_byte(const WptFatalSingleByteCase& c) {
    for (int b = 0; b < 256; ++b) {
        char              ch = static_cast<char>(static_cast<uint8_t>(b));
        std::array<char, 1> input{ch};
        bool              has_error = false;
        for (auto&& r : input | whatwg_decode_or_error<C>) {
            if (!r.has_value()) {
                has_error = true;
                break;
            }
        }
        bool expect_error =
            std::ranges::contains(c.bad, static_cast<uint8_t>(b));
        INFO("encoding=" << c.encoding << " byte=0x" << std::hex << b);
        CHECK(has_error == expect_error);
    }
}

} // namespace

TEST_CASE("WPT fatal single-byte: IBM866", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::ibm866>(wpt_fatal_single_byte_cases[0]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-2", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_2>(wpt_fatal_single_byte_cases[1]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-3", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_3>(wpt_fatal_single_byte_cases[2]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-4", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_4>(wpt_fatal_single_byte_cases[3]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-5", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_5>(wpt_fatal_single_byte_cases[4]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-6", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_6>(wpt_fatal_single_byte_cases[5]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-7", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_7>(wpt_fatal_single_byte_cases[6]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-8", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_8>(wpt_fatal_single_byte_cases[7]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-8-I", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_8_i>(wpt_fatal_single_byte_cases[8]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-10", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_10>(wpt_fatal_single_byte_cases[9]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-13", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_13>(wpt_fatal_single_byte_cases[10]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-14", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_14>(wpt_fatal_single_byte_cases[11]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-15", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_15>(wpt_fatal_single_byte_cases[12]);
}
TEST_CASE("WPT fatal single-byte: ISO-8859-16", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::iso_8859_16>(wpt_fatal_single_byte_cases[13]);
}
TEST_CASE("WPT fatal single-byte: KOI8-R", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::koi8_r>(wpt_fatal_single_byte_cases[14]);
}
TEST_CASE("WPT fatal single-byte: KOI8-U", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::koi8_u>(wpt_fatal_single_byte_cases[15]);
}
TEST_CASE("WPT fatal single-byte: macintosh", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::macintosh>(wpt_fatal_single_byte_cases[16]);
}
TEST_CASE("WPT fatal single-byte: windows-874", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::windows_874>(wpt_fatal_single_byte_cases[17]);
}
TEST_CASE("WPT fatal single-byte: windows-1250", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::windows_1250>(wpt_fatal_single_byte_cases[18]);
}
TEST_CASE("WPT fatal single-byte: windows-1251", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::windows_1251>(wpt_fatal_single_byte_cases[19]);
}
TEST_CASE("WPT fatal single-byte: windows-1252", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::windows_1252>(wpt_fatal_single_byte_cases[20]);
}
TEST_CASE("WPT fatal single-byte: windows-1253", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::windows_1253>(wpt_fatal_single_byte_cases[21]);
}
TEST_CASE("WPT fatal single-byte: windows-1254", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::windows_1254>(wpt_fatal_single_byte_cases[22]);
}
TEST_CASE("WPT fatal single-byte: windows-1255", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::windows_1255>(wpt_fatal_single_byte_cases[23]);
}
TEST_CASE("WPT fatal single-byte: windows-1256", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::windows_1256>(wpt_fatal_single_byte_cases[24]);
}
TEST_CASE("WPT fatal single-byte: windows-1257", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::windows_1257>(wpt_fatal_single_byte_cases[25]);
}
TEST_CASE("WPT fatal single-byte: windows-1258", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::windows_1258>(wpt_fatal_single_byte_cases[26]);
}
TEST_CASE("WPT fatal single-byte: x-mac-cyrillic", "[wpt::fatal_single_byte]") {
    check_fatal_single_byte<codec::x_mac_cyrillic>(wpt_fatal_single_byte_cases[27]);
}
