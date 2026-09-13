// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef TESTS_BEMAN_TRANSCODE_ICONV_MOCK_HPP
#define TESTS_BEMAN_TRANSCODE_ICONV_MOCK_HPP

#include <iconv.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <cstddef>

namespace beman::transcoding::tests {

// Identity mock: copies input bytes verbatim to output (ASCII pass-through).
inline iconv_t mock_iconv_open(const char*, const char*) { return (iconv_t)1; }

// Failure mock: always returns (iconv_t)-1 from iconv_open.
// Used to test early-return paths when iconv_open fails.
inline iconv_t mock_iconv_open_fail(const char*, const char*) { return (iconv_t)-1; }

inline size_t mock_iconv(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    size_t n = std::min(*inleft, *outleft);
    std::memcpy(*out, *in, n);
    *in += n;
    *inleft -= n;
    *out += n;
    *outleft -= n;
    return 0;
}

inline int mock_iconv_close(iconv_t) { return 0; }

// Unexpected-error mock: POSIX does not list EBADF as a conversion error.
// It exercises the system_error fallback instead of an input-error path.
inline size_t mock_iconv_system_error(iconv_t, char**, size_t*, char**, size_t*) {
    errno = EBADF;
    return (size_t)-1;
}

// Writes and consumes one byte before reporting an unexpected error.
inline size_t mock_iconv_output_then_system_error(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    if (*inleft > 0 && *outleft > 0) {
        **out = **in;
        ++*in;
        --*inleft;
        ++*out;
        --*outleft;
    }
    errno = EBADF;
    return (size_t)-1;
}

// Converts input normally, then reports an unexpected error during flush.
inline size_t mock_iconv_flush_system_error(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr) {
        errno = EBADF;
        return (size_t)-1;
    }
    return mock_iconv((iconv_t)1, in, inleft, out, outleft);
}

inline size_t mock_iconv_flush_eilseq(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr) {
        errno = EILSEQ;
        return (size_t)-1;
    }
    return mock_iconv((iconv_t)1, in, inleft, out, outleft);
}

inline size_t mock_iconv_flush_einval(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr) {
        errno = EINVAL;
        return (size_t)-1;
    }
    return mock_iconv((iconv_t)1, in, inleft, out, outleft);
}

// Writes one reset byte before reporting an unexpected error during flush.
inline size_t
mock_iconv_flush_output_then_system_error(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in != nullptr && *in != nullptr)
        return mock_iconv((iconv_t)1, in, inleft, out, outleft);
    if (*outleft > 0) {
        **out = 0x0F;
        ++*out;
        --*outleft;
    }
    errno = EBADF;
    return (size_t)-1;
}

// Converts input normally, then cannot emit even one byte during flush.
inline size_t mock_iconv_flush_e2big_zero_output(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr) {
        errno = E2BIG;
        return (size_t)-1;
    }
    return mock_iconv((iconv_t)1, in, inleft, out, outleft);
}

// Stateful callable set whose flush needs two calls. The first call fills the
// output buffer and returns E2BIG; the second writes one final reset byte and
// succeeds.
struct mock_iconv_multistep_flush_fns {
    std::size_t* flush_calls;

    iconv_t open(const char*, const char*) const { return (iconv_t)1; }

    size_t convert(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) const {
        if (in != nullptr && *in != nullptr) {
            size_t n = std::min(*inleft, *outleft);
            std::memcpy(*out, *in, n);
            *in += n;
            *inleft -= n;
            *out += n;
            *outleft -= n;
            return 0;
        }

        ++*flush_calls;
        if (*flush_calls == 1) {
            while (*outleft > 0) {
                **out = 0x0E;
                ++*out;
                --*outleft;
            }
            errno = E2BIG;
            return (size_t)-1;
        }

        if (*flush_calls == 2 && *outleft > 0) {
            **out = 0x0F;
            ++*out;
            --*outleft;
        }
        return 0;
    }

    int close(iconv_t) const { return 0; }
};

// Pairwise mock: needs exactly 2 input bytes to produce 2 output bytes.
// Returns EINVAL if given fewer than 2 bytes (simulates an incomplete multibyte
// sequence), forcing the iterator to accumulate bytes before converting.
inline size_t mock_iconv_pairwise(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    if (*inleft < 2) {
        errno = EINVAL;
        return (size_t)-1;
    }
    size_t pairs = std::min(*inleft / 2, *outleft / 2);
    if (pairs == 0) {
        errno = E2BIG;
        return (size_t)-1;
    }
    size_t n = pairs * 2;
    std::memcpy(*out, *in, n);
    *in += n;
    *inleft -= n;
    *out += n;
    *outleft -= n;
    return 0;
}

// E2BIG mock: identity conversion, but always returns E2BIG after writing 1 byte.
// Used to verify that the iterator continues yielding output despite repeated E2BIG.
inline size_t mock_iconv_e2big(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    if (*inleft == 0)
        return 0;
    if (*outleft == 0) {
        errno = E2BIG;
        return (size_t)-1;
    }
    **out = **in;
    ++*in;
    --*inleft;
    ++*out;
    --*outleft;
    errno = E2BIG;
    return (size_t)-1;
}

// EILSEQ mock: always signals an invalid byte sequence (0 bytes written, 0 consumed).
inline size_t mock_iconv_eilseq(iconv_t, char** in, size_t*, char**, size_t*) {
    if (in == nullptr || *in == nullptr)
        return 0;
    errno = EILSEQ;
    return (size_t)-1;
}

// Partial-consume mock: consumes 1 byte and writes 1 byte, then returns EINVAL.
// Simulates a codec where partial progress is made before needing more input.
// Triggers the staging-shift loop (consumed > 0 with inleft > 0).
inline size_t mock_iconv_partial_consume(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    if (*inleft < 2) {
        errno = EINVAL;
        return (size_t)-1;
    }
    if (*outleft == 0) {
        errno = E2BIG;
        return (size_t)-1;
    }
    **out = **in;
    ++*in;
    --*inleft;
    ++*out;
    --*outleft;
    errno = EINVAL;
    return (size_t)-1;
}

// E2BIG-zero-output mock: always returns E2BIG without writing any output.
// Simulates an output buffer too small to hold even one conversion unit.
inline size_t mock_iconv_e2big_zero_output(iconv_t, char**, size_t*, char**, size_t*) {
    errno = E2BIG;
    return (size_t)-1;
}

// Stateful mock: identity conversion, but on flush (inbuf=nullptr) writes a
// reset byte (0x0F) to simulate a stateful encoding's shift-in sequence.
inline size_t mock_iconv_stateful(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr) {
        if (*outleft < 1) {
            errno = E2BIG;
            return (size_t)-1;
        }
        **out = 0x0F;
        ++*out;
        --*outleft;
        return 0;
    }
    size_t n = std::min(*inleft, *outleft);
    std::memcpy(*out, *in, n);
    *in += n;
    *inleft -= n;
    *out += n;
    *outleft -= n;
    return 0;
}

// Converts an initial 'A', then leaves the following byte unconsumed and
// reports EILSEQ. Subsequent bytes are invalid and produce no output.
inline size_t mock_iconv_output_then_eilseq(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    if (*inleft == 0)
        return 0;
    if (**in == 'A' && *outleft > 0) {
        **out = **in;
        ++*out;
        --*outleft;
        ++*in;
        --*inleft;
    }
    errno = EILSEQ;
    return (size_t)-1;
}

// Output-then-E2BIG mock: writes 1 byte then returns E2BIG.
// Always returns E2BIG; this tests the "output before error" path.
inline size_t mock_iconv_output_then_e2big(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    if (*inleft == 0)
        return 0;
    // Write one byte of output
    if (*outleft > 0) {
        **out = **in;
        ++*out;
        --*outleft;
        ++*in;
        --*inleft;
    }
    errno = E2BIG;
    return (size_t)-1;
}

// No-output-EILSEQ-multi-staging mock: consumes multiple staging bytes
// on EILSEQ with no output, forcing the shift loop.
// Needs 3+ staging bytes to trigger the loop iteration.
inline size_t mock_iconv_eilseq_multi_byte(iconv_t,
                                           char**                   in,
                                           [[maybe_unused]] size_t* inleft,
                                           [[maybe_unused]] char**  out,
                                           [[maybe_unused]] size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    // If we have 3+ bytes in staging, signal EILSEQ with no output
    // (the iterator's caller staging will have 3+ bytes)
    errno = EILSEQ;
    return (size_t)-1;
}

// Success-no-output mock: returns success while consuming input but producing no output.
// Simulates a codec that buffers input without producing output immediately.
// Tests the "success with nothing yielded" path (lines 188-190 in transcode, 178-180 in or_error).
inline size_t mock_iconv_success_no_output(
    iconv_t, char** in, size_t* inleft, [[maybe_unused]] char** out, [[maybe_unused]] size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    // Consume all input without producing any output (success)
    *in += *inleft;
    *inleft = 0;
    return 0;
}

// Shift-loop-E2BIG mock: returns E2BIG without output for multi-byte staging.
// Forces the byte-shifting loop for E2BIG error handling (line 222 in or_error, 215 in transcode).
inline size_t mock_iconv_shift_loop_e2big(iconv_t,
                                          char**                   in,
                                          [[maybe_unused]] size_t* inleft,
                                          [[maybe_unused]] char**  out,
                                          [[maybe_unused]] size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    // Always return E2BIG with no output/consumption (forces shift loop)
    errno = E2BIG;
    return (size_t)-1;
}

// Shift-loop-EILSEQ mock: returns EILSEQ without output for multi-byte staging.
// Forces the byte-shifting loop for EILSEQ error handling (line 207 in or_error).
inline size_t mock_iconv_shift_loop_eilseq(iconv_t,
                                           char**                   in,
                                           [[maybe_unused]] size_t* inleft,
                                           [[maybe_unused]] char**  out,
                                           [[maybe_unused]] size_t* outleft) {
    if (in == nullptr || *in == nullptr)
        return 0;
    // Always return EILSEQ with no output/consumption (forces shift loop)
    errno = EILSEQ;
    return (size_t)-1;
}

} // namespace beman::transcoding::tests

#endif // TESTS_BEMAN_TRANSCODE_ICONV_MOCK_HPP
