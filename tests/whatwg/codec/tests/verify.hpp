namespace whatwg::codec::tests {

// Forward declaration of your hypothetically constexpr-friendly codec
// Conceptually, it takes ranges of bytes and yields ranges of char32_t.
template <typename DecoderMock>
consteval bool verify_decoder_vectors(DecoderMock&& decoder) {
    for (const auto& test : decoder_test_vectors) {
        if (test.fatal_flag) {
            // In a consteval context, throwing an exception halts compilation.
            // A conforming C++26 consteval test should check for std::expected or an error type
            // if we are avoiding exceptions for compile-time fatal errors.
            auto result = decoder.decode(test.encoding_label, test.input_bytes, test.fatal_flag);
            if (result.has_value()) {
                return false; // Expected a fatal error, but succeeded
            }
        } else {
            auto result = decoder.decode(test.encoding_label, test.input_bytes, test.fatal_flag);
            if (!result.has_value()) {
                return false; // Unexpected error
            }
            // Use C++26 ranges algorithms to verify output
            if (!std::ranges::equal(result.value(), test.expected_code_points)) {
                return false; // Decoded string did not match expected output
            }
        }
    }
    return true;
}

} // namespace whatwg::codec::tests

// --- Test Invocation ---
// This static_assert forces the compiler to run the tests.
// (Assuming a mock or the actual codec is passed in).

/* static_assert(
    whatwg::codec::tests::verify_decoder_vectors(whatwg::codec::TextDecoder{}),
    "WHATWG Decoder WPT Base Tests Failed at Compile Time!"
);
*/
