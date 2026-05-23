// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TRANSCODE_TESTS_ICONV_MOCK_HPP
#define BEMAN_TRANSCODE_TESTS_ICONV_MOCK_HPP

#include <iconv.h>

#include <algorithm>
#include <cstring>

namespace beman::transcoding::tests {

// Identity mock: copies input bytes verbatim to output (ASCII pass-through).
inline iconv_t mock_iconv_open(const char*, const char*) {
    return (iconv_t)1;
}

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

} // namespace beman::transcoding::tests

#endif // BEMAN_TRANSCODE_TESTS_ICONV_MOCK_HPP
