# Testing C++ Range Adapters over `iconv`

Testing the range adapter that wraps the `iconv` C API is a different job from
testing a codec.  It does not need megabytes of WPT JSON.  A small set of
vectors aimed at the boundary conditions of the `iconv` state machine is enough.

The view has to translate range iteration into `char**` buffer manipulation, so
what the tests have to pin down is whether it reads `iconv`'s return values
correctly and carries its internal buffers across iterations intact.

## 1. The `iconv` View Boundary Conditions

POSIX `iconv` communicates state through its return value (`(size_t)-1`) and
`errno`.  There are four cases the iterator has to get right.  Simple UTF-8 to
UTF-16 or UTF-32 conversions trigger all of them.

### A. Buffer Exhaustion (`E2BIG`)

A view that processes data in chunks will see `E2BIG` when the output buffer
fills before the input is consumed.

* **Test vector:** a long valid string, say 100 repetitions of `A`.
* **Expected:** the iterator yields the converted characters from the filled
  buffer, and on the next increment resumes by passing the *unconsumed* input
  pointer back into `iconv`.  No characters dropped.

### B. The Split Multi-byte Sequence (`EINVAL`)

This is the test a range adapter is most likely to fail.  When the underlying
data source delivers bytes in chunks — a socket, a file — a multi-byte
character can be sliced in half at the chunk boundary, and `iconv` stops with
`EINVAL`.

* **Test vector:** the UTF-8 sequence for "𝄞" (U+1D11E), `0xF0 0x9D 0x84 0x9E`.
* **Procedure:** feed the view an input range that yields `0xF0 0x9D` on the
  first iteration and `0x84 0x9E` on the second.
* **Expected:** the view caches the leftover bytes, prepends them to the next
  chunk it pulls from the underlying range, and yields the code point.  No
  error.

### C. Invalid Byte Sequences (`EILSEQ`)

The view keeps going past malformed data, per the C++ API design: a replacement
character, or a `std::expected` error state.

* **Test vector:** `0xFF 0xFF`.
* **Expected:** the view sees `errno == EILSEQ` and does not infinite-loop.  It
  either advances the input pointer past the bad bytes (usually by one) and
  emits a replacement character, or halts the range and surfaces the error.

### D. End-of-Range Stateful Flush

Legacy encodings use shift states.  At the end of a sequence `iconv` needs a
final call with `inbuf == nullptr` to flush the reset bytes — switching back to
ASCII, say — into the output buffer.

* **Test vector:** ISO-2022-JP encoding of "あ", `0x1B 0x24 0x42 0x24 0x22`.
* **Expected:** when the underlying input range reaches `std::ranges::end`, the
  iterator makes the flush call.  If the flush produces bytes, the iterator
  yields them before it compares equal to the sentinel.

---

## 2. Mocking `iconv` for Deterministic View Testing

Testing *view logic* against the system `iconv` is unreliable, because system
implementations vary in how strictly they handle `EILSEQ` and in which
encodings they know about at all.

The view therefore takes its three `iconv` entry points through the
`iconv_functions` struct, so a test can substitute its own.
`tests/beman/transcode/iconv_mock.hpp` is that substitute: it exercises the API
contract without depending on the platform's tables.

```cpp
#include <cerrno>
#include <cstdint>
#include <cstddef>
#include <string_view>
#include <vector>

namespace beman::transcoding::tests {

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

} // namespace beman::transcoding::tests
```

---

## 3. Testing Execution Plan

Three steps.  Build the pipeline —
`auto transcoded = input_bytes | views::iconv_transcode("UTF-8", "UTF-16");`.
Force `EINVAL` across chunk boundaries by running the test vectors through
`std::views::chunk` before piping them into the transcode view.  Force `E2BIG`
with a tiny static buffer in the test loop, which is what shows whether the
iterator drops state between increments.
