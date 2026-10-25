// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: a reverse_index stores pointer + 1 in 16 bits, so a
// table of 0xFFFF or more entries must be rejected at compile time rather
// than silently truncating the pointers it hands back.
// This file must NOT compile — it is registered as a WILL_FAIL ctest target.

#include <beman/transcode/detail/reverse_index.hpp>

namespace beman::transcoding::tests {

inline constexpr char32_t wide[0xFFFF] = {U'\x3000'};

} // namespace beman::transcoding::tests

const auto& index = beman::transcoding::detail::reverse_index_for<beman::transcoding::tests::wide>;
