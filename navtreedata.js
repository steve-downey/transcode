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
    [ "Benchmark Platform Notes", "md_docs_benchmarks_PLATFORM_NOTES.html", [
      [ "<tt>simdutf</tt> ceiling baseline (optional — requires explicit CMake opt-in)", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md1", [
        [ "Enabling", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md2", null ],
        [ "Files", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md3", null ],
        [ "Platform availability", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md4", null ],
        [ "Benchmarks provided", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md5", null ],
        [ "Expected comparison", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md6", null ]
      ] ],
      [ "<tt>encoding_rs</tt> baseline (optional — requires Rust/Cargo)", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md8", [
        [ "Enabling", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md9", null ],
        [ "Files", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md10", null ],
        [ "Platform availability", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md11", null ],
        [ "Benchmarks provided", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md12", null ],
        [ "Results on this machine (Asan build — indicative only)", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md13", null ]
      ] ],
      [ "<tt>std::codecvt</tt> / <tt>std::wstring_convert</tt> (negative baseline)", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md15", null ]
    ] ],
    [ "Benchmarking Guide", "md_docs_benchmarks_README.html", [
      [ "Overview", "md_docs_benchmarks_README.html#autotoc_md18", null ],
      [ "Quick Start", "md_docs_benchmarks_README.html#autotoc_md20", null ],
      [ "Corpus Files", "md_docs_benchmarks_README.html#autotoc_md22", [
        [ "Small Corpus (checked-in)", "md_docs_benchmarks_README.html#autotoc_md23", null ],
        [ "Large Corpus (downloaded)", "md_docs_benchmarks_README.html#autotoc_md24", null ]
      ] ],
      [ "Generating Reports", "md_docs_benchmarks_README.html#autotoc_md26", [
        [ "From a Single XML File", "md_docs_benchmarks_README.html#autotoc_md27", null ],
        [ "With Large Corpus Sizes", "md_docs_benchmarks_README.html#autotoc_md28", null ],
        [ "With a Vega-Lite Chart", "md_docs_benchmarks_README.html#autotoc_md29", null ],
        [ "Comparing Multiple Toolchains", "md_docs_benchmarks_README.html#autotoc_md30", null ]
      ] ],
      [ "Throughput Formula", "md_docs_benchmarks_README.html#autotoc_md32", null ],
      [ "Reproducing Published Results", "md_docs_benchmarks_README.html#autotoc_md34", null ],
      [ "Benchmark Suites", "md_docs_benchmarks_README.html#autotoc_md36", null ]
    ] ],
    [ "Running Benchmarks", "md_docs_benchmarks_RUNNING.html", [
      [ "Quick Smoke Run (Default Build)", "md_docs_benchmarks_RUNNING.html#autotoc_md38", null ],
      [ "Optimized Builds via CMake Presets", "md_docs_benchmarks_RUNNING.html#autotoc_md39", [
        [ "GCC -O3 (smoke)", "md_docs_benchmarks_RUNNING.html#autotoc_md40", null ],
        [ "GCC -O3 -flto (smoke)", "md_docs_benchmarks_RUNNING.html#autotoc_md41", null ],
        [ "Clang -O3 -flto (smoke)", "md_docs_benchmarks_RUNNING.html#autotoc_md42", null ]
      ] ],
      [ "Full Compiler Matrix", "md_docs_benchmarks_RUNNING.html#autotoc_md43", null ],
      [ "Capturing Results", "md_docs_benchmarks_RUNNING.html#autotoc_md44", null ],
      [ "Environment Metadata", "md_docs_benchmarks_RUNNING.html#autotoc_md45", null ],
      [ "Manually Configuring a Preset", "md_docs_benchmarks_RUNNING.html#autotoc_md46", null ],
      [ "Matrix Slices", "md_docs_benchmarks_RUNNING.html#autotoc_md47", null ],
      [ "Notes", "md_docs_benchmarks_RUNNING.html#autotoc_md48", null ]
    ] ],
    [ "Benchmark Corpora — Wikipedia Mars Article", "md_docs_benchmarks_SOURCE.html", [
      [ "Purpose", "md_docs_benchmarks_SOURCE.html#autotoc_md50", null ],
      [ "Source", "md_docs_benchmarks_SOURCE.html#autotoc_md51", null ],
      [ "License", "md_docs_benchmarks_SOURCE.html#autotoc_md52", [
        [ "BibTeX", "md_docs_benchmarks_SOURCE.html#autotoc_md53", null ]
      ] ],
      [ "Encoding Conversions", "md_docs_benchmarks_SOURCE.html#autotoc_md54", null ],
      [ "Fallback Corpus", "md_docs_benchmarks_SOURCE.html#autotoc_md55", null ],
      [ "Manifest", "md_docs_benchmarks_SOURCE.html#autotoc_md56", null ]
    ] ],
    [ "Testing C++ Range Adapters over <tt>iconv</tt>", "md_docs_iconv_testing.html", [
      [ "1. The <tt>iconv</tt> View Boundary Conditions", "md_docs_iconv_testing.html#autotoc_md58", [
        [ "A. Buffer Exhaustion (<tt>E2BIG</tt>)", "md_docs_iconv_testing.html#autotoc_md59", null ],
        [ "B. The Split Multi-byte Sequence (<tt>EINVAL</tt>)", "md_docs_iconv_testing.html#autotoc_md60", null ],
        [ "C. Invalid Byte Sequences (<tt>EILSEQ</tt>)", "md_docs_iconv_testing.html#autotoc_md61", null ],
        [ "D. End-of-Range Stateful Flush", "md_docs_iconv_testing.html#autotoc_md62", null ]
      ] ],
      [ "2. Mocking <tt>iconv</tt> for Deterministic View Testing", "md_docs_iconv_testing.html#autotoc_md64", null ],
      [ "3. Testing Execution Plan", "md_docs_iconv_testing.html#autotoc_md66", null ]
    ] ],
    [ "p2728-alignment", "md_docs_p2728_alignment.html", [
      [ "Tracking P2728: UTF Transcoding alignment", "md_docs_p2728_alignment.html#autotoc_md67", [
        [ "How the two relate", "md_docs_p2728_alignment.html#autotoc_md68", null ],
        [ "The model in brief (P2728R13)", "md_docs_p2728_alignment.html#autotoc_md69", null ],
        [ "Revision history of the model", "md_docs_p2728_alignment.html#autotoc_md70", [
          [ "Design intent from committee review", "md_docs_p2728_alignment.html#autotoc_md71", null ]
        ] ],
        [ "Where <tt>beman.transcode</tt> already follows the model", "md_docs_p2728_alignment.html#autotoc_md72", null ],
        [ "Where it diverges, and how to track", "md_docs_p2728_alignment.html#autotoc_md73", [
          [ "1. Collapse the <tt>_view</tt> / <tt>_or_error_view</tt> pairs into one template — <strong>done</strong>", "md_docs_p2728_alignment.html#autotoc_md74", null ],
          [ "2. Unspecified underlying type for the error enums (R13)", "md_docs_p2728_alignment.html#autotoc_md75", null ],
          [ "3. Verify <tt>base()</tt> semantics", "md_docs_p2728_alignment.html#autotoc_md76", null ],
          [ "Intentional, non-convergent differences (do not \"fix\" toward P2728)", "md_docs_p2728_alignment.html#autotoc_md77", null ]
        ] ],
        [ "Action items", "md_docs_p2728_alignment.html#autotoc_md78", null ],
        [ "Future bolt-ons that the model leaves open", "md_docs_p2728_alignment.html#autotoc_md79", null ]
      ] ]
    ] ],
    [ "Handoff: Coverage Audit Complete — Ready for Phase 5 Planning", "md_docs_plans_handoff_next.html", [
      [ "Completed", "md_docs_plans_handoff_next.html#autotoc_md81", null ],
      [ "What was done in the Coverage Audit", "md_docs_plans_handoff_next.html#autotoc_md82", [
        [ "<tt>tests/beman/transcode/iconv_mock.hpp</tt>", "md_docs_plans_handoff_next.html#autotoc_md83", null ],
        [ "<tt>tests/beman/transcode/iconv_bulk.test.cpp</tt> (+7 tests)", "md_docs_plans_handoff_next.html#autotoc_md84", null ],
        [ "<tt>tests/beman/transcode/bulk_transcode.test.cpp</tt> (+2 tests)", "md_docs_plans_handoff_next.html#autotoc_md85", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode_or_error.test.cpp</tt> (+1 test)", "md_docs_plans_handoff_next.html#autotoc_md86", null ],
        [ "<tt>tests/beman/transcode/labels.test.cpp</tt> (+1 test)", "md_docs_plans_handoff_next.html#autotoc_md87", null ],
        [ "After audit coverage", "md_docs_plans_handoff_next.html#autotoc_md88", null ]
      ] ],
      [ "Remaining Coverage Gaps (Explained)", "md_docs_plans_handoff_next.html#autotoc_md89", null ],
      [ "Current State", "md_docs_plans_handoff_next.html#autotoc_md90", null ],
      [ "Branch State", "md_docs_plans_handoff_next.html#autotoc_md91", null ],
      [ "What Comes Next", "md_docs_plans_handoff_next.html#autotoc_md92", null ]
    ] ],
    [ "P3-Step 1: Benchmark Harness Scaffolding", "md_docs_plans_p3_step1_benchmark_harness.html", [
      [ "Goal", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md95", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md96", [
        [ "Key files to reference", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md97", null ],
        [ "Build conventions", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md98", null ],
        [ "Library usage in benchmark code", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md99", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md100", null ],
      [ "Procedure", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md101", null ],
      [ "Makefile Target Pattern", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md102", null ],
      [ "Verification", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md103", null ],
      [ "Handoff to Step 2", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md104", null ]
    ] ],
    [ "P3-Step 10: Chunked/Lazy Boundary Stress Benchmarks", "md_docs_plans_p3_step10_boundary_stress.html", [
      [ "Goal", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md107", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md108", null ],
      [ "Deliverables", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md109", null ],
      [ "Constraints", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md110", null ],
      [ "Procedure", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md111", null ],
      [ "Verification", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md112", null ],
      [ "Notes", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md113", null ],
      [ "Handoff to Step 11", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md114", null ]
    ] ],
    [ "P3-Step 11: Toolchain Matrix Automation", "md_docs_plans_p3_step11_toolchain_matrix.html", [
      [ "Goal", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md117", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md118", null ],
      [ "Matrix To Support", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md119", null ],
      [ "Deliverables", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md120", null ],
      [ "Constraints", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md121", null ],
      [ "Procedure", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md122", null ],
      [ "Verification", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md123", null ],
      [ "Notes", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md124", null ],
      [ "Handoff to Step 12", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md125", null ]
    ] ],
    [ "P3-Step 12: Reporting, Dashboard, and Reproducibility Pack", "md_docs_plans_p3_step12_reporting_and_repro.html", [
      [ "Goal", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md128", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md129", null ],
      [ "Deliverables", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md130", null ],
      [ "Files Expected", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md131", null ],
      [ "Constraints", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md132", null ],
      [ "Procedure", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md133", null ],
      [ "Verification", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md134", null ],
      [ "Notes", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md135", null ],
      [ "Handoff (Phase 3 Complete)", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md136", null ]
    ] ],
    [ "P3-Step 2: Benchmark Corpus Acquisition and Provenance", "md_docs_plans_p3_step2_benchmark_data.html", [
      [ "Goal", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md139", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md140", [
        [ "MediaWiki API for extraction", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md141", null ],
        [ "Python encoding conversion", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md142", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md143", null ],
      [ "Corpus Manifest Schema", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md144", null ],
      [ "Fallback Corpus (checked in)", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md145", null ],
      [ "Procedure", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md146", null ],
      [ "Verification", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md147", null ],
      [ "Handoff to Step 3", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md148", null ]
    ] ],
    [ "P3-Step 3: Benchmark Fixture Library and Result Schema", "md_docs_plans_p3_step3_benchmark_fixtures.html", [
      [ "Goal", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md151", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md152", null ],
      [ "Deliverables", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md153", null ],
      [ "API Design", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md154", [
        [ "benchmark_fixture.hpp", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md155", null ],
        [ "benchmark_sink.hpp", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md156", null ]
      ] ],
      [ "Procedure", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md157", null ],
      [ "Verification", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md158", null ],
      [ "Handoff to Step 4", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md159", null ]
    ] ],
    [ "P3-Step 4: UTF-Family and ASCII Fast-Path Benchmarks", "md_docs_plans_p3_step4_utf_benchmarks.html", [
      [ "Goal", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md162", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md163", null ],
      [ "Deliverables", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md164", null ],
      [ "Benchmark Cases", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md165", null ],
      [ "Procedure", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md166", null ],
      [ "Verification", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md167", null ],
      [ "Handoff to Step 5", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md168", null ]
    ] ],
    [ "P3-Step 5: Legacy WHATWG Codec Benchmarks", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html", [
      [ "Goal", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md171", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md172", null ],
      [ "Deliverables", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md173", null ],
      [ "Benchmark Cases", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md174", [
        [ "whatwg_benchmarks.bench.cpp", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md175", null ],
        [ "pluggable_codec_benchmarks.bench.cpp", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md176", null ]
      ] ],
      [ "Procedure", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md177", null ],
      [ "Verification", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md178", null ],
      [ "Handoff to Step 6", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md179", null ]
    ] ],
    [ "P3-Step 6: <tt>iconv</tt> Baselines", "md_docs_plans_p3_step6_iconv_baselines.html", [
      [ "Goal", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md182", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md183", null ],
      [ "Deliverables", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md184", null ],
      [ "Benchmark Cases", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md185", null ],
      [ "Constraints", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md186", null ],
      [ "Procedure", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md187", null ],
      [ "Verification", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md188", null ],
      [ "Handoff to Step 7", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md189", null ]
    ] ],
    [ "P3-Step 7: <tt>std::codecvt</tt> Negative Baseline", "md_docs_plans_p3_step7_codecvt_baseline.html", [
      [ "Goal", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md192", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md193", null ],
      [ "Deliverables", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md194", null ],
      [ "Constraints", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md195", null ],
      [ "Procedure", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md196", null ],
      [ "Verification", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md197", null ],
      [ "Notes", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md198", null ],
      [ "Handoff to Step 8", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md199", null ]
    ] ],
    [ "P3-Step 8: <tt>encoding_rs</tt> Baseline", "md_docs_plans_p3_step8_encoding_rs_baseline.html", [
      [ "Goal", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md202", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md203", null ],
      [ "Deliverables", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md204", null ],
      [ "Constraints", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md205", null ],
      [ "Procedure", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md206", null ],
      [ "Verification", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md207", null ],
      [ "Notes", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md208", null ],
      [ "Handoff to Step 9", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md209", null ]
    ] ],
    [ "P3-Step 9: <tt>simdutf</tt> Ceiling Baseline", "md_docs_plans_p3_step9_simdutf_baseline.html", [
      [ "Goal", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md212", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md213", null ],
      [ "Deliverables", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md214", null ],
      [ "Constraints", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md215", null ],
      [ "Procedure", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md216", null ],
      [ "Verification", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md217", null ],
      [ "Notes", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md218", null ],
      [ "Handoff to Step 10", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md219", null ]
    ] ],
    [ "P4-Step 1: Pluggable Encode View", "md_docs_plans_p4_step1_pluggable_encode.html", [
      [ "Goal", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md222", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md223", [
        [ "Key files to reference", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md224", null ],
        [ "Key differences from decode_view", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md225", null ],
        [ "The encode_codec concept", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md226", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md227", null ],
      [ "Constraints", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md228", null ],
      [ "Procedure", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md229", null ],
      [ "Verification", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md230", null ],
      [ "Handoff to Step 2", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md231", null ]
    ] ],
    [ "P4-Step 2: Pluggable Bulk Operations", "md_docs_plans_p4_step2_pluggable_bulk.html", [
      [ "Goal", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md234", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md235", [
        [ "Signature difference", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md236", null ],
        [ "Fast path opportunity", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md237", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md238", null ],
      [ "Procedure", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md239", null ],
      [ "Verification", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md240", null ],
      [ "Handoff to Step 3", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md241", null ]
    ] ],
    [ "P4-Step 3: Pluggable Transcode Pipeline", "md_docs_plans_p4_step3_pluggable_transcode.html", [
      [ "Goal", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md244", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md245", [
        [ "Implementation", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md246", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md247", null ],
      [ "Procedure", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md248", null ],
      [ "Verification", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md249", null ],
      [ "Handoff", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md250", null ]
    ] ],
    [ "P4-Step 4: iconv Bulk Operations", "md_docs_plans_p4_step4_iconv_bulk.html", [
      [ "Goal", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md253", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md254", [
        [ "Dependency injection", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md255", null ],
        [ "Key files to reference", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md256", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md257", null ],
      [ "Constraints", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md258", null ],
      [ "Procedure", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md259", null ],
      [ "Verification", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md260", null ],
      [ "Handoff to Step 5", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md261", null ]
    ] ],
    [ "P4-Step 5: iconv Null-Terminated Input", "md_docs_plans_p4_step5_iconv_null_term.html", [
      [ "Goal", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md264", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md265", [
        [ "Possible outcomes", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md266", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md267", null ],
      [ "Procedure", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md268", null ],
      [ "Verification", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md269", null ],
      [ "Handoff to Step 6", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md270", null ]
    ] ],
    [ "P4-Step 6: Error Enum Coherence", "md_docs_plans_p4_step6_error_coherence.html", [
      [ "Goal", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md273", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md274", [
        [ "whatwg_error (include/beman/transcode/detail/error.hpp)", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md275", null ],
        [ "decode_error (include/beman/transcode/detail/codec_result.hpp)", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md276", null ],
        [ "iconv_error (include/beman/transcode/detail/error.hpp)", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md277", null ],
        [ "Why they differ", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md278", null ],
        [ "The problem", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md279", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md280", null ],
      [ "Procedure", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md281", null ],
      [ "Verification", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md282", null ],
      [ "Handoff (Phase 4 Complete)", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md283", null ]
    ] ],
    [ "P5-Step 1: specgen Harness and Paper Transclusion", "md_docs_plans_p5_step1_specgen_harness.html", [
      [ "Goal", "md_docs_plans_p5_step1_specgen_harness.html#autotoc_md286", null ],
      [ "Context for executing agent", "md_docs_plans_p5_step1_specgen_harness.html#autotoc_md287", null ],
      [ "Tasks", "md_docs_plans_p5_step1_specgen_harness.html#autotoc_md288", null ],
      [ "Acceptance", "md_docs_plans_p5_step1_specgen_harness.html#autotoc_md289", null ],
      [ "Notes", "md_docs_plans_p5_step1_specgen_harness.html#autotoc_md290", null ]
    ] ],
    [ "P5-Step 10: Paper Assembly and the Drift Gate", "md_docs_plans_p5_step10_paper_assembly.html", [
      [ "Goal", "md_docs_plans_p5_step10_paper_assembly.html#autotoc_md293", null ],
      [ "Tasks", "md_docs_plans_p5_step10_paper_assembly.html#autotoc_md294", null ],
      [ "Acceptance", "md_docs_plans_p5_step10_paper_assembly.html#autotoc_md295", null ],
      [ "Outcome (2026-09-08)", "md_docs_plans_p5_step10_paper_assembly.html#autotoc_md297", [
        [ "Three things that were wrong and are not any more", "md_docs_plans_p5_step10_paper_assembly.html#autotoc_md298", null ],
        [ "Task 7: the code blocks, and one thing this step did not fix", "md_docs_plans_p5_step10_paper_assembly.html#autotoc_md299", null ],
        [ "The drift gate, and the workflow that did not survive", "md_docs_plans_p5_step10_paper_assembly.html#autotoc_md300", null ]
      ] ]
    ] ],
    [ "P5-Step 2: Wording Outline and Stable Names", "md_docs_plans_p5_step2_wording_outline.html", [
      [ "Goal", "md_docs_plans_p5_step2_wording_outline.html#autotoc_md303", null ],
      [ "Context for executing agent", "md_docs_plans_p5_step2_wording_outline.html#autotoc_md304", null ],
      [ "Tasks", "md_docs_plans_p5_step2_wording_outline.html#autotoc_md305", null ],
      [ "Acceptance", "md_docs_plans_p5_step2_wording_outline.html#autotoc_md306", null ]
    ] ],
    [ "P5-Step 3: Spec-Facing Header Shape", "md_docs_plans_p5_step3_spec_header_shape.html", [
      [ "Goal", "md_docs_plans_p5_step3_spec_header_shape.html#autotoc_md309", null ],
      [ "Context for executing agent", "md_docs_plans_p5_step3_spec_header_shape.html#autotoc_md310", null ],
      [ "Tasks", "md_docs_plans_p5_step3_spec_header_shape.html#autotoc_md311", null ],
      [ "Outcome, and what is deferred", "md_docs_plans_p5_step3_spec_header_shape.html#autotoc_md312", null ],
      [ "Acceptance", "md_docs_plans_p5_step3_spec_header_shape.html#autotoc_md313", null ],
      [ "Notes", "md_docs_plans_p5_step3_spec_header_shape.html#autotoc_md314", null ]
    ] ],
    [ "P5-Step 3b: Push the Codec State Machines Down", "md_docs_plans_p5_step3b_codec_pushdown.html", [
      [ "Goal", "md_docs_plans_p5_step3b_codec_pushdown.html#autotoc_md317", null ],
      [ "What was done", "md_docs_plans_p5_step3b_codec_pushdown.html#autotoc_md318", null ],
      [ "What was not done, and the order to do it in", "md_docs_plans_p5_step3b_codec_pushdown.html#autotoc_md319", null ],
      [ "What this means for Step 3 task 4", "md_docs_plans_p5_step3b_codec_pushdown.html#autotoc_md320", null ],
      [ "Acceptance", "md_docs_plans_p5_step3b_codec_pushdown.html#autotoc_md321", null ]
    ] ],
    [ "P5-Step 4: Markup — Errors, Range Requirements, <tt>null_term</tt>", "md_docs_plans_p5_step4_errors_concepts_null_term.html", [
      [ "Goal", "md_docs_plans_p5_step4_errors_concepts_null_term.html#autotoc_md324", null ],
      [ "The markup loop", "md_docs_plans_p5_step4_errors_concepts_null_term.html#autotoc_md325", null ],
      [ "What is different here", "md_docs_plans_p5_step4_errors_concepts_null_term.html#autotoc_md326", null ],
      [ "Acceptance", "md_docs_plans_p5_step4_errors_concepts_null_term.html#autotoc_md327", null ],
      [ "Outcome (2026-09-06)", "md_docs_plans_p5_step4_errors_concepts_null_term.html#autotoc_md329", null ]
    ] ],
    [ "P5-Step 5: Markup — Encodings, Labels, BOM Sniffing", "md_docs_plans_p5_step5_codec_labels_sniff.html", [
      [ "Goal", "md_docs_plans_p5_step5_codec_labels_sniff.html#autotoc_md332", null ],
      [ "What is different here", "md_docs_plans_p5_step5_codec_labels_sniff.html#autotoc_md333", null ],
      [ "Acceptance", "md_docs_plans_p5_step5_codec_labels_sniff.html#autotoc_md334", null ],
      [ "Outcome (2026-09-07)", "md_docs_plans_p5_step5_codec_labels_sniff.html#autotoc_md336", null ]
    ] ],
    [ "P5-Step 6: Markup — WHATWG Decode and Encode Views", "md_docs_plans_p5_step6_whatwg_views.html", [
      [ "Goal", "md_docs_plans_p5_step6_whatwg_views.html#autotoc_md339", null ],
      [ "What is different here", "md_docs_plans_p5_step6_whatwg_views.html#autotoc_md340", null ],
      [ "Acceptance", "md_docs_plans_p5_step6_whatwg_views.html#autotoc_md341", null ],
      [ "Outcome (2026-09-08)", "md_docs_plans_p5_step6_whatwg_views.html#autotoc_md343", null ]
    ] ],
    [ "P5-Step 7: Markup — Codec Requirements and Pluggable Views", "md_docs_plans_p5_step7_pluggable_codecs.html", [
      [ "Goal", "md_docs_plans_p5_step7_pluggable_codecs.html#autotoc_md346", null ],
      [ "What is different here", "md_docs_plans_p5_step7_pluggable_codecs.html#autotoc_md347", null ],
      [ "Acceptance", "md_docs_plans_p5_step7_pluggable_codecs.html#autotoc_md348", null ],
      [ "Outcome (2026-09-08)", "md_docs_plans_p5_step7_pluggable_codecs.html#autotoc_md350", null ]
    ] ],
    [ "P5-Step 8: Markup — Pipelines and Eager Transcoding", "md_docs_plans_p5_step8_transcode_pipeline.html", [
      [ "Goal", "md_docs_plans_p5_step8_transcode_pipeline.html#autotoc_md353", null ],
      [ "What is different here", "md_docs_plans_p5_step8_transcode_pipeline.html#autotoc_md354", null ],
      [ "Acceptance", "md_docs_plans_p5_step8_transcode_pipeline.html#autotoc_md355", null ],
      [ "Outcome (2026-09-08)", "md_docs_plans_p5_step8_transcode_pipeline.html#autotoc_md357", null ]
    ] ],
    [ "P5-Step 9: Markup — iconv Adaptors", "md_docs_plans_p5_step9_iconv.html", [
      [ "Goal", "md_docs_plans_p5_step9_iconv.html#autotoc_md360", null ],
      [ "What is different here", "md_docs_plans_p5_step9_iconv.html#autotoc_md361", null ],
      [ "Acceptance", "md_docs_plans_p5_step9_iconv.html#autotoc_md362", null ],
      [ "Outcome (2026-09-08)", "md_docs_plans_p5_step9_iconv.html#autotoc_md364", [
        [ "Four markup faults, all of them placement", "md_docs_plans_p5_step9_iconv.html#autotoc_md365", null ],
        [ "W1 reaches the borrowed-range specializations", "md_docs_plans_p5_step9_iconv.html#autotoc_md366", null ],
        [ "One specgen defect, the last one standing", "md_docs_plans_p5_step9_iconv.html#autotoc_md367", null ]
      ] ]
    ] ],
    [ "P6-Step 0: Revise the Wording Staleness Gate", "md_docs_plans_p6_step0_wording_gate.html", [
      [ "Goal", "md_docs_plans_p6_step0_wording_gate.html#autotoc_md370", null ],
      [ "Why", "md_docs_plans_p6_step0_wording_gate.html#autotoc_md371", null ],
      [ "Design", "md_docs_plans_p6_step0_wording_gate.html#autotoc_md372", null ],
      [ "What to change", "md_docs_plans_p6_step0_wording_gate.html#autotoc_md373", null ],
      [ "Tests", "md_docs_plans_p6_step0_wording_gate.html#autotoc_md374", null ],
      [ "Done when", "md_docs_plans_p6_step0_wording_gate.html#autotoc_md375", null ],
      [ "Note on specgen", "md_docs_plans_p6_step0_wording_gate.html#autotoc_md376", null ]
    ] ],
    [ "P6-Step 1: <tt>sniff_encoding</tt> Requires a Forward Range", "md_docs_plans_p6_step1_sniff_forward_range.html", [
      [ "Goal", "md_docs_plans_p6_step1_sniff_forward_range.html#autotoc_md379", null ],
      [ "Why", "md_docs_plans_p6_step1_sniff_forward_range.html#autotoc_md380", null ],
      [ "What to change", "md_docs_plans_p6_step1_sniff_forward_range.html#autotoc_md381", null ],
      [ "Tests", "md_docs_plans_p6_step1_sniff_forward_range.html#autotoc_md382", null ],
      [ "Watch for", "md_docs_plans_p6_step1_sniff_forward_range.html#autotoc_md383", null ],
      [ "Done when", "md_docs_plans_p6_step1_sniff_forward_range.html#autotoc_md384", null ]
    ] ],
    [ "P6-Step 10: Verification", "md_docs_plans_p6_step10_verification.html", [
      [ "Goal", "md_docs_plans_p6_step10_verification.html#autotoc_md387", null ],
      [ "Why a step of its own", "md_docs_plans_p6_step10_verification.html#autotoc_md388", null ],
      [ "What to run", "md_docs_plans_p6_step10_verification.html#autotoc_md389", [
        [ "Build matrix", "md_docs_plans_p6_step10_verification.html#autotoc_md390", null ],
        [ "Install and consume", "md_docs_plans_p6_step10_verification.html#autotoc_md391", null ],
        [ "Examples", "md_docs_plans_p6_step10_verification.html#autotoc_md392", null ],
        [ "Wording", "md_docs_plans_p6_step10_verification.html#autotoc_md393", null ],
        [ "The tools the review could not run", "md_docs_plans_p6_step10_verification.html#autotoc_md394", null ],
        [ "Platform scope", "md_docs_plans_p6_step10_verification.html#autotoc_md395", null ],
        [ "Coverage", "md_docs_plans_p6_step10_verification.html#autotoc_md396", null ]
      ] ],
      [ "Report", "md_docs_plans_p6_step10_verification.html#autotoc_md397", null ],
      [ "Done when", "md_docs_plans_p6_step10_verification.html#autotoc_md398", null ]
    ] ],
    [ "P6-Step 2: The iconv Error Model", "md_docs_plans_p6_step2_iconv_error_model.html", [
      [ "Goal", "md_docs_plans_p6_step2_iconv_error_model.html#autotoc_md401", null ],
      [ "Why", "md_docs_plans_p6_step2_iconv_error_model.html#autotoc_md402", null ],
      [ "What to change", "md_docs_plans_p6_step2_iconv_error_model.html#autotoc_md403", [
        [ "<tt>error.hpp</tt>", "md_docs_plans_p6_step2_iconv_error_model.html#autotoc_md404", null ],
        [ "The two streaming views", "md_docs_plans_p6_step2_iconv_error_model.html#autotoc_md405", null ],
        [ "The bulk helpers", "md_docs_plans_p6_step2_iconv_error_model.html#autotoc_md406", null ]
      ] ],
      [ "Tests", "md_docs_plans_p6_step2_iconv_error_model.html#autotoc_md407", null ],
      [ "Done when", "md_docs_plans_p6_step2_iconv_error_model.html#autotoc_md408", null ],
      [ "Hand off to Step 8", "md_docs_plans_p6_step2_iconv_error_model.html#autotoc_md409", null ]
    ] ],
    [ "P6-Step 3: One Lossy Policy — Skip, Do Not Replace", "md_docs_plans_p6_step3_iconv_lossy_skip.html", [
      [ "Goal", "md_docs_plans_p6_step3_iconv_lossy_skip.html#autotoc_md412", null ],
      [ "Why", "md_docs_plans_p6_step3_iconv_lossy_skip.html#autotoc_md413", null ],
      [ "What to change", "md_docs_plans_p6_step3_iconv_lossy_skip.html#autotoc_md414", null ],
      [ "Tests", "md_docs_plans_p6_step3_iconv_lossy_skip.html#autotoc_md415", null ],
      [ "Done when", "md_docs_plans_p6_step3_iconv_lossy_skip.html#autotoc_md416", null ]
    ] ],
    [ "P6-Step 4: <tt>null_term_view</tt> Says What It Requires", "md_docs_plans_p6_step4_null_term_precondition.html", [
      [ "Goal", "md_docs_plans_p6_step4_null_term_precondition.html#autotoc_md419", null ],
      [ "Why", "md_docs_plans_p6_step4_null_term_precondition.html#autotoc_md420", null ],
      [ "What to change", "md_docs_plans_p6_step4_null_term_precondition.html#autotoc_md421", null ],
      [ "Tests", "md_docs_plans_p6_step4_null_term_precondition.html#autotoc_md422", null ],
      [ "Done when", "md_docs_plans_p6_step4_null_term_precondition.html#autotoc_md423", null ]
    ] ],
    [ "P6-Step 5: <tt>transcode_string</tt> Stops Signalling Failure With Emptiness", "md_docs_plans_p6_step5_transcode_string_result.html", [
      [ "Goal", "md_docs_plans_p6_step5_transcode_string_result.html#autotoc_md426", null ],
      [ "Why", "md_docs_plans_p6_step5_transcode_string_result.html#autotoc_md427", null ],
      [ "What to change", "md_docs_plans_p6_step5_transcode_string_result.html#autotoc_md428", null ],
      [ "Callers", "md_docs_plans_p6_step5_transcode_string_result.html#autotoc_md429", null ],
      [ "Tests", "md_docs_plans_p6_step5_transcode_string_result.html#autotoc_md430", null ],
      [ "Done when", "md_docs_plans_p6_step5_transcode_string_result.html#autotoc_md431", null ],
      [ "Note", "md_docs_plans_p6_step5_transcode_string_result.html#autotoc_md432", null ]
    ] ],
    [ "P6-Step 6: UTF-32 Input Validation, Specified and Uniform", "md_docs_plans_p6_step6_utf32_validation.html", [
      [ "</blockquote>", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md434", null ],
      [ "Goal", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md435", null ],
      [ "Why", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md436", [
        [ "The precondition was attributed to the wrong layer", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md437", null ],
        [ "And the encoder-layer reading gives an answer nobody wants", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md438", null ],
        [ "The implementation already does the right thing", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md439", null ],
        [ "C-08: the diagnosis is not uniform (new)", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md440", null ],
        [ "D-15: the documentation says ‘’?'` and UTF-8 says otherwise (new)", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md441", null ]
      ] ],
      [ "What to change", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md442", [
        [ "Validate above dispatch", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md443", null ],
        [ "Say what is now guaranteed", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md444", null ],
        [ "The WPT test stays, re-attributed", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md445", null ]
      ] ],
      [ "Tests", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md446", null ],
      [ "Done when", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md447", null ],
      [ "Hand off to Step 8", "md_docs_plans_p6_step6_utf32_validation.html#autotoc_md448", null ]
    ] ],
    [ "P6-Step 7: U+FFFD Is Reserved, and the Cost Is Stated", "md_docs_plans_p6_step7_fffd_reservation.html", [
      [ "Goal", "md_docs_plans_p6_step7_fffd_reservation.html#autotoc_md451", null ],
      [ "Why", "md_docs_plans_p6_step7_fffd_reservation.html#autotoc_md452", null ],
      [ "What to change", "md_docs_plans_p6_step7_fffd_reservation.html#autotoc_md453", null ],
      [ "Paper", "md_docs_plans_p6_step7_fffd_reservation.html#autotoc_md454", null ],
      [ "Tests", "md_docs_plans_p6_step7_fffd_reservation.html#autotoc_md455", null ],
      [ "Done when", "md_docs_plans_p6_step7_fffd_reservation.html#autotoc_md456", null ]
    ] ],
    [ "P6-Step 8: The Paper, and One Regeneration", "md_docs_plans_p6_step8_paper_and_wording.html", [
      [ "Goal", "md_docs_plans_p6_step8_paper_and_wording.html#autotoc_md459", null ],
      [ "Before anything else: build specgen", "md_docs_plans_p6_step8_paper_and_wording.html#autotoc_md460", null ],
      [ "P-02: an ISO synopsis that names a POSIX type", "md_docs_plans_p6_step8_paper_and_wording.html#autotoc_md461", null ],
      [ "P-01: the optional-iconv story", "md_docs_plans_p6_step8_paper_and_wording.html#autotoc_md462", null ],
      [ "S-04: prose in the synopsis", "md_docs_plans_p6_step8_paper_and_wording.html#autotoc_md463", null ],
      [ "S-05: claims narrower than their evidence", "md_docs_plans_p6_step8_paper_and_wording.html#autotoc_md464", null ],
      [ "The drift items", "md_docs_plans_p6_step8_paper_and_wording.html#autotoc_md465", null ],
      [ "Carried in from earlier steps", "md_docs_plans_p6_step8_paper_and_wording.html#autotoc_md466", null ],
      [ "Order of work", "md_docs_plans_p6_step8_paper_and_wording.html#autotoc_md467", null ],
      [ "Done when", "md_docs_plans_p6_step8_paper_and_wording.html#autotoc_md468", null ]
    ] ],
    [ "P6-Step 9: Documentation, Build and CI Drift", "md_docs_plans_p6_step9_docs_build_ci.html", [
      [ "Goal", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md471", null ],
      [ "Why together", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md472", null ],
      [ "The items", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md473", [
        [ "D-05 — the build instructions name the wrong framework and the wrong standard", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md474", null ],
        [ "D-06 — the compiler matrix disagrees with the configure check", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md475", null ],
        [ "D-07 — <tt>table_codec</tt> is documented as public API and is not", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md476", null ],
        [ "D-08 — ICU does not expose the iconv API", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md477", null ],
        [ "D-09 — a tracked file named <tt>-</tt>", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md478", null ],
        [ "D-10 — <tt>docs/plans/handoff-next.md</tt> reads as current", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md479", null ],
        [ "D-11 — \"Manual Lint\" runs the ordinary lint", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md480", null ],
        [ "D-12 — module packaging is untested and the interface unit has no licence", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md481", null ],
        [ "D-14 — the gitleaks hook is broken (not in the review)", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md482", null ],
        [ "D-13 — shebang on line 2", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md483", null ]
      ] ],
      [ "Repository hygiene", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md484", null ],
      [ "Done when", "md_docs_plans_p6_step9_docs_build_ci.html#autotoc_md485", null ]
    ] ],
    [ "Phase 2 Progress Checklist", "md_docs_plans_phase2_checklist.html", [
      [ "Step 14: <tt>codec::replacement</tt> (<tt>step14-replacement-codec</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md488", null ],
      [ "Step 15: <tt>codec::x_user_defined</tt> (<tt>step15-x-user-defined</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md489", null ],
      [ "Step 16: Single-byte infra + <tt>windows_1252</tt> (<tt>step16-single-byte-infra</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md490", null ],
      [ "Step 17: Data tooling (<tt>step17-data-tooling</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md491", null ],
      [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md492", null ],
      [ "Step 19: Single-byte encoder infra + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md493", null ],
      [ "Step 20: UTF-8 encoder (<tt>step20-utf8-encoder</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md494", null ],
      [ "Step 21: UTF-16 decode + encode (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md495", null ],
      [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md496", null ],
      [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md497", null ],
      [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md498", null ],
      [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md499", null ],
      [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md500", null ],
      [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md501", null ],
      [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md502", null ],
      [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md503", null ],
      [ "Step 30: WPT test vector integration (<tt>step30-wpt-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md505", null ],
      [ "Step 31: WPT ISO-2022-JP + single-byte exhaustive (<tt>step31-wpt-iso2022jp-singlebyte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md506", null ],
      [ "Step 32: ISO-2022-JP full WHATWG conformance (<tt>step32-iso2022jp-conformance</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md507", null ],
      [ "Step 33: WPT UTF-16 surrogate conformance (<tt>step33-wpt-utf16-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md508", null ],
      [ "Step 34: WPT fatal mode vectors (<tt>step34-wpt-fatal-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md509", null ],
      [ "Step 35: WPT BOM stripping conformance (<tt>step35-wpt-bom-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md510", null ],
      [ "Step 37: WPT EOF vectors + Big5 conformance fix (<tt>step37-wpt-eof-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md512", null ],
      [ "Step 36: WPT fatal single-byte (<tt>step36-wpt-fatal-single-byte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md514", null ],
      [ "Step 38: WPT UTF-8 surrogate encode vectors (<tt>step38-wpt-surrogates-utf8</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md515", null ],
      [ "Upcoming", "md_docs_plans_phase2_checklist.html#autotoc_md517", null ],
      [ "Step 39: WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md518", [
        [ "WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>) — DONE", "md_docs_plans_phase2_checklist.html#autotoc_md519", null ],
        [ "Label lookup API (<tt>step39-label-lookup</tt> or <tt>step40-label-lookup</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md520", null ]
      ] ],
      [ "Step 41: <tt>sniff_encoding</tt> BOM detection (<tt>step41-sniff-encoding</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md521", null ],
      [ "Step 42: Umbrella header (<tt>step42-umbrella-header</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md522", null ],
      [ "Step 43: <tt>transcode_string</tt> one-shot function (<tt>step43-transcode-string</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md523", null ],
      [ "Step 44: Coverage audit of <tt>transcode_string</tt> (<tt>step44-coverage-audit</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md524", null ],
      [ "Step 45: <tt>transcode_string</tt> label overload (<tt>step45-transcode-string-label</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md525", null ],
      [ "Step 46: <tt>transcode_view</tt> pipe composition helper (<tt>step46-transcode-view</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md526", null ],
      [ "Step 47: <tt>transcode_view</tt> negative compile test + consteval test (<tt>step47-transcode-view-negative-consteval</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md527", null ],
      [ "Step 48: <tt>sniff_encoding</tt> negative compile test + Python lint fix (<tt>step48-sniff-negative-lint-fix</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md528", null ],
      [ "Step 49: <tt>whatwg_decode_view</tt> coverage improvement (<tt>step49-decode-view-coverage</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md529", null ],
      [ "Step 50: iconv view boundary-condition tests (<tt>step50-iconv-boundary-tests</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md530", null ],
      [ "Step 51: iconv stateful flush (<tt>step51-iconv-stateful-flush</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md531", null ],
      [ "Step 52: real-iconv ISO-2022-JP flush integration tests (<tt>step52-iconv-iso2022jp-flush-test</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md532", null ],
      [ "Step 53: coverage improvements (<tt>step53-coverage-improvements</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md533", null ],
      [ "Step 53b: unreachable code annotations (<tt>step53b-unreachable-annotations</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md534", null ],
      [ "Step 54: C++23 module support audit (<tt>step54-module-audit</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md535", null ],
      [ "Step 55: Module integration and smoke tests (<tt>step55-module-tests</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md536", null ],
      [ "Step 56: Coverage analysis — whatwg_decode_view.hpp (<tt>step56-decode-coverage</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md537", null ],
      [ "Notes", "md_docs_plans_phase2_checklist.html#autotoc_md539", null ]
    ] ],
    [ "Phase 2: Full WHATWG Codec Implementation", "md_docs_plans_phase2_index.html", [
      [ "Overview", "md_docs_plans_phase2_index.html#autotoc_md541", null ],
      [ "WHATWG Encoding Inventory", "md_docs_plans_phase2_index.html#autotoc_md543", [
        [ "Already implemented", "md_docs_plans_phase2_index.html#autotoc_md544", null ],
        [ "Single-byte (22 total, 1 done)", "md_docs_plans_phase2_index.html#autotoc_md545", null ],
        [ "Multi-byte (7 codecs, each with unique algorithm)", "md_docs_plans_phase2_index.html#autotoc_md546", null ],
        [ "Other", "md_docs_plans_phase2_index.html#autotoc_md547", null ]
      ] ],
      [ "Steps", "md_docs_plans_phase2_index.html#autotoc_md549", [
        [ "Completed", "md_docs_plans_phase2_index.html#autotoc_md550", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step14-replacement-codec.md \"Step 14: `codec::replacement`\"", "md_docs_plans_phase2_index.html#autotoc_md551", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step15-x-user-defined.md \"Step 15: `codec::x_user_defined`\"", "md_docs_plans_phase2_index.html#autotoc_md552", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step16-single-byte-infra.md \"Step 16: Single-byte infrastructure + `windows_1252`\"", "md_docs_plans_phase2_index.html#autotoc_md553", null ]
        ] ],
        [ "Data tooling", "md_docs_plans_phase2_index.html#autotoc_md554", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step17-iso-8859-tables.md \"Step 17: Data tooling — WHATWG index acquisition & table generation\"", "md_docs_plans_phase2_index.html#autotoc_md555", null ]
        ] ],
        [ "Single-byte codec loop", "md_docs_plans_phase2_index.html#autotoc_md556", [
          [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md557", null ],
          [ "Step 19: Single-byte encoder infrastructure + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md558", null ]
        ] ],
        [ "UTF-8 encoder", "md_docs_plans_phase2_index.html#autotoc_md559", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step18-utf8-encoder.md \"Step 20: UTF-8 encoder\"", "md_docs_plans_phase2_index.html#autotoc_md560", null ]
        ] ],
        [ "UTF-16", "md_docs_plans_phase2_index.html#autotoc_md561", [
          [ "Step 21: UTF-16 decoder + encoder (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_index.html#autotoc_md562", null ]
        ] ],
        [ "Multi-byte codecs (decode + encode per codec)", "md_docs_plans_phase2_index.html#autotoc_md563", [
          [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_index.html#autotoc_md564", null ],
          [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_index.html#autotoc_md565", null ],
          [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_index.html#autotoc_md566", null ],
          [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_index.html#autotoc_md567", null ],
          [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md568", null ],
          [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md569", null ],
          [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_index.html#autotoc_md570", null ]
        ] ],
        [ "Composition", "md_docs_plans_phase2_index.html#autotoc_md571", [
          [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_index.html#autotoc_md572", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_plans_phase2_index.html#autotoc_md574", null ]
    ] ],
    [ "Phase 3 Progress Checklist", "md_docs_plans_phase3_checklist.html", [
      [ "P3-Step 1: Benchmark harness scaffolding (<tt>p3-step1-benchmark-harness</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md577", null ],
      [ "P3-Step 2: Benchmark corpus acquisition and provenance (<tt>p3-step2-benchmark-data</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md578", null ],
      [ "P3-Step 3: Benchmark fixture library and result schema (<tt>p3-step3-benchmark-fixtures</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md579", null ],
      [ "P3-Step 4: UTF-family and ASCII fast-path benchmarks (<tt>p3-step4-utf-benchmarks</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md580", null ],
      [ "P3-Step 5: Legacy WHATWG codec benchmarks (<tt>p3-step5-legacy-whatwg-benchmarks</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md581", null ],
      [ "P3-Step 6: <tt>iconv</tt> baselines (<tt>p3-step6-iconv-baselines</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md582", null ],
      [ "P3-Step 7: <tt>std::codecvt</tt> negative baseline (<tt>p3-step7-codecvt-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md583", null ],
      [ "P3-Step 8: <tt>encoding_rs</tt> baseline (<tt>p3-step8-encoding-rs-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md584", null ],
      [ "P3-Step 9: <tt>simdutf</tt> ceiling baseline (<tt>p3-step9-simdutf-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md585", null ],
      [ "P3-Step 10: Chunked/lazy boundary stress benchmarks (<tt>p3-step10-boundary-stress</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md586", null ],
      [ "P3-Step 11: Toolchain matrix automation (<tt>p3-step11-toolchain-matrix</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md587", null ],
      [ "P3-Step 12: Reporting, dashboard, and reproducibility pack (<tt>p3-step12-reporting-and-repro</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md588", null ],
      [ "Notes", "md_docs_plans_phase3_checklist.html#autotoc_md590", null ]
    ] ],
    [ "Phase 3 General Handoff — Read This First", "md_docs_plans_phase3_handoff.html", [
      [ "Project Identity", "md_docs_plans_phase3_handoff.html#autotoc_md593", null ],
      [ "Directory Layout", "md_docs_plans_phase3_handoff.html#autotoc_md594", null ],
      [ "Key Make Targets", "md_docs_plans_phase3_handoff.html#autotoc_md595", null ],
      [ "Coding Rules (Enforced)", "md_docs_plans_phase3_handoff.html#autotoc_md596", null ],
      [ "Library API Surface (Current)", "md_docs_plans_phase3_handoff.html#autotoc_md597", [
        [ "WhatWG codec views (enum-based, closed set)", "md_docs_plans_phase3_handoff.html#autotoc_md598", null ],
        [ "Pluggable codec protocol (type-based, open for extension)", "md_docs_plans_phase3_handoff.html#autotoc_md599", null ],
        [ "iconv views", "md_docs_plans_phase3_handoff.html#autotoc_md600", null ],
        [ "Bulk operations", "md_docs_plans_phase3_handoff.html#autotoc_md601", null ],
        [ "Runtime dispatch", "md_docs_plans_phase3_handoff.html#autotoc_md602", null ]
      ] ],
      [ "Catch2 Benchmark Macros", "md_docs_plans_phase3_handoff.html#autotoc_md603", null ],
      [ "iconv Mock Library", "md_docs_plans_phase3_handoff.html#autotoc_md604", null ],
      [ "Branch and Commit Discipline", "md_docs_plans_phase3_handoff.html#autotoc_md605", null ],
      [ "What to Write in handoff-next.md", "md_docs_plans_phase3_handoff.html#autotoc_md606", null ]
    ] ],
    [ "Phase 3: Benchmarking and Performance Characterization", "md_docs_plans_phase3_index.html", [
      [ "Overview", "md_docs_plans_phase3_index.html#autotoc_md608", null ],
      [ "Why Phase 3 Is Separate", "md_docs_plans_phase3_index.html#autotoc_md610", null ],
      [ "Phase Scope", "md_docs_plans_phase3_index.html#autotoc_md612", null ],
      [ "P3 Steps", "md_docs_plans_phase3_index.html#autotoc_md614", [
        [ "Harness and data foundations", "md_docs_plans_phase3_index.html#autotoc_md615", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step1-benchmark-harness.md \"P3-Step 1: Benchmark harness scaffolding\"", "md_docs_plans_phase3_index.html#autotoc_md616", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step2-benchmark-data.md \"P3-Step 2: Benchmark corpus acquisition and provenance\"", "md_docs_plans_phase3_index.html#autotoc_md617", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step3-benchmark-fixtures.md \"P3-Step 3: Benchmark fixture library and result schema\"", "md_docs_plans_phase3_index.html#autotoc_md618", null ]
        ] ],
        [ "Native <tt>beman::transcode</tt> measurement", "md_docs_plans_phase3_index.html#autotoc_md619", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step4-utf-benchmarks.md \"P3-Step 4: UTF-family and ASCII fast-path benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md620", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step5-legacy-whatwg-benchmarks.md \"P3-Step 5: Legacy WHATWG codec benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md621", null ]
        ] ],
        [ "Baseline implementations", "md_docs_plans_phase3_index.html#autotoc_md622", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step6-iconv-baselines.md \"P3-Step 6: `iconv` baselines\"", "md_docs_plans_phase3_index.html#autotoc_md623", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step7-codecvt-baseline.md \"P3-Step 7: `std::codecvt` negative baseline\"", "md_docs_plans_phase3_index.html#autotoc_md624", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step8-encoding-rs-baseline.md \"P3-Step 8: `encoding_rs` baseline\"", "md_docs_plans_phase3_index.html#autotoc_md625", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step9-simdutf-baseline.md \"P3-Step 9: `simdutf` ceiling baseline\"", "md_docs_plans_phase3_index.html#autotoc_md626", null ]
        ] ],
        [ "Stress paths and final reporting", "md_docs_plans_phase3_index.html#autotoc_md627", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step10-boundary-stress.md \"P3-Step 10: Chunked/lazy boundary stress benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md628", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step11-toolchain-matrix.md \"P3-Step 11: Toolchain matrix automation\"", "md_docs_plans_phase3_index.html#autotoc_md629", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step12-reporting-and-repro.md \"P3-Step 12: Reporting, dashboard, and reproducibility pack\"", "md_docs_plans_phase3_index.html#autotoc_md630", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_plans_phase3_index.html#autotoc_md632", null ],
      [ "Agent Execution Model", "md_docs_plans_phase3_index.html#autotoc_md634", [
        [ "Worktree Discipline", "md_docs_plans_phase3_index.html#autotoc_md635", null ],
        [ "What to write in handoff-next.md", "md_docs_plans_phase3_index.html#autotoc_md636", null ],
        [ "Rules", "md_docs_plans_phase3_index.html#autotoc_md637", null ]
      ] ]
    ] ],
    [ "Phase 4: API Surface Gap Closure", "md_docs_plans_phase4_index.html", [
      [ "Context", "md_docs_plans_phase4_index.html#autotoc_md639", null ],
      [ "Step Index", "md_docs_plans_phase4_index.html#autotoc_md640", null ],
      [ "Standing Conventions", "md_docs_plans_phase4_index.html#autotoc_md641", null ]
    ] ],
    [ "Phase 5: Specification Wording from the Headers", "md_docs_plans_phase5_index.html", [
      [ "Context", "md_docs_plans_phase5_index.html#autotoc_md643", null ],
      [ "Measurements taken before planning (2026-09-03)", "md_docs_plans_phase5_index.html#autotoc_md644", null ],
      [ "Decisions", "md_docs_plans_phase5_index.html#autotoc_md645", null ],
      [ "Step index", "md_docs_plans_phase5_index.html#autotoc_md646", null ],
      [ "Standing conventions", "md_docs_plans_phase5_index.html#autotoc_md647", null ],
      [ "External dependencies (work in <tt>~/src/specgen/main</tt>)", "md_docs_plans_phase5_index.html#autotoc_md648", [
        [ "The gathered-region pattern", "md_docs_plans_phase5_index.html#autotoc_md649", null ],
        [ "Closed", "md_docs_plans_phase5_index.html#autotoc_md650", null ]
      ] ]
    ] ],
    [ "Phase 6: Acting on the 2026-09-11 Review", "md_docs_plans_phase6_index.html", [
      [ "Context", "md_docs_plans_phase6_index.html#autotoc_md652", null ],
      [ "Measurements taken before planning (2026-09-11)", "md_docs_plans_phase6_index.html#autotoc_md653", null ],
      [ "Decisions taken", "md_docs_plans_phase6_index.html#autotoc_md654", [
        [ "S-01, resolved", "md_docs_plans_phase6_index.html#autotoc_md655", null ]
      ] ],
      [ "The wording gate, and why it is revised first", "md_docs_plans_phase6_index.html#autotoc_md656", null ],
      [ "Step index", "md_docs_plans_phase6_index.html#autotoc_md657", null ],
      [ "Standing conventions", "md_docs_plans_phase6_index.html#autotoc_md658", null ],
      [ "Verification", "md_docs_plans_phase6_index.html#autotoc_md659", null ],
      [ "Risks", "md_docs_plans_phase6_index.html#autotoc_md660", null ]
    ] ],
    [ "Step 14: <tt>codec::replacement</tt> Decoder", "md_docs_plans_step14_replacement_codec.html", [
      [ "Goal", "md_docs_plans_step14_replacement_codec.html#autotoc_md663", null ],
      [ "WHATWG Specification", "md_docs_plans_step14_replacement_codec.html#autotoc_md664", null ],
      [ "Files Modified", "md_docs_plans_step14_replacement_codec.html#autotoc_md665", [
        [ "<tt>include/beman/transcode/whatwg_decode_view.hpp</tt>", "md_docs_plans_step14_replacement_codec.html#autotoc_md666", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode.test.cpp</tt>", "md_docs_plans_step14_replacement_codec.html#autotoc_md667", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode_or_error.test.cpp</tt>", "md_docs_plans_step14_replacement_codec.html#autotoc_md668", null ]
      ] ],
      [ "No New Files Created", "md_docs_plans_step14_replacement_codec.html#autotoc_md669", null ],
      [ "No Negative Compile Tests", "md_docs_plans_step14_replacement_codec.html#autotoc_md670", null ],
      [ "Verification", "md_docs_plans_step14_replacement_codec.html#autotoc_md671", null ]
    ] ],
    [ "Step 15: <tt>codec::x_user_defined</tt> Decoder", "md_docs_plans_step15_x_user_defined.html", [
      [ "Goal", "md_docs_plans_step15_x_user_defined.html#autotoc_md674", null ],
      [ "WHATWG Specification", "md_docs_plans_step15_x_user_defined.html#autotoc_md675", null ],
      [ "Files Created", "md_docs_plans_step15_x_user_defined.html#autotoc_md676", [
        [ "<tt>include/beman/transcode/detail/x_user_defined.hpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md677", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step15_x_user_defined.html#autotoc_md678", [
        [ "<tt>include/beman/transcode/whatwg_decode_view.hpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md679", null ],
        [ "<tt>include/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md680", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode.test.cpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md681", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode_or_error.test.cpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md682", null ]
      ] ],
      [ "Verification", "md_docs_plans_step15_x_user_defined.html#autotoc_md683", null ]
    ] ],
    [ "Step 16: Single-Byte Table Infrastructure + <tt>codec::windows_1252</tt>", "md_docs_plans_step16_single_byte_infra.html", [
      [ "Goal", "md_docs_plans_step16_single_byte_infra.html#autotoc_md686", null ],
      [ "WHATWG Specification — Single-Byte Decoder Algorithm", "md_docs_plans_step16_single_byte_infra.html#autotoc_md687", null ],
      [ "Files Created", "md_docs_plans_step16_single_byte_infra.html#autotoc_md688", [
        [ "<tt>include/beman/transcode/detail/single_byte.hpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md689", null ],
        [ "<tt>include/beman/transcode/detail/tables/windows_1252.hpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md690", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step16_single_byte_infra.html#autotoc_md691", [
        [ "<tt>include/beman/transcode/whatwg_decode_view.hpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md692", null ],
        [ "<tt>include/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md693", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode.test.cpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md694", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode_or_error.test.cpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md695", null ]
      ] ],
      [ "Verification", "md_docs_plans_step16_single_byte_infra.html#autotoc_md696", null ]
    ] ],
    [ "Step 17: Data Tooling — WHATWG Index Acquisition & Table Generation", "md_docs_plans_step17_iso_8859_tables.html", [
      [ "Goal", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md699", null ],
      [ "Why This Step Exists", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md700", null ],
      [ "WHATWG Index Data Sources", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md701", [
        [ "Single-byte indexes (one per codec)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md702", null ],
        [ "Multi-byte indexes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md703", null ],
        [ "Metadata", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md704", null ]
      ] ],
      [ "Directory Layout", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md705", [
        [ "<tt>docs/whatwg/</tt> — pristine upstream data", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md706", null ],
        [ "<tt>docs/wpt/</tt> — Web Platform Tests (future)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md707", null ],
        [ "<tt>data/</tt> — our derived/processed artifacts", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md708", null ]
      ] ],
      [ "Provenance: <tt>docs/whatwg/SOURCE.md</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md709", null ],
      [ "Provenance: <tt>docs/whatwg/source.bib</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md710", null ],
      [ "Python Quality Requirements", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md711", null ],
      [ "Deliverables", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md712", [
        [ "Script: <tt>tools/download_indexes.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md713", null ],
        [ "Script: <tt>tools/generate_tables.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md714", null ],
        [ "Tests: <tt>tools/tests/</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md715", null ],
        [ "Generated files (single-byte, 22 total)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md716", null ]
      ] ],
      [ "Procedure", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md717", null ],
      [ "Verification", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md718", null ],
      [ "Notes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md719", null ]
    ] ],
    [ "Step 18: UTF-8 Encoder — <tt>whatwg_encode_view<codec::utf_8></tt>", "md_docs_plans_step18_utf8_encoder.html", [
      [ "Goal", "md_docs_plans_step18_utf8_encoder.html#autotoc_md722", null ],
      [ "WHATWG Specification — UTF-8 Encoder", "md_docs_plans_step18_utf8_encoder.html#autotoc_md723", null ],
      [ "Files Created", "md_docs_plans_step18_utf8_encoder.html#autotoc_md724", [
        [ "<tt>include/beman/transcode/detail/utf8_encode.hpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md725", null ],
        [ "<tt>include/beman/transcode/whatwg_encode_view.hpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md726", null ],
        [ "<tt>tests/beman/transcode/whatwg_encode.test.cpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md727", null ],
        [ "<tt>tests/beman/transcode/whatwg_encode_or_error.test.cpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md728", null ],
        [ "Negative compile test: <tt>whatwg_encode_reject_char_range_fail.cpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md729", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step18_utf8_encoder.html#autotoc_md730", [
        [ "<tt>include/beman/transcode/detail/concepts.hpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md731", null ],
        [ "<tt>include/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md732", null ],
        [ "<tt>tests/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md733", null ]
      ] ],
      [ "Verification", "md_docs_plans_step18_utf8_encoder.html#autotoc_md734", null ]
    ] ],
    [ "Step 19: Round-Trip Composition Tests", "md_docs_plans_step19_roundtrip.html", [
      [ "Goal", "md_docs_plans_step19_roundtrip.html#autotoc_md737", null ],
      [ "Composition Syntax", "md_docs_plans_step19_roundtrip.html#autotoc_md738", null ],
      [ "Files Created", "md_docs_plans_step19_roundtrip.html#autotoc_md739", [
        [ "<tt>tests/beman/transcode/roundtrip.test.cpp</tt>", "md_docs_plans_step19_roundtrip.html#autotoc_md740", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step19_roundtrip.html#autotoc_md741", [
        [ "<tt>tests/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step19_roundtrip.html#autotoc_md742", null ]
      ] ],
      [ "Verification", "md_docs_plans_step19_roundtrip.html#autotoc_md743", null ],
      [ "Notes", "md_docs_plans_step19_roundtrip.html#autotoc_md744", null ]
    ] ],
    [ "review-findings-2026-09-11-closeout", "md_docs_review_findings_2026_09_11_closeout.html", [
      [ "Repository review findings — 2026-09-11 closeout", "md_docs_review_findings_2026_09_11_closeout.html#autotoc_md745", [
        [ "Finding accounting", "md_docs_review_findings_2026_09_11_closeout.html#autotoc_md746", null ],
        [ "Verification results", "md_docs_review_findings_2026_09_11_closeout.html#autotoc_md747", null ]
      ] ]
    ] ],
    [ "review-findings-2026-09-11", "md_docs_review_findings_2026_09_11.html", [
      [ "Repository review findings — 2026-09-11", "md_docs_review_findings_2026_09_11.html#autotoc_md748", [
        [ "Must address before proposal or public release", "md_docs_review_findings_2026_09_11.html#autotoc_md749", [
          [ "C-01: <tt>sniff_encoding</tt> consumes single-pass input", "md_docs_review_findings_2026_09_11.html#autotoc_md750", null ],
          [ "C-02: A too-small iconv view buffer silently loses valid input", "md_docs_review_findings_2026_09_11.html#autotoc_md751", null ],
          [ "C-03: Streaming iconv flush is attempted only once", "md_docs_review_findings_2026_09_11.html#autotoc_md752", null ],
          [ "C-04: Lossy iconv bulk replacement can corrupt the destination encoding", "md_docs_review_findings_2026_09_11.html#autotoc_md753", null ],
          [ "C-05: iconv open and unexpected system errors are conflated", "md_docs_review_findings_2026_09_11.html#autotoc_md754", null ],
          [ "C-06: <tt>null_term_view</tt> has no reachable-terminator precondition", "md_docs_review_findings_2026_09_11.html#autotoc_md755", null ],
          [ "C-07: Runtime transcoding uses empty output as an unsupported-target error", "md_docs_review_findings_2026_09_11.html#autotoc_md756", null ],
          [ "P-01: The optional/portable iconv story does not match the build", "md_docs_review_findings_2026_09_11.html#autotoc_md757", null ],
          [ "P-02: Proposed ISO C++ wording exposes an undefined POSIX type", "md_docs_review_findings_2026_09_11.html#autotoc_md758", null ]
        ] ],
        [ "Specification and API decisions", "md_docs_review_findings_2026_09_11.html#autotoc_md759", [
          [ "S-01: Invalid <tt>char32_t</tt> is both a precondition violation and a reported error", "md_docs_review_findings_2026_09_11.html#autotoc_md760", null ],
          [ "S-02: Lossy iconv APIs do not share one recovery policy", "md_docs_review_findings_2026_09_11.html#autotoc_md761", null ],
          [ "S-03: <tt>random_access_decode_codec_type</tt> reserves U+FFFD as an error sentinel", "md_docs_review_findings_2026_09_11.html#autotoc_md762", null ],
          [ "S-04: Review generated free-standing synopsis prose", "md_docs_review_findings_2026_09_11.html#autotoc_md763", null ],
          [ "S-05: Narrow or substantiate factual and performance claims", "md_docs_review_findings_2026_09_11.html#autotoc_md764", null ]
        ] ],
        [ "Documentation, paper, build, and CI drift", "md_docs_review_findings_2026_09_11.html#autotoc_md765", [
          [ "D-01: Duplicate author email key", "md_docs_review_findings_2026_09_11.html#autotoc_md766", null ],
          [ "D-02: Feature-test macro placeholders disagree", "md_docs_review_findings_2026_09_11.html#autotoc_md767", null ],
          [ "D-03: Documentation claims unsupported UTF-32 BOM sniffing", "md_docs_review_findings_2026_09_11.html#autotoc_md768", null ],
          [ "D-04: Paper and related proposal revisions have drifted", "md_docs_review_findings_2026_09_11.html#autotoc_md769", null ],
          [ "D-05: Build instructions name the wrong test framework and language level", "md_docs_review_findings_2026_09_11.html#autotoc_md770", null ],
          [ "D-06: Advertised compiler support disagrees with CMake", "md_docs_review_findings_2026_09_11.html#autotoc_md771", null ],
          [ "D-07: The documented <tt>table_codec</tt> API and example are not public examples", "md_docs_review_findings_2026_09_11.html#autotoc_md772", null ],
          [ "D-08: ICU does not expose the POSIX iconv API as stated", "md_docs_review_findings_2026_09_11.html#autotoc_md773", null ],
          [ "D-09: A root file literally named <tt>-</tt> contains stale generated wording", "md_docs_review_findings_2026_09_11.html#autotoc_md774", null ],
          [ "D-10: <tt>docs/plans/handoff-next.md</tt> is stale operational guidance", "md_docs_review_findings_2026_09_11.html#autotoc_md775", null ],
          [ "D-11: “Manual Lint” CI repeats the normal lint target", "md_docs_review_findings_2026_09_11.html#autotoc_md776", null ],
          [ "D-12: Module packaging needs a clean install-consumer test", "md_docs_review_findings_2026_09_11.html#autotoc_md777", null ],
          [ "D-13: Telemetry script shebang is on line 2", "md_docs_review_findings_2026_09_11.html#autotoc_md778", null ]
        ] ],
        [ "Verification results and gaps", "md_docs_review_findings_2026_09_11.html#autotoc_md779", null ],
        [ "Suggested triage order", "md_docs_review_findings_2026_09_11.html#autotoc_md780", null ]
      ] ]
    ] ],
    [ "WHATWG Encoding Standard — Index Data", "md_docs_whatwg_SOURCE.html", [
      [ "File Provenance", "md_docs_whatwg_SOURCE.html#autotoc_md782", null ],
      [ "Full Checksums", "md_docs_whatwg_SOURCE.html#autotoc_md783", null ]
    ] ],
    [ "wording-outline", "md_docs_wording_outline.html", [
      [ "Wording outline", "md_docs_wording_outline.html#autotoc_md784", [
        [ "How the tree maps onto fragments", "md_docs_wording_outline.html#autotoc_md785", null ],
        [ "<tt><transcode></tt>", "md_docs_wording_outline.html#autotoc_md786", null ],
        [ "<tt><null_term></tt>", "md_docs_wording_outline.html#autotoc_md787", null ],
        [ "Exposition-only, omitted, and not proposed", "md_docs_wording_outline.html#autotoc_md788", [
          [ "Exposition-only (<tt>\\expos</tt>)", "md_docs_wording_outline.html#autotoc_md789", null ],
          [ "Omitted (<tt>\\omit</tt>)", "md_docs_wording_outline.html#autotoc_md790", null ],
          [ "Not proposed at all", "md_docs_wording_outline.html#autotoc_md791", null ]
        ] ],
        [ "The <tt>detail::</tt> audit (Step 3 task 3)", "md_docs_wording_outline.html#autotoc_md792", null ],
        [ "Decisions this outline settles", "md_docs_wording_outline.html#autotoc_md793", null ],
        [ "Open, and deliberately not settled here", "md_docs_wording_outline.html#autotoc_md794", null ]
      ] ]
    ] ],
    [ "Web Platform Tests — Encoding Test Vectors", "md_docs_wpt_SOURCE.html", [
      [ "File Provenance", "md_docs_wpt_SOURCE.html#autotoc_md796", null ],
      [ "Full Checksums", "md_docs_wpt_SOURCE.html#autotoc_md797", null ]
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
"classbeman_1_1transcoding_1_1iconv__transcode__or__error__view_1_1iterator.html#af8a9b4adecc82c71253756d09f293102",
"classbeman_1_1transcoding_1_1random__access__whatwg__encode__view.html#a691b2756443ceba36117bd283cb569ef",
"classbeman_1_1transcoding_1_1whatwg__decode__view_1_1iterator.html#ace8c5f8b648684ca94ac0684306a80f8",
"ibm866_8hpp_source.html",
"md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md214",
"md_docs_plans_phase2_checklist.html#autotoc_md499",
"md_docs_review_findings_2026_09_11.html#autotoc_md773",
"structbeman_1_1transcoding_1_1iconv__transcode__closure.html#a7552c4a543709a5a9611b6d84aa5570e"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';