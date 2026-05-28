// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ICONV_BULK_HPP
#define INCLUDE_BEMAN_TRANSCODE_ICONV_BULK_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/detail/concepts.hpp>
#include <beman/transcode/detail/error.hpp>
#include <beman/transcode/iconv_real.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>

#include <iconv.h>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <array>
    #include <cerrno>
    #include <expected>
    #include <iterator>
    #include <ranges>
    #include <string>
    #include <vector>

#endif
namespace beman::transcoding {

namespace detail {

// RAII handle for iconv_t — calls fns.close() on destruction.
template <typename IconvFns>
struct iconv_guard {
    iconv_t  handle;
    IconvFns fns;
    ~iconv_guard() {
        if (handle != (iconv_t)-1)
            fns.close(handle);
    }
};

} // namespace detail

// iconv_transcode_to<Container>(source, from, to, fns)
//
// Transcodes source bytes from encoding `from` to encoding `to` using the
// iconv-compatible callables in `fns`. Invalid sequences are replaced with '?'.
// Stateful encodings are flushed after all input is consumed.
// IconvFns enables dependency injection for testing (see iconv_mock.hpp).
template <typename Container = std::string, typename IconvFns, legacy_byte_range R>
Container iconv_transcode_to(R&& source, const char* from, const char* to, IconvFns fns) {
    std::vector<char> input_buf;
    for (auto b : source)
        input_buf.push_back(static_cast<char>(b));

    detail::iconv_guard<IconvFns> guard{fns.open(to, from), fns};
    if (guard.handle == (iconv_t)-1)
        return Container{};

    size_t            buf_size = input_buf.size() * 4;
    if (buf_size < 256)
        buf_size = 256;
    std::vector<char> out_buf(buf_size);

    char*  inp      = input_buf.data();
    size_t inp_left = input_buf.size();
    char*  out      = out_buf.data();
    size_t out_left = out_buf.size();

    while (inp_left > 0) {
        size_t rc = fns.convert(guard.handle, &inp, &inp_left, &out, &out_left);
        if (rc == (size_t)-1) {
            if (errno == E2BIG) {
                size_t used = static_cast<size_t>(out - out_buf.data());
                buf_size *= 2;
                out_buf.resize(buf_size);
                out      = out_buf.data() + used;
                out_left = buf_size - used;
            } else if (errno == EILSEQ) {
                if (inp_left > 0) {
                    ++inp;
                    --inp_left;
                }
                if (out_left == 0) {
                    size_t used = static_cast<size_t>(out - out_buf.data());
                    buf_size *= 2;
                    out_buf.resize(buf_size);
                    out      = out_buf.data() + used;
                    out_left = buf_size - used;
                }
                *out++ = '?';
                --out_left;
            } else {
                // EINVAL: skip remaining incomplete bytes, insert replacement
                inp += inp_left;
                inp_left = 0;
                if (out_left == 0) {
                    size_t used = static_cast<size_t>(out - out_buf.data());
                    buf_size *= 2;
                    out_buf.resize(buf_size);
                    out      = out_buf.data() + used;
                    out_left = buf_size - used;
                }
                *out++ = '?';
                --out_left;
            }
        }
    }

    // Flush stateful encodings
    while (true) {
        size_t rc = fns.convert(guard.handle, nullptr, nullptr, &out, &out_left);
        if (rc != (size_t)-1)
            break;
        if (errno == E2BIG) {
            size_t used = static_cast<size_t>(out - out_buf.data());
            buf_size *= 2;
            out_buf.resize(buf_size);
            out      = out_buf.data() + used;
            out_left = buf_size - used;
        } else {
            break;
        }
    }

    size_t    out_used = static_cast<size_t>(out - out_buf.data());
    Container result;
    for (size_t i = 0; i < out_used; ++i)
        result.push_back(out_buf[i]);
    return result;
}

// iconv_transcode_to<Container>(source, from, to)
// Overload using real iconv (no dependency injection).
template <typename Container = std::string, legacy_byte_range R>
Container iconv_transcode_to(R&& source, const char* from, const char* to) {
    return iconv_transcode_to<Container>(std::forward<R>(source), from, to, make_real_iconv_fns());
}

// iconv_transcode_into(source, from, to, output, fns)
//
// Transcodes source bytes from `from` to `to` and writes each output char to
// the output iterator. Invalid sequences are replaced with '?'. Uses a fixed
// temporary buffer internally. Returns the advanced output iterator.
template <typename IconvFns, legacy_byte_range R, std::output_iterator<char> Output>
Output iconv_transcode_into(R&& source, const char* from, const char* to, Output output, IconvFns fns) {
    std::vector<char> input_buf;
    for (auto b : source)
        input_buf.push_back(static_cast<char>(b));

    detail::iconv_guard<IconvFns> guard{fns.open(to, from), fns};
    if (guard.handle == (iconv_t)-1)
        return output;

    char*                  inp      = input_buf.data();
    size_t                 inp_left = input_buf.size();
    std::array<char, 4096> tmp{};

    while (inp_left > 0) {
        char*  out      = tmp.data();
        size_t out_left = tmp.size();
        size_t rc = fns.convert(guard.handle, &inp, &inp_left, &out, &out_left);

        size_t written = tmp.size() - out_left;
        for (size_t i = 0; i < written; ++i)
            *output++ = tmp[i];

        if (rc == (size_t)-1) {
            if (errno == E2BIG) {
                // Temp buffer filled; loop refreshes it on the next iteration
            } else if (errno == EILSEQ) {
                if (inp_left > 0) {
                    ++inp;
                    --inp_left;
                }
                *output++ = '?';
            } else {
                // EINVAL: skip remaining incomplete bytes, insert replacement
                inp += inp_left;
                inp_left = 0;
                *output++ = '?';
            }
        }
    }

    // Flush stateful encodings
    while (true) {
        char*  out      = tmp.data();
        size_t out_left = tmp.size();
        size_t rc = fns.convert(guard.handle, nullptr, nullptr, &out, &out_left);
        size_t written = tmp.size() - out_left;
        for (size_t i = 0; i < written; ++i)
            *output++ = tmp[i];
        if (rc != (size_t)-1 || errno != E2BIG)
            break;
    }

    return output;
}

// iconv_transcode_into(source, from, to, output)
// Overload using real iconv.
template <legacy_byte_range R, std::output_iterator<char> Output>
Output iconv_transcode_into(R&& source, const char* from, const char* to, Output output) {
    return iconv_transcode_into(std::forward<R>(source), from, to, std::move(output), make_real_iconv_fns());
}

// iconv_transcode_to_or_error<Container>(source, from, to, fns)
//
// Like iconv_transcode_to but returns std::unexpected on the first invalid or
// incomplete sequence instead of inserting a replacement character.
template <typename Container = std::string, typename IconvFns, legacy_byte_range R>
std::expected<Container, iconv_error>
iconv_transcode_to_or_error(R&& source, const char* from, const char* to, IconvFns fns) {
    std::vector<char> input_buf;
    for (auto b : source)
        input_buf.push_back(static_cast<char>(b));

    detail::iconv_guard<IconvFns> guard{fns.open(to, from), fns};
    if (guard.handle == (iconv_t)-1)
        return std::unexpected(iconv_error::invalid_sequence);

    size_t            buf_size = input_buf.size() * 4;
    if (buf_size < 256)
        buf_size = 256;
    std::vector<char> out_buf(buf_size);

    char*  inp      = input_buf.data();
    size_t inp_left = input_buf.size();
    char*  out      = out_buf.data();
    size_t out_left = out_buf.size();

    while (inp_left > 0) {
        size_t rc = fns.convert(guard.handle, &inp, &inp_left, &out, &out_left);
        if (rc == (size_t)-1) {
            if (errno == E2BIG) {
                size_t used = static_cast<size_t>(out - out_buf.data());
                buf_size *= 2;
                out_buf.resize(buf_size);
                out      = out_buf.data() + used;
                out_left = buf_size - used;
            } else if (errno == EILSEQ) {
                return std::unexpected(iconv_error::invalid_sequence);
            } else {
                // EINVAL: incomplete sequence
                return std::unexpected(iconv_error::incomplete_sequence);
            }
        }
    }

    // Flush stateful encodings
    while (true) {
        size_t rc = fns.convert(guard.handle, nullptr, nullptr, &out, &out_left);
        if (rc != (size_t)-1)
            break;
        if (errno == E2BIG) {
            size_t used = static_cast<size_t>(out - out_buf.data());
            buf_size *= 2;
            out_buf.resize(buf_size);
            out      = out_buf.data() + used;
            out_left = buf_size - used;
        } else {
            break;
        }
    }

    size_t    out_used = static_cast<size_t>(out - out_buf.data());
    Container result;
    for (size_t i = 0; i < out_used; ++i)
        result.push_back(out_buf[i]);
    return result;
}

// iconv_transcode_to_or_error<Container>(source, from, to)
// Overload using real iconv.
template <typename Container = std::string, legacy_byte_range R>
std::expected<Container, iconv_error>
iconv_transcode_to_or_error(R&& source, const char* from, const char* to) {
    return iconv_transcode_to_or_error<Container>(
        std::forward<R>(source), from, to, make_real_iconv_fns());
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_ICONV_BULK_HPP
