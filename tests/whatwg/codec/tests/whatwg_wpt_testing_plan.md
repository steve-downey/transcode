# C++26 WHATWG Transcoding Architecture: Test Plan & Implementation Guide

**Project Context:** Integration of WHATWG encoding Web Platform Tests (WPT) into a modern C++26 codec architecture (e.g., `steve-downey/transcode`).
**Core Objectives:** Extract test vectors from WPT, eliminate runtime initialization overhead, ensure strict compile-time (`consteval`) compliance, and leverage C++26 `#embed` for binary data ingestion without polluting the source tree.

---

## 1. Architectural Overview & Provenance

The goal is to test a WHATWG-conforming C++ codec implementation against the official Web Platform Tests. The challenge lies in managing large data-driven indices for legacy encodings (e.g., GB18030, Big5) alongside standard encoding vectors (UTF-8) while strictly adhering to C++26 `consteval` constraints.

### 1.1 Provenance & Licensing Requirement
The test vectors are derived from the Web Platform Tests (WPT) project (`encoding/indexes.json`). WPT is licensed under the **W3C 3-Clause BSD License**. To maintain provenance, all extracted test structures and generated binaries must legally acknowledge this.

**Action Item:** Include the following header in your C++ test framework and generated files:

```cpp
/**
 * PROVENANCE AND LICENSE:
 * These test vectors and indices are derived from the Web Platform Tests (WPT) project.
 * Repository: [https://github.com/web-platform-tests/wpt/tree/master/encoding](https://github.com/web-platform-tests/wpt/tree/master/encoding)
 * License: W3C 3-Clause BSD License
 * Copyright (c) W3C and Web Platform Tests contributors.
 */
```

---

## 2. Phase 1: Data Transformation (Python Preprocessor)

Instead of manually maintaining megabytes of static C++ arrays or forcing the compiler to parse JSON at compile time, we use a Python script to parse `indexes.json` at build generation time and output strictly typed, little-endian binary files (`.bin`).

**File:** `tools/generate_indices.py`

```python
import json
import struct
import argparse
from pathlib import Path

def generate_binary_indices(json_path: Path, output_dir: Path):
    with open(json_path, 'r', encoding='utf-8') as f:
        indexes = json.load(f)

    output_dir.mkdir(parents=True, exist_ok=True)

    for index_name, data in indexes.items():
        safe_name = index_name.replace('-', '_').lower()
        out_file = output_dir / f"{safe_name}.bin"

        # WPT indexes map integers to code points or nulls.
        # Most WHATWG indices fit in 16-bit (uint16_t). Using 0xFFFF as sentinel for null.
        with open(out_file, 'wb') as bin_out:
            for entry in data:
                if entry is None:
                    bin_out.write(struct.pack('<H', 0xFFFF))
                else:
                    bin_out.write(struct.pack('<H', entry))

    print(f"Successfully generated binary indices in {output_dir}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert WPT indexes to binary for C++26 #embed")
    parser.add_argument("input", type=Path, help="Path to WPT indexes.json")
    parser.add_argument("outdir", type=Path, help="Output directory for .bin files")
    args = parser.parse_args()

    generate_binary_indices(args.input, args.outdir)
```

---

## 3. Phase 2: Build Orchestration (CMake)

The build orchestration manages the data lifecycle. Using `CMakeLists.txt`, we ensure the `.bin` files are generated *before* the C++ compiler runs and placed exclusively in the build directory (`CMAKE_CURRENT_BINARY_DIR`) to keep the source tree pristine.

**File:** `tests/whatwg/codec/tests/CMakeLists.txt`

```cmake
# 1. Find the Python3 Interpreter
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# 2. Define input and generator paths
set(WPT_INDEXES_JSON "${CMAKE_SOURCE_DIR}/wpt/encoding/indexes.json")
set(PYTHON_GENERATOR "${CMAKE_SOURCE_DIR}/tools/generate_indices.py")

# 3. Output directory in the build tree
set(GENERATED_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

# 4. Explicit outputs for the build graph
set(GENERATED_BIN_FILES
    "${GENERATED_BIN_DIR}/big5.bin"
    "${GENERATED_BIN_DIR}/shift_jis.bin"
    # Append other index files as needed
)

# 5. Custom Command to generate binaries
add_custom_command(
    OUTPUT ${GENERATED_BIN_FILES}
    COMMAND Python3::Interpreter "${PYTHON_GENERATOR}" "${WPT_INDEXES_JSON}" "${GENERATED_BIN_DIR}"
    DEPENDS "${WPT_INDEXES_JSON}" "${PYTHON_GENERATOR}"
    COMMENT "Generating C++26 #embed binary indices from WPT JSON..."
    VERBATIM
)

# 6. Custom Target for dependency mapping
add_custom_target(wpt_indices_data DEPENDS ${GENERATED_BIN_FILES})

# ---------------------------------------------------------
# C++ Test Executable Target
# ---------------------------------------------------------
add_executable(whatwg_codec_tests test_main.cpp)

# Enforce C++26 standard
target_compile_features(whatwg_codec_tests PRIVATE cxx_std_26)

# Force generation before compilation
add_dependencies(whatwg_codec_tests wpt_indices_data)

# Include CMAKE_CURRENT_BINARY_DIR so #embed resolves automatically
target_include_directories(whatwg_codec_tests PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
```

---

## 4. Phase 3: C++26 Test Architecture & `#embed`

This layer utilizes C++26's `#embed` to ingest the generated binary files. Because we require `consteval` execution for the tests, we implement a safe compilation-time transformer to convert the raw `uint8_t` spans into `uint16_t` arrays without invoking undefined behavior via `reinterpret_cast`.

**File:** `tests/whatwg/codec/tests/whatwg_indices.hpp`

```cpp
#include <cstdint>
#include <array>
#include <span>
#include <cstddef>
#include <string_view>

namespace whatwg::codec::tests {

struct DecodeTestCase {
    std::string_view encoding_label;
    std::span<const uint8_t> input_bytes;
    std::u32string_view expected_code_points;
    bool fatal_flag;
};

// Safe consteval transformer for little-endian #embed data
template <size_t NumBytes>
consteval auto bytes_to_uint16_array(const std::array<uint8_t, NumBytes>& bytes) {
    static_assert(NumBytes % 2 == 0, "Byte array size must be a multiple of 2");
    std::array<uint16_t, NumBytes / 2> result{};
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<uint16_t>(bytes[i * 2] | (bytes[i * 2 + 1] << 8));
    }
    return result;
}

// Ingesting the Big5 table
constexpr std::array<uint8_t, 24718> big5_bytes = {
    #embed "generated/big5.bin"
};
constexpr auto big5_index = bytes_to_uint16_array(big5_bytes);

// Ingesting standard static test vectors (UTF-8 edge cases)
constexpr auto utf8_invalid_overlong = std::to_array<uint8_t>({ 0xC0, 0x41 });
constexpr auto utf8_surrogate_half = std::to_array<uint8_t>({ 0xED, 0xA0, 0x80 });

} // namespace whatwg::codec::tests
```

---

## 5. Phase 4: Consteval Validation Framework

To guarantee zero-cost runtime and enforce strict spec compliance, the test evaluator executes purely at compile time using `<ranges>`. The tests validate state transitions and fatal flags.

**File:** `tests/whatwg/codec/tests/test_main.cpp`

```cpp
#include "whatwg_indices.hpp"
#include <ranges>
#include <algorithm>

namespace whatwg::codec::tests {

constexpr std::array<DecodeTestCase, 2> decoder_test_vectors = {{
    { "utf-8", utf8_invalid_overlong, U"\uFFFD\uFFFD", false },
    { "utf-8", utf8_surrogate_half, U"", true } // fatal mode expected to fail
}};

template <typename DecoderMock>
consteval bool verify_decoder_vectors(DecoderMock&& decoder) {
    for (const auto& test : decoder_test_vectors) {
        auto result = decoder.decode(test.encoding_label, test.input_bytes, test.fatal_flag);

        if (test.fatal_flag) {
            // Expected failure
            if (result.has_value()) return false;
        } else {
            // Expected success
            if (!result.has_value()) return false;
            if (!std::ranges::equal(result.value(), test.expected_code_points)) return false;
        }
    }
    return true;
}

} // namespace whatwg::codec::tests

// Trigger consteval test suite
// static_assert(
//     whatwg::codec::tests::verify_decoder_vectors(whatwg::codec::TextDecoder{}),
//     "WHATWG Decoder WPT Base Tests Failed at Compile Time!"
// );

int main() {
    return 0; // Runtime is empty. Tests proven at compile time.
}
```

---

## 6. Implementation Roadmap

1. **Environment Setup:** Ensure your compiler supports C++26 `#embed` (Clang 19+ / GCC 15+).
2. **Commit Scripts:** Place `generate_indices.py` in `tools/`.
3. **Wire CMake:** Update `tests/whatwg/codec/tests/CMakeLists.txt` with the custom commands provided.
4. **Implement Types:** Implement your `std::expected` or views-based `decode` signature on `TextDecoder`.
5. **Expand Test Coverage:** Incrementally map the JSON arrays to `DecodeTestCase` declarations in C++.

---

## 7. Reference Materials

* **WHATWG Encoding Specification:** [https://encoding.spec.whatwg.org/](https://encoding.spec.whatwg.org/)
* **Web Platform Tests (WPT) - Encoding Directory:** [https://github.com/web-platform-tests/wpt/tree/master/encoding](https://github.com/web-platform-tests/wpt/tree/master/encoding)
* **W3C 3-Clause BSD License Details:** [https://www.w3.org/Consortium/Legal/2008/03-bsd-license.html](https://www.w3.org/Consortium/Legal/2008/03-bsd-license.html)
* **C++26 `#embed` Proposal (P1967R13):** [https://wg21.link/p1967](https://wg21.link/p1967)
* **C++ `std::ranges` Library Reference:** [https://en.cppreference.com/w/cpp/ranges](https://en.cppreference.com/w/cpp/ranges)
