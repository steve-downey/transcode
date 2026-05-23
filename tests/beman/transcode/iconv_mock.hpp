// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef TESTS_BEMAN_TRANSCODE_ICONV_MOCK_HPP
#define TESTS_BEMAN_TRANSCODE_ICONV_MOCK_HPP

#include <iconv.h>

#include <algorithm>
#include <cerrno>
#include <cstring>

namespace beman::transcoding::tests {

// Identity mock: copies input bytes verbatim to output (ASCII pass-through).
inline iconv_t mock_iconv_open(const char*, const char*) { return (iconv_t)1; }

inline size_t mock_iconv(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
    size_t n = std::min(*inleft, *outleft);
    std::memcpy(*out, *in, n);
    *in += n;
    *inleft -= n;
    *out += n;
    *outleft -= n;
    return 0;
}

inline int mock_iconv_close(iconv_t) { return 0; }

// Pairwise mock: needs exactly 2 input bytes to produce 2 output bytes.
// Returns EINVAL if given fewer than 2 bytes (simulates an incomplete multibyte
// sequence), forcing the iterator to accumulate bytes before converting.
inline size_t mock_iconv_pairwise(iconv_t, char** in, size_t* inleft, char** out, size_t* outleft) {
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
inline size_t mock_iconv_eilseq(iconv_t, char**, size_t*, char**, size_t*) {
    errno = EILSEQ;
    return (size_t)-1;
}

} // namespace beman::transcoding::tests

#endif // TESTS_BEMAN_TRANSCODE_ICONV_MOCK_HPP
