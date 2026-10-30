// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_WHATWG_TEXT_ENCODING_HPP
#define INCLUDE_BEMAN_TRANSCODE_WHATWG_TEXT_ENCODING_HPP

#include <beman/transcode/config.hpp>

#include <beman/transcode/whatwg/encoding.hpp>
#include <beman/transcode/whatwg/label.hpp>
#include <beman/transcode/detail/labels.hpp>

#if !BEMAN_TRANSCODE_USE_MODULES()
    #include <optional>
    #include <string_view>
    #include <version>

#endif
#ifdef __cpp_lib_text_encoding
    #if !BEMAN_TRANSCODE_USE_MODULES()
        #include <text_encoding>

    #endif
namespace beman::transcoding::whatwg {

// \ref{transcode.whatwg.label}, label lookup

constexpr std::optional<encoding> get_encoding(std::text_encoding::id id) noexcept;

constexpr std::optional<std::text_encoding::id> to_text_encoding_id(encoding e) noexcept;

// ---------------------------------------------------------------------------
// Out-of-line definitions
// ---------------------------------------------------------------------------

//! \returns `get_encoding(label)` \iref{transcode.whatwg.label} for the first of
//! the name and the aliases of `std::text_encoding(id)` that is a label of an
//! encoding, and `nullopt` if there is none, which includes `id::other` and
//! `id::unknown`.
//! \remarks This is the Encoding Standard's "get an encoding" applied to the
//! registered names, so `std::text_encoding::id::ISOLatin1` is
//! `encoding::windows_1252`.
constexpr std::optional<encoding> get_encoding(std::text_encoding::id id) noexcept {
    if (id == std::text_encoding::id::other || id == std::text_encoding::id::unknown)
        return std::nullopt;
    const std::text_encoding registered{id};
    if (auto r = get_encoding(std::string_view{registered.name()}))
        return r;
    for (const char* alias : registered.aliases()) {
        if (auto r = get_encoding(std::string_view{alias}))
            return r;
    }
    return std::nullopt;
}

//! \returns The `id` of the registered encoding whose name is the name of `e`
//! in the Encoding Standard, and `nullopt` if that name has no registry entry,
//! which includes `encoding::replacement`, `encoding::x_user_defined` and
//! `encoding::x_mac_cyrillic`.
//! \remarks IANA registers the Encoding Standard's names, so
//! `encoding::windows_1252` is `std::text_encoding::id::windows1252` and not
//! one of its aliases.
constexpr std::optional<std::text_encoding::id> to_text_encoding_id(encoding e) noexcept {
    if (e == encoding::replacement || e == encoding::x_user_defined)
        return std::nullopt;
    for (const auto& entry : detail::name_table) {
        if (entry.value != e)
            continue;
        const std::text_encoding registered{std::string_view{entry.name}};
        if (registered.mib() != std::text_encoding::id::other && registered.mib() != std::text_encoding::id::unknown)
            return registered.mib();
        return std::nullopt;
    }
    return std::nullopt;
}

} // namespace beman::transcoding::whatwg

#endif

#endif // INCLUDE_BEMAN_TRANSCODE_WHATWG_TEXT_ENCODING_HPP
