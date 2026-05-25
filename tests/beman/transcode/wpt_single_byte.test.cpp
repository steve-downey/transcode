// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/wpt_single_byte_vectors.hpp>

#include <cstdint>
#include <vector>

using namespace beman::transcoding;
using beman::transcoding::tests::wpt::wpt_single_byte_indexes;
using beman::transcoding::tests::wpt::WptSingleByteIndex;

namespace {

template <codec C>
void check_single_byte_index(const WptSingleByteIndex& idx) {
    for (int i = 0; i < 128; ++i) {
        uint8_t               byte = static_cast<uint8_t>(i + 0x80);
        std::vector<char>     input{static_cast<char>(byte)};
        std::vector<char32_t> result;
        for (char32_t cp : input | whatwg_decode<C>)
            result.push_back(cp);
        REQUIRE(result.size() == 1);
        INFO("encoding=" << idx.name << " byte=0x" << std::hex << static_cast<int>(byte));
        CHECK(result[0] == idx.codepoints[i]);
    }
}

} // namespace

TEST_CASE("WPT single-byte: IBM866", "[wpt::single_byte]") {
    check_single_byte_index<codec::ibm866>(wpt_single_byte_indexes[0]);
}

TEST_CASE("WPT single-byte: ISO-8859-2", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_2>(wpt_single_byte_indexes[1]);
}

TEST_CASE("WPT single-byte: ISO-8859-3", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_3>(wpt_single_byte_indexes[2]);
}

TEST_CASE("WPT single-byte: ISO-8859-4", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_4>(wpt_single_byte_indexes[3]);
}

TEST_CASE("WPT single-byte: ISO-8859-5", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_5>(wpt_single_byte_indexes[4]);
}

TEST_CASE("WPT single-byte: ISO-8859-6", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_6>(wpt_single_byte_indexes[5]);
}

TEST_CASE("WPT single-byte: ISO-8859-7", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_7>(wpt_single_byte_indexes[6]);
}

TEST_CASE("WPT single-byte: ISO-8859-8", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_8>(wpt_single_byte_indexes[7]);
}

TEST_CASE("WPT single-byte: ISO-8859-10", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_10>(wpt_single_byte_indexes[8]);
}

TEST_CASE("WPT single-byte: ISO-8859-13", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_13>(wpt_single_byte_indexes[9]);
}

TEST_CASE("WPT single-byte: ISO-8859-14", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_14>(wpt_single_byte_indexes[10]);
}

TEST_CASE("WPT single-byte: ISO-8859-15", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_15>(wpt_single_byte_indexes[11]);
}

TEST_CASE("WPT single-byte: ISO-8859-16", "[wpt::single_byte]") {
    check_single_byte_index<codec::iso_8859_16>(wpt_single_byte_indexes[12]);
}

TEST_CASE("WPT single-byte: KOI8-R", "[wpt::single_byte]") {
    check_single_byte_index<codec::koi8_r>(wpt_single_byte_indexes[13]);
}

TEST_CASE("WPT single-byte: KOI8-U", "[wpt::single_byte]") {
    check_single_byte_index<codec::koi8_u>(wpt_single_byte_indexes[14]);
}

TEST_CASE("WPT single-byte: macintosh", "[wpt::single_byte]") {
    check_single_byte_index<codec::macintosh>(wpt_single_byte_indexes[15]);
}

TEST_CASE("WPT single-byte: windows-874", "[wpt::single_byte]") {
    check_single_byte_index<codec::windows_874>(wpt_single_byte_indexes[16]);
}

TEST_CASE("WPT single-byte: windows-1250", "[wpt::single_byte]") {
    check_single_byte_index<codec::windows_1250>(wpt_single_byte_indexes[17]);
}

TEST_CASE("WPT single-byte: windows-1251", "[wpt::single_byte]") {
    check_single_byte_index<codec::windows_1251>(wpt_single_byte_indexes[18]);
}

TEST_CASE("WPT single-byte: windows-1252", "[wpt::single_byte]") {
    check_single_byte_index<codec::windows_1252>(wpt_single_byte_indexes[19]);
}

TEST_CASE("WPT single-byte: windows-1253", "[wpt::single_byte]") {
    check_single_byte_index<codec::windows_1253>(wpt_single_byte_indexes[20]);
}

TEST_CASE("WPT single-byte: windows-1254", "[wpt::single_byte]") {
    check_single_byte_index<codec::windows_1254>(wpt_single_byte_indexes[21]);
}

TEST_CASE("WPT single-byte: windows-1255", "[wpt::single_byte]") {
    check_single_byte_index<codec::windows_1255>(wpt_single_byte_indexes[22]);
}

TEST_CASE("WPT single-byte: windows-1256", "[wpt::single_byte]") {
    check_single_byte_index<codec::windows_1256>(wpt_single_byte_indexes[23]);
}

TEST_CASE("WPT single-byte: windows-1257", "[wpt::single_byte]") {
    check_single_byte_index<codec::windows_1257>(wpt_single_byte_indexes[24]);
}

TEST_CASE("WPT single-byte: windows-1258", "[wpt::single_byte]") {
    check_single_byte_index<codec::windows_1258>(wpt_single_byte_indexes[25]);
}

TEST_CASE("WPT single-byte: x-mac-cyrillic", "[wpt::single_byte]") {
    check_single_byte_index<codec::x_mac_cyrillic>(wpt_single_byte_indexes[26]);
}
