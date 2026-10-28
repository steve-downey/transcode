// installtest/test.cpp                                               -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/transcode.hpp>
#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/whatwg/label.hpp>
#include <beman/transcode/whatwg/bom_sniff.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace beman::transcoding;

int main() {
    // Test 1: whatwg_decode
    std::vector<char> utf8_input{'h', 'e', 'l', 'l', 'o'};
    std::u32string    decoded;
    for (char32_t cp : utf8_input | whatwg_decode<whatwg::encoding::utf_8>) {
        decoded.push_back(cp);
    }
    if (decoded != U"hello") {
        std::cerr << "FAIL: whatwg_decode\n";
        return 1;
    }
    std::cout << "PASS: whatwg_decode\n";

    // Test 2: whatwg_encode
    std::u32string encoded_input = U"hello";
    std::string    encoded;
    for (char b : encoded_input | whatwg_encode<whatwg::encoding::utf_8>) {
        encoded.push_back(b);
    }
    if (encoded != "hello") {
        std::cerr << "FAIL: whatwg_encode\n";
        return 1;
    }
    std::cout << "PASS: whatwg_encode\n";

    // Test 3: whatwg::get_encoding label lookup
    if (whatwg::get_encoding("utf-8") != whatwg::encoding::utf_8) {
        std::cerr << "FAIL: whatwg::get_encoding\n";
        return 1;
    }
    std::cout << "PASS: whatwg::get_encoding\n";

    // Test 4: whatwg::bom_sniff BOM detection
    std::vector<unsigned char> bom = {0xEF, 0xBB, 0xBF, 'h', 'i'};
    if (whatwg::bom_sniff(bom) != whatwg::encoding::utf_8) {
        std::cerr << "FAIL: whatwg::bom_sniff\n";
        return 1;
    }
    std::cout << "PASS: whatwg::bom_sniff\n";

    // Test 5: iconv_functions accessibility (verify struct is available)
    iconv_functions fns = make_real_iconv_fns();
    if (fns.open == nullptr || fns.convert == nullptr || fns.close == nullptr) {
        std::cerr << "FAIL: iconv_functions\n";
        return 1;
    }
    std::cout << "PASS: iconv_functions\n";

    // Test 6: iconv_transcode pipe (UTF-8 → UTF-32LE)
    std::vector<char> iconv_utf8{'h', 'i'};
    std::vector<char> buf(64);
    std::vector<char> iconv_result;
    for (char ch : iconv_utf8 | iconv_transcode("UTF-8", "UTF-32LE", buf)) {
        iconv_result.push_back(ch);
    }
    // UTF-32LE of "hi" = 8 bytes
    if (iconv_result.size() != 8) {
        std::cerr << "FAIL: iconv_transcode (size=" << iconv_result.size() << ")\n";
        return 1;
    }
    std::cout << "PASS: iconv_transcode\n";

    // Test 7: Round-trip UTF-8 decode/encode
    std::vector<char> roundtrip_input{'\xE4', '\xB8', '\xAD'}; // 中 in UTF-8
    std::u32string    rt_decoded;
    for (char32_t cp : roundtrip_input | whatwg_decode<whatwg::encoding::utf_8>) {
        rt_decoded.push_back(cp);
    }
    std::vector<char> rt_encoded;
    for (char b : rt_decoded | whatwg_encode<whatwg::encoding::utf_8>) {
        rt_encoded.push_back(b);
    }
    if (rt_encoded != roundtrip_input) {
        std::cerr << "FAIL: round-trip decode/encode\n";
        return 1;
    }
    std::cout << "PASS: round-trip decode/encode\n";

    std::cout << "All tests passed!\n";
    return 0;
}
