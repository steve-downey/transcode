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
    [ "C++29 Transcoding Architecture", "md_docs_C__29_Transcoding_Architecture.html", [
      [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md0", [
        [ "<strong>Executive Summary</strong>", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md1", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md2", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md3", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md4", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md5", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md6", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md7", null ]
      ] ]
    ] ],
    [ "Phase 2 Progress Checklist", "md_docs_plans_phase2_checklist.html", [
      [ "Step 14: <tt>codec::replacement</tt> (<tt>step14-replacement-codec</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md10", null ],
      [ "Step 15: <tt>codec::x_user_defined</tt> (<tt>step15-x-user-defined</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md11", null ],
      [ "Step 16: Single-byte infra + <tt>windows_1252</tt> (<tt>step16-single-byte-infra</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md12", null ],
      [ "Step 17: Data tooling (<tt>step17-data-tooling</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md13", null ],
      [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md14", null ],
      [ "Step 19: Single-byte encoder infra + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md15", null ],
      [ "Step 20: UTF-8 encoder (<tt>step20-utf8-encoder</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md16", null ],
      [ "Step 21: UTF-16 decode + encode (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md17", null ],
      [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md18", null ],
      [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md19", null ],
      [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md20", null ],
      [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md21", null ],
      [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md22", null ],
      [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md23", null ],
      [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md24", null ],
      [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md25", null ],
      [ "Step 30: WPT test vector integration (<tt>step30-wpt-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md27", null ],
      [ "Step 31: WPT ISO-2022-JP + single-byte exhaustive (<tt>step31-wpt-iso2022jp-singlebyte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md28", null ],
      [ "Step 32: ISO-2022-JP full WHATWG conformance (<tt>step32-iso2022jp-conformance</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md29", null ],
      [ "Step 33: WPT UTF-16 surrogate conformance (<tt>step33-wpt-utf16-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md30", null ],
      [ "Step 34: WPT fatal mode vectors (<tt>step34-wpt-fatal-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md31", null ],
      [ "Step 35: WPT BOM stripping conformance (<tt>step35-wpt-bom-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md32", null ],
      [ "Step 37: WPT EOF vectors + Big5 conformance fix (<tt>step37-wpt-eof-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md34", null ],
      [ "Step 36: WPT fatal single-byte (<tt>step36-wpt-fatal-single-byte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md36", null ],
      [ "Step 38: WPT UTF-8 surrogate encode vectors (<tt>step38-wpt-surrogates-utf8</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md37", null ],
      [ "Upcoming", "md_docs_plans_phase2_checklist.html#autotoc_md39", null ],
      [ "Step 39: WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md40", [
        [ "WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>) — DONE", "md_docs_plans_phase2_checklist.html#autotoc_md41", null ],
        [ "Label lookup API (<tt>step39-label-lookup</tt> or <tt>step40-label-lookup</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md42", null ]
      ] ],
      [ "Step 41: <tt>sniff_encoding</tt> BOM detection (<tt>step41-sniff-encoding</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md43", null ],
      [ "Step 42: Umbrella header (<tt>step42-umbrella-header</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md44", null ],
      [ "Notes", "md_docs_plans_phase2_checklist.html#autotoc_md46", null ]
    ] ],
    [ "Phase 2: Full WHATWG Codec Implementation", "md_docs_plans_phase2_index.html", [
      [ "Overview", "md_docs_plans_phase2_index.html#autotoc_md48", null ],
      [ "WHATWG Encoding Inventory", "md_docs_plans_phase2_index.html#autotoc_md50", [
        [ "Already implemented", "md_docs_plans_phase2_index.html#autotoc_md51", null ],
        [ "Single-byte (22 total, 1 done)", "md_docs_plans_phase2_index.html#autotoc_md52", null ],
        [ "Multi-byte (7 codecs, each with unique algorithm)", "md_docs_plans_phase2_index.html#autotoc_md53", null ],
        [ "Other", "md_docs_plans_phase2_index.html#autotoc_md54", null ]
      ] ],
      [ "Steps", "md_docs_plans_phase2_index.html#autotoc_md56", [
        [ "Completed", "md_docs_plans_phase2_index.html#autotoc_md57", [
          [ "@ref step14-replacement-codec.md \"Step 14: `codec::replacement`\"", "md_docs_plans_phase2_index.html#autotoc_md58", null ],
          [ "@ref step15-x-user-defined.md \"Step 15: `codec::x_user_defined`\"", "md_docs_plans_phase2_index.html#autotoc_md59", null ],
          [ "@ref step16-single-byte-infra.md \"Step 16: Single-byte infrastructure + `windows_1252`\"", "md_docs_plans_phase2_index.html#autotoc_md60", null ]
        ] ],
        [ "Data tooling", "md_docs_plans_phase2_index.html#autotoc_md61", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step17-iso-8859-tables.md \"Step 17: Data tooling — WHATWG index acquisition & table generation\"", "md_docs_plans_phase2_index.html#autotoc_md62", null ]
        ] ],
        [ "Single-byte codec loop", "md_docs_plans_phase2_index.html#autotoc_md63", [
          [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md64", null ],
          [ "Step 19: Single-byte encoder infrastructure + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md65", null ]
        ] ],
        [ "UTF-8 encoder", "md_docs_plans_phase2_index.html#autotoc_md66", [
          [ "@ref step18-utf8-encoder.md \"Step 20: UTF-8 encoder\"", "md_docs_plans_phase2_index.html#autotoc_md67", null ]
        ] ],
        [ "UTF-16", "md_docs_plans_phase2_index.html#autotoc_md68", [
          [ "Step 21: UTF-16 decoder + encoder (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_index.html#autotoc_md69", null ]
        ] ],
        [ "Multi-byte codecs (decode + encode per codec)", "md_docs_plans_phase2_index.html#autotoc_md70", [
          [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_index.html#autotoc_md71", null ],
          [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_index.html#autotoc_md72", null ],
          [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_index.html#autotoc_md73", null ],
          [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_index.html#autotoc_md74", null ],
          [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md75", null ],
          [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md76", null ],
          [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_index.html#autotoc_md77", null ]
        ] ],
        [ "Composition", "md_docs_plans_phase2_index.html#autotoc_md78", [
          [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_index.html#autotoc_md79", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_plans_phase2_index.html#autotoc_md81", null ]
    ] ],
    [ "Step 17: Data Tooling — WHATWG Index Acquisition & Table Generation", "md_docs_plans_step17_iso_8859_tables.html", [
      [ "Goal", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md84", null ],
      [ "Why This Step Exists", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md85", null ],
      [ "WHATWG Index Data Sources", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md86", [
        [ "Single-byte indexes (one per codec)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md87", null ],
        [ "Multi-byte indexes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md88", null ],
        [ "Metadata", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md89", null ]
      ] ],
      [ "Directory Layout", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md90", [
        [ "<tt>docs/whatwg/</tt> — pristine upstream data", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md91", null ],
        [ "<tt>docs/wpt/</tt> — Web Platform Tests (future)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md92", null ],
        [ "<tt>data/</tt> — our derived/processed artifacts", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md93", null ]
      ] ],
      [ "Provenance: <tt>docs/whatwg/SOURCE.md</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md94", null ],
      [ "Provenance: <tt>docs/whatwg/source.bib</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md95", null ],
      [ "Python Quality Requirements", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md96", null ],
      [ "Deliverables", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md97", [
        [ "Script: <tt>tools/download_indexes.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md98", null ],
        [ "Script: <tt>tools/generate_tables.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md99", null ],
        [ "Tests: <tt>tools/tests/</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md100", null ],
        [ "Generated files (single-byte, 22 total)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md101", null ]
      ] ],
      [ "Procedure", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md102", null ],
      [ "Verification", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md103", null ],
      [ "Notes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md104", null ]
    ] ],
    [ "WHATWG Encoding Standard — Index Data", "md_docs_whatwg_SOURCE.html", [
      [ "File Provenance", "md_docs_whatwg_SOURCE.html#autotoc_md106", null ],
      [ "Full Checksums", "md_docs_whatwg_SOURCE.html#autotoc_md107", null ]
    ] ],
    [ "Web Platform Tests — Encoding Test Vectors", "md_docs_wpt_SOURCE.html", [
      [ "File Provenance", "md_docs_wpt_SOURCE.html#autotoc_md109", null ],
      [ "Full Checksums", "md_docs_wpt_SOURCE.html#autotoc_md110", null ]
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
"classbeman_1_1transcoding_1_1whatwg__decode__view.html#ae0746308e0cb6ee07bfccf21c750bdc1",
"iso__8859__15_8hpp_source.html",
"structbeman_1_1transcoding_1_1null__sentinel__t.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';