// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef TESTS_BEMAN_TRANSCODE_TEST_UTILITIES_HPP
#define TESTS_BEMAN_TRANSCODE_TEST_UTILITIES_HPP

namespace beman::transcoding::tests {

// Evaluate and return an expression in a consteval context, verifying that
// the expression is usable in constant evaluation (i.e. truly constexpr).
auto consteval constify(auto expr) { return expr; }

} // namespace beman::transcoding::tests

#endif // TESTS_BEMAN_TRANSCODE_TEST_UTILITIES_HPP
