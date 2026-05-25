// installtest/test_module.cpp                                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import beman.transcode;

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace beman::transcoding;

int main() {
    // Test 1: whatwg_decode via module
    std::vector<char> utf8_input{'h', 'e', 'l', 'l', 'o'};
    std::u32string    decoded;
    for (char32_t cp : utf8_input | whatwg_decode<codec::utf_8>) {
        decoded.push_back(cp);
    }
    if (decoded != U"hello") {
        std::cerr << "FAIL: module whatwg_decode\n";
        return 1;
    }
    std::cout << "PASS: module whatwg_decode\n";

    // Test 2: whatwg_encode via module
    std::u32string encoded_input = U"hello";
    std::string    encoded;
    for (char b : encoded_input | whatwg_encode<codec::utf_8>) {
        encoded.push_back(b);
    }
    if (encoded != "hello") {
        std::cerr << "FAIL: module whatwg_encode\n";
        return 1;
    }
    std::cout << "PASS: module whatwg_encode\n";

    // Test 3: get_encoding via module
    if (get_encoding("utf-8") != codec::utf_8) {
        std::cerr << "FAIL: module get_encoding\n";
        return 1;
    }
    std::cout << "PASS: module get_encoding\n";

    // Test 4: sniff_encoding via module
    std::vector<unsigned char> bom = {0xEF, 0xBB, 0xBF, 'h', 'i'};
    if (sniff_encoding(bom) != codec::utf_8) {
        std::cerr << "FAIL: module sniff_encoding\n";
        return 1;
    }
    std::cout << "PASS: module sniff_encoding\n";

    // Test 5: iconv_functions via module
    iconv_functions fns = make_real_iconv_fns();
    if (fns.open == nullptr || fns.convert == nullptr || fns.close == nullptr) {
        std::cerr << "FAIL: module iconv_functions\n";
        return 1;
    }
    std::cout << "PASS: module iconv_functions\n";

    // Test 6: iconv_transcode via module
    std::vector<char> iconv_utf8{'h', 'i'};
    std::vector<char> buf(64);
    std::vector<char> iconv_result;
    for (char ch : iconv_utf8 | iconv_transcode("UTF-8", "UTF-32LE", buf)) {
        iconv_result.push_back(ch);
    }
    if (iconv_result.size() != 8) {
        std::cerr << "FAIL: module iconv_transcode (size=" << iconv_result.size() << ")\n";
        return 1;
    }
    std::cout << "PASS: module iconv_transcode\n";

    std::cout << "All module tests passed!\n";
    return 0;
}
