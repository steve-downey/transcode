// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// NEGATIVE COMPILE TEST: sniff_encoding requires a forward_range.
// This file must NOT compile — it is registered as a WILL_FAIL ctest target.
//
// sniff_encoding reads up to three bytes and advances the iterator to do it.
// On a single-pass range those bytes are gone, and the caller gets a range
// that is missing the very prefix it just asked about — silently, since
// nothing in the signature says the argument was consumed.  So a range that
// cannot be restarted is rejected, and the docblock's promise that "the range
// can be passed on unchanged" is one the constraint keeps.
//
// The range below is a legacy_byte_range: it is an input_range over char.
// What it is not is a forward_range, and it says so the way this library's own
// single-pass iterators do — `iterator_concept = input_iterator_tag`, as in
// iconv_transcode_view::iterator.  Declaring the category rather than
// arranging for some requirement to fail incidentally keeps the diagnostic
// about the thing under test.

#include <beman/transcode/sniff.hpp>

#include <cstddef>
#include <iterator>
#include <ranges>

namespace {

struct single_pass_bytes {
    struct iterator {
        using iterator_concept = std::input_iterator_tag;
        using value_type       = char;
        using difference_type  = std::ptrdiff_t;

        const char* p = nullptr;

        char      operator*() const { return *p; }
        iterator& operator++() {
            ++p;
            return *this;
        }
        void operator++(int) { ++p; }

        bool operator==(const iterator&) const = default;
        bool operator==(std::default_sentinel_t) const { return *p == '\0'; }
    };

    const char* data = "";

    iterator                begin() const { return iterator{data}; }
    std::default_sentinel_t end() const { return std::default_sentinel; }
};

static_assert(std::ranges::input_range<single_pass_bytes>);
static_assert(!std::ranges::forward_range<single_pass_bytes>);

} // namespace

void test() {
    single_pass_bytes r{"\xEF\xBB\xBF"
                        "hi"};
    // Single-pass: sniffing would eat the mark — must fail to compile.
    auto result = beman::transcoding::sniff_encoding(r);
    (void)result;
}
