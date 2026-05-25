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
    [ "P3-Step 1: Benchmark Harness Scaffolding", "md_docs_plans_p3_step1_benchmark_harness.html", [
      [ "Goal", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md55", null ],
      [ "Why This Step Exists", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md56", null ],
      [ "Deliverables", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md57", null ],
      [ "Files Expected", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md58", null ],
      [ "Constraints", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md59", null ],
      [ "Procedure", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md60", null ],
      [ "Verification", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md61", null ],
      [ "Notes", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md62", null ]
    ] ],
    [ "P3-Step 10: Chunked/Lazy Boundary Stress Benchmarks", "md_docs_plans_p3_step10_boundary_stress.html", [
      [ "Goal", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md65", null ],
      [ "Deliverables", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md66", null ],
      [ "Constraints", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md67", null ],
      [ "Procedure", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md68", null ],
      [ "Verification", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md69", null ],
      [ "Notes", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md70", null ]
    ] ],
    [ "P3-Step 11: Toolchain Matrix Automation", "md_docs_plans_p3_step11_toolchain_matrix.html", [
      [ "Goal", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md73", null ],
      [ "Matrix To Support", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md74", null ],
      [ "Deliverables", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md75", null ],
      [ "Constraints", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md76", null ],
      [ "Procedure", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md77", null ],
      [ "Verification", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md78", null ],
      [ "Notes", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md79", null ]
    ] ],
    [ "P3-Step 12: Reporting, Dashboard, and Reproducibility Pack", "md_docs_plans_p3_step12_reporting_and_repro.html", [
      [ "Goal", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md82", null ],
      [ "Deliverables", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md83", null ],
      [ "Files Expected", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md84", null ],
      [ "Constraints", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md85", null ],
      [ "Procedure", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md86", null ],
      [ "Verification", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md87", null ],
      [ "Notes", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md88", null ]
    ] ],
    [ "P3-Step 2: Benchmark Corpus Acquisition and Provenance", "md_docs_plans_p3_step2_benchmark_data.html", [
      [ "Goal", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md91", null ],
      [ "Data To Cover", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md92", null ],
      [ "Deliverables", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md93", null ],
      [ "Files Expected", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md94", null ],
      [ "Constraints", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md95", null ],
      [ "Procedure", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md96", null ],
      [ "Verification", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md97", null ],
      [ "Notes", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md98", null ]
    ] ],
    [ "P3-Step 3: Benchmark Fixture Library and Result Schema", "md_docs_plans_p3_step3_benchmark_fixtures.html", [
      [ "Goal", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md101", null ],
      [ "Deliverables", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md102", null ],
      [ "Files Expected", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md103", null ],
      [ "Design Requirements", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md104", null ],
      [ "Procedure", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md105", null ],
      [ "Verification", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md106", null ],
      [ "Notes", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md107", null ]
    ] ],
    [ "P3-Step 4: UTF-Family and ASCII Fast-Path Benchmarks", "md_docs_plans_p3_step4_utf_benchmarks.html", [
      [ "Goal", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md110", null ],
      [ "Cases To Cover", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md111", null ],
      [ "Deliverables", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md112", null ],
      [ "Constraints", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md113", null ],
      [ "Procedure", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md114", null ],
      [ "Verification", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md115", null ],
      [ "Notes", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md116", null ]
    ] ],
    [ "P3-Step 5: Legacy WHATWG Codec Benchmarks", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html", [
      [ "Goal", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md119", null ],
      [ "Representative Coverage", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md120", null ],
      [ "Deliverables", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md121", null ],
      [ "Constraints", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md122", null ],
      [ "Procedure", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md123", null ],
      [ "Verification", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md124", null ],
      [ "Notes", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md125", null ]
    ] ],
    [ "P3-Step 6: <tt>iconv</tt> Baselines", "md_docs_plans_p3_step6_iconv_baselines.html", [
      [ "Goal", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md128", null ],
      [ "Deliverables", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md129", null ],
      [ "Constraints", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md130", null ],
      [ "Procedure", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md131", null ],
      [ "Verification", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md132", null ],
      [ "Notes", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md133", null ]
    ] ],
    [ "P3-Step 7: <tt>std::codecvt</tt> Negative Baseline", "md_docs_plans_p3_step7_codecvt_baseline.html", [
      [ "Goal", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md136", null ],
      [ "Deliverables", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md137", null ],
      [ "Constraints", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md138", null ],
      [ "Procedure", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md139", null ],
      [ "Verification", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md140", null ],
      [ "Notes", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md141", null ]
    ] ],
    [ "P3-Step 8: <tt>encoding_rs</tt> Baseline", "md_docs_plans_p3_step8_encoding_rs_baseline.html", [
      [ "Goal", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md144", null ],
      [ "Deliverables", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md145", null ],
      [ "Constraints", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md146", null ],
      [ "Procedure", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md147", null ],
      [ "Verification", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md148", null ],
      [ "Notes", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md149", null ]
    ] ],
    [ "P3-Step 9: <tt>simdutf</tt> Ceiling Baseline", "md_docs_plans_p3_step9_simdutf_baseline.html", [
      [ "Goal", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md152", null ],
      [ "Deliverables", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md153", null ],
      [ "Constraints", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md154", null ],
      [ "Procedure", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md155", null ],
      [ "Verification", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md156", null ],
      [ "Notes", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md157", null ]
    ] ],
    [ "Phase 2 Progress Checklist", "md_docs_plans_phase2_checklist.html", [
      [ "Step 14: <tt>codec::replacement</tt> (<tt>step14-replacement-codec</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md160", null ],
      [ "Step 15: <tt>codec::x_user_defined</tt> (<tt>step15-x-user-defined</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md161", null ],
      [ "Step 16: Single-byte infra + <tt>windows_1252</tt> (<tt>step16-single-byte-infra</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md162", null ],
      [ "Step 17: Data tooling (<tt>step17-data-tooling</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md163", null ],
      [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md164", null ],
      [ "Step 19: Single-byte encoder infra + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md165", null ],
      [ "Step 20: UTF-8 encoder (<tt>step20-utf8-encoder</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md166", null ],
      [ "Step 21: UTF-16 decode + encode (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md167", null ],
      [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md168", null ],
      [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md169", null ],
      [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md170", null ],
      [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md171", null ],
      [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md172", null ],
      [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md173", null ],
      [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md174", null ],
      [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md175", null ],
      [ "Step 30: WPT test vector integration (<tt>step30-wpt-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md177", null ],
      [ "Step 31: WPT ISO-2022-JP + single-byte exhaustive (<tt>step31-wpt-iso2022jp-singlebyte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md178", null ],
      [ "Step 32: ISO-2022-JP full WHATWG conformance (<tt>step32-iso2022jp-conformance</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md179", null ],
      [ "Step 33: WPT UTF-16 surrogate conformance (<tt>step33-wpt-utf16-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md180", null ],
      [ "Step 34: WPT fatal mode vectors (<tt>step34-wpt-fatal-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md181", null ],
      [ "Step 35: WPT BOM stripping conformance (<tt>step35-wpt-bom-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md182", null ],
      [ "Step 37: WPT EOF vectors + Big5 conformance fix (<tt>step37-wpt-eof-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md184", null ],
      [ "Step 36: WPT fatal single-byte (<tt>step36-wpt-fatal-single-byte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md186", null ],
      [ "Step 38: WPT UTF-8 surrogate encode vectors (<tt>step38-wpt-surrogates-utf8</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md187", null ],
      [ "Upcoming", "md_docs_plans_phase2_checklist.html#autotoc_md189", null ],
      [ "Step 39: WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md190", [
        [ "WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>) — DONE", "md_docs_plans_phase2_checklist.html#autotoc_md191", null ],
        [ "Label lookup API (<tt>step39-label-lookup</tt> or <tt>step40-label-lookup</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md192", null ]
      ] ],
      [ "Step 41: <tt>sniff_encoding</tt> BOM detection (<tt>step41-sniff-encoding</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md193", null ],
      [ "Step 42: Umbrella header (<tt>step42-umbrella-header</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md194", null ],
      [ "Step 43: <tt>transcode_string</tt> one-shot function (<tt>step43-transcode-string</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md195", null ],
      [ "Step 44: Coverage audit of <tt>transcode_string</tt> (<tt>step44-coverage-audit</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md196", null ],
      [ "Step 45: <tt>transcode_string</tt> label overload (<tt>step45-transcode-string-label</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md197", null ],
      [ "Step 46: <tt>transcode_view</tt> pipe composition helper (<tt>step46-transcode-view</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md198", null ],
      [ "Step 47: <tt>transcode_view</tt> negative compile test + consteval test (<tt>step47-transcode-view-negative-consteval</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md199", null ],
      [ "Step 48: <tt>sniff_encoding</tt> negative compile test + Python lint fix (<tt>step48-sniff-negative-lint-fix</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md200", null ],
      [ "Step 49: <tt>whatwg_decode_view</tt> coverage improvement (<tt>step49-decode-view-coverage</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md201", null ],
      [ "Step 50: iconv view boundary-condition tests (<tt>step50-iconv-boundary-tests</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md202", null ],
      [ "Step 51: iconv stateful flush (<tt>step51-iconv-stateful-flush</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md203", null ],
      [ "Step 52: real-iconv ISO-2022-JP flush integration tests (<tt>step52-iconv-iso2022jp-flush-test</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md204", null ],
      [ "Step 53: coverage improvements (<tt>step53-coverage-improvements</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md205", null ],
      [ "Notes", "md_docs_plans_phase2_checklist.html#autotoc_md207", null ]
    ] ],
    [ "Phase 2: Full WHATWG Codec Implementation", "md_docs_plans_phase2_index.html", [
      [ "Overview", "md_docs_plans_phase2_index.html#autotoc_md209", null ],
      [ "WHATWG Encoding Inventory", "md_docs_plans_phase2_index.html#autotoc_md211", [
        [ "Already implemented", "md_docs_plans_phase2_index.html#autotoc_md212", null ],
        [ "Single-byte (22 total, 1 done)", "md_docs_plans_phase2_index.html#autotoc_md213", null ],
        [ "Multi-byte (7 codecs, each with unique algorithm)", "md_docs_plans_phase2_index.html#autotoc_md214", null ],
        [ "Other", "md_docs_plans_phase2_index.html#autotoc_md215", null ]
      ] ],
      [ "Steps", "md_docs_plans_phase2_index.html#autotoc_md217", [
        [ "Completed", "md_docs_plans_phase2_index.html#autotoc_md218", [
          [ "@ref step14-replacement-codec.md \"Step 14: `codec::replacement`\"", "md_docs_plans_phase2_index.html#autotoc_md219", null ],
          [ "@ref step15-x-user-defined.md \"Step 15: `codec::x_user_defined`\"", "md_docs_plans_phase2_index.html#autotoc_md220", null ],
          [ "@ref step16-single-byte-infra.md \"Step 16: Single-byte infrastructure + `windows_1252`\"", "md_docs_plans_phase2_index.html#autotoc_md221", null ]
        ] ],
        [ "Data tooling", "md_docs_plans_phase2_index.html#autotoc_md222", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step17-iso-8859-tables.md \"Step 17: Data tooling — WHATWG index acquisition & table generation\"", "md_docs_plans_phase2_index.html#autotoc_md223", null ]
        ] ],
        [ "Single-byte codec loop", "md_docs_plans_phase2_index.html#autotoc_md224", [
          [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md225", null ],
          [ "Step 19: Single-byte encoder infrastructure + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md226", null ]
        ] ],
        [ "UTF-8 encoder", "md_docs_plans_phase2_index.html#autotoc_md227", [
          [ "@ref step18-utf8-encoder.md \"Step 20: UTF-8 encoder\"", "md_docs_plans_phase2_index.html#autotoc_md228", null ]
        ] ],
        [ "UTF-16", "md_docs_plans_phase2_index.html#autotoc_md229", [
          [ "Step 21: UTF-16 decoder + encoder (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_index.html#autotoc_md230", null ]
        ] ],
        [ "Multi-byte codecs (decode + encode per codec)", "md_docs_plans_phase2_index.html#autotoc_md231", [
          [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_index.html#autotoc_md232", null ],
          [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_index.html#autotoc_md233", null ],
          [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_index.html#autotoc_md234", null ],
          [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_index.html#autotoc_md235", null ],
          [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md236", null ],
          [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md237", null ],
          [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_index.html#autotoc_md238", null ]
        ] ],
        [ "Composition", "md_docs_plans_phase2_index.html#autotoc_md239", [
          [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_index.html#autotoc_md240", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_plans_phase2_index.html#autotoc_md242", null ]
    ] ],
    [ "Phase 3 Progress Checklist", "md_docs_plans_phase3_checklist.html", [
      [ "P3-Step 1: Benchmark harness scaffolding (<tt>p3-step1-benchmark-harness</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md245", null ],
      [ "P3-Step 2: Benchmark corpus acquisition and provenance (<tt>p3-step2-benchmark-data</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md246", null ],
      [ "P3-Step 3: Benchmark fixture library and result schema (<tt>p3-step3-benchmark-fixtures</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md247", null ],
      [ "P3-Step 4: UTF-family and ASCII fast-path benchmarks (<tt>p3-step4-utf-benchmarks</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md248", null ],
      [ "P3-Step 5: Legacy WHATWG codec benchmarks (<tt>p3-step5-legacy-whatwg-benchmarks</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md249", null ],
      [ "P3-Step 6: <tt>iconv</tt> baselines (<tt>p3-step6-iconv-baselines</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md250", null ],
      [ "P3-Step 7: <tt>std::codecvt</tt> negative baseline (<tt>p3-step7-codecvt-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md251", null ],
      [ "P3-Step 8: <tt>encoding_rs</tt> baseline (<tt>p3-step8-encoding-rs-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md252", null ],
      [ "P3-Step 9: <tt>simdutf</tt> ceiling baseline (<tt>p3-step9-simdutf-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md253", null ],
      [ "P3-Step 10: Chunked/lazy boundary stress benchmarks (<tt>p3-step10-boundary-stress</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md254", null ],
      [ "P3-Step 11: Toolchain matrix automation (<tt>p3-step11-toolchain-matrix</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md255", null ],
      [ "P3-Step 12: Reporting, dashboard, and reproducibility pack (<tt>p3-step12-reporting-and-repro</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md256", null ],
      [ "Notes", "md_docs_plans_phase3_checklist.html#autotoc_md258", null ]
    ] ],
    [ "Phase 3: Benchmarking and Performance Characterization", "md_docs_plans_phase3_index.html", [
      [ "Overview", "md_docs_plans_phase3_index.html#autotoc_md260", null ],
      [ "Why Phase 3 Is Separate", "md_docs_plans_phase3_index.html#autotoc_md262", null ],
      [ "Phase Scope", "md_docs_plans_phase3_index.html#autotoc_md264", null ],
      [ "P3 Steps", "md_docs_plans_phase3_index.html#autotoc_md266", [
        [ "Harness and data foundations", "md_docs_plans_phase3_index.html#autotoc_md267", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step1-benchmark-harness.md \"P3-Step 1: Benchmark harness scaffolding\"", "md_docs_plans_phase3_index.html#autotoc_md268", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step2-benchmark-data.md \"P3-Step 2: Benchmark corpus acquisition and provenance\"", "md_docs_plans_phase3_index.html#autotoc_md269", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step3-benchmark-fixtures.md \"P3-Step 3: Benchmark fixture library and result schema\"", "md_docs_plans_phase3_index.html#autotoc_md270", null ]
        ] ],
        [ "Native <tt>beman::transcode</tt> measurement", "md_docs_plans_phase3_index.html#autotoc_md271", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step4-utf-benchmarks.md \"P3-Step 4: UTF-family and ASCII fast-path benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md272", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step5-legacy-whatwg-benchmarks.md \"P3-Step 5: Legacy WHATWG codec benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md273", null ]
        ] ],
        [ "Baseline implementations", "md_docs_plans_phase3_index.html#autotoc_md274", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step6-iconv-baselines.md \"P3-Step 6: `iconv` baselines\"", "md_docs_plans_phase3_index.html#autotoc_md275", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step7-codecvt-baseline.md \"P3-Step 7: `std::codecvt` negative baseline\"", "md_docs_plans_phase3_index.html#autotoc_md276", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step8-encoding-rs-baseline.md \"P3-Step 8: `encoding_rs` baseline\"", "md_docs_plans_phase3_index.html#autotoc_md277", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step9-simdutf-baseline.md \"P3-Step 9: `simdutf` ceiling baseline\"", "md_docs_plans_phase3_index.html#autotoc_md278", null ]
        ] ],
        [ "Stress paths and final reporting", "md_docs_plans_phase3_index.html#autotoc_md279", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step10-boundary-stress.md \"P3-Step 10: Chunked/lazy boundary stress benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md280", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step11-toolchain-matrix.md \"P3-Step 11: Toolchain matrix automation\"", "md_docs_plans_phase3_index.html#autotoc_md281", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step12-reporting-and-repro.md \"P3-Step 12: Reporting, dashboard, and reproducibility pack\"", "md_docs_plans_phase3_index.html#autotoc_md282", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_plans_phase3_index.html#autotoc_md284", null ],
      [ "Handoff Notes For This Phase", "md_docs_plans_phase3_index.html#autotoc_md286", null ]
    ] ],
    [ "Step 17: Data Tooling — WHATWG Index Acquisition & Table Generation", "md_docs_plans_step17_iso_8859_tables.html", [
      [ "Goal", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md289", null ],
      [ "Why This Step Exists", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md290", null ],
      [ "WHATWG Index Data Sources", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md291", [
        [ "Single-byte indexes (one per codec)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md292", null ],
        [ "Multi-byte indexes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md293", null ],
        [ "Metadata", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md294", null ]
      ] ],
      [ "Directory Layout", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md295", [
        [ "<tt>docs/whatwg/</tt> — pristine upstream data", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md296", null ],
        [ "<tt>docs/wpt/</tt> — Web Platform Tests (future)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md297", null ],
        [ "<tt>data/</tt> — our derived/processed artifacts", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md298", null ]
      ] ],
      [ "Provenance: <tt>docs/whatwg/SOURCE.md</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md299", null ],
      [ "Provenance: <tt>docs/whatwg/source.bib</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md300", null ],
      [ "Python Quality Requirements", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md301", null ],
      [ "Deliverables", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md302", [
        [ "Script: <tt>tools/download_indexes.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md303", null ],
        [ "Script: <tt>tools/generate_tables.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md304", null ],
        [ "Tests: <tt>tools/tests/</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md305", null ],
        [ "Generated files (single-byte, 22 total)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md306", null ]
      ] ],
      [ "Procedure", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md307", null ],
      [ "Verification", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md308", null ],
      [ "Notes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md309", null ]
    ] ],
    [ "<strong>System Architecture and Implementation Strategies for C++ Transcoding: Integrating encoding_rs as an Oracle for P1439-compliant Interfaces</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html", [
      [ "<strong>The Standardization Landscape and the Crisis in Text Processing</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md311", null ],
      [ "<strong>Deconstructing the P1439 Transcoding Model</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md312", [
        [ "<strong>The Imperative of Range-Based Composability</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md313", null ]
      ] ],
      [ "<strong>The Internal Engine: Profiling encoding_rs and WHATWG Compliance</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md314", [
        [ "<strong>WHATWG Encoding Standard Compliance</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md315", null ],
        [ "<strong>Streaming versus Non-Streaming Execution Modes</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md316", null ]
      ] ],
      [ "<strong>Bridging the FFI Boundary: The Constraints of encoding_c</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md317", [
        [ "<strong>The Panic=Abort Imperative and Exception Safety</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md318", null ],
        [ "<strong>Type Impedance and the Guideline Support Library (GSL)</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md319", null ]
      ] ],
      [ "<strong>Implementation Strategy: Mapping Ranges to Rust Decoders</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md320", [
        [ "<strong>Bypassing Iterator Overhead via Contiguous Specialization</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md321", null ],
        [ "<strong>Managing Non-Contiguous State and Pipeline Eagerness</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md322", null ]
      ] ],
      [ "<strong>Theoretical Foundations: Coinductive Types, Trees, and Traversables</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md323", null ],
      [ "<strong>Monadic Error Handling: Integration with views::maybe and std::optional</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md324", [
        [ "<strong>The Role of views::maybe in Transcoding Pipelines</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md325", null ],
        [ "<strong>std::optional and Monadic Continuations</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md326", null ]
      ] ],
      [ "<strong>The Oracle Paradigm: Fuzzing and Implementor Relief</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md327", [
        [ "<strong>Defining the Testing Oracle</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md328", null ],
        [ "<strong>Differential Fuzzing Strategy</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md329", null ]
      ] ],
      [ "<strong>Managing Mojibake, Identifier Normalization, and Security</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md330", null ],
      [ "<strong>Navigating Build Systems and Ecosystem Integration</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md331", null ],
      [ "<strong>Conclusion</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md332", null ]
    ] ],
    [ "Review of transcode as a C++29 Library Candidate and Beman Inclusion Candidate", "md_docs_transcode_as_a_C__29_Library_Candidate.html", [
      [ "Executive summary", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md335", [
        [ "Audit summary", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md336", null ]
      ] ],
      [ "Repository status and engineering quality", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md337", null ],
      [ "API design and code-level behavior", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md338", [
        [ "Design issues and concrete defects", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md339", null ],
        [ "Targeted code suggestions", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md340", null ]
      ] ],
      [ "Single-byte support and the feasibility of random-access iteration or indexing", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md341", [
        [ "Feasibility matrix for random access", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md342", null ]
      ] ],
      [ "Relationship to P2728", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md343", [
        [ "Current behavior versus P2728-like expectations", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md344", null ],
        [ "Suggested architectural split", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md345", null ]
      ] ],
      [ "Concrete changes, tests to add, and documentation fixes", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md346", null ],
      [ "Acceptance checklist for Beman inclusion and proposal readiness", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md347", [
        [ "Bottom-line recommendation", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md348", null ]
      ] ]
    ] ],
    [ "WHATWG Encoding Standard — Index Data", "md_docs_whatwg_SOURCE.html", [
      [ "File Provenance", "md_docs_whatwg_SOURCE.html#autotoc_md350", null ],
      [ "Full Checksums", "md_docs_whatwg_SOURCE.html#autotoc_md351", null ]
    ] ],
    [ "Web Platform Tests — Encoding Test Vectors", "md_docs_wpt_SOURCE.html", [
      [ "File Provenance", "md_docs_wpt_SOURCE.html#autotoc_md353", null ],
      [ "Full Checksums", "md_docs_wpt_SOURCE.html#autotoc_md354", null ]
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
"md_docs_plans_phase2_index.html#autotoc_md218",
"structbeman_1_1transcoding_1_1whatwg__decode__or__error__closure.html#ab63322e8dacc5d947c887c0fb7654083"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';