Here is the comprehensive execution plan for the C++29 Transcoding Architecture project, synthesizing the architectural blueprint, Beman project guidelines, and the robust, clean-room testing strategy we have developed.

# ---

**Execution Plan: C++29 Transcoding Architecture (Beman Project)**

## **Executive Summary**

This project will deliver a C++29 standard proposal and reference implementation for pipeline-oriented string transcoding, extending P2728R12 patterns to support the WHATWG Encoding Standard and POSIX iconv. The implementation will strictly adhere to Beman Project standards. To guarantee adoption viability by conservative C++ standard library vendors, the project will structurally eliminate undefined behavior at compile-time and enforce a strict "clean room" out-of-tree testing architecture for POSIX conformance to avoid GPL/IP contamination.

## ---

**Phase 1: Project Scaffolding & Beman Conformance**

Initialize the repository according to the exact specifications of the bemanproject/exemplar template.

1. **Repository Generation:** Create bemanproject/transcoding using the Exemplar template.
2. **Boilerplate Instantiation:** Execute ./stamp.sh with the project name transcoding and the intended proposal number (e.g., PXXXXRX).
3. **Licensing Enforcement:** Ensure all source files include the // SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception header.
4. **Build System Setup:** Configure CMakeLists.txt to expose beman::transcoding as the primary library target, with toggleable BEMAN\_TRANSCODING\_BUILD\_TESTS and BEMAN\_TRANSCODING\_BUILD\_EXAMPLES.
5. **Directory Strictness:** Enforce the split between include/beman/transcoding/ (public interface), include/beman/transcoding/detail/ (private traits/concepts), tests/, examples/, and papers/.

## ---

**Phase 2: Core Library Implementation & Safety Constraints**

Implement the views using modern C++20/C++23 concepts to make incorrect API usage unrepresentable at compile-time.

1. **Core Types & Concepts (detail/concepts.hpp):**
   * Implement whatwg\_error and iconv\_error enums.
   * Define detail::legacy\_byte\_range to strictly accept char, signed char, unsigned char, and std::byte, explicitly rejecting char8/16/32\_t.
   * **Array Decay Ban:** Constrain input ranges to reject const char\[N\] to prevent silent null-byte inclusion. Require explicit use of std::views::null\_term (P3705R2).
2. **WHATWG View (whatwg\_decode\_view.hpp):**
   * Implement the "Dispenser Pattern" as a single-pass std::input\_iterator.
   * Pass the codec (e.g., windows\_1252) as a Non-Type Template Parameter (NTTP).
   * Implement whatwg\_decode (yielding U+FFFD on failure) and whatwg\_decode\_or\_error (yielding std::expected or monadic equivalents on failure).
3. **POSIX iconv View (iconv\_transcode\_view.hpp):**
   * Implement as a move-only std::input\_iterator to safeguard the uncopyable iconv\_t resource.
   * Accept an external std::span\<char\> working buffer to prevent internal heap allocations.
   * Inject the iconv execution routine as a callable to decouple the view from the host OS libc.

## ---

**Phase 3: In-Tree Unit Testing & Structural Validation**

Write unit tests within tests/beman/transcoding/ to validate C++ semantics and boundary resilience without relying on external system state.

1. **Compile-Time Constraint Tests:**
   * static\_assert that array literals fail to compile.
   * static\_assert that iconv\_iterator is strictly move-only (\!std::copy\_constructible).
   * Verify dangling iterator prevention (decltype(std::string("temp") | whatwg\_decode) yields std::ranges::dangling).
2. **Dependency Injection & Edge Cases (mock\_iconv\_fn):**
   * Inject a mock iconv callable to simulate POSIX errors deterministically.
   * **The 1-Byte Buffer Test:** Force an E2BIG error and assert the view caches state, yields available bytes, and resumes correctly.
   * **Incomplete Sequence Test:** Force an EINVAL error and assert the view fetches more bytes from the underlying iterator.
   * Verify destructors properly call iconv\_close even when the underlying range throws an exception mid-iteration.

## ---

**Phase 4: WHATWG Conformance (Data-Driven)**

Integrate standard web conformance tests into the main repository using permissively licensed data.

1. **Data Sourcing:** Download the JSON encoding test vectors directly from the W3C Web Platform Tests (WPT) repository (github.com/web-platform-tests/wpt).
2. **Test Generation:** Create a Python or CMake script in tests/data/ that parses the WPT JSON and generates C++ constexpr arrays.
3. **Parameterization:** Use the Beman testing framework to feed these arrays through whatwg\_decode\_view to prove 100% adherence to the WHATWG specification, validating BOM sniffing, replacement character insertion, and complex shift-state legacy web encodings.

## ---

**Phase 5: Out-of-Tree POSIX Conformance Suite (Clean Room)**

To avoid GPL/LGPL contamination from GNU libiconv and ensure major library vendors (LLVM, MSVC) can validate the architecture safely, build a physically separated black-box testing harness.

1. **Repository:** Initialize bemanproject/iconv-conformance-suite (separate from the main codebase).
2. **Test Vector Aggregation:** Gather exhaustive edge-case conversions, shift-state transitions, and failure states from GNU libiconv, glibc, and host OS fuzzing. Store these strictly as language-agnostic JSON/YAML files.
3. **Black-Box Test Harness:** Write an Apache 2.0-licensed harness that reads the JSON data and feeds it into whatever iconv\_transcode\_view implementation the user points it to.
4. **Vendor Workflow:** Standard library maintainers run the harness against their native OS libc to validate state-machine boundaries without ever viewing the encumbered test data.

## ---

**Phase 6: Standardization Packaging**

Package the architecture and findings into a formal WG21 standard proposal.

1. **Wording Draft (papers/PXXXX.md):** Translate the Beman C++ implementation into standardese. Define the exposition-only concepts, the constexpr requirements, and the strict single-pass/move-only iterator constraints.
2. **Rationale & Defense:** Highlight the compile-time elimination of array decay (via P3705R2 null\_term) and the architectural resilience against E2BIG/EINVAL POSIX attacks.
3. **The "Clean Room" Selling Point:** Explicitly document the out-of-tree conformance suite in the proposal. Assure SG16 (Text) and SG9 (Ranges) that the architecture is exhaustively proven while completely shielding standard library implementers from intellectual property risks.
