// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_ICONV_BULK_HPP
#define INCLUDE_BEMAN_TRANSCODE_ICONV_BULK_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/concepts.hpp>
#include <beman/transcode/error.hpp>
#include <beman/transcode/iconv_real.hpp>
#include <beman/transcode/iconv_transcode_view.hpp>

#include <iconv.h>
#include <cerrno>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <algorithm>
    #include <array>
    #include <expected>
    #include <iterator>
    #include <ranges>
    #include <string>
    #include <utility>
    #include <vector>

#endif
namespace beman::transcoding {

namespace detail {

//! \omit
template <typename IconvFns>
struct iconv_guard {
    iconv_t  handle;
    IconvFns fns;
    ~iconv_guard() {
        if (handle != (iconv_t)-1)
            fns.close(handle);
    }
};

//! \omit
struct iconv_input_buf {
    std::vector<char> storage;
    char*             data;
    size_t            size;
};

//! \omit
template <legacy_byte_range R>
iconv_input_buf materialize_iconv_input(R&& source) {
    using range_t = std::remove_cvref_t<R>;
    if constexpr (std::ranges::contiguous_range<range_t> && std::ranges::sized_range<range_t>) {
        auto  sz = static_cast<size_t>(std::ranges::size(source));
        auto* p  = const_cast<char*>(reinterpret_cast<const char*>(std::ranges::data(source)));
        return {{}, p, sz};
    } else {
        iconv_input_buf result;
        if constexpr (std::ranges::sized_range<range_t>)
            result.storage.reserve(static_cast<size_t>(std::ranges::size(source)));
        for (auto b : source)
            result.storage.push_back(static_cast<char>(b));
        result.data = result.storage.data();
        result.size = result.storage.size();
        return result;
    }
}

} // namespace detail

// iconv_transcode_to<Container>(source, from, to, fns)
//
// Transcodes source bytes from encoding `from` to encoding `to` using the
// iconv-compatible callables in `fns`. Invalid sequences are skipped.
// Stateful encodings are flushed after all input is consumed.
// IconvFns enables dependency injection for testing (see iconv_mock.hpp).
// \ref{transcode.iconv}, eager conversion

//! \returns A `Container` holding the bytes of `source` converted from `from`
//! to `to` by `fns`.  Input the conversion does not accept is skipped, as it
//! is by `iconv_transcode_view` \iref{transcode.iconv}.  The result is empty
//! when the conversion descriptor cannot be opened -- which is what
//! `iconv_open` failing means, and is not distinguishable here from an empty
//! input.  An unexpected system failure ends conversion and returns the bytes
//! produced before the failure.  Use `iconv_transcode_to_or_error` when either
//! distinction matters.
template <typename Container = std::string, typename IconvFns, legacy_byte_range R>
Container iconv_transcode_to(R&& source, const char* from, const char* to, IconvFns fns) {
    auto input = detail::materialize_iconv_input(std::forward<R>(source));

    detail::iconv_guard<IconvFns> guard{fns.open(to, from), fns};
    if (guard.handle == (iconv_t)-1)
        return Container{};

    size_t buf_size = input.size * 4;
    buf_size        = std::max<size_t>(buf_size, 256);
    std::vector<char> out_buf(buf_size);

    char*  inp      = input.data;
    size_t inp_left = input.size;
    char*  out      = out_buf.data();
    size_t out_left = out_buf.size();

    while (inp_left > 0) {
        size_t rc = fns.convert(guard.handle, &inp, &inp_left, &out, &out_left);
        if (rc == iconv_error_rc) {
            if (errno == E2BIG) {
                auto used = static_cast<size_t>(out - out_buf.data());
                buf_size *= 2;
                out_buf.resize(buf_size);
                out      = out_buf.data() + used;
                out_left = buf_size - used;
            } else if (errno == EILSEQ) {
                if (inp_left > 0) {
                    ++inp;
                    --inp_left;
                }
            } else if (errno == EINVAL) {
                // Skip remaining incomplete bytes.
                inp += inp_left;
                inp_left = 0;
            } else {
                auto out_used = static_cast<size_t>(out - out_buf.data());
                return Container(out_buf.data(), out_buf.data() + out_used);
            }
        }
    }

    // Flush stateful encodings
    while (true) {
        size_t rc = fns.convert(guard.handle, nullptr, nullptr, &out, &out_left);
        if (rc != iconv_error_rc)
            break;
        if (errno == E2BIG) {
            auto used = static_cast<size_t>(out - out_buf.data());
            buf_size *= 2;
            out_buf.resize(buf_size);
            out      = out_buf.data() + used;
            out_left = buf_size - used;
        } else {
            break;
        }
    }

    auto out_used = static_cast<size_t>(out - out_buf.data());
    return Container(out_buf.data(), out_buf.data() + out_used);
}

// iconv_transcode_to<Container>(source, from, to)
// Overload using real iconv (no dependency injection).
//! \returns-equiv
template <typename Container = std::string, legacy_byte_range R>
Container iconv_transcode_to(R&& source, const char* from, const char* to) {
    return iconv_transcode_to<Container>(std::forward<R>(source), from, to, make_real_iconv_fns());
}

// iconv_transcode_into(source, from, to, output, fns)
//
// Transcodes source bytes from `from` to `to` and writes each output char to
// the output iterator. Invalid sequences are skipped. Uses a fixed
// temporary buffer internally. Returns the advanced output iterator.
//! \effects Converts the bytes of `source` from `from` to `to` by `fns` and
//! writes them through `output`.
//! \returns The value of `output` after the last byte written.
//! \remarks If the conversion descriptor cannot be opened, no bytes are
//! written and the unchanged iterator is returned.  This is not
//! distinguishable from empty input; use `iconv_transcode_to_or_error` when
//! the distinction matters.  An unexpected system failure similarly returns
//! the iterator after the last byte successfully written.
template <typename IconvFns, legacy_byte_range R, std::output_iterator<char> Output>
Output iconv_transcode_into(R&& source, const char* from, const char* to, Output output, IconvFns fns) {
    auto input = detail::materialize_iconv_input(std::forward<R>(source));

    detail::iconv_guard<IconvFns> guard{fns.open(to, from), fns};
    if (guard.handle == (iconv_t)-1)
        return output;

    char*                  inp      = input.data;
    size_t                 inp_left = input.size;
    std::array<char, 4096> tmp{};

    while (inp_left > 0) {
        char*  out      = tmp.data();
        size_t out_left = tmp.size();
        size_t rc       = fns.convert(guard.handle, &inp, &inp_left, &out, &out_left);

        size_t written = tmp.size() - out_left;
        output         = std::copy_n(tmp.data(), written, output);

        if (rc == iconv_error_rc) {
            if (errno == E2BIG) {
                // Temp buffer filled; loop refreshes it on the next iteration
            } else if (errno == EILSEQ) {
                if (inp_left > 0) {
                    ++inp;
                    --inp_left;
                }
            } else if (errno == EINVAL) {
                // Skip remaining incomplete bytes.
                inp += inp_left;
                inp_left = 0;
            } else {
                return output;
            }
        }
    }

    // Flush stateful encodings
    while (true) {
        char*  out      = tmp.data();
        size_t out_left = tmp.size();
        size_t rc       = fns.convert(guard.handle, nullptr, nullptr, &out, &out_left);
        size_t written  = tmp.size() - out_left;
        output          = std::copy_n(tmp.data(), written, output);
        if (rc != iconv_error_rc || errno != E2BIG)
            break;
    }

    return output;
}

// iconv_transcode_into(source, from, to, output)
// Overload using real iconv.
//! \returns-equiv
template <legacy_byte_range R, std::output_iterator<char> Output>
Output iconv_transcode_into(R&& source, const char* from, const char* to, Output output) {
    return iconv_transcode_into(std::forward<R>(source), from, to, std::move(output), make_real_iconv_fns());
}

// iconv_transcode_to_or_error<Container>(source, from, to, fns)
//
// Like iconv_transcode_to but returns std::unexpected on the first invalid or
// incomplete sequence instead of skipping it.
//! \returns A `Container` holding the converted bytes, or the first
//! `iconv_error` the conversion reported.
//! \remarks This is the eager form of `iconv_transcode_or_error`
//! \iref{transcode.iconv}: it stops at the first failure rather than skipping
//! it, which is the difference between a conversion a program wants to know
//! about and one it wants to get through.
template <typename Container = std::string, typename IconvFns, legacy_byte_range R>
std::expected<Container, iconv_error>
iconv_transcode_to_or_error(R&& source, const char* from, const char* to, IconvFns fns) {
    auto input = detail::materialize_iconv_input(std::forward<R>(source));

    detail::iconv_guard<IconvFns> guard{fns.open(to, from), fns};
    if (guard.handle == (iconv_t)-1)
        return std::unexpected(iconv_error::open_failed);

    size_t buf_size = input.size * 4;
    buf_size        = std::max<size_t>(buf_size, 256);
    std::vector<char> out_buf(buf_size);

    char*  inp      = input.data;
    size_t inp_left = input.size;
    char*  out      = out_buf.data();
    size_t out_left = out_buf.size();

    while (inp_left > 0) {
        size_t rc = fns.convert(guard.handle, &inp, &inp_left, &out, &out_left);
        if (rc == iconv_error_rc) {
            if (errno == E2BIG) {
                auto used = static_cast<size_t>(out - out_buf.data());
                buf_size *= 2;
                out_buf.resize(buf_size);
                out      = out_buf.data() + used;
                out_left = buf_size - used;
            } else if (errno == EILSEQ) {
                return std::unexpected(iconv_error::invalid_sequence);
            } else if (errno == EINVAL) {
                return std::unexpected(iconv_error::incomplete_sequence);
            } else {
                return std::unexpected(iconv_error::system_error);
            }
        }
    }

    // Flush stateful encodings
    while (true) {
        size_t rc = fns.convert(guard.handle, nullptr, nullptr, &out, &out_left);
        if (rc != iconv_error_rc)
            break;
        if (errno == E2BIG) {
            auto used = static_cast<size_t>(out - out_buf.data());
            buf_size *= 2;
            out_buf.resize(buf_size);
            out      = out_buf.data() + used;
            out_left = buf_size - used;
        } else if (errno == EILSEQ) {
            return std::unexpected(iconv_error::invalid_sequence);
        } else if (errno == EINVAL) {
            return std::unexpected(iconv_error::incomplete_sequence);
        } else {
            return std::unexpected(iconv_error::system_error);
        }
    }

    auto out_used = static_cast<size_t>(out - out_buf.data());
    return Container(out_buf.data(), out_buf.data() + out_used);
}

// iconv_transcode_to_or_error<Container>(source, from, to)
// Overload using real iconv.
//! \returns-equiv
template <typename Container = std::string, legacy_byte_range R>
std::expected<Container, iconv_error> iconv_transcode_to_or_error(R&& source, const char* from, const char* to) {
    return iconv_transcode_to_or_error<Container>(std::forward<R>(source), from, to, make_real_iconv_fns());
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_ICONV_BULK_HPP
