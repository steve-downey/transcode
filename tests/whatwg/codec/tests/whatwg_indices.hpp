#include <cstdint>
#include <array>
#include <span>
#include <cstddef>

namespace whatwg::codec::indices {

// Helper to convert little-endian embedded bytes into a constexpr uint16_t array.
// This avoids reinterpret_cast, making it 100% consteval/constexpr compliant.
template <size_t NumBytes>
consteval auto bytes_to_uint16_array(const std::array<uint8_t, NumBytes>& bytes) {
    static_assert(NumBytes % 2 == 0, "Byte array size must be a multiple of 2 for uint16_t conversion");

    std::array<uint16_t, NumBytes / 2> result{};
    for (size_t i = 0; i < result.size(); ++i) {
        // Reconstruct from little-endian
        result[i] = static_cast<uint16_t>(bytes[i * 2] | (bytes[i * 2 + 1] << 8));
    }
    return result;
}

// --- Big5 Index ---
constexpr std::array<uint8_t, 24718> big5_bytes = {
// #embed requires a specific path. Your build system will set the include directories.
#embed "generated/big5.bin"
};
constexpr auto big5_index = bytes_to_uint16_array(big5_bytes);

// --- Shift_JIS Index ---
constexpr std::array<uint8_t, 16954> sjis_bytes = {
#embed "generated/shift_jis.bin"
};
constexpr auto sjis_index = bytes_to_uint16_array(sjis_bytes);

// Note: For actual implementation, the sizes (e.g., 24718) can be omitted if you use:
// constexpr std::array big5_bytes = { #embed "generated/big5.bin" };
// (Relying on C++ template argument deduction for std::array, though explicit sizing
// is sometimes safer for preventing silent regressions in standard index sizes).

} // namespace whatwg::codec::indices
