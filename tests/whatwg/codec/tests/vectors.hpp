namespace whatwg::codec::tests {

// --- Byte Sequence Definitions ---
// C++20/C++26 std::to_array deduces the size and guarantees constexpr safe array construction.

constexpr auto utf8_valid_1 =
    std::to_array<uint8_t>({0x7A, 0xC2, 0xA2, 0xE6, 0xB0, 0xB4, 0xF0, 0x9D, 0x84, 0x9E}); // "z¢水𝄞"

constexpr auto utf8_invalid_overlong = std::to_array<uint8_t>({0xC0, 0x41}); // Invalid ASCII 'A'

constexpr auto utf8_lonely_continuation = std::to_array<uint8_t>({0x80});

constexpr auto utf8_surrogate_half = std::to_array<uint8_t>({0xED, 0xA0, 0x80});

constexpr auto win1252_valid_1 = std::to_array<uint8_t>({0x80, 0x93, 0x94}); // €, “, ”

constexpr auto sjis_valid_1 = std::to_array<uint8_t>({0x82, 0xA0}); // あ

// --- Decoder Test Vectors ---

constexpr std::array<DecodeTestCase, 6> decoder_test_vectors = {
    {{"utf-8", utf8_valid_1, U"z\u00A2\u6C34\U0001D11E", false},
     {"utf-8", utf8_invalid_overlong, U"\uFFFD\uFFFD", false},
     {"utf-8", utf8_lonely_continuation, U"", true}, // Fatal flag tests
     {"utf-8", utf8_surrogate_half, U"\uFFFD\uFFFD\uFFFD", false},
     {"windows-1252", win1252_valid_1, U"\u20AC\u201C\u201D", false},
     {"shift_jis", sjis_valid_1, U"\u3042", false}}};

// --- Encoder Test Vectors ---
// WHATWG TextEncoder only encodes to UTF-8.

constexpr auto utf8_isolated_surrogate =
    std::to_array<uint8_t>({0x61, 0x62, 0x63, 0xEF, 0xBF, 0xBD, 0x64, 0x65, 0x66});

constexpr std::array<EncodeTestCase, 2> encoder_test_vectors = {
    {{"utf-8", U"z\u00A2\u6C34\U0001D11E", utf8_valid_1, false},
     {"utf-8", U"abc\uD800def", utf8_isolated_surrogate, false}}};

} // namespace whatwg::codec::tests
