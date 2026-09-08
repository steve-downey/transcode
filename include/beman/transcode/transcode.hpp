// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP
#define INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP

#include <beman/transcode/config.hpp>

#if BEMAN_TRANSCODE_USE_MODULES() && !defined(BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT)

import beman.transcode;

#else

    // `<null_term>` is a proposed header of its own, and its own specgen
    // document.  It is included here because `<transcode>` is the header a
    // program includes to get the library, and outside the region below
    // because its declarations belong to `[null.term.syn]`, not to this
    // header's synopsis.
    #include <beman/transcode/null_term.hpp>

    // The header synopsis is the files included between here and the fence
    // (see docs/plans/phase5-index.md, D1): specgen follows them, and their
    // declarations are this header's, in this order.  The order is the one
    // the compiler needs -- a header's dependencies precede it -- and it is
    // also the order the synopsis reads in.  The clause each declaration is
    // described in is decided by the `\ref` group headers those files carry,
    // not by their order here, so the two are free to differ.
    //
    // An include *outside* the region is implementation.  That is the whole
    // of the rule: nothing under `detail/` needs a marker saying it is not
    // specification, and nothing here needs one saying it is.

    // \rSec2[transcode.syn]{Header `<transcode>` synopsis}

    #include <beman/transcode/error.hpp>
    #include <beman/transcode/concepts.hpp>
    #include <beman/transcode/codec.hpp>
    #include <beman/transcode/label.hpp>
    #include <beman/transcode/codec_result.hpp>
    #include <beman/transcode/codec_concepts.hpp>
    #include <beman/transcode/whatwg_decode_view.hpp>
    #include <beman/transcode/whatwg_encode_view.hpp>
    #include <beman/transcode/decode_view.hpp>
    #include <beman/transcode/encode_view.hpp>
    #include <beman/transcode/transcode_view.hpp>
    #include <beman/transcode/transcode_string.hpp>
    #include <beman/transcode/sniff.hpp>
    #include <beman/transcode/iconv_transcode_view.hpp>
    #include <beman/transcode/iconv_transcode_or_error_view.hpp>
    #include <beman/transcode/iconv_real.hpp>
    #include <beman/transcode/iconv_bulk.hpp>

    /// END [transcode.syn]

    // Implementation.  The single-byte views, the bulk helpers, the generated
    // label table and the table-driven codec engine are not proposed
    // (docs/wording-outline.md, "Not proposed at all"), and being outside the
    // region is how they say so.
    #include <beman/transcode/detail/single_byte_decode_view.hpp>
    #include <beman/transcode/detail/single_byte_encode_view.hpp>
    #include <beman/transcode/detail/bulk_transcode.hpp>
    #include <beman/transcode/detail/table_codec.hpp>

    // The clauses.  Each is a heading and nothing else: the declarations are in
    // the synopsis above, and the wording reaches these sections from the
    // followed headers by `\ref` group header or `\at`.

    // \rSec2[transcode.errors]{Error types}

    // \rSec2[transcode.reqs]{Range requirements}

    // \rSec2[transcode.codec]{Encodings}

    // \rSec3[transcode.codec.label]{Label lookup}

    // \rSec3[transcode.codec.sniff]{Byte order mark sniffing}

    // \rSec2[transcode.whatwg.decode]{Decoding views}

    // \rSec3[transcode.whatwg.decode.iterator]{Class `whatwg_decode_view::iterator`}

    // \rSec2[transcode.whatwg.encode]{Encoding views}

    // \rSec3[transcode.whatwg.encode.iterator]{Class `whatwg_encode_view::iterator`}

    // \rSec2[transcode.custom.reqs]{Codec requirements}

    // \rSec2[transcode.custom.decode]{Class template `decode_view`}

    // \rSec3[transcode.custom.decode.iterator]{Class `decode_view::iterator`}

    // \rSec2[transcode.custom.encode]{Class template `encode_view`}

    // \rSec3[transcode.custom.encode.iterator]{Class `encode_view::iterator`}

    // \rSec2[transcode.pipeline]{Transcoding pipelines}

    // \rSec2[transcode.string]{Eager transcoding}

#endif // BEMAN_TRANSCODE_USE_MODULES() &&
       // !defined(BEMAN_TRANSCODE_INCLUDED_FROM_INTERFACE_UNIT)

#endif // INCLUDE_BEMAN_TRANSCODE_TRANSCODE_HPP
