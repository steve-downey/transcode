# Testing C++ Range Adapters over `iconv`

If the goal is purely to test the C++ range adapter (the view) that wraps the `iconv` C API, you don't need megabytes of WPT JSON data. You only need a tiny, surgical set of test vectors designed specifically to trigger the boundary conditions of your `iconv` state machine.

Because a C++ view (especially a lazy or chunked one) must translate range iteration into `char**` buffer manipulations, your tests must verify that the view correctly interprets `iconv`'s return values and manages its internal buffers across iterations.

## 1. The `iconv` View Boundary Conditions

The POSIX `iconv` function communicates state via its return value (returning `(size_t)-1`) and `errno`. Your C++ view iterator must handle these precisely. We will use simple UTF-8 to UTF-16 (or UTF-32) conversions to trigger these states.

### A. Buffer Exhaustion (`E2BIG`)
If your view processes data in chunks, `iconv` will return `E2BIG` when the output buffer fills up before the input is consumed.
* **Test Vector:** A long, valid string (e.g., 100 repetitions of `A`).
* **View Expectation:** The iterator must yield the converted characters from the filled buffer, and upon the next increment, seamlessly resume passing the *unconsumed* input pointer back into `iconv` without dropping characters.

### B. The Split Multi-byte Sequence (`EINVAL`)
This is the most critical test for a range adapter. If the underlying data source provides bytes in chunks (e.g., reading from a socket or file), a multi-byte character might be sliced exactly in half at the chunk boundary. `iconv` will stop and return `EINVAL`.
* **Test Vector:** The UTF-8 sequence for "𝄞" (U+1D11E): `0xF0 0x9D 0x84 0x9E`.
* **Procedure:** Feed the view an input range that yields `0xF0 0x9D` on the first iteration, and `0x84 0x9E` on the second.
* **View Expectation:** The view must cache the leftover bytes, prepend them to the next chunk it pulls from the underlying range, and successfully yield the code point without throwing an error.

### C. Invalid Byte Sequences (`EILSEQ`)
The view must handle strictly malformed data gracefully, according to your C++ API design (either throwing, returning a `std::expected` error state, or yielding a replacement character).
* **Test Vector:** `0xFF 0xFF`.
* **View Expectation:** The view evaluates `errno == EILSEQ`. It must not infinite-loop. It should advance the input pointer past the bad bytes (usually by 1) if it is designed to emit replacement characters, or immediately halt the range and bubble up the error.

### D. End-of-Range Stateful Flush
Legacy encodings use shift states. When a sequence ends, `iconv` requires a final call with `inbuf == nullptr` to flush any reset bytes (like switching back to ASCII) into the output buffer.
* **Test Vector:** ISO-2022-JP encoding of "あ" (`0x1B 0x24 0x42 0x24 0x22`).
* **View Expectation:** When the underlying input range reaches `std::ranges::end`, the view's iterator must make the flush call to `iconv`. If the flush produces additional bytes, the iterator must yield them before finally marking itself as equal to the sentinel/end iterator.

---

## 2. Mocking `iconv` for Deterministic View Testing

Relying on the system's `iconv` for unit testing the *view logic* can be risky because system implementations vary in how strictly they handle `EILSEQ` or undocumented encodings.

The standard C++ approach for this is to inject a mock `iconv` wrapper into the view during testing. This guarantees your view is handling the API contract, completely independent of the underlying C library.

```cpp
#include <cerrno>
#include <cstdint>
#include <cstddef>
#include <string_view>
#include <vector>

namespace whatwg::codec::tests {

// A mock signature matching the POSIX iconv API
size_t mock_iconv(void* /*cd*/,
                  char** inbuf, size_t* inbytesleft,
                  char** outbuf, size_t* outbytesleft) {

    if (inbuf == nullptr || *inbuf == nullptr) {
        // Simulate end-of-range flush (e.g., write 1 byte of state reset)
        if (*outbytesleft < 1) {
            errno = E2BIG;
            return (size_t)-1;
        }
        **outbuf = 0x0F; // Mock shift-in byte
        (*outbuf)++;
        (*outbytesleft)--;
        return 0;
    }

    // Simulate EINVAL (Incomplete sequence at the end of input)
    // If the input is exactly the byte 0xF0, pretend it needs more bytes.
    if (*inbytesleft == 1 && static_cast<uint8_t>((*inbuf)[0]) == 0xF0) {
        errno = EINVAL;
        return (size_t)-1;
    }

    // Simulate E2BIG (Output buffer full)
    if (*outbytesleft == 0 && *inbytesleft > 0) {
        errno = E2BIG;
        return (size_t)-1;
    }

    // ... Implementation for simply passing bytes through for EILSEQ checks ...

    return 0; // Success
}

} // namespace whatwg::codec::tests
```

---

## 3. Testing Execution Plan

To test the view, you will:
1.  **Define your C++26 View:** E.g., `auto transcoded = input_bytes | views::iconv_transcode("UTF-8", "UTF-16");`
2.  **Pass chunked inputs:** Use `std::views::chunk` on your test vectors before piping them into your transcode view to artificially force `EINVAL` across chunk boundaries.
3.  **Constrain the output:** Use a tiny static buffer inside your test loop to artificially force `E2BIG`, verifying the view iterator doesn't drop state between increments.
