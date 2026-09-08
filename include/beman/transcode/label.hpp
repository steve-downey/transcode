// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_LABEL_HPP
#define INCLUDE_BEMAN_TRANSCODE_LABEL_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/codec.hpp>
#include <beman/transcode/detail/labels.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <algorithm>
    #include <optional>
    #include <string_view>

#endif
namespace beman::transcoding {

// \ref{transcode.codec.label}, label lookup

constexpr std::optional<codec> get_encoding(std::string_view label) noexcept;

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

//! \returns The `codec` whose WHATWG label set contains `label`, and
//! `nullopt` if no encoding has that label.  Leading and trailing ASCII
//! whitespace is ignored and the comparison is ASCII case-insensitive, which
//! is the Encoding Standard's "get an encoding" algorithm.
//! \remarks An encoding's labels are the alternative names a document may
//! spell it with -- `latin1`, `iso-8859-1` and `windows-1252` are all labels
//! of `codec::windows_1252` -- so a lookup that fails is a name no encoding
//! claims, not an encoding that cannot be represented.
constexpr std::optional<codec> get_encoding(std::string_view label) noexcept {
    // Strip ASCII whitespace from both ends (\t \n \f \r \x20).
    auto is_ws = [](char c) noexcept { return c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r'; };
    while (!label.empty() && is_ws(label.front()))
        label.remove_prefix(1);
    while (!label.empty() && is_ws(label.back()))
        label.remove_suffix(1);

    if (label.empty())
        return std::nullopt;

    // ASCII-lowercase the label into a fixed-size buffer (longest label < 64).
    char buf[64];
    if (label.size() >= sizeof(buf))
        return std::nullopt;
    for (std::size_t i = 0; i < label.size(); ++i) {
        char c = label[i];
        buf[i] = (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
    }
    std::string_view lower{buf, label.size()};

    // Binary search the sorted table.
    const auto* it = std::ranges::lower_bound(
        detail::label_table, lower, {}, [](const detail::label_entry& e) { return std::string_view{e.label}; });
    if (it != std::end(detail::label_table) && std::string_view{it->label} == lower)
        return it->value;
    return std::nullopt;
}

} // namespace beman::transcoding

#endif // INCLUDE_BEMAN_TRANSCODE_LABEL_HPP
