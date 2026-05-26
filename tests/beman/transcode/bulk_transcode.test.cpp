// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/detail/bulk_transcode.hpp>
#include <beman/transcode/detail/bulk_transcode.hpp>
#include <catch2/catch_all.hpp>

#include <array>
#include <string>
#include <vector>

using namespace beman::transcoding;

TEST_CASE("bulk_transcode: decode_to UTF-8", "[bulk]") {
    const std::string input = "Hello";
    auto result = decode_to<codec::utf_8>(input);
    REQUIRE(result.size() == 5);
    CHECK(result[0] == U'H');
    CHECK(result[1] == U'e');
    CHECK(result[2] == U'l');
    CHECK(result[3] == U'l');
    CHECK(result[4] == U'o');
}

TEST_CASE("bulk_transcode: decode_to single-byte (fast path)", "[bulk]") {
    const std::string input = "ABC";
    auto result = decode_to<codec::iso_8859_15>(input);
    REQUIRE(result.size() == 3);
    CHECK(result[0] == U'A');
    CHECK(result[1] == U'B');
    CHECK(result[2] == U'C');
}

TEST_CASE("bulk_transcode: encode_to UTF-8", "[bulk]") {
    const std::u32string input = U"Hello";
    auto result = encode_to<codec::utf_8>(input);
    REQUIRE(result.size() == 5);
    CHECK(result == "Hello");
}

TEST_CASE("bulk_transcode: encode_to single-byte (fast path)", "[bulk]") {
    const std::u32string input = U"ABC";
    auto result = encode_to<codec::iso_8859_15>(input);
    REQUIRE(result.size() == 3);
    CHECK(result == "ABC");
}

TEST_CASE("bulk_transcode: encode_to<std::vector<char>>", "[bulk]") {
    const std::u32string input = U"Hi";
    auto result = encode_to<std::vector<char>, codec::utf_8>(input);
    REQUIRE(result.size() == 2);
    CHECK(result[0] == 'H');
    CHECK(result[1] == 'i');
}

TEST_CASE("bulk_transcode: decode_into", "[bulk]") {
    const std::string input = "AB";
    std::vector<char32_t> output;
    decode_into<codec::utf_8>(input, std::back_inserter(output));
    REQUIRE(output.size() == 2);
    CHECK(output[0] == U'A');
    CHECK(output[1] == U'B');
}

TEST_CASE("bulk_transcode: encode_into", "[bulk]") {
    const std::u32string input = U"XY";
    std::string output;
    encode_into<codec::utf_8>(input, std::back_inserter(output));
    REQUIRE(output.size() == 2);
    CHECK(output == "XY");
}

TEST_CASE("bulk_transcode: round-trip UTF-8", "[bulk]") {
    const std::string original = "Hello, World!";
    auto decoded = decode_to<codec::utf_8>(original);
    auto encoded = encode_to<codec::utf_8>(decoded);
    CHECK(encoded == original);
}

TEST_CASE("bulk_transcode: round-trip ISO-8859-15 (fast path)", "[bulk]") {
    const std::string original = "Gruss";
    auto decoded = decode_to<codec::iso_8859_15>(original);
    auto encoded = encode_to<codec::iso_8859_15>(decoded);
    CHECK(encoded == original);
}

TEST_CASE("bulk_transcode: vector to vector via encode_to<std::vector<char>>", "[bulk]") {
    const std::u32string input = U"Test";
    auto output_vec = encode_to<std::vector<char>, codec::utf_8>(input);
    auto output_str = encode_to<codec::utf_8>(input);
    REQUIRE(output_vec.size() == output_str.size());
    for (size_t i = 0; i < output_vec.size(); ++i) {
        CHECK(output_vec[i] == output_str[i]);
    }
}

TEST_CASE("bulk_transcode: empty input", "[bulk]") {
    const std::string empty_bytes;
    const std::u32string empty_codepoints;

    auto decoded = decode_to<codec::utf_8>(empty_bytes);
    auto encoded = encode_to<codec::utf_8>(empty_codepoints);

    CHECK(decoded.empty());
    CHECK(encoded.empty());
}
