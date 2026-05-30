# Step 19: Round-Trip Composition Tests

**Branch:** `step19-roundtrip`
**Depends on:** Step 18 (UTF-8 encoder must exist)

---

## Goal

Validate that `whatwg_decode<codec::utf_8>` composed with `whatwg_encode<codec::utf_8>` is a round-trip for valid UTF-8 input. This is the first test of view composition in the library and confirms the pipeline architecture works end-to-end.

**No new library code is added.** This step creates only a new test file.

## Composition Syntax

```cpp
auto result = bytes | whatwg_decode<codec::utf_8> | whatwg_encode<codec::utf_8>;
```

This should produce the same byte sequence as the input, provided the input is valid UTF-8.

## Files Created

### `tests/beman/transcode/roundtrip.test.cpp`

```cpp
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_decode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <beman/transcode/whatwg_encode_view.hpp>
#include <catch2/catch_all.hpp>
#include <tests/beman/transcode/test_utilities.hpp>

#include <ranges>
#include <span>
#include <string>
#include <vector>

using namespace beman::transcoding;

namespace {
std::vector<char> roundtrip(std::span<const char> input) {
    std::vector<char> output;
    for (char c : input | whatwg_decode<codec::utf_8> | whatwg_encode<codec::utf_8>)
        output.push_back(c);
    return output;
}
} // namespace

TEST_CASE("roundtrip ASCII", "[transcoding::roundtrip]") {
    std::string input = "Hello";
    auto        result = roundtrip(std::span<const char>(input.data(), input.size()));
    CHECK(std::string(result.begin(), result.end()) == input);
}

TEST_CASE("roundtrip 2-byte UTF-8", "[transcoding::roundtrip]") {
    // U+00E9 é = {0xC3, 0xA9}
    std::vector<char> input{'\xC3', '\xA9'};
    CHECK(roundtrip(input) == input);
}

TEST_CASE("roundtrip 3-byte UTF-8", "[transcoding::roundtrip]") {
    // U+20AC € = {0xE2, 0x82, 0xAC}
    std::vector<char> input{'\xE2', '\x82', '\xAC'};
    CHECK(roundtrip(input) == input);
}

TEST_CASE("roundtrip 4-byte UTF-8", "[transcoding::roundtrip]") {
    // U+1F600 = {0xF0, 0x9F, 0x98, 0x80}
    std::vector<char> input{'\xF0', '\x9F', '\x98', '\x80'};
    CHECK(roundtrip(input) == input);
}

TEST_CASE("roundtrip boundary codepoints", "[transcoding::roundtrip]") {
    // U+007F (1-byte max), U+0080 (2-byte min), U+07FF (2-byte max),
    // U+0800 (3-byte min), U+FFFF (3-byte max, excluding surrogates),
    // U+10000 (4-byte min), U+10FFFF (4-byte max)
    auto encode_cp = [](char32_t cp) {
        std::vector<char32_t> cps{cp};
        std::vector<char>     bytes;
        for (char c : cps | whatwg_encode<codec::utf_8>)
            bytes.push_back(c);
        return bytes;
    };

    for (char32_t cp : {char32_t(0x7F), char32_t(0x80), char32_t(0x7FF),
                        char32_t(0x800), char32_t(0xFFFF),
                        char32_t(0x10000), char32_t(0x10FFFF)}) {
        auto bytes = encode_cp(cp);
        CHECK(roundtrip(bytes) == bytes);
    }
}

TEST_CASE("roundtrip consteval", "[transcoding::roundtrip]") {
    using beman::transcoding::tests::constify;
    constexpr auto rt = []() consteval {
        constexpr char        bytes[] = {'\xC3', '\xA9'};  // U+00E9
        std::span<const char> sp(bytes, 2);
        // Decode then re-encode; check first output byte
        auto view = sp | whatwg_decode<codec::utf_8> | whatwg_encode<codec::utf_8>;
        return *view.begin();
    };
    CHECK(constify(rt()) == '\xC3');
}
```

## Files Modified

### `tests/beman/transcode/CMakeLists.txt`

Add new test executable:

```cmake
add_executable(beman.transcode.tests.roundtrip)
target_sources(beman.transcode.tests.roundtrip PRIVATE roundtrip.test.cpp)
target_include_directories(
    beman.transcode.tests.roundtrip
    PRIVATE ${TEST_INCLUDE_DIR}
)
target_link_libraries(
    beman.transcode.tests.roundtrip
    PRIVATE beman::transcode Catch2::Catch2WithMain
)
```

Add to `catch_discover_tests`:
```cmake
catch_discover_tests(beman.transcode.tests.roundtrip)
```

## Verification

```bash
make test   # all previous + ~6 new tests pass
make lint   # clean
```

## Notes

If invalid UTF-8 round-trips, the decode step produces U+FFFD and the encode step re-encodes that as {0xEF, 0xBF, 0xBD}. This is correct behavior — the round-trip identity property only holds for valid UTF-8 input.
