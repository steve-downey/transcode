/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "beman.transcode", "index.html", [
    [ "<strong>Methodological Framework for Benchmarking C++29 Transcoding Pipelines: Evaluating P2728 and the Beman utf_view Implementation</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html", [
      [ "<strong>1\\. Architectural Context and the Standardization of C++ Transcoding</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md1", [
        [ "<strong>1.1 Structural Constraints of the Proposed Architecture</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md2", null ]
      ] ],
      [ "<strong>2\\. The Competitive Landscape: Baseline Transcoding Implementations</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md3", [
        [ "<strong>2.1 The Negative Baseline: std::locale and <codecvt></strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md4", null ],
        [ "<strong>2.2 The Legacy POSIX Pivot: iconv</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md5", null ],
        [ "<strong>2.3 The Web Platform Standard: encoding_rs</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md6", null ],
        [ "<strong>2.4 The Modern Performance Ceiling: simdutf</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md7", null ]
      ] ],
      [ "<strong>3\\. Analysis of Existing Comparative Benchmarks</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md8", [
        [ "<strong>3.1 Transcoding Throughput Claims and Industry Metrics</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md9", null ],
        [ "<strong>3.2 SIMD Dominance and the Abstraction Delta</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md10", null ],
        [ "<strong>3.3 The Iconv Wrapper Variances</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md11", null ]
      ] ],
      [ "<strong>4\\. Bounding the Performance Envelope: Pathological and Optimal Execution Pathways</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md12", [
        [ "<strong>4.1 The Optimal Execution Paths (Best-Case Scenarios)</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md13", [
          [ "<strong>Category B: Table-Driven Single-Byte Encoders</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md14", null ],
          [ "<strong>Category A: The Algorithmic UTF Family</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md15", null ]
        ] ],
        [ "<strong>4.2 The Pathological Execution Paths (Worst-Case Scenarios)</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md16", [
          [ "<strong>Category C: Multi-Byte CJK Complex Indexing</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md17", null ],
          [ "<strong>Lazy Pipeline Boundary Interruptions</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md18", null ]
        ] ]
      ] ],
      [ "<strong>5\\. Linguistic Entropy and Standard Corpora Selection</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md19", [
        [ "<strong>5.1 The Text Encoding Initiative (TEI) and Linguistic Reality</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md20", null ],
        [ "<strong>5.2 The \"Wikipedia Mars\" Heuristic</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md21", null ],
        [ "<strong>5.3 Synthetic Boundary and Fuzzing Data</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md22", null ]
      ] ],
      [ "<strong>6\\. Harnessing Catch2 for Micro-Architectural Evaluation</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md23", [
        [ "<strong>6.1 Dynamic Estimation and Statistical Sampling</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md24", null ],
        [ "<strong>6.2 Combating the Compiler Optimizer: BENCHMARK_ADVANCED</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md25", null ],
        [ "<strong>6.3 Limitations and Augmentations of Catch2 Throughput Reporting</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md26", null ],
        [ "<strong>6.4 Mocking the Substrate: Deterministic State Machine Validation</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md27", null ]
      ] ],
      [ "<strong>7\\. The Compiler Optimizer as a Critical Variable</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md28", [
        [ "<strong>7.1 Maximum Optimization (-O3) and Auto-Vectorization</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md29", null ],
        [ "<strong>7.2 Link Time Optimization (-flto)</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md30", null ]
      ] ],
      [ "<strong>8\\. The Proposed Execution Matrix and Synthesized Implications</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md31", [
        [ "<strong>8.1 The Execution Matrix Topology</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md32", null ],
        [ "<strong>8.2 Synthesized Analytical Insights</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md33", [
          [ "<strong>Works cited</strong>", "md_docs_Benchmarking_Transcode_Implementation_Performance.html#autotoc_md34", null ]
        ] ]
      ] ]
    ] ],
    [ "C++29 Transcoding Architecture", "md_docs_C__29_Transcoding_Architecture.html", [
      [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md35", [
        [ "<strong>Executive Summary</strong>", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md36", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md37", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md38", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md39", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md40", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md41", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md42", null ]
      ] ]
    ] ],
    [ "Testing C++ Range Adapters over <tt>iconv</tt>", "md_docs_iconv_testing.html", [
      [ "1. The <tt>iconv</tt> View Boundary Conditions", "md_docs_iconv_testing.html#autotoc_md44", [
        [ "A. Buffer Exhaustion (<tt>E2BIG</tt>)", "md_docs_iconv_testing.html#autotoc_md45", null ],
        [ "B. The Split Multi-byte Sequence (<tt>EINVAL</tt>)", "md_docs_iconv_testing.html#autotoc_md46", null ],
        [ "C. Invalid Byte Sequences (<tt>EILSEQ</tt>)", "md_docs_iconv_testing.html#autotoc_md47", null ],
        [ "D. End-of-Range Stateful Flush", "md_docs_iconv_testing.html#autotoc_md48", null ]
      ] ],
      [ "2. Mocking <tt>iconv</tt> for Deterministic View Testing", "md_docs_iconv_testing.html#autotoc_md50", null ],
      [ "3. Testing Execution Plan", "md_docs_iconv_testing.html#autotoc_md52", null ]
    ] ],
    [ "Phase 2 Progress Checklist", "md_docs_plans_phase2_checklist.html", [
      [ "Step 14: <tt>codec::replacement</tt> (<tt>step14-replacement-codec</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md55", null ],
      [ "Step 15: <tt>codec::x_user_defined</tt> (<tt>step15-x-user-defined</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md56", null ],
      [ "Step 16: Single-byte infra + <tt>windows_1252</tt> (<tt>step16-single-byte-infra</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md57", null ],
      [ "Step 17: Data tooling (<tt>step17-data-tooling</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md58", null ],
      [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md59", null ],
      [ "Step 19: Single-byte encoder infra + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md60", null ],
      [ "Step 20: UTF-8 encoder (<tt>step20-utf8-encoder</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md61", null ],
      [ "Step 21: UTF-16 decode + encode (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md62", null ],
      [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md63", null ],
      [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md64", null ],
      [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md65", null ],
      [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md66", null ],
      [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md67", null ],
      [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md68", null ],
      [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md69", null ],
      [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md70", null ],
      [ "Step 30: WPT test vector integration (<tt>step30-wpt-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md72", null ],
      [ "Step 31: WPT ISO-2022-JP + single-byte exhaustive (<tt>step31-wpt-iso2022jp-singlebyte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md73", null ],
      [ "Step 32: ISO-2022-JP full WHATWG conformance (<tt>step32-iso2022jp-conformance</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md74", null ],
      [ "Step 33: WPT UTF-16 surrogate conformance (<tt>step33-wpt-utf16-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md75", null ],
      [ "Step 34: WPT fatal mode vectors (<tt>step34-wpt-fatal-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md76", null ],
      [ "Step 35: WPT BOM stripping conformance (<tt>step35-wpt-bom-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md77", null ],
      [ "Step 37: WPT EOF vectors + Big5 conformance fix (<tt>step37-wpt-eof-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md79", null ],
      [ "Step 36: WPT fatal single-byte (<tt>step36-wpt-fatal-single-byte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md81", null ],
      [ "Step 38: WPT UTF-8 surrogate encode vectors (<tt>step38-wpt-surrogates-utf8</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md82", null ],
      [ "Upcoming", "md_docs_plans_phase2_checklist.html#autotoc_md84", null ],
      [ "Step 39: WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md85", [
        [ "WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>) — DONE", "md_docs_plans_phase2_checklist.html#autotoc_md86", null ],
        [ "Label lookup API (<tt>step39-label-lookup</tt> or <tt>step40-label-lookup</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md87", null ]
      ] ],
      [ "Step 41: <tt>sniff_encoding</tt> BOM detection (<tt>step41-sniff-encoding</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md88", null ],
      [ "Step 42: Umbrella header (<tt>step42-umbrella-header</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md89", null ],
      [ "Step 43: <tt>transcode_string</tt> one-shot function (<tt>step43-transcode-string</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md90", null ],
      [ "Step 44: Coverage audit of <tt>transcode_string</tt> (<tt>step44-coverage-audit</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md91", null ],
      [ "Step 45: <tt>transcode_string</tt> label overload (<tt>step45-transcode-string-label</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md92", null ],
      [ "Step 46: <tt>transcode_view</tt> pipe composition helper (<tt>step46-transcode-view</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md93", null ],
      [ "Step 47: <tt>transcode_view</tt> negative compile test + consteval test (<tt>step47-transcode-view-negative-consteval</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md94", null ],
      [ "Step 48: <tt>sniff_encoding</tt> negative compile test + Python lint fix (<tt>step48-sniff-negative-lint-fix</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md95", null ],
      [ "Step 49: <tt>whatwg_decode_view</tt> coverage improvement (<tt>step49-decode-view-coverage</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md96", null ],
      [ "Step 50: iconv view boundary-condition tests (<tt>step50-iconv-boundary-tests</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md97", null ],
      [ "Step 51: iconv stateful flush (<tt>step51-iconv-stateful-flush</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md98", null ],
      [ "Notes", "md_docs_plans_phase2_checklist.html#autotoc_md100", null ]
    ] ],
    [ "Phase 2: Full WHATWG Codec Implementation", "md_docs_plans_phase2_index.html", [
      [ "Overview", "md_docs_plans_phase2_index.html#autotoc_md102", null ],
      [ "WHATWG Encoding Inventory", "md_docs_plans_phase2_index.html#autotoc_md104", [
        [ "Already implemented", "md_docs_plans_phase2_index.html#autotoc_md105", null ],
        [ "Single-byte (22 total, 1 done)", "md_docs_plans_phase2_index.html#autotoc_md106", null ],
        [ "Multi-byte (7 codecs, each with unique algorithm)", "md_docs_plans_phase2_index.html#autotoc_md107", null ],
        [ "Other", "md_docs_plans_phase2_index.html#autotoc_md108", null ]
      ] ],
      [ "Steps", "md_docs_plans_phase2_index.html#autotoc_md110", [
        [ "Completed", "md_docs_plans_phase2_index.html#autotoc_md111", [
          [ "@ref step14-replacement-codec.md \"Step 14: `codec::replacement`\"", "md_docs_plans_phase2_index.html#autotoc_md112", null ],
          [ "@ref step15-x-user-defined.md \"Step 15: `codec::x_user_defined`\"", "md_docs_plans_phase2_index.html#autotoc_md113", null ],
          [ "@ref step16-single-byte-infra.md \"Step 16: Single-byte infrastructure + `windows_1252`\"", "md_docs_plans_phase2_index.html#autotoc_md114", null ]
        ] ],
        [ "Data tooling", "md_docs_plans_phase2_index.html#autotoc_md115", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step17-iso-8859-tables.md \"Step 17: Data tooling — WHATWG index acquisition & table generation\"", "md_docs_plans_phase2_index.html#autotoc_md116", null ]
        ] ],
        [ "Single-byte codec loop", "md_docs_plans_phase2_index.html#autotoc_md117", [
          [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md118", null ],
          [ "Step 19: Single-byte encoder infrastructure + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md119", null ]
        ] ],
        [ "UTF-8 encoder", "md_docs_plans_phase2_index.html#autotoc_md120", [
          [ "@ref step18-utf8-encoder.md \"Step 20: UTF-8 encoder\"", "md_docs_plans_phase2_index.html#autotoc_md121", null ]
        ] ],
        [ "UTF-16", "md_docs_plans_phase2_index.html#autotoc_md122", [
          [ "Step 21: UTF-16 decoder + encoder (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_index.html#autotoc_md123", null ]
        ] ],
        [ "Multi-byte codecs (decode + encode per codec)", "md_docs_plans_phase2_index.html#autotoc_md124", [
          [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_index.html#autotoc_md125", null ],
          [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_index.html#autotoc_md126", null ],
          [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_index.html#autotoc_md127", null ],
          [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_index.html#autotoc_md128", null ],
          [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md129", null ],
          [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md130", null ],
          [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_index.html#autotoc_md131", null ]
        ] ],
        [ "Composition", "md_docs_plans_phase2_index.html#autotoc_md132", [
          [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_index.html#autotoc_md133", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_plans_phase2_index.html#autotoc_md135", null ]
    ] ],
    [ "Step 17: Data Tooling — WHATWG Index Acquisition & Table Generation", "md_docs_plans_step17_iso_8859_tables.html", [
      [ "Goal", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md138", null ],
      [ "Why This Step Exists", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md139", null ],
      [ "WHATWG Index Data Sources", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md140", [
        [ "Single-byte indexes (one per codec)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md141", null ],
        [ "Multi-byte indexes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md142", null ],
        [ "Metadata", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md143", null ]
      ] ],
      [ "Directory Layout", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md144", [
        [ "<tt>docs/whatwg/</tt> — pristine upstream data", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md145", null ],
        [ "<tt>docs/wpt/</tt> — Web Platform Tests (future)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md146", null ],
        [ "<tt>data/</tt> — our derived/processed artifacts", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md147", null ]
      ] ],
      [ "Provenance: <tt>docs/whatwg/SOURCE.md</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md148", null ],
      [ "Provenance: <tt>docs/whatwg/source.bib</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md149", null ],
      [ "Python Quality Requirements", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md150", null ],
      [ "Deliverables", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md151", [
        [ "Script: <tt>tools/download_indexes.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md152", null ],
        [ "Script: <tt>tools/generate_tables.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md153", null ],
        [ "Tests: <tt>tools/tests/</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md154", null ],
        [ "Generated files (single-byte, 22 total)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md155", null ]
      ] ],
      [ "Procedure", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md156", null ],
      [ "Verification", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md157", null ],
      [ "Notes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md158", null ]
    ] ],
    [ "<strong>System Architecture and Implementation Strategies for C++ Transcoding: Integrating encoding_rs as an Oracle for P1439-compliant Interfaces</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html", [
      [ "<strong>The Standardization Landscape and the Crisis in Text Processing</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md160", null ],
      [ "<strong>Deconstructing the P1439 Transcoding Model</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md161", [
        [ "<strong>The Imperative of Range-Based Composability</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md162", null ]
      ] ],
      [ "<strong>The Internal Engine: Profiling encoding_rs and WHATWG Compliance</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md163", [
        [ "<strong>WHATWG Encoding Standard Compliance</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md164", null ],
        [ "<strong>Streaming versus Non-Streaming Execution Modes</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md165", null ]
      ] ],
      [ "<strong>Bridging the FFI Boundary: The Constraints of encoding_c</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md166", [
        [ "<strong>The Panic=Abort Imperative and Exception Safety</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md167", null ],
        [ "<strong>Type Impedance and the Guideline Support Library (GSL)</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md168", null ]
      ] ],
      [ "<strong>Implementation Strategy: Mapping Ranges to Rust Decoders</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md169", [
        [ "<strong>Bypassing Iterator Overhead via Contiguous Specialization</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md170", null ],
        [ "<strong>Managing Non-Contiguous State and Pipeline Eagerness</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md171", null ]
      ] ],
      [ "<strong>Theoretical Foundations: Coinductive Types, Trees, and Traversables</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md172", null ],
      [ "<strong>Monadic Error Handling: Integration with views::maybe and std::optional</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md173", [
        [ "<strong>The Role of views::maybe in Transcoding Pipelines</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md174", null ],
        [ "<strong>std::optional and Monadic Continuations</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md175", null ]
      ] ],
      [ "<strong>The Oracle Paradigm: Fuzzing and Implementor Relief</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md176", [
        [ "<strong>Defining the Testing Oracle</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md177", null ],
        [ "<strong>Differential Fuzzing Strategy</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md178", null ]
      ] ],
      [ "<strong>Managing Mojibake, Identifier Normalization, and Security</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md179", null ],
      [ "<strong>Navigating Build Systems and Ecosystem Integration</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md180", null ],
      [ "<strong>Conclusion</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md181", null ]
    ] ],
    [ "Review of transcode as a C++29 Library Candidate and Beman Inclusion Candidate", "md_docs_transcode_as_a_C__29_Library_Candidate.html", [
      [ "Executive summary", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md184", [
        [ "Audit summary", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md185", null ]
      ] ],
      [ "Repository status and engineering quality", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md186", null ],
      [ "API design and code-level behavior", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md187", [
        [ "Design issues and concrete defects", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md188", null ],
        [ "Targeted code suggestions", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md189", null ]
      ] ],
      [ "Single-byte support and the feasibility of random-access iteration or indexing", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md190", [
        [ "Feasibility matrix for random access", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md191", null ]
      ] ],
      [ "Relationship to P2728", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md192", [
        [ "Current behavior versus P2728-like expectations", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md193", null ],
        [ "Suggested architectural split", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md194", null ]
      ] ],
      [ "Concrete changes, tests to add, and documentation fixes", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md195", null ],
      [ "Acceptance checklist for Beman inclusion and proposal readiness", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md196", [
        [ "Bottom-line recommendation", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md197", null ]
      ] ]
    ] ],
    [ "WHATWG Encoding Standard — Index Data", "md_docs_whatwg_SOURCE.html", [
      [ "File Provenance", "md_docs_whatwg_SOURCE.html#autotoc_md199", null ],
      [ "Full Checksums", "md_docs_whatwg_SOURCE.html#autotoc_md200", null ]
    ] ],
    [ "Web Platform Tests — Encoding Test Vectors", "md_docs_wpt_SOURCE.html", [
      [ "File Provenance", "md_docs_wpt_SOURCE.html#autotoc_md202", null ],
      [ "Full Checksums", "md_docs_wpt_SOURCE.html#autotoc_md203", null ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Concepts", "concepts.html", "concepts" ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Related Functions", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"classbeman_1_1transcoding_1_1whatwg__decode__view.html#a76921f5693fdc33d9aa18b0362343715",
"iso__8859__13_8hpp_source.html",
"namespacebeman_1_1transcoding.html#a20dfb324ebd26c6127d014520f93005aa870b1c8d709ae13c6f3125309e99e10a"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';