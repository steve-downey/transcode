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
    [ "beman.transcode", "index.html#mainpage", null ],
    [ "Benchmark Platform Notes", "md_docs_2benchmarks_2PLATFORM__NOTES.html", [
      [ "<span class=\"tt\">simdutf</span> ceiling baseline (optional — requires explicit CMake opt-in)", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md1", [
        [ "Enabling", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md2", null ],
        [ "Files", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md3", null ],
        [ "Platform availability", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md4", null ],
        [ "Benchmarks provided", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md5", null ],
        [ "Expected comparison", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md6", null ]
      ] ],
      [ "<span class=\"tt\">encoding_rs</span> baseline (optional — requires Rust/Cargo)", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md8", [
        [ "Enabling", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md9", null ],
        [ "Files", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md10", null ],
        [ "Platform availability", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md11", null ],
        [ "Benchmarks provided", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md12", null ],
        [ "Results on this machine (Asan build — indicative only)", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md13", null ]
      ] ],
      [ "<span class=\"tt\">std::codecvt</span> / <span class=\"tt\">std::wstring_convert</span> (negative baseline)", "md_docs_2benchmarks_2PLATFORM__NOTES.html#autotoc_md15", null ]
    ] ],
    [ "Running Benchmarks", "md_docs_2benchmarks_2RUNNING.html", [
      [ "Quick Smoke Run (Default Build)", "md_docs_2benchmarks_2RUNNING.html#autotoc_md42", null ],
      [ "Optimized Builds via CMake Presets", "md_docs_2benchmarks_2RUNNING.html#autotoc_md43", [
        [ "GCC -O3 (smoke)", "md_docs_2benchmarks_2RUNNING.html#autotoc_md44", null ],
        [ "GCC -O3 -flto (smoke)", "md_docs_2benchmarks_2RUNNING.html#autotoc_md45", null ],
        [ "Clang -O3 -flto (smoke)", "md_docs_2benchmarks_2RUNNING.html#autotoc_md46", null ]
      ] ],
      [ "Full Compiler Matrix", "md_docs_2benchmarks_2RUNNING.html#autotoc_md47", null ],
      [ "Capturing Results", "md_docs_2benchmarks_2RUNNING.html#autotoc_md48", null ],
      [ "Environment Metadata", "md_docs_2benchmarks_2RUNNING.html#autotoc_md49", null ],
      [ "Manually Configuring a Preset", "md_docs_2benchmarks_2RUNNING.html#autotoc_md50", null ],
      [ "Matrix Slices", "md_docs_2benchmarks_2RUNNING.html#autotoc_md51", null ],
      [ "Notes", "md_docs_2benchmarks_2RUNNING.html#autotoc_md52", null ]
    ] ],
    [ "Benchmark Corpora — Wikipedia Mars Article", "md_docs_2benchmarks_2SOURCE.html", [
      [ "Purpose", "md_docs_2benchmarks_2SOURCE.html#autotoc_md54", null ],
      [ "Source", "md_docs_2benchmarks_2SOURCE.html#autotoc_md55", null ],
      [ "License", "md_docs_2benchmarks_2SOURCE.html#autotoc_md56", [
        [ "BibTeX", "md_docs_2benchmarks_2SOURCE.html#autotoc_md57", null ]
      ] ],
      [ "Encoding Conversions", "md_docs_2benchmarks_2SOURCE.html#autotoc_md58", null ],
      [ "Fallback Corpus", "md_docs_2benchmarks_2SOURCE.html#autotoc_md59", null ],
      [ "Manifest", "md_docs_2benchmarks_2SOURCE.html#autotoc_md60", null ]
    ] ],
    [ "Testing C++ Range Adapters over <span class=\"tt\">iconv</span>", "md_docs_2iconv-testing.html", [
      [ "1. The <span class=\"tt\">iconv</span> View Boundary Conditions", "md_docs_2iconv-testing.html#autotoc_md62", [
        [ "A. Buffer Exhaustion (<span class=\"tt\">E2BIG</span>)", "md_docs_2iconv-testing.html#autotoc_md63", null ],
        [ "B. The Split Multi-byte Sequence (<span class=\"tt\">EINVAL</span>)", "md_docs_2iconv-testing.html#autotoc_md64", null ],
        [ "C. Invalid Byte Sequences (<span class=\"tt\">EILSEQ</span>)", "md_docs_2iconv-testing.html#autotoc_md65", null ],
        [ "D. End-of-Range Stateful Flush", "md_docs_2iconv-testing.html#autotoc_md66", null ]
      ] ],
      [ "2. Mocking <span class=\"tt\">iconv</span> for Deterministic View Testing", "md_docs_2iconv-testing.html#autotoc_md68", null ],
      [ "3. Testing Execution Plan", "md_docs_2iconv-testing.html#autotoc_md70", null ]
    ] ],
    [ "p2728-alignment", "md_docs_2p2728-alignment.html", [
      [ "Tracking P2728: UTF Transcoding alignment", "md_docs_2p2728-alignment.html#autotoc_md71", [
        [ "How the two relate", "md_docs_2p2728-alignment.html#autotoc_md72", null ],
        [ "The model in brief (P2728R13)", "md_docs_2p2728-alignment.html#autotoc_md73", null ],
        [ "Revision history of the model", "md_docs_2p2728-alignment.html#autotoc_md74", [
          [ "Design intent from committee review", "md_docs_2p2728-alignment.html#autotoc_md75", null ]
        ] ],
        [ "Where <span class=\"tt\">beman.transcode</span> already follows the model", "md_docs_2p2728-alignment.html#autotoc_md76", null ],
        [ "Where it diverges, and how to track", "md_docs_2p2728-alignment.html#autotoc_md77", [
          [ "1. Collapse the <span class=\"tt\">_view</span> / <span class=\"tt\">_or_error_view</span> pairs into one template — <b>done</b>", "md_docs_2p2728-alignment.html#autotoc_md78", null ],
          [ "2. Unspecified underlying type for the error enums (R13)", "md_docs_2p2728-alignment.html#autotoc_md79", null ],
          [ "3. Verify <span class=\"tt\">base()</span> semantics", "md_docs_2p2728-alignment.html#autotoc_md80", null ],
          [ "Intentional, non-convergent differences (do not \"fix\" toward P2728)", "md_docs_2p2728-alignment.html#autotoc_md81", null ]
        ] ],
        [ "Action items", "md_docs_2p2728-alignment.html#autotoc_md82", null ],
        [ "Future bolt-ons that the model leaves open", "md_docs_2p2728-alignment.html#autotoc_md83", null ]
      ] ]
    ] ],
    [ "Benchmark Comparison", "md_docs_2plans_2bench-comparison.html", [
      [ "Goal", "md_docs_2plans_2bench-comparison.html#autotoc_md86", null ],
      [ "Constraints", "md_docs_2plans_2bench-comparison.html#autotoc_md87", null ],
      [ "Model", "md_docs_2plans_2bench-comparison.html#autotoc_md88", null ],
      [ "Stages", "md_docs_2plans_2bench-comparison.html#autotoc_md89", [
        [ "multi-run", "md_docs_2plans_2bench-comparison.html#autotoc_md90", null ],
        [ "machine-load", "md_docs_2plans_2bench-comparison.html#autotoc_md91", null ],
        [ "ratio-views", "md_docs_2plans_2bench-comparison.html#autotoc_md92", null ],
        [ "repeated-runs", "md_docs_2plans_2bench-comparison.html#autotoc_md93", null ],
        [ "cjk-corpora", "md_docs_2plans_2bench-comparison.html#autotoc_md94", null ],
        [ "instruction-counts", "md_docs_2plans_2bench-comparison.html#autotoc_md95", null ],
        [ "runner-stability", "md_docs_2plans_2bench-comparison.html#autotoc_md96", null ]
      ] ],
      [ "Open questions", "md_docs_2plans_2bench-comparison.html#autotoc_md97", [
        [ "match-key", "md_docs_2plans_2bench-comparison.html#autotoc_md98", null ],
        [ "reference-library", "md_docs_2plans_2bench-comparison.html#autotoc_md99", null ],
        [ "change-significance", "md_docs_2plans_2bench-comparison.html#autotoc_md100", null ],
        [ "aggregate-statistic", "md_docs_2plans_2bench-comparison.html#autotoc_md101", null ],
        [ "busy-threshold", "md_docs_2plans_2bench-comparison.html#autotoc_md102", null ],
        [ "count-mode-driver", "md_docs_2plans_2bench-comparison.html#autotoc_md103", null ],
        [ "baseline-storage", "md_docs_2plans_2bench-comparison.html#autotoc_md104", null ],
        [ "runner-trust", "md_docs_2plans_2bench-comparison.html#autotoc_md105", null ]
      ] ]
    ] ],
    [ "Handoff: Coverage Audit Complete — Ready for Phase 5 Planning", "md_docs_2plans_2handoff-next.html", [
      [ "Completed", "md_docs_2plans_2handoff-next.html#autotoc_md107", null ],
      [ "What was done in the Coverage Audit", "md_docs_2plans_2handoff-next.html#autotoc_md108", [
        [ "<span class=\"tt\">tests/beman/transcode/iconv_mock.hpp</span>", "md_docs_2plans_2handoff-next.html#autotoc_md109", null ],
        [ "<span class=\"tt\">tests/beman/transcode/iconv_bulk.test.cpp</span> (+7 tests)", "md_docs_2plans_2handoff-next.html#autotoc_md110", null ],
        [ "<span class=\"tt\">tests/beman/transcode/bulk_transcode.test.cpp</span> (+2 tests)", "md_docs_2plans_2handoff-next.html#autotoc_md111", null ],
        [ "<span class=\"tt\">tests/beman/transcode/whatwg_decode_or_error.test.cpp</span> (+1 test)", "md_docs_2plans_2handoff-next.html#autotoc_md112", null ],
        [ "<span class=\"tt\">tests/beman/transcode/labels.test.cpp</span> (+1 test)", "md_docs_2plans_2handoff-next.html#autotoc_md113", null ],
        [ "After audit coverage", "md_docs_2plans_2handoff-next.html#autotoc_md114", null ]
      ] ],
      [ "Remaining Coverage Gaps (Explained)", "md_docs_2plans_2handoff-next.html#autotoc_md115", null ],
      [ "Current State", "md_docs_2plans_2handoff-next.html#autotoc_md116", null ],
      [ "Branch State", "md_docs_2plans_2handoff-next.html#autotoc_md117", null ],
      [ "What Comes Next", "md_docs_2plans_2handoff-next.html#autotoc_md118", null ]
    ] ],
    [ "P3-Step 1: Benchmark Harness Scaffolding", "md_docs_2plans_2p3-step1-benchmark-harness.html", [
      [ "Goal", "md_docs_2plans_2p3-step1-benchmark-harness.html#autotoc_md121", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step1-benchmark-harness.html#autotoc_md122", [
        [ "Key files to reference", "md_docs_2plans_2p3-step1-benchmark-harness.html#autotoc_md123", null ],
        [ "Build conventions", "md_docs_2plans_2p3-step1-benchmark-harness.html#autotoc_md124", null ],
        [ "Library usage in benchmark code", "md_docs_2plans_2p3-step1-benchmark-harness.html#autotoc_md125", null ]
      ] ],
      [ "Deliverables", "md_docs_2plans_2p3-step1-benchmark-harness.html#autotoc_md126", null ],
      [ "Procedure", "md_docs_2plans_2p3-step1-benchmark-harness.html#autotoc_md127", null ],
      [ "Makefile Target Pattern", "md_docs_2plans_2p3-step1-benchmark-harness.html#autotoc_md128", null ],
      [ "Verification", "md_docs_2plans_2p3-step1-benchmark-harness.html#autotoc_md129", null ],
      [ "Handoff to Step 2", "md_docs_2plans_2p3-step1-benchmark-harness.html#autotoc_md130", null ]
    ] ],
    [ "P3-Step 10: Chunked/Lazy Boundary Stress Benchmarks", "md_docs_2plans_2p3-step10-boundary-stress.html", [
      [ "Goal", "md_docs_2plans_2p3-step10-boundary-stress.html#autotoc_md133", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step10-boundary-stress.html#autotoc_md134", null ],
      [ "Deliverables", "md_docs_2plans_2p3-step10-boundary-stress.html#autotoc_md135", null ],
      [ "Constraints", "md_docs_2plans_2p3-step10-boundary-stress.html#autotoc_md136", null ],
      [ "Procedure", "md_docs_2plans_2p3-step10-boundary-stress.html#autotoc_md137", null ],
      [ "Verification", "md_docs_2plans_2p3-step10-boundary-stress.html#autotoc_md138", null ],
      [ "Notes", "md_docs_2plans_2p3-step10-boundary-stress.html#autotoc_md139", null ],
      [ "Handoff to Step 11", "md_docs_2plans_2p3-step10-boundary-stress.html#autotoc_md140", null ]
    ] ],
    [ "P3-Step 11: Toolchain Matrix Automation", "md_docs_2plans_2p3-step11-toolchain-matrix.html", [
      [ "Goal", "md_docs_2plans_2p3-step11-toolchain-matrix.html#autotoc_md143", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step11-toolchain-matrix.html#autotoc_md144", null ],
      [ "Matrix To Support", "md_docs_2plans_2p3-step11-toolchain-matrix.html#autotoc_md145", null ],
      [ "Deliverables", "md_docs_2plans_2p3-step11-toolchain-matrix.html#autotoc_md146", null ],
      [ "Constraints", "md_docs_2plans_2p3-step11-toolchain-matrix.html#autotoc_md147", null ],
      [ "Procedure", "md_docs_2plans_2p3-step11-toolchain-matrix.html#autotoc_md148", null ],
      [ "Verification", "md_docs_2plans_2p3-step11-toolchain-matrix.html#autotoc_md149", null ],
      [ "Notes", "md_docs_2plans_2p3-step11-toolchain-matrix.html#autotoc_md150", null ],
      [ "Handoff to Step 12", "md_docs_2plans_2p3-step11-toolchain-matrix.html#autotoc_md151", null ]
    ] ],
    [ "P3-Step 12: Reporting, Dashboard, and Reproducibility Pack", "md_docs_2plans_2p3-step12-reporting-and-repro.html", [
      [ "Goal", "md_docs_2plans_2p3-step12-reporting-and-repro.html#autotoc_md154", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step12-reporting-and-repro.html#autotoc_md155", null ],
      [ "Deliverables", "md_docs_2plans_2p3-step12-reporting-and-repro.html#autotoc_md156", null ],
      [ "Files Expected", "md_docs_2plans_2p3-step12-reporting-and-repro.html#autotoc_md157", null ],
      [ "Constraints", "md_docs_2plans_2p3-step12-reporting-and-repro.html#autotoc_md158", null ],
      [ "Procedure", "md_docs_2plans_2p3-step12-reporting-and-repro.html#autotoc_md159", null ],
      [ "Verification", "md_docs_2plans_2p3-step12-reporting-and-repro.html#autotoc_md160", null ],
      [ "Notes", "md_docs_2plans_2p3-step12-reporting-and-repro.html#autotoc_md161", null ],
      [ "Handoff (Phase 3 Complete)", "md_docs_2plans_2p3-step12-reporting-and-repro.html#autotoc_md162", null ]
    ] ],
    [ "P3-Step 2: Benchmark Corpus Acquisition and Provenance", "md_docs_2plans_2p3-step2-benchmark-data.html", [
      [ "Goal", "md_docs_2plans_2p3-step2-benchmark-data.html#autotoc_md165", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step2-benchmark-data.html#autotoc_md166", [
        [ "MediaWiki API for extraction", "md_docs_2plans_2p3-step2-benchmark-data.html#autotoc_md167", null ],
        [ "Python encoding conversion", "md_docs_2plans_2p3-step2-benchmark-data.html#autotoc_md168", null ]
      ] ],
      [ "Deliverables", "md_docs_2plans_2p3-step2-benchmark-data.html#autotoc_md169", null ],
      [ "Corpus Manifest Schema", "md_docs_2plans_2p3-step2-benchmark-data.html#autotoc_md170", null ],
      [ "Fallback Corpus (checked in)", "md_docs_2plans_2p3-step2-benchmark-data.html#autotoc_md171", null ],
      [ "Procedure", "md_docs_2plans_2p3-step2-benchmark-data.html#autotoc_md172", null ],
      [ "Verification", "md_docs_2plans_2p3-step2-benchmark-data.html#autotoc_md173", null ],
      [ "Handoff to Step 3", "md_docs_2plans_2p3-step2-benchmark-data.html#autotoc_md174", null ]
    ] ],
    [ "P3-Step 3: Benchmark Fixture Library and Result Schema", "md_docs_2plans_2p3-step3-benchmark-fixtures.html", [
      [ "Goal", "md_docs_2plans_2p3-step3-benchmark-fixtures.html#autotoc_md177", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step3-benchmark-fixtures.html#autotoc_md178", null ],
      [ "Deliverables", "md_docs_2plans_2p3-step3-benchmark-fixtures.html#autotoc_md179", null ],
      [ "API Design", "md_docs_2plans_2p3-step3-benchmark-fixtures.html#autotoc_md180", [
        [ "benchmark_fixture.hpp", "md_docs_2plans_2p3-step3-benchmark-fixtures.html#autotoc_md181", null ],
        [ "benchmark_sink.hpp", "md_docs_2plans_2p3-step3-benchmark-fixtures.html#autotoc_md182", null ]
      ] ],
      [ "Procedure", "md_docs_2plans_2p3-step3-benchmark-fixtures.html#autotoc_md183", null ],
      [ "Verification", "md_docs_2plans_2p3-step3-benchmark-fixtures.html#autotoc_md184", null ],
      [ "Handoff to Step 4", "md_docs_2plans_2p3-step3-benchmark-fixtures.html#autotoc_md185", null ]
    ] ],
    [ "P3-Step 4: UTF-Family and ASCII Fast-Path Benchmarks", "md_docs_2plans_2p3-step4-utf-benchmarks.html", [
      [ "Goal", "md_docs_2plans_2p3-step4-utf-benchmarks.html#autotoc_md188", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step4-utf-benchmarks.html#autotoc_md189", null ],
      [ "Deliverables", "md_docs_2plans_2p3-step4-utf-benchmarks.html#autotoc_md190", null ],
      [ "Benchmark Cases", "md_docs_2plans_2p3-step4-utf-benchmarks.html#autotoc_md191", null ],
      [ "Procedure", "md_docs_2plans_2p3-step4-utf-benchmarks.html#autotoc_md192", null ],
      [ "Verification", "md_docs_2plans_2p3-step4-utf-benchmarks.html#autotoc_md193", null ],
      [ "Handoff to Step 5", "md_docs_2plans_2p3-step4-utf-benchmarks.html#autotoc_md194", null ]
    ] ],
    [ "P3-Step 5: Legacy WHATWG Codec Benchmarks", "md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html", [
      [ "Goal", "md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html#autotoc_md197", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html#autotoc_md198", null ],
      [ "Deliverables", "md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html#autotoc_md199", null ],
      [ "Benchmark Cases", "md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html#autotoc_md200", [
        [ "whatwg_benchmarks.bench.cpp", "md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html#autotoc_md201", null ],
        [ "pluggable_codec_benchmarks.bench.cpp", "md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html#autotoc_md202", null ]
      ] ],
      [ "Procedure", "md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html#autotoc_md203", null ],
      [ "Verification", "md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html#autotoc_md204", null ],
      [ "Handoff to Step 6", "md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html#autotoc_md205", null ]
    ] ],
    [ "P3-Step 6: <span class=\"tt\">iconv</span> Baselines", "md_docs_2plans_2p3-step6-iconv-baselines.html", [
      [ "Goal", "md_docs_2plans_2p3-step6-iconv-baselines.html#autotoc_md208", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step6-iconv-baselines.html#autotoc_md209", null ],
      [ "Deliverables", "md_docs_2plans_2p3-step6-iconv-baselines.html#autotoc_md210", null ],
      [ "Benchmark Cases", "md_docs_2plans_2p3-step6-iconv-baselines.html#autotoc_md211", null ],
      [ "Constraints", "md_docs_2plans_2p3-step6-iconv-baselines.html#autotoc_md212", null ],
      [ "Procedure", "md_docs_2plans_2p3-step6-iconv-baselines.html#autotoc_md213", null ],
      [ "Verification", "md_docs_2plans_2p3-step6-iconv-baselines.html#autotoc_md214", null ],
      [ "Handoff to Step 7", "md_docs_2plans_2p3-step6-iconv-baselines.html#autotoc_md215", null ]
    ] ],
    [ "P3-Step 7: <span class=\"tt\">std::codecvt</span> Negative Baseline", "md_docs_2plans_2p3-step7-codecvt-baseline.html", [
      [ "Goal", "md_docs_2plans_2p3-step7-codecvt-baseline.html#autotoc_md218", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step7-codecvt-baseline.html#autotoc_md219", null ],
      [ "Deliverables", "md_docs_2plans_2p3-step7-codecvt-baseline.html#autotoc_md220", null ],
      [ "Constraints", "md_docs_2plans_2p3-step7-codecvt-baseline.html#autotoc_md221", null ],
      [ "Procedure", "md_docs_2plans_2p3-step7-codecvt-baseline.html#autotoc_md222", null ],
      [ "Verification", "md_docs_2plans_2p3-step7-codecvt-baseline.html#autotoc_md223", null ],
      [ "Notes", "md_docs_2plans_2p3-step7-codecvt-baseline.html#autotoc_md224", null ],
      [ "Handoff to Step 8", "md_docs_2plans_2p3-step7-codecvt-baseline.html#autotoc_md225", null ]
    ] ],
    [ "P3-Step 8: <span class=\"tt\">encoding_rs</span> Baseline", "md_docs_2plans_2p3-step8-encoding-rs-baseline.html", [
      [ "Goal", "md_docs_2plans_2p3-step8-encoding-rs-baseline.html#autotoc_md228", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step8-encoding-rs-baseline.html#autotoc_md229", null ],
      [ "Deliverables", "md_docs_2plans_2p3-step8-encoding-rs-baseline.html#autotoc_md230", null ],
      [ "Constraints", "md_docs_2plans_2p3-step8-encoding-rs-baseline.html#autotoc_md231", null ],
      [ "Procedure", "md_docs_2plans_2p3-step8-encoding-rs-baseline.html#autotoc_md232", null ],
      [ "Verification", "md_docs_2plans_2p3-step8-encoding-rs-baseline.html#autotoc_md233", null ],
      [ "Notes", "md_docs_2plans_2p3-step8-encoding-rs-baseline.html#autotoc_md234", null ],
      [ "Handoff to Step 9", "md_docs_2plans_2p3-step8-encoding-rs-baseline.html#autotoc_md235", null ]
    ] ],
    [ "P3-Step 9: <span class=\"tt\">simdutf</span> Ceiling Baseline", "md_docs_2plans_2p3-step9-simdutf-baseline.html", [
      [ "Goal", "md_docs_2plans_2p3-step9-simdutf-baseline.html#autotoc_md238", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p3-step9-simdutf-baseline.html#autotoc_md239", null ],
      [ "Deliverables", "md_docs_2plans_2p3-step9-simdutf-baseline.html#autotoc_md240", null ],
      [ "Constraints", "md_docs_2plans_2p3-step9-simdutf-baseline.html#autotoc_md241", null ],
      [ "Procedure", "md_docs_2plans_2p3-step9-simdutf-baseline.html#autotoc_md242", null ],
      [ "Verification", "md_docs_2plans_2p3-step9-simdutf-baseline.html#autotoc_md243", null ],
      [ "Notes", "md_docs_2plans_2p3-step9-simdutf-baseline.html#autotoc_md244", null ],
      [ "Handoff to Step 10", "md_docs_2plans_2p3-step9-simdutf-baseline.html#autotoc_md245", null ]
    ] ],
    [ "P4-Step 1: Pluggable Encode View", "md_docs_2plans_2p4-step1-pluggable-encode.html", [
      [ "Goal", "md_docs_2plans_2p4-step1-pluggable-encode.html#autotoc_md248", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p4-step1-pluggable-encode.html#autotoc_md249", [
        [ "Key files to reference", "md_docs_2plans_2p4-step1-pluggable-encode.html#autotoc_md250", null ],
        [ "Key differences from decode_view", "md_docs_2plans_2p4-step1-pluggable-encode.html#autotoc_md251", null ],
        [ "The encode_codec concept", "md_docs_2plans_2p4-step1-pluggable-encode.html#autotoc_md252", null ]
      ] ],
      [ "Deliverables", "md_docs_2plans_2p4-step1-pluggable-encode.html#autotoc_md253", null ],
      [ "Constraints", "md_docs_2plans_2p4-step1-pluggable-encode.html#autotoc_md254", null ],
      [ "Procedure", "md_docs_2plans_2p4-step1-pluggable-encode.html#autotoc_md255", null ],
      [ "Verification", "md_docs_2plans_2p4-step1-pluggable-encode.html#autotoc_md256", null ],
      [ "Handoff to Step 2", "md_docs_2plans_2p4-step1-pluggable-encode.html#autotoc_md257", null ]
    ] ],
    [ "P4-Step 2: Pluggable Bulk Operations", "md_docs_2plans_2p4-step2-pluggable-bulk.html", [
      [ "Goal", "md_docs_2plans_2p4-step2-pluggable-bulk.html#autotoc_md260", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p4-step2-pluggable-bulk.html#autotoc_md261", [
        [ "Signature difference", "md_docs_2plans_2p4-step2-pluggable-bulk.html#autotoc_md262", null ],
        [ "Fast path opportunity", "md_docs_2plans_2p4-step2-pluggable-bulk.html#autotoc_md263", null ]
      ] ],
      [ "Deliverables", "md_docs_2plans_2p4-step2-pluggable-bulk.html#autotoc_md264", null ],
      [ "Procedure", "md_docs_2plans_2p4-step2-pluggable-bulk.html#autotoc_md265", null ],
      [ "Verification", "md_docs_2plans_2p4-step2-pluggable-bulk.html#autotoc_md266", null ],
      [ "Handoff to Step 3", "md_docs_2plans_2p4-step2-pluggable-bulk.html#autotoc_md267", null ]
    ] ],
    [ "P4-Step 3: Pluggable Transcode Pipeline", "md_docs_2plans_2p4-step3-pluggable-transcode.html", [
      [ "Goal", "md_docs_2plans_2p4-step3-pluggable-transcode.html#autotoc_md270", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p4-step3-pluggable-transcode.html#autotoc_md271", [
        [ "Implementation", "md_docs_2plans_2p4-step3-pluggable-transcode.html#autotoc_md272", null ]
      ] ],
      [ "Deliverables", "md_docs_2plans_2p4-step3-pluggable-transcode.html#autotoc_md273", null ],
      [ "Procedure", "md_docs_2plans_2p4-step3-pluggable-transcode.html#autotoc_md274", null ],
      [ "Verification", "md_docs_2plans_2p4-step3-pluggable-transcode.html#autotoc_md275", null ],
      [ "Handoff", "md_docs_2plans_2p4-step3-pluggable-transcode.html#autotoc_md276", null ]
    ] ],
    [ "P4-Step 4: iconv Bulk Operations", "md_docs_2plans_2p4-step4-iconv-bulk.html", [
      [ "Goal", "md_docs_2plans_2p4-step4-iconv-bulk.html#autotoc_md279", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p4-step4-iconv-bulk.html#autotoc_md280", [
        [ "Dependency injection", "md_docs_2plans_2p4-step4-iconv-bulk.html#autotoc_md281", null ],
        [ "Key files to reference", "md_docs_2plans_2p4-step4-iconv-bulk.html#autotoc_md282", null ]
      ] ],
      [ "Deliverables", "md_docs_2plans_2p4-step4-iconv-bulk.html#autotoc_md283", null ],
      [ "Constraints", "md_docs_2plans_2p4-step4-iconv-bulk.html#autotoc_md284", null ],
      [ "Procedure", "md_docs_2plans_2p4-step4-iconv-bulk.html#autotoc_md285", null ],
      [ "Verification", "md_docs_2plans_2p4-step4-iconv-bulk.html#autotoc_md286", null ],
      [ "Handoff to Step 5", "md_docs_2plans_2p4-step4-iconv-bulk.html#autotoc_md287", null ]
    ] ],
    [ "P4-Step 5: iconv Null-Terminated Input", "md_docs_2plans_2p4-step5-iconv-null-term.html", [
      [ "Goal", "md_docs_2plans_2p4-step5-iconv-null-term.html#autotoc_md290", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p4-step5-iconv-null-term.html#autotoc_md291", [
        [ "Possible outcomes", "md_docs_2plans_2p4-step5-iconv-null-term.html#autotoc_md292", null ]
      ] ],
      [ "Deliverables", "md_docs_2plans_2p4-step5-iconv-null-term.html#autotoc_md293", null ],
      [ "Procedure", "md_docs_2plans_2p4-step5-iconv-null-term.html#autotoc_md294", null ],
      [ "Verification", "md_docs_2plans_2p4-step5-iconv-null-term.html#autotoc_md295", null ],
      [ "Handoff to Step 6", "md_docs_2plans_2p4-step5-iconv-null-term.html#autotoc_md296", null ]
    ] ],
    [ "P4-Step 6: Error Enum Coherence", "md_docs_2plans_2p4-step6-error-coherence.html", [
      [ "Goal", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md299", null ],
      [ "Context for Executing Agent", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md300", [
        [ "whatwg_error (include/beman/transcode/detail/error.hpp)", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md301", null ],
        [ "decode_error (include/beman/transcode/detail/codec_result.hpp)", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md302", null ],
        [ "iconv_error (include/beman/transcode/detail/error.hpp)", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md303", null ],
        [ "Why they differ", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md304", null ],
        [ "The problem", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md305", null ]
      ] ],
      [ "Deliverables", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md306", null ],
      [ "Procedure", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md307", null ],
      [ "Verification", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md308", null ],
      [ "Handoff (Phase 4 Complete)", "md_docs_2plans_2p4-step6-error-coherence.html#autotoc_md309", null ]
    ] ],
    [ "P5-Step 1: specgen Harness and Paper Transclusion", "md_docs_2plans_2p5-step1-specgen-harness.html", [
      [ "Goal", "md_docs_2plans_2p5-step1-specgen-harness.html#autotoc_md312", null ],
      [ "Context for executing agent", "md_docs_2plans_2p5-step1-specgen-harness.html#autotoc_md313", null ],
      [ "Tasks", "md_docs_2plans_2p5-step1-specgen-harness.html#autotoc_md314", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step1-specgen-harness.html#autotoc_md315", null ],
      [ "Notes", "md_docs_2plans_2p5-step1-specgen-harness.html#autotoc_md316", null ]
    ] ],
    [ "P5-Step 10: Paper Assembly and the Drift Gate", "md_docs_2plans_2p5-step10-paper-assembly.html", [
      [ "Goal", "md_docs_2plans_2p5-step10-paper-assembly.html#autotoc_md319", null ],
      [ "Tasks", "md_docs_2plans_2p5-step10-paper-assembly.html#autotoc_md320", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step10-paper-assembly.html#autotoc_md321", null ],
      [ "Outcome (2026-09-08)", "md_docs_2plans_2p5-step10-paper-assembly.html#autotoc_md323", [
        [ "Three things that were wrong and are not any more", "md_docs_2plans_2p5-step10-paper-assembly.html#autotoc_md324", null ],
        [ "Task 7: the code blocks, and one thing this step did not fix", "md_docs_2plans_2p5-step10-paper-assembly.html#autotoc_md325", null ],
        [ "The drift gate, and the workflow that did not survive", "md_docs_2plans_2p5-step10-paper-assembly.html#autotoc_md326", null ]
      ] ]
    ] ],
    [ "P5-Step 2: Wording Outline and Stable Names", "md_docs_2plans_2p5-step2-wording-outline.html", [
      [ "Goal", "md_docs_2plans_2p5-step2-wording-outline.html#autotoc_md329", null ],
      [ "Context for executing agent", "md_docs_2plans_2p5-step2-wording-outline.html#autotoc_md330", null ],
      [ "Tasks", "md_docs_2plans_2p5-step2-wording-outline.html#autotoc_md331", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step2-wording-outline.html#autotoc_md332", null ]
    ] ],
    [ "P5-Step 3: Spec-Facing Header Shape", "md_docs_2plans_2p5-step3-spec-header-shape.html", [
      [ "Goal", "md_docs_2plans_2p5-step3-spec-header-shape.html#autotoc_md335", null ],
      [ "Context for executing agent", "md_docs_2plans_2p5-step3-spec-header-shape.html#autotoc_md336", null ],
      [ "Tasks", "md_docs_2plans_2p5-step3-spec-header-shape.html#autotoc_md337", null ],
      [ "Outcome, and what is deferred", "md_docs_2plans_2p5-step3-spec-header-shape.html#autotoc_md338", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step3-spec-header-shape.html#autotoc_md339", null ],
      [ "Notes", "md_docs_2plans_2p5-step3-spec-header-shape.html#autotoc_md340", null ]
    ] ],
    [ "P5-Step 3b: Push the Codec State Machines Down", "md_docs_2plans_2p5-step3b-codec-pushdown.html", [
      [ "Goal", "md_docs_2plans_2p5-step3b-codec-pushdown.html#autotoc_md343", null ],
      [ "What was done", "md_docs_2plans_2p5-step3b-codec-pushdown.html#autotoc_md344", null ],
      [ "What was not done, and the order to do it in", "md_docs_2plans_2p5-step3b-codec-pushdown.html#autotoc_md345", null ],
      [ "What this means for Step 3 task 4", "md_docs_2plans_2p5-step3b-codec-pushdown.html#autotoc_md346", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step3b-codec-pushdown.html#autotoc_md347", null ]
    ] ],
    [ "P5-Step 4: Markup — Errors, Range Requirements, <span class=\"tt\">null_term</span>", "md_docs_2plans_2p5-step4-errors-concepts-null-term.html", [
      [ "Goal", "md_docs_2plans_2p5-step4-errors-concepts-null-term.html#autotoc_md350", null ],
      [ "The markup loop", "md_docs_2plans_2p5-step4-errors-concepts-null-term.html#autotoc_md351", null ],
      [ "What is different here", "md_docs_2plans_2p5-step4-errors-concepts-null-term.html#autotoc_md352", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step4-errors-concepts-null-term.html#autotoc_md353", null ],
      [ "Outcome (2026-09-06)", "md_docs_2plans_2p5-step4-errors-concepts-null-term.html#autotoc_md355", null ]
    ] ],
    [ "P5-Step 5: Markup — Encodings, Labels, BOM Sniffing", "md_docs_2plans_2p5-step5-codec-labels-sniff.html", [
      [ "Goal", "md_docs_2plans_2p5-step5-codec-labels-sniff.html#autotoc_md358", null ],
      [ "What is different here", "md_docs_2plans_2p5-step5-codec-labels-sniff.html#autotoc_md359", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step5-codec-labels-sniff.html#autotoc_md360", null ],
      [ "Outcome (2026-09-07)", "md_docs_2plans_2p5-step5-codec-labels-sniff.html#autotoc_md362", null ]
    ] ],
    [ "P5-Step 6: Markup — WHATWG Decode and Encode Views", "md_docs_2plans_2p5-step6-whatwg-views.html", [
      [ "Goal", "md_docs_2plans_2p5-step6-whatwg-views.html#autotoc_md365", null ],
      [ "What is different here", "md_docs_2plans_2p5-step6-whatwg-views.html#autotoc_md366", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step6-whatwg-views.html#autotoc_md367", null ],
      [ "Outcome (2026-09-08)", "md_docs_2plans_2p5-step6-whatwg-views.html#autotoc_md369", null ]
    ] ],
    [ "P5-Step 7: Markup — Codec Requirements and Pluggable Views", "md_docs_2plans_2p5-step7-pluggable-codecs.html", [
      [ "Goal", "md_docs_2plans_2p5-step7-pluggable-codecs.html#autotoc_md372", null ],
      [ "What is different here", "md_docs_2plans_2p5-step7-pluggable-codecs.html#autotoc_md373", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step7-pluggable-codecs.html#autotoc_md374", null ],
      [ "Outcome (2026-09-08)", "md_docs_2plans_2p5-step7-pluggable-codecs.html#autotoc_md376", null ]
    ] ],
    [ "P5-Step 8: Markup — Pipelines and Eager Transcoding", "md_docs_2plans_2p5-step8-transcode-pipeline.html", [
      [ "Goal", "md_docs_2plans_2p5-step8-transcode-pipeline.html#autotoc_md379", null ],
      [ "What is different here", "md_docs_2plans_2p5-step8-transcode-pipeline.html#autotoc_md380", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step8-transcode-pipeline.html#autotoc_md381", null ],
      [ "Outcome (2026-09-08)", "md_docs_2plans_2p5-step8-transcode-pipeline.html#autotoc_md383", null ]
    ] ],
    [ "P5-Step 9: Markup — iconv Adaptors", "md_docs_2plans_2p5-step9-iconv.html", [
      [ "Goal", "md_docs_2plans_2p5-step9-iconv.html#autotoc_md386", null ],
      [ "What is different here", "md_docs_2plans_2p5-step9-iconv.html#autotoc_md387", null ],
      [ "Acceptance", "md_docs_2plans_2p5-step9-iconv.html#autotoc_md388", null ],
      [ "Outcome (2026-09-08)", "md_docs_2plans_2p5-step9-iconv.html#autotoc_md390", [
        [ "Four markup faults, all of them placement", "md_docs_2plans_2p5-step9-iconv.html#autotoc_md391", null ],
        [ "W1 reaches the borrowed-range specializations", "md_docs_2plans_2p5-step9-iconv.html#autotoc_md392", null ],
        [ "One specgen defect, the last one standing", "md_docs_2plans_2p5-step9-iconv.html#autotoc_md393", null ]
      ] ]
    ] ],
    [ "P6-Step 0: Revise the Wording Staleness Gate", "md_docs_2plans_2p6-step0-wording-gate.html", [
      [ "Goal", "md_docs_2plans_2p6-step0-wording-gate.html#autotoc_md396", null ],
      [ "Why", "md_docs_2plans_2p6-step0-wording-gate.html#autotoc_md397", null ],
      [ "Design", "md_docs_2plans_2p6-step0-wording-gate.html#autotoc_md398", null ],
      [ "What to change", "md_docs_2plans_2p6-step0-wording-gate.html#autotoc_md399", null ],
      [ "Tests", "md_docs_2plans_2p6-step0-wording-gate.html#autotoc_md400", null ],
      [ "Done when", "md_docs_2plans_2p6-step0-wording-gate.html#autotoc_md401", null ],
      [ "Note on specgen", "md_docs_2plans_2p6-step0-wording-gate.html#autotoc_md402", null ]
    ] ],
    [ "P6-Step 1: <span class=\"tt\">sniff_encoding</span> Requires a Forward Range", "md_docs_2plans_2p6-step1-sniff-forward-range.html", [
      [ "Goal", "md_docs_2plans_2p6-step1-sniff-forward-range.html#autotoc_md405", null ],
      [ "Why", "md_docs_2plans_2p6-step1-sniff-forward-range.html#autotoc_md406", null ],
      [ "What to change", "md_docs_2plans_2p6-step1-sniff-forward-range.html#autotoc_md407", null ],
      [ "Tests", "md_docs_2plans_2p6-step1-sniff-forward-range.html#autotoc_md408", null ],
      [ "Watch for", "md_docs_2plans_2p6-step1-sniff-forward-range.html#autotoc_md409", null ],
      [ "Done when", "md_docs_2plans_2p6-step1-sniff-forward-range.html#autotoc_md410", null ]
    ] ],
    [ "P6-Step 10: Verification", "md_docs_2plans_2p6-step10-verification.html", [
      [ "Goal", "md_docs_2plans_2p6-step10-verification.html#autotoc_md413", null ],
      [ "Why a step of its own", "md_docs_2plans_2p6-step10-verification.html#autotoc_md414", null ],
      [ "What to run", "md_docs_2plans_2p6-step10-verification.html#autotoc_md415", [
        [ "Build matrix", "md_docs_2plans_2p6-step10-verification.html#autotoc_md416", null ],
        [ "Install and consume", "md_docs_2plans_2p6-step10-verification.html#autotoc_md417", null ],
        [ "Examples", "md_docs_2plans_2p6-step10-verification.html#autotoc_md418", null ],
        [ "Wording", "md_docs_2plans_2p6-step10-verification.html#autotoc_md419", null ],
        [ "The tools the review could not run", "md_docs_2plans_2p6-step10-verification.html#autotoc_md420", null ],
        [ "Platform scope", "md_docs_2plans_2p6-step10-verification.html#autotoc_md421", null ],
        [ "Coverage", "md_docs_2plans_2p6-step10-verification.html#autotoc_md422", null ]
      ] ],
      [ "Report", "md_docs_2plans_2p6-step10-verification.html#autotoc_md423", null ],
      [ "Done when", "md_docs_2plans_2p6-step10-verification.html#autotoc_md424", null ]
    ] ],
    [ "P6-Step 2: The iconv Error Model", "md_docs_2plans_2p6-step2-iconv-error-model.html", [
      [ "Goal", "md_docs_2plans_2p6-step2-iconv-error-model.html#autotoc_md427", null ],
      [ "Why", "md_docs_2plans_2p6-step2-iconv-error-model.html#autotoc_md428", null ],
      [ "What to change", "md_docs_2plans_2p6-step2-iconv-error-model.html#autotoc_md429", [
        [ "<span class=\"tt\">error.hpp</span>", "md_docs_2plans_2p6-step2-iconv-error-model.html#autotoc_md430", null ],
        [ "The two streaming views", "md_docs_2plans_2p6-step2-iconv-error-model.html#autotoc_md431", null ],
        [ "The bulk helpers", "md_docs_2plans_2p6-step2-iconv-error-model.html#autotoc_md432", null ]
      ] ],
      [ "Tests", "md_docs_2plans_2p6-step2-iconv-error-model.html#autotoc_md433", null ],
      [ "Done when", "md_docs_2plans_2p6-step2-iconv-error-model.html#autotoc_md434", null ],
      [ "Hand off to Step 8", "md_docs_2plans_2p6-step2-iconv-error-model.html#autotoc_md435", null ]
    ] ],
    [ "P6-Step 3: One Lossy Policy — Skip, Do Not Replace", "md_docs_2plans_2p6-step3-iconv-lossy-skip.html", [
      [ "Goal", "md_docs_2plans_2p6-step3-iconv-lossy-skip.html#autotoc_md438", null ],
      [ "Why", "md_docs_2plans_2p6-step3-iconv-lossy-skip.html#autotoc_md439", null ],
      [ "What to change", "md_docs_2plans_2p6-step3-iconv-lossy-skip.html#autotoc_md440", null ],
      [ "Tests", "md_docs_2plans_2p6-step3-iconv-lossy-skip.html#autotoc_md441", null ],
      [ "Done when", "md_docs_2plans_2p6-step3-iconv-lossy-skip.html#autotoc_md442", null ]
    ] ],
    [ "P6-Step 4: <span class=\"tt\">null_term_view</span> Says What It Requires", "md_docs_2plans_2p6-step4-null-term-precondition.html", [
      [ "Goal", "md_docs_2plans_2p6-step4-null-term-precondition.html#autotoc_md445", null ],
      [ "Why", "md_docs_2plans_2p6-step4-null-term-precondition.html#autotoc_md446", null ],
      [ "What to change", "md_docs_2plans_2p6-step4-null-term-precondition.html#autotoc_md447", null ],
      [ "Tests", "md_docs_2plans_2p6-step4-null-term-precondition.html#autotoc_md448", null ],
      [ "Done when", "md_docs_2plans_2p6-step4-null-term-precondition.html#autotoc_md449", null ]
    ] ],
    [ "P6-Step 5: <span class=\"tt\">transcode_string</span> Stops Signalling Failure With Emptiness", "md_docs_2plans_2p6-step5-transcode-string-result.html", [
      [ "Goal", "md_docs_2plans_2p6-step5-transcode-string-result.html#autotoc_md452", null ],
      [ "Why", "md_docs_2plans_2p6-step5-transcode-string-result.html#autotoc_md453", null ],
      [ "What to change", "md_docs_2plans_2p6-step5-transcode-string-result.html#autotoc_md454", null ],
      [ "Callers", "md_docs_2plans_2p6-step5-transcode-string-result.html#autotoc_md455", null ],
      [ "Tests", "md_docs_2plans_2p6-step5-transcode-string-result.html#autotoc_md456", null ],
      [ "Done when", "md_docs_2plans_2p6-step5-transcode-string-result.html#autotoc_md457", null ],
      [ "Note", "md_docs_2plans_2p6-step5-transcode-string-result.html#autotoc_md458", null ]
    ] ],
    [ "P6-Step 6: UTF-32 Input Validation, Specified and Uniform", "md_docs_2plans_2p6-step6-utf32-validation.html", [
      [ "Goal", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md461", null ],
      [ "Why", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md462", [
        [ "The precondition was attributed to the wrong layer", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md463", null ],
        [ "And the encoder-layer reading gives an answer nobody wants", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md464", null ],
        [ "The implementation already does the right thing", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md465", null ],
        [ "C-08: the diagnosis is not uniform (new)", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md466", null ],
        [ "D-15: the documentation says <span class=\"tt\">'?'</span> and UTF-8 says otherwise (new)", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md467", null ]
      ] ],
      [ "What to change", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md468", [
        [ "Validate above dispatch", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md469", null ],
        [ "Say what is now guaranteed", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md470", null ],
        [ "The WPT test stays, re-attributed", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md471", null ]
      ] ],
      [ "Tests", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md472", null ],
      [ "Done when", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md473", null ],
      [ "Hand off to Step 8", "md_docs_2plans_2p6-step6-utf32-validation.html#autotoc_md474", null ]
    ] ],
    [ "P6-Step 7: U+FFFD Is Reserved, and the Cost Is Stated", "md_docs_2plans_2p6-step7-fffd-reservation.html", [
      [ "Goal", "md_docs_2plans_2p6-step7-fffd-reservation.html#autotoc_md477", null ],
      [ "Why", "md_docs_2plans_2p6-step7-fffd-reservation.html#autotoc_md478", null ],
      [ "What to change", "md_docs_2plans_2p6-step7-fffd-reservation.html#autotoc_md479", null ],
      [ "Paper", "md_docs_2plans_2p6-step7-fffd-reservation.html#autotoc_md480", null ],
      [ "Tests", "md_docs_2plans_2p6-step7-fffd-reservation.html#autotoc_md481", null ],
      [ "Done when", "md_docs_2plans_2p6-step7-fffd-reservation.html#autotoc_md482", null ]
    ] ],
    [ "P6-Step 8: The Paper, and One Regeneration", "md_docs_2plans_2p6-step8-paper-and-wording.html", [
      [ "Goal", "md_docs_2plans_2p6-step8-paper-and-wording.html#autotoc_md485", null ],
      [ "Before anything else: build specgen", "md_docs_2plans_2p6-step8-paper-and-wording.html#autotoc_md486", null ],
      [ "P-02: an ISO synopsis that names a POSIX type", "md_docs_2plans_2p6-step8-paper-and-wording.html#autotoc_md487", null ],
      [ "P-01: the optional-iconv story", "md_docs_2plans_2p6-step8-paper-and-wording.html#autotoc_md488", null ],
      [ "S-04: prose in the synopsis", "md_docs_2plans_2p6-step8-paper-and-wording.html#autotoc_md489", null ],
      [ "S-05: claims narrower than their evidence", "md_docs_2plans_2p6-step8-paper-and-wording.html#autotoc_md490", null ],
      [ "The drift items", "md_docs_2plans_2p6-step8-paper-and-wording.html#autotoc_md491", null ],
      [ "Carried in from earlier steps", "md_docs_2plans_2p6-step8-paper-and-wording.html#autotoc_md492", null ],
      [ "Order of work", "md_docs_2plans_2p6-step8-paper-and-wording.html#autotoc_md493", null ],
      [ "Done when", "md_docs_2plans_2p6-step8-paper-and-wording.html#autotoc_md494", null ]
    ] ],
    [ "P6-Step 9: Documentation, Build and CI Drift", "md_docs_2plans_2p6-step9-docs-build-ci.html", [
      [ "Goal", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md497", null ],
      [ "Why together", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md498", null ],
      [ "The items", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md499", [
        [ "D-05 — the build instructions name the wrong framework and the wrong standard", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md500", null ],
        [ "D-06 — the compiler matrix disagrees with the configure check", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md501", null ],
        [ "D-07 — <span class=\"tt\">table_codec</span> is documented as public API and is not", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md502", null ],
        [ "D-08 — ICU does not expose the iconv API", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md503", null ],
        [ "D-09 — a tracked file named <span class=\"tt\">-</span>", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md504", null ],
        [ "D-10 — <span class=\"tt\">docs/plans/handoff-next.md</span> reads as current", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md505", null ],
        [ "D-11 — \"Manual Lint\" runs the ordinary lint", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md506", null ],
        [ "D-12 — module packaging is untested and the interface unit has no licence", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md507", null ],
        [ "D-14 — the gitleaks hook is broken (not in the review)", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md508", null ],
        [ "D-13 — shebang on line 2", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md509", null ]
      ] ],
      [ "Repository hygiene", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md510", null ],
      [ "Done when", "md_docs_2plans_2p6-step9-docs-build-ci.html#autotoc_md511", null ]
    ] ],
    [ "Phase 2 Progress Checklist", "md_docs_2plans_2phase2-checklist.html", [
      [ "Step 14: <span class=\"tt\">codec::replacement</span> (<span class=\"tt\">step14-replacement-codec</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md514", null ],
      [ "Step 15: <span class=\"tt\">codec::x_user_defined</span> (<span class=\"tt\">step15-x-user-defined</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md515", null ],
      [ "Step 16: Single-byte infra + <span class=\"tt\">windows_1252</span> (<span class=\"tt\">step16-single-byte-infra</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md516", null ],
      [ "Step 17: Data tooling (<span class=\"tt\">step17-data-tooling</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md517", null ],
      [ "Step 18: All single-byte decoders (<span class=\"tt\">step18-all-single-byte-decode</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md518", null ],
      [ "Step 19: Single-byte encoder infra + all encoders (<span class=\"tt\">step19-single-byte-encode</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md519", null ],
      [ "Step 20: UTF-8 encoder (<span class=\"tt\">step20-utf8-encoder</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md520", null ],
      [ "Step 21: UTF-16 decode + encode (<span class=\"tt\">step21-utf16</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md521", null ],
      [ "Step 22: GBK decode + encode (<span class=\"tt\">step22-gbk</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md522", null ],
      [ "Step 23: gb18030 decode + encode (<span class=\"tt\">step23-gb18030</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md523", null ],
      [ "Step 24: Big5 decode + encode (<span class=\"tt\">step24-big5</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md524", null ],
      [ "Step 25: Shift_JIS decode + encode (<span class=\"tt\">step25-shift-jis</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md525", null ],
      [ "Step 26: EUC-JP decode + encode (<span class=\"tt\">step26-euc-jp</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md526", null ],
      [ "Step 27: ISO-2022-JP decode + encode (<span class=\"tt\">step27-iso-2022-jp</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md527", null ],
      [ "Step 28: EUC-KR decode + encode (<span class=\"tt\">step28-euc-kr</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md528", null ],
      [ "Step 29: Round-trip composition (<span class=\"tt\">step29-roundtrip</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md529", null ],
      [ "Step 30: WPT test vector integration (<span class=\"tt\">step30-wpt-vectors</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md531", null ],
      [ "Step 31: WPT ISO-2022-JP + single-byte exhaustive (<span class=\"tt\">step31-wpt-iso2022jp-singlebyte</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md532", null ],
      [ "Step 32: ISO-2022-JP full WHATWG conformance (<span class=\"tt\">step32-iso2022jp-conformance</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md533", null ],
      [ "Step 33: WPT UTF-16 surrogate conformance (<span class=\"tt\">step33-wpt-utf16-surrogates</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md534", null ],
      [ "Step 34: WPT fatal mode vectors (<span class=\"tt\">step34-wpt-fatal-vectors</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md535", null ],
      [ "Step 35: WPT BOM stripping conformance (<span class=\"tt\">step35-wpt-bom-vectors</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md536", null ],
      [ "Step 37: WPT EOF vectors + Big5 conformance fix (<span class=\"tt\">step37-wpt-eof-vectors</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md538", null ],
      [ "Step 36: WPT fatal single-byte (<span class=\"tt\">step36-wpt-fatal-single-byte</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md540", null ],
      [ "Step 38: WPT UTF-8 surrogate encode vectors (<span class=\"tt\">step38-wpt-surrogates-utf8</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md541", null ],
      [ "Upcoming", "md_docs_2plans_2phase2-checklist.html#autotoc_md543", null ],
      [ "Step 39: WPT TextEncoder UTF-16 surrogate round-trip (<span class=\"tt\">step39-wpt-encoder-surrogates</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md544", [
        [ "WPT TextEncoder UTF-16 surrogate round-trip (<span class=\"tt\">step39-wpt-encoder-surrogates</span>) — DONE", "md_docs_2plans_2phase2-checklist.html#autotoc_md545", null ],
        [ "Label lookup API (<span class=\"tt\">step39-label-lookup</span> or <span class=\"tt\">step40-label-lookup</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md546", null ]
      ] ],
      [ "Step 41: <span class=\"tt\">sniff_encoding</span> BOM detection (<span class=\"tt\">step41-sniff-encoding</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md547", null ],
      [ "Step 42: Umbrella header (<span class=\"tt\">step42-umbrella-header</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md548", null ],
      [ "Step 43: <span class=\"tt\">transcode_string</span> one-shot function (<span class=\"tt\">step43-transcode-string</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md549", null ],
      [ "Step 44: Coverage audit of <span class=\"tt\">transcode_string</span> (<span class=\"tt\">step44-coverage-audit</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md550", null ],
      [ "Step 45: <span class=\"tt\">transcode_string</span> label overload (<span class=\"tt\">step45-transcode-string-label</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md551", null ],
      [ "Step 46: <span class=\"tt\">transcode_view</span> pipe composition helper (<span class=\"tt\">step46-transcode-view</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md552", null ],
      [ "Step 47: <span class=\"tt\">transcode_view</span> negative compile test + consteval test (<span class=\"tt\">step47-transcode-view-negative-consteval</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md553", null ],
      [ "Step 48: <span class=\"tt\">sniff_encoding</span> negative compile test + Python lint fix (<span class=\"tt\">step48-sniff-negative-lint-fix</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md554", null ],
      [ "Step 49: <span class=\"tt\">whatwg_decode_view</span> coverage improvement (<span class=\"tt\">step49-decode-view-coverage</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md555", null ],
      [ "Step 50: iconv view boundary-condition tests (<span class=\"tt\">step50-iconv-boundary-tests</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md556", null ],
      [ "Step 51: iconv stateful flush (<span class=\"tt\">step51-iconv-stateful-flush</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md557", null ],
      [ "Step 52: real-iconv ISO-2022-JP flush integration tests (<span class=\"tt\">step52-iconv-iso2022jp-flush-test</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md558", null ],
      [ "Step 53: coverage improvements (<span class=\"tt\">step53-coverage-improvements</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md559", null ],
      [ "Step 53b: unreachable code annotations (<span class=\"tt\">step53b-unreachable-annotations</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md560", null ],
      [ "Step 54: C++23 module support audit (<span class=\"tt\">step54-module-audit</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md561", null ],
      [ "Step 55: Module integration and smoke tests (<span class=\"tt\">step55-module-tests</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md562", null ],
      [ "Step 56: Coverage analysis — whatwg_decode_view.hpp (<span class=\"tt\">step56-decode-coverage</span>)", "md_docs_2plans_2phase2-checklist.html#autotoc_md563", null ],
      [ "Notes", "md_docs_2plans_2phase2-checklist.html#autotoc_md565", null ]
    ] ],
    [ "Phase 2: Full WHATWG Codec Implementation", "md_docs_2plans_2phase2-index.html", [
      [ "Overview", "md_docs_2plans_2phase2-index.html#autotoc_md567", null ],
      [ "WHATWG Encoding Inventory", "md_docs_2plans_2phase2-index.html#autotoc_md569", [
        [ "Already implemented", "md_docs_2plans_2phase2-index.html#autotoc_md570", null ],
        [ "Single-byte (22 total, 1 done)", "md_docs_2plans_2phase2-index.html#autotoc_md571", null ],
        [ "Multi-byte (7 codecs, each with unique algorithm)", "md_docs_2plans_2phase2-index.html#autotoc_md572", null ],
        [ "Other", "md_docs_2plans_2phase2-index.html#autotoc_md573", null ]
      ] ],
      [ "Steps", "md_docs_2plans_2phase2-index.html#autotoc_md575", [
        [ "Completed", "md_docs_2plans_2phase2-index.html#autotoc_md576", [
          [ "<a class=\"el\" href=\"md_docs_2plans_2step14-replacement-codec.html\">Step 14: <span class=\"tt\">codec::replacement</span></a>", "md_docs_2plans_2phase2-index.html#autotoc_md577", null ],
          [ "<a class=\"el\" href=\"md_docs_2plans_2step15-x-user-defined.html\">Step 15: <span class=\"tt\">codec::x_user_defined</span></a>", "md_docs_2plans_2phase2-index.html#autotoc_md578", null ],
          [ "<a class=\"el\" href=\"md_docs_2plans_2step16-single-byte-infra.html\">Step 16: Single-byte infrastructure + <span class=\"tt\">windows_1252</span></a>", "md_docs_2plans_2phase2-index.html#autotoc_md579", null ]
        ] ],
        [ "Data tooling", "md_docs_2plans_2phase2-index.html#autotoc_md580", [
          [ "<a class=\"el\" href=\"md_docs_2plans_2step17-iso-8859-tables.html\">Step 17: Data tooling — WHATWG index acquisition &amp; table generation</a>", "md_docs_2plans_2phase2-index.html#autotoc_md581", null ]
        ] ],
        [ "Single-byte codec loop", "md_docs_2plans_2phase2-index.html#autotoc_md582", [
          [ "Step 18: All single-byte decoders (<span class=\"tt\">step18-all-single-byte-decode</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md583", null ],
          [ "Step 19: Single-byte encoder infrastructure + all encoders (<span class=\"tt\">step19-single-byte-encode</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md584", null ]
        ] ],
        [ "UTF-8 encoder", "md_docs_2plans_2phase2-index.html#autotoc_md585", [
          [ "<a class=\"el\" href=\"md_docs_2plans_2step18-utf8-encoder.html\">Step 20: UTF-8 encoder</a>", "md_docs_2plans_2phase2-index.html#autotoc_md586", null ]
        ] ],
        [ "UTF-16", "md_docs_2plans_2phase2-index.html#autotoc_md587", [
          [ "Step 21: UTF-16 decoder + encoder (<span class=\"tt\">step21-utf16</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md588", null ]
        ] ],
        [ "Multi-byte codecs (decode + encode per codec)", "md_docs_2plans_2phase2-index.html#autotoc_md589", [
          [ "Step 22: GBK decode + encode (<span class=\"tt\">step22-gbk</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md590", null ],
          [ "Step 23: gb18030 decode + encode (<span class=\"tt\">step23-gb18030</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md591", null ],
          [ "Step 24: Big5 decode + encode (<span class=\"tt\">step24-big5</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md592", null ],
          [ "Step 25: Shift_JIS decode + encode (<span class=\"tt\">step25-shift-jis</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md593", null ],
          [ "Step 26: EUC-JP decode + encode (<span class=\"tt\">step26-euc-jp</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md594", null ],
          [ "Step 27: ISO-2022-JP decode + encode (<span class=\"tt\">step27-iso-2022-jp</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md595", null ],
          [ "Step 28: EUC-KR decode + encode (<span class=\"tt\">step28-euc-kr</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md596", null ]
        ] ],
        [ "Composition", "md_docs_2plans_2phase2-index.html#autotoc_md597", [
          [ "Step 29: Round-trip composition (<span class=\"tt\">step29-roundtrip</span>)", "md_docs_2plans_2phase2-index.html#autotoc_md598", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_2plans_2phase2-index.html#autotoc_md600", null ]
    ] ],
    [ "Phase 3 Progress Checklist", "md_docs_2plans_2phase3-checklist.html", [
      [ "P3-Step 1: Benchmark harness scaffolding (<span class=\"tt\">p3-step1-benchmark-harness</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md603", null ],
      [ "P3-Step 2: Benchmark corpus acquisition and provenance (<span class=\"tt\">p3-step2-benchmark-data</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md604", null ],
      [ "P3-Step 3: Benchmark fixture library and result schema (<span class=\"tt\">p3-step3-benchmark-fixtures</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md605", null ],
      [ "P3-Step 4: UTF-family and ASCII fast-path benchmarks (<span class=\"tt\">p3-step4-utf-benchmarks</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md606", null ],
      [ "P3-Step 5: Legacy WHATWG codec benchmarks (<span class=\"tt\">p3-step5-legacy-whatwg-benchmarks</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md607", null ],
      [ "P3-Step 6: <span class=\"tt\">iconv</span> baselines (<span class=\"tt\">p3-step6-iconv-baselines</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md608", null ],
      [ "P3-Step 7: <span class=\"tt\">std::codecvt</span> negative baseline (<span class=\"tt\">p3-step7-codecvt-baseline</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md609", null ],
      [ "P3-Step 8: <span class=\"tt\">encoding_rs</span> baseline (<span class=\"tt\">p3-step8-encoding-rs-baseline</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md610", null ],
      [ "P3-Step 9: <span class=\"tt\">simdutf</span> ceiling baseline (<span class=\"tt\">p3-step9-simdutf-baseline</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md611", null ],
      [ "P3-Step 10: Chunked/lazy boundary stress benchmarks (<span class=\"tt\">p3-step10-boundary-stress</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md612", null ],
      [ "P3-Step 11: Toolchain matrix automation (<span class=\"tt\">p3-step11-toolchain-matrix</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md613", null ],
      [ "P3-Step 12: Reporting, dashboard, and reproducibility pack (<span class=\"tt\">p3-step12-reporting-and-repro</span>)", "md_docs_2plans_2phase3-checklist.html#autotoc_md614", null ],
      [ "Notes", "md_docs_2plans_2phase3-checklist.html#autotoc_md616", null ]
    ] ],
    [ "Phase 3 General Handoff — Read This First", "md_docs_2plans_2phase3-handoff.html", [
      [ "Project Identity", "md_docs_2plans_2phase3-handoff.html#autotoc_md619", null ],
      [ "Directory Layout", "md_docs_2plans_2phase3-handoff.html#autotoc_md620", null ],
      [ "Key Make Targets", "md_docs_2plans_2phase3-handoff.html#autotoc_md621", null ],
      [ "Coding Rules (Enforced)", "md_docs_2plans_2phase3-handoff.html#autotoc_md622", null ],
      [ "Library API Surface (Current)", "md_docs_2plans_2phase3-handoff.html#autotoc_md623", [
        [ "WhatWG codec views (enum-based, closed set)", "md_docs_2plans_2phase3-handoff.html#autotoc_md624", null ],
        [ "Pluggable codec protocol (type-based, open for extension)", "md_docs_2plans_2phase3-handoff.html#autotoc_md625", null ],
        [ "iconv views", "md_docs_2plans_2phase3-handoff.html#autotoc_md626", null ],
        [ "Bulk operations", "md_docs_2plans_2phase3-handoff.html#autotoc_md627", null ],
        [ "Runtime dispatch", "md_docs_2plans_2phase3-handoff.html#autotoc_md628", null ]
      ] ],
      [ "Catch2 Benchmark Macros", "md_docs_2plans_2phase3-handoff.html#autotoc_md629", null ],
      [ "iconv Mock Library", "md_docs_2plans_2phase3-handoff.html#autotoc_md630", null ],
      [ "Branch and Commit Discipline", "md_docs_2plans_2phase3-handoff.html#autotoc_md631", null ],
      [ "What to Write in handoff-next.md", "md_docs_2plans_2phase3-handoff.html#autotoc_md632", null ]
    ] ],
    [ "Phase 3: Benchmarking and Performance Characterization", "md_docs_2plans_2phase3-index.html", [
      [ "Overview", "md_docs_2plans_2phase3-index.html#autotoc_md634", null ],
      [ "Why Phase 3 Is Separate", "md_docs_2plans_2phase3-index.html#autotoc_md636", null ],
      [ "Phase Scope", "md_docs_2plans_2phase3-index.html#autotoc_md638", null ],
      [ "P3 Steps", "md_docs_2plans_2phase3-index.html#autotoc_md640", [
        [ "Harness and data foundations", "md_docs_2plans_2phase3-index.html#autotoc_md641", [
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step1-benchmark-harness.html\">P3-Step 1: Benchmark harness scaffolding</a>", "md_docs_2plans_2phase3-index.html#autotoc_md642", null ],
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step2-benchmark-data.html\">P3-Step 2: Benchmark corpus acquisition and provenance</a>", "md_docs_2plans_2phase3-index.html#autotoc_md643", null ],
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step3-benchmark-fixtures.html\">P3-Step 3: Benchmark fixture library and result schema</a>", "md_docs_2plans_2phase3-index.html#autotoc_md644", null ]
        ] ],
        [ "Native <span class=\"tt\">beman::transcode</span> measurement", "md_docs_2plans_2phase3-index.html#autotoc_md645", [
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step4-utf-benchmarks.html\">P3-Step 4: UTF-family and ASCII fast-path benchmarks</a>", "md_docs_2plans_2phase3-index.html#autotoc_md646", null ],
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step5-legacy-whatwg-benchmarks.html\">P3-Step 5: Legacy WHATWG codec benchmarks</a>", "md_docs_2plans_2phase3-index.html#autotoc_md647", null ]
        ] ],
        [ "Baseline implementations", "md_docs_2plans_2phase3-index.html#autotoc_md648", [
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step6-iconv-baselines.html\">P3-Step 6: <span class=\"tt\">iconv</span> baselines</a>", "md_docs_2plans_2phase3-index.html#autotoc_md649", null ],
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step7-codecvt-baseline.html\">P3-Step 7: <span class=\"tt\">std::codecvt</span> negative baseline</a>", "md_docs_2plans_2phase3-index.html#autotoc_md650", null ],
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step8-encoding-rs-baseline.html\">P3-Step 8: <span class=\"tt\">encoding_rs</span> baseline</a>", "md_docs_2plans_2phase3-index.html#autotoc_md651", null ],
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step9-simdutf-baseline.html\">P3-Step 9: <span class=\"tt\">simdutf</span> ceiling baseline</a>", "md_docs_2plans_2phase3-index.html#autotoc_md652", null ]
        ] ],
        [ "Stress paths and final reporting", "md_docs_2plans_2phase3-index.html#autotoc_md653", [
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step10-boundary-stress.html\">P3-Step 10: Chunked/lazy boundary stress benchmarks</a>", "md_docs_2plans_2phase3-index.html#autotoc_md654", null ],
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step11-toolchain-matrix.html\">P3-Step 11: Toolchain matrix automation</a>", "md_docs_2plans_2phase3-index.html#autotoc_md655", null ],
          [ "<a class=\"el\" href=\"md_docs_2plans_2p3-step12-reporting-and-repro.html\">P3-Step 12: Reporting, dashboard, and reproducibility pack</a>", "md_docs_2plans_2phase3-index.html#autotoc_md656", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_2plans_2phase3-index.html#autotoc_md658", null ],
      [ "Agent Execution Model", "md_docs_2plans_2phase3-index.html#autotoc_md660", [
        [ "Worktree Discipline", "md_docs_2plans_2phase3-index.html#autotoc_md661", null ],
        [ "What to write in handoff-next.md", "md_docs_2plans_2phase3-index.html#autotoc_md662", null ],
        [ "Rules", "md_docs_2plans_2phase3-index.html#autotoc_md663", null ]
      ] ]
    ] ],
    [ "Phase 4: API Surface Gap Closure", "md_docs_2plans_2phase4-index.html", [
      [ "Context", "md_docs_2plans_2phase4-index.html#autotoc_md665", null ],
      [ "Step Index", "md_docs_2plans_2phase4-index.html#autotoc_md666", null ],
      [ "Standing Conventions", "md_docs_2plans_2phase4-index.html#autotoc_md667", null ]
    ] ],
    [ "Phase 5: Specification Wording from the Headers", "md_docs_2plans_2phase5-index.html", [
      [ "Context", "md_docs_2plans_2phase5-index.html#autotoc_md669", null ],
      [ "Measurements taken before planning (2026-09-03)", "md_docs_2plans_2phase5-index.html#autotoc_md670", null ],
      [ "Decisions", "md_docs_2plans_2phase5-index.html#autotoc_md671", null ],
      [ "Step index", "md_docs_2plans_2phase5-index.html#autotoc_md672", null ],
      [ "Standing conventions", "md_docs_2plans_2phase5-index.html#autotoc_md673", null ],
      [ "External dependencies (work in <span class=\"tt\">~/src/specgen/main</span>)", "md_docs_2plans_2phase5-index.html#autotoc_md674", [
        [ "The gathered-region pattern", "md_docs_2plans_2phase5-index.html#autotoc_md675", null ],
        [ "Closed", "md_docs_2plans_2phase5-index.html#autotoc_md676", null ]
      ] ]
    ] ],
    [ "Phase 6: Acting on the 2026-09-11 Review", "md_docs_2plans_2phase6-index.html", [
      [ "Context", "md_docs_2plans_2phase6-index.html#autotoc_md678", null ],
      [ "Measurements taken before planning (2026-09-11)", "md_docs_2plans_2phase6-index.html#autotoc_md679", null ],
      [ "Decisions taken", "md_docs_2plans_2phase6-index.html#autotoc_md680", [
        [ "S-01, resolved", "md_docs_2plans_2phase6-index.html#autotoc_md681", null ]
      ] ],
      [ "The wording gate, and why it is revised first", "md_docs_2plans_2phase6-index.html#autotoc_md682", null ],
      [ "Step index", "md_docs_2plans_2phase6-index.html#autotoc_md683", null ],
      [ "Standing conventions", "md_docs_2plans_2phase6-index.html#autotoc_md684", null ],
      [ "Verification", "md_docs_2plans_2phase6-index.html#autotoc_md685", null ],
      [ "Risks", "md_docs_2plans_2phase6-index.html#autotoc_md686", null ]
    ] ],
    [ "Step 14: <span class=\"tt\">codec::replacement</span> Decoder", "md_docs_2plans_2step14-replacement-codec.html", [
      [ "Goal", "md_docs_2plans_2step14-replacement-codec.html#autotoc_md689", null ],
      [ "WHATWG Specification", "md_docs_2plans_2step14-replacement-codec.html#autotoc_md690", null ],
      [ "Files Modified", "md_docs_2plans_2step14-replacement-codec.html#autotoc_md691", [
        [ "<span class=\"tt\">include/beman/transcode/whatwg_decode_view.hpp</span>", "md_docs_2plans_2step14-replacement-codec.html#autotoc_md692", null ],
        [ "<span class=\"tt\">tests/beman/transcode/whatwg_decode.test.cpp</span>", "md_docs_2plans_2step14-replacement-codec.html#autotoc_md693", null ],
        [ "<span class=\"tt\">tests/beman/transcode/whatwg_decode_or_error.test.cpp</span>", "md_docs_2plans_2step14-replacement-codec.html#autotoc_md694", null ]
      ] ],
      [ "No New Files Created", "md_docs_2plans_2step14-replacement-codec.html#autotoc_md695", null ],
      [ "No Negative Compile Tests", "md_docs_2plans_2step14-replacement-codec.html#autotoc_md696", null ],
      [ "Verification", "md_docs_2plans_2step14-replacement-codec.html#autotoc_md697", null ]
    ] ],
    [ "Step 15: <span class=\"tt\">codec::x_user_defined</span> Decoder", "md_docs_2plans_2step15-x-user-defined.html", [
      [ "Goal", "md_docs_2plans_2step15-x-user-defined.html#autotoc_md700", null ],
      [ "WHATWG Specification", "md_docs_2plans_2step15-x-user-defined.html#autotoc_md701", null ],
      [ "Files Created", "md_docs_2plans_2step15-x-user-defined.html#autotoc_md702", [
        [ "<span class=\"tt\">include/beman/transcode/detail/x_user_defined.hpp</span>", "md_docs_2plans_2step15-x-user-defined.html#autotoc_md703", null ]
      ] ],
      [ "Files Modified", "md_docs_2plans_2step15-x-user-defined.html#autotoc_md704", [
        [ "<span class=\"tt\">include/beman/transcode/whatwg_decode_view.hpp</span>", "md_docs_2plans_2step15-x-user-defined.html#autotoc_md705", null ],
        [ "<span class=\"tt\">include/beman/transcode/CMakeLists.txt</span>", "md_docs_2plans_2step15-x-user-defined.html#autotoc_md706", null ],
        [ "<span class=\"tt\">tests/beman/transcode/whatwg_decode.test.cpp</span>", "md_docs_2plans_2step15-x-user-defined.html#autotoc_md707", null ],
        [ "<span class=\"tt\">tests/beman/transcode/whatwg_decode_or_error.test.cpp</span>", "md_docs_2plans_2step15-x-user-defined.html#autotoc_md708", null ]
      ] ],
      [ "Verification", "md_docs_2plans_2step15-x-user-defined.html#autotoc_md709", null ]
    ] ],
    [ "Step 16: Single-Byte Table Infrastructure + <span class=\"tt\">codec::windows_1252</span>", "md_docs_2plans_2step16-single-byte-infra.html", [
      [ "Goal", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md712", null ],
      [ "WHATWG Specification — Single-Byte Decoder Algorithm", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md713", null ],
      [ "Files Created", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md714", [
        [ "<span class=\"tt\">include/beman/transcode/detail/single_byte.hpp</span>", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md715", null ],
        [ "<span class=\"tt\">include/beman/transcode/detail/tables/windows_1252.hpp</span>", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md716", null ]
      ] ],
      [ "Files Modified", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md717", [
        [ "<span class=\"tt\">include/beman/transcode/whatwg_decode_view.hpp</span>", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md718", null ],
        [ "<span class=\"tt\">include/beman/transcode/CMakeLists.txt</span>", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md719", null ],
        [ "<span class=\"tt\">tests/beman/transcode/whatwg_decode.test.cpp</span>", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md720", null ],
        [ "<span class=\"tt\">tests/beman/transcode/whatwg_decode_or_error.test.cpp</span>", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md721", null ]
      ] ],
      [ "Verification", "md_docs_2plans_2step16-single-byte-infra.html#autotoc_md722", null ]
    ] ],
    [ "Step 17: Data Tooling — WHATWG Index Acquisition &amp; Table Generation", "md_docs_2plans_2step17-iso-8859-tables.html", [
      [ "Goal", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md725", null ],
      [ "Why This Step Exists", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md726", null ],
      [ "WHATWG Index Data Sources", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md727", [
        [ "Single-byte indexes (one per codec)", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md728", null ],
        [ "Multi-byte indexes", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md729", null ],
        [ "Metadata", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md730", null ]
      ] ],
      [ "Directory Layout", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md731", [
        [ "<span class=\"tt\">docs/whatwg/</span> — pristine upstream data", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md732", null ],
        [ "<span class=\"tt\">docs/wpt/</span> — Web Platform Tests (future)", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md733", null ],
        [ "<span class=\"tt\">data/</span> — our derived/processed artifacts", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md734", null ]
      ] ],
      [ "Provenance: <span class=\"tt\">docs/whatwg/SOURCE.md</span>", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md735", null ],
      [ "Provenance: <span class=\"tt\">docs/whatwg/source.bib</span>", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md736", null ],
      [ "Python Quality Requirements", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md737", null ],
      [ "Deliverables", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md738", [
        [ "Script: <span class=\"tt\">tools/download_indexes.py</span>", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md739", null ],
        [ "Script: <span class=\"tt\">tools/generate_tables.py</span>", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md740", null ],
        [ "Tests: <span class=\"tt\">tools/tests/</span>", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md741", null ],
        [ "Generated files (single-byte, 22 total)", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md742", null ]
      ] ],
      [ "Procedure", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md743", null ],
      [ "Verification", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md744", null ],
      [ "Notes", "md_docs_2plans_2step17-iso-8859-tables.html#autotoc_md745", null ]
    ] ],
    [ "Step 18: UTF-8 Encoder — <span class=\"tt\">whatwg_encode_view&lt;codec::utf_8&gt;</span>", "md_docs_2plans_2step18-utf8-encoder.html", [
      [ "Goal", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md748", null ],
      [ "WHATWG Specification — UTF-8 Encoder", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md749", null ],
      [ "Files Created", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md750", [
        [ "<span class=\"tt\">include/beman/transcode/detail/utf8_encode.hpp</span>", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md751", null ],
        [ "<span class=\"tt\">include/beman/transcode/whatwg_encode_view.hpp</span>", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md752", null ],
        [ "<span class=\"tt\">tests/beman/transcode/whatwg_encode.test.cpp</span>", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md753", null ],
        [ "<span class=\"tt\">tests/beman/transcode/whatwg_encode_or_error.test.cpp</span>", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md754", null ],
        [ "Negative compile test: <span class=\"tt\">whatwg_encode_reject_char_range_fail.cpp</span>", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md755", null ]
      ] ],
      [ "Files Modified", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md756", [
        [ "<span class=\"tt\">include/beman/transcode/detail/concepts.hpp</span>", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md757", null ],
        [ "<span class=\"tt\">include/beman/transcode/CMakeLists.txt</span>", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md758", null ],
        [ "<span class=\"tt\">tests/beman/transcode/CMakeLists.txt</span>", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md759", null ]
      ] ],
      [ "Verification", "md_docs_2plans_2step18-utf8-encoder.html#autotoc_md760", null ]
    ] ],
    [ "Step 19: Round-Trip Composition Tests", "md_docs_2plans_2step19-roundtrip.html", [
      [ "Goal", "md_docs_2plans_2step19-roundtrip.html#autotoc_md763", null ],
      [ "Composition Syntax", "md_docs_2plans_2step19-roundtrip.html#autotoc_md764", null ],
      [ "Files Created", "md_docs_2plans_2step19-roundtrip.html#autotoc_md765", [
        [ "<span class=\"tt\">tests/beman/transcode/roundtrip.test.cpp</span>", "md_docs_2plans_2step19-roundtrip.html#autotoc_md766", null ]
      ] ],
      [ "Files Modified", "md_docs_2plans_2step19-roundtrip.html#autotoc_md767", [
        [ "<span class=\"tt\">tests/beman/transcode/CMakeLists.txt</span>", "md_docs_2plans_2step19-roundtrip.html#autotoc_md768", null ]
      ] ],
      [ "Verification", "md_docs_2plans_2step19-roundtrip.html#autotoc_md769", null ],
      [ "Notes", "md_docs_2plans_2step19-roundtrip.html#autotoc_md770", null ]
    ] ],
    [ "review-findings-2026-09-11-closeout", "md_docs_2review-findings-2026-09-11-closeout.html", [
      [ "Repository review findings — 2026-09-11 closeout", "md_docs_2review-findings-2026-09-11-closeout.html#autotoc_md771", [
        [ "Finding accounting", "md_docs_2review-findings-2026-09-11-closeout.html#autotoc_md772", null ],
        [ "Verification results", "md_docs_2review-findings-2026-09-11-closeout.html#autotoc_md773", null ]
      ] ]
    ] ],
    [ "review-findings-2026-09-11", "md_docs_2review-findings-2026-09-11.html", [
      [ "Repository review findings — 2026-09-11", "md_docs_2review-findings-2026-09-11.html#autotoc_md774", [
        [ "Must address before proposal or public release", "md_docs_2review-findings-2026-09-11.html#autotoc_md775", [
          [ "C-01: <span class=\"tt\">sniff_encoding</span> consumes single-pass input", "md_docs_2review-findings-2026-09-11.html#autotoc_md776", null ],
          [ "C-02: A too-small iconv view buffer silently loses valid input", "md_docs_2review-findings-2026-09-11.html#autotoc_md777", null ],
          [ "C-03: Streaming iconv flush is attempted only once", "md_docs_2review-findings-2026-09-11.html#autotoc_md778", null ],
          [ "C-04: Lossy iconv bulk replacement can corrupt the destination encoding", "md_docs_2review-findings-2026-09-11.html#autotoc_md779", null ],
          [ "C-05: iconv open and unexpected system errors are conflated", "md_docs_2review-findings-2026-09-11.html#autotoc_md780", null ],
          [ "C-06: <span class=\"tt\">null_term_view</span> has no reachable-terminator precondition", "md_docs_2review-findings-2026-09-11.html#autotoc_md781", null ],
          [ "C-07: Runtime transcoding uses empty output as an unsupported-target error", "md_docs_2review-findings-2026-09-11.html#autotoc_md782", null ],
          [ "P-01: The optional/portable iconv story does not match the build", "md_docs_2review-findings-2026-09-11.html#autotoc_md783", null ],
          [ "P-02: Proposed ISO C++ wording exposes an undefined POSIX type", "md_docs_2review-findings-2026-09-11.html#autotoc_md784", null ]
        ] ],
        [ "Specification and API decisions", "md_docs_2review-findings-2026-09-11.html#autotoc_md785", [
          [ "S-01: Invalid <span class=\"tt\">char32_t</span> is both a precondition violation and a reported error", "md_docs_2review-findings-2026-09-11.html#autotoc_md786", null ],
          [ "S-02: Lossy iconv APIs do not share one recovery policy", "md_docs_2review-findings-2026-09-11.html#autotoc_md787", null ],
          [ "S-03: <span class=\"tt\">random_access_decode_codec_type</span> reserves U+FFFD as an error sentinel", "md_docs_2review-findings-2026-09-11.html#autotoc_md788", null ],
          [ "S-04: Review generated free-standing synopsis prose", "md_docs_2review-findings-2026-09-11.html#autotoc_md789", null ],
          [ "S-05: Narrow or substantiate factual and performance claims", "md_docs_2review-findings-2026-09-11.html#autotoc_md790", null ]
        ] ],
        [ "Documentation, paper, build, and CI drift", "md_docs_2review-findings-2026-09-11.html#autotoc_md791", [
          [ "D-01: Duplicate author email key", "md_docs_2review-findings-2026-09-11.html#autotoc_md792", null ],
          [ "D-02: Feature-test macro placeholders disagree", "md_docs_2review-findings-2026-09-11.html#autotoc_md793", null ],
          [ "D-03: Documentation claims unsupported UTF-32 BOM sniffing", "md_docs_2review-findings-2026-09-11.html#autotoc_md794", null ],
          [ "D-04: Paper and related proposal revisions have drifted", "md_docs_2review-findings-2026-09-11.html#autotoc_md795", null ],
          [ "D-05: Build instructions name the wrong test framework and language level", "md_docs_2review-findings-2026-09-11.html#autotoc_md796", null ],
          [ "D-06: Advertised compiler support disagrees with CMake", "md_docs_2review-findings-2026-09-11.html#autotoc_md797", null ],
          [ "D-07: The documented <span class=\"tt\">table_codec</span> API and example are not public examples", "md_docs_2review-findings-2026-09-11.html#autotoc_md798", null ],
          [ "D-08: ICU does not expose the POSIX iconv API as stated", "md_docs_2review-findings-2026-09-11.html#autotoc_md799", null ],
          [ "D-09: A root file literally named <span class=\"tt\">-</span> contains stale generated wording", "md_docs_2review-findings-2026-09-11.html#autotoc_md800", null ],
          [ "D-10: <span class=\"tt\">docs/plans/handoff-next.md</span> is stale operational guidance", "md_docs_2review-findings-2026-09-11.html#autotoc_md801", null ],
          [ "D-11: “Manual Lint” CI repeats the normal lint target", "md_docs_2review-findings-2026-09-11.html#autotoc_md802", null ],
          [ "D-12: Module packaging needs a clean install-consumer test", "md_docs_2review-findings-2026-09-11.html#autotoc_md803", null ],
          [ "D-13: Telemetry script shebang is on line 2", "md_docs_2review-findings-2026-09-11.html#autotoc_md804", null ]
        ] ],
        [ "Verification results and gaps", "md_docs_2review-findings-2026-09-11.html#autotoc_md805", null ],
        [ "Suggested triage order", "md_docs_2review-findings-2026-09-11.html#autotoc_md806", null ]
      ] ]
    ] ],
    [ "WHATWG Encoding Standard — Index Data", "md_docs_2whatwg_2SOURCE.html", [
      [ "File Provenance", "md_docs_2whatwg_2SOURCE.html#autotoc_md808", null ],
      [ "Full Checksums", "md_docs_2whatwg_2SOURCE.html#autotoc_md809", null ]
    ] ],
    [ "wording-outline", "md_docs_2wording-outline.html", [
      [ "Wording outline", "md_docs_2wording-outline.html#autotoc_md810", [
        [ "How the tree maps onto fragments", "md_docs_2wording-outline.html#autotoc_md811", null ],
        [ "<span class=\"tt\">&lt;transcode&gt;</span>", "md_docs_2wording-outline.html#autotoc_md812", null ],
        [ "<span class=\"tt\">&lt;null_term&gt;</span>", "md_docs_2wording-outline.html#autotoc_md813", null ],
        [ "Exposition-only, omitted, and not proposed", "md_docs_2wording-outline.html#autotoc_md814", [
          [ "Exposition-only (<span class=\"tt\">\\expos</span>)", "md_docs_2wording-outline.html#autotoc_md815", null ],
          [ "Omitted (<span class=\"tt\">\\omit</span>)", "md_docs_2wording-outline.html#autotoc_md816", null ],
          [ "Not proposed at all", "md_docs_2wording-outline.html#autotoc_md817", null ]
        ] ],
        [ "The <span class=\"tt\">detail::</span> audit (Step 3 task 3)", "md_docs_2wording-outline.html#autotoc_md818", null ],
        [ "Decisions this outline settles", "md_docs_2wording-outline.html#autotoc_md819", null ],
        [ "Open, and deliberately not settled here", "md_docs_2wording-outline.html#autotoc_md820", null ]
      ] ]
    ] ],
    [ "Web Platform Tests — Encoding Test Vectors", "md_docs_2wpt_2SOURCE.html", [
      [ "File Provenance", "md_docs_2wpt_2SOURCE.html#autotoc_md822", null ],
      [ "Full Checksums", "md_docs_2wpt_2SOURCE.html#autotoc_md823", null ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
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
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"classbeman_1_1transcoding_1_1iconv__transcode__or__error__view_1_1iterator.html#aefea5e5e3fa7152efdb3684924358270",
"classbeman_1_1transcoding_1_1random__access__whatwg__encode__view.html#a691b2756443ceba36117bd283cb569ef",
"classbeman_1_1transcoding_1_1whatwg__decode__view_1_1iterator.html#ae251e0092176f363928876caf81be7d5",
"md_docs_2benchmarks_2RUNNING.html#autotoc_md49",
"md_docs_2plans_2p5-step10-paper-assembly.html#autotoc_md321",
"md_docs_2plans_2phase3-checklist.html#autotoc_md608",
"namespacebeman_1_1transcoding.html#a20dfb324ebd26c6127d014520f93005aa88b12deabfb301d0d62b6fb5fa8f2565",
"structbeman_1_1transcoding_1_1table__codec.html#a4da3fd52530be58a6ece822ef315082c"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';
var LISTOFALLMEMBERS = 'List of all members';