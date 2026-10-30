<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->

<!--
The authored part of the site's landing page.  `tools/build_site.py` renders
it, then appends the generated sections: the API references, the paper, and
every document it rendered or copied from docs/ and papers/.  Links here may
name repository files; the tool points them at the rendered pages.
-->

# beman.transcode

`beman.transcode` is a header-only C++23 library for Unicode transcoding with
ranges and views.  It implements the
[WHATWG Encoding Standard](https://encoding.spec.whatwg.org/) at compile
time, and wraps the POSIX `iconv` API as a range adaptor for the encodings a
platform provides.  It is the reference implementation for WG21 paper P4246,
"Transcoding Text Views", and part of the
[Beman Project](https://github.com/bemanproject).

- Source: [github.com/steve-downey/transcode](https://github.com/steve-downey/transcode)
- Overview and usage: the [README](README.md)
- Contributing: [CONTRIBUTING](CONTRIBUTING.md)
