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
          [ "1. Collapse the <tt>_view</tt> / <tt>_or_error_view</tt> pairs into one template (highest value)", "md_docs_p2728_alignment.html#autotoc_md74", null ],
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
    [ "Phase 2 Progress Checklist", "md_docs_plans_phase2_checklist.html", [
      [ "Step 14: <tt>codec::replacement</tt> (<tt>step14-replacement-codec</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md286", null ],
      [ "Step 15: <tt>codec::x_user_defined</tt> (<tt>step15-x-user-defined</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md287", null ],
      [ "Step 16: Single-byte infra + <tt>windows_1252</tt> (<tt>step16-single-byte-infra</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md288", null ],
      [ "Step 17: Data tooling (<tt>step17-data-tooling</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md289", null ],
      [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md290", null ],
      [ "Step 19: Single-byte encoder infra + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md291", null ],
      [ "Step 20: UTF-8 encoder (<tt>step20-utf8-encoder</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md292", null ],
      [ "Step 21: UTF-16 decode + encode (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md293", null ],
      [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md294", null ],
      [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md295", null ],
      [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md296", null ],
      [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md297", null ],
      [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md298", null ],
      [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md299", null ],
      [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md300", null ],
      [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md301", null ],
      [ "Step 30: WPT test vector integration (<tt>step30-wpt-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md303", null ],
      [ "Step 31: WPT ISO-2022-JP + single-byte exhaustive (<tt>step31-wpt-iso2022jp-singlebyte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md304", null ],
      [ "Step 32: ISO-2022-JP full WHATWG conformance (<tt>step32-iso2022jp-conformance</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md305", null ],
      [ "Step 33: WPT UTF-16 surrogate conformance (<tt>step33-wpt-utf16-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md306", null ],
      [ "Step 34: WPT fatal mode vectors (<tt>step34-wpt-fatal-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md307", null ],
      [ "Step 35: WPT BOM stripping conformance (<tt>step35-wpt-bom-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md308", null ],
      [ "Step 37: WPT EOF vectors + Big5 conformance fix (<tt>step37-wpt-eof-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md310", null ],
      [ "Step 36: WPT fatal single-byte (<tt>step36-wpt-fatal-single-byte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md312", null ],
      [ "Step 38: WPT UTF-8 surrogate encode vectors (<tt>step38-wpt-surrogates-utf8</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md313", null ],
      [ "Upcoming", "md_docs_plans_phase2_checklist.html#autotoc_md315", null ],
      [ "Step 39: WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md316", [
        [ "WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>) — DONE", "md_docs_plans_phase2_checklist.html#autotoc_md317", null ],
        [ "Label lookup API (<tt>step39-label-lookup</tt> or <tt>step40-label-lookup</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md318", null ]
      ] ],
      [ "Step 41: <tt>sniff_encoding</tt> BOM detection (<tt>step41-sniff-encoding</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md319", null ],
      [ "Step 42: Umbrella header (<tt>step42-umbrella-header</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md320", null ],
      [ "Step 43: <tt>transcode_string</tt> one-shot function (<tt>step43-transcode-string</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md321", null ],
      [ "Step 44: Coverage audit of <tt>transcode_string</tt> (<tt>step44-coverage-audit</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md322", null ],
      [ "Step 45: <tt>transcode_string</tt> label overload (<tt>step45-transcode-string-label</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md323", null ],
      [ "Step 46: <tt>transcode_view</tt> pipe composition helper (<tt>step46-transcode-view</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md324", null ],
      [ "Step 47: <tt>transcode_view</tt> negative compile test + consteval test (<tt>step47-transcode-view-negative-consteval</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md325", null ],
      [ "Step 48: <tt>sniff_encoding</tt> negative compile test + Python lint fix (<tt>step48-sniff-negative-lint-fix</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md326", null ],
      [ "Step 49: <tt>whatwg_decode_view</tt> coverage improvement (<tt>step49-decode-view-coverage</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md327", null ],
      [ "Step 50: iconv view boundary-condition tests (<tt>step50-iconv-boundary-tests</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md328", null ],
      [ "Step 51: iconv stateful flush (<tt>step51-iconv-stateful-flush</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md329", null ],
      [ "Step 52: real-iconv ISO-2022-JP flush integration tests (<tt>step52-iconv-iso2022jp-flush-test</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md330", null ],
      [ "Step 53: coverage improvements (<tt>step53-coverage-improvements</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md331", null ],
      [ "Step 53b: unreachable code annotations (<tt>step53b-unreachable-annotations</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md332", null ],
      [ "Step 54: C++23 module support audit (<tt>step54-module-audit</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md333", null ],
      [ "Step 55: Module integration and smoke tests (<tt>step55-module-tests</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md334", null ],
      [ "Step 56: Coverage analysis — whatwg_decode_view.hpp (<tt>step56-decode-coverage</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md335", null ],
      [ "Notes", "md_docs_plans_phase2_checklist.html#autotoc_md337", null ]
    ] ],
    [ "Phase 2: Full WHATWG Codec Implementation", "md_docs_plans_phase2_index.html", [
      [ "Overview", "md_docs_plans_phase2_index.html#autotoc_md339", null ],
      [ "WHATWG Encoding Inventory", "md_docs_plans_phase2_index.html#autotoc_md341", [
        [ "Already implemented", "md_docs_plans_phase2_index.html#autotoc_md342", null ],
        [ "Single-byte (22 total, 1 done)", "md_docs_plans_phase2_index.html#autotoc_md343", null ],
        [ "Multi-byte (7 codecs, each with unique algorithm)", "md_docs_plans_phase2_index.html#autotoc_md344", null ],
        [ "Other", "md_docs_plans_phase2_index.html#autotoc_md345", null ]
      ] ],
      [ "Steps", "md_docs_plans_phase2_index.html#autotoc_md347", [
        [ "Completed", "md_docs_plans_phase2_index.html#autotoc_md348", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step14-replacement-codec.md \"Step 14: `codec::replacement`\"", "md_docs_plans_phase2_index.html#autotoc_md349", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step15-x-user-defined.md \"Step 15: `codec::x_user_defined`\"", "md_docs_plans_phase2_index.html#autotoc_md350", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step16-single-byte-infra.md \"Step 16: Single-byte infrastructure + `windows_1252`\"", "md_docs_plans_phase2_index.html#autotoc_md351", null ]
        ] ],
        [ "Data tooling", "md_docs_plans_phase2_index.html#autotoc_md352", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step17-iso-8859-tables.md \"Step 17: Data tooling — WHATWG index acquisition & table generation\"", "md_docs_plans_phase2_index.html#autotoc_md353", null ]
        ] ],
        [ "Single-byte codec loop", "md_docs_plans_phase2_index.html#autotoc_md354", [
          [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md355", null ],
          [ "Step 19: Single-byte encoder infrastructure + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md356", null ]
        ] ],
        [ "UTF-8 encoder", "md_docs_plans_phase2_index.html#autotoc_md357", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step18-utf8-encoder.md \"Step 20: UTF-8 encoder\"", "md_docs_plans_phase2_index.html#autotoc_md358", null ]
        ] ],
        [ "UTF-16", "md_docs_plans_phase2_index.html#autotoc_md359", [
          [ "Step 21: UTF-16 decoder + encoder (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_index.html#autotoc_md360", null ]
        ] ],
        [ "Multi-byte codecs (decode + encode per codec)", "md_docs_plans_phase2_index.html#autotoc_md361", [
          [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_index.html#autotoc_md362", null ],
          [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_index.html#autotoc_md363", null ],
          [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_index.html#autotoc_md364", null ],
          [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_index.html#autotoc_md365", null ],
          [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md366", null ],
          [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md367", null ],
          [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_index.html#autotoc_md368", null ]
        ] ],
        [ "Composition", "md_docs_plans_phase2_index.html#autotoc_md369", [
          [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_index.html#autotoc_md370", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_plans_phase2_index.html#autotoc_md372", null ]
    ] ],
    [ "Phase 3 Progress Checklist", "md_docs_plans_phase3_checklist.html", [
      [ "P3-Step 1: Benchmark harness scaffolding (<tt>p3-step1-benchmark-harness</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md375", null ],
      [ "P3-Step 2: Benchmark corpus acquisition and provenance (<tt>p3-step2-benchmark-data</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md376", null ],
      [ "P3-Step 3: Benchmark fixture library and result schema (<tt>p3-step3-benchmark-fixtures</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md377", null ],
      [ "P3-Step 4: UTF-family and ASCII fast-path benchmarks (<tt>p3-step4-utf-benchmarks</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md378", null ],
      [ "P3-Step 5: Legacy WHATWG codec benchmarks (<tt>p3-step5-legacy-whatwg-benchmarks</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md379", null ],
      [ "P3-Step 6: <tt>iconv</tt> baselines (<tt>p3-step6-iconv-baselines</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md380", null ],
      [ "P3-Step 7: <tt>std::codecvt</tt> negative baseline (<tt>p3-step7-codecvt-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md381", null ],
      [ "P3-Step 8: <tt>encoding_rs</tt> baseline (<tt>p3-step8-encoding-rs-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md382", null ],
      [ "P3-Step 9: <tt>simdutf</tt> ceiling baseline (<tt>p3-step9-simdutf-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md383", null ],
      [ "P3-Step 10: Chunked/lazy boundary stress benchmarks (<tt>p3-step10-boundary-stress</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md384", null ],
      [ "P3-Step 11: Toolchain matrix automation (<tt>p3-step11-toolchain-matrix</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md385", null ],
      [ "P3-Step 12: Reporting, dashboard, and reproducibility pack (<tt>p3-step12-reporting-and-repro</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md386", null ],
      [ "Notes", "md_docs_plans_phase3_checklist.html#autotoc_md388", null ]
    ] ],
    [ "Phase 3 General Handoff — Read This First", "md_docs_plans_phase3_handoff.html", [
      [ "Project Identity", "md_docs_plans_phase3_handoff.html#autotoc_md391", null ],
      [ "Directory Layout", "md_docs_plans_phase3_handoff.html#autotoc_md392", null ],
      [ "Key Make Targets", "md_docs_plans_phase3_handoff.html#autotoc_md393", null ],
      [ "Coding Rules (Enforced)", "md_docs_plans_phase3_handoff.html#autotoc_md394", null ],
      [ "Library API Surface (Current)", "md_docs_plans_phase3_handoff.html#autotoc_md395", [
        [ "WhatWG codec views (enum-based, closed set)", "md_docs_plans_phase3_handoff.html#autotoc_md396", null ],
        [ "Pluggable codec protocol (type-based, open for extension)", "md_docs_plans_phase3_handoff.html#autotoc_md397", null ],
        [ "iconv views", "md_docs_plans_phase3_handoff.html#autotoc_md398", null ],
        [ "Bulk operations", "md_docs_plans_phase3_handoff.html#autotoc_md399", null ],
        [ "Runtime dispatch", "md_docs_plans_phase3_handoff.html#autotoc_md400", null ]
      ] ],
      [ "Catch2 Benchmark Macros", "md_docs_plans_phase3_handoff.html#autotoc_md401", null ],
      [ "iconv Mock Library", "md_docs_plans_phase3_handoff.html#autotoc_md402", null ],
      [ "Branch and Commit Discipline", "md_docs_plans_phase3_handoff.html#autotoc_md403", null ],
      [ "What to Write in handoff-next.md", "md_docs_plans_phase3_handoff.html#autotoc_md404", null ]
    ] ],
    [ "Phase 3: Benchmarking and Performance Characterization", "md_docs_plans_phase3_index.html", [
      [ "Overview", "md_docs_plans_phase3_index.html#autotoc_md406", null ],
      [ "Why Phase 3 Is Separate", "md_docs_plans_phase3_index.html#autotoc_md408", null ],
      [ "Phase Scope", "md_docs_plans_phase3_index.html#autotoc_md410", null ],
      [ "P3 Steps", "md_docs_plans_phase3_index.html#autotoc_md412", [
        [ "Harness and data foundations", "md_docs_plans_phase3_index.html#autotoc_md413", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step1-benchmark-harness.md \"P3-Step 1: Benchmark harness scaffolding\"", "md_docs_plans_phase3_index.html#autotoc_md414", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step2-benchmark-data.md \"P3-Step 2: Benchmark corpus acquisition and provenance\"", "md_docs_plans_phase3_index.html#autotoc_md415", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step3-benchmark-fixtures.md \"P3-Step 3: Benchmark fixture library and result schema\"", "md_docs_plans_phase3_index.html#autotoc_md416", null ]
        ] ],
        [ "Native <tt>beman::transcode</tt> measurement", "md_docs_plans_phase3_index.html#autotoc_md417", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step4-utf-benchmarks.md \"P3-Step 4: UTF-family and ASCII fast-path benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md418", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step5-legacy-whatwg-benchmarks.md \"P3-Step 5: Legacy WHATWG codec benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md419", null ]
        ] ],
        [ "Baseline implementations", "md_docs_plans_phase3_index.html#autotoc_md420", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step6-iconv-baselines.md \"P3-Step 6: `iconv` baselines\"", "md_docs_plans_phase3_index.html#autotoc_md421", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step7-codecvt-baseline.md \"P3-Step 7: `std::codecvt` negative baseline\"", "md_docs_plans_phase3_index.html#autotoc_md422", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step8-encoding-rs-baseline.md \"P3-Step 8: `encoding_rs` baseline\"", "md_docs_plans_phase3_index.html#autotoc_md423", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step9-simdutf-baseline.md \"P3-Step 9: `simdutf` ceiling baseline\"", "md_docs_plans_phase3_index.html#autotoc_md424", null ]
        ] ],
        [ "Stress paths and final reporting", "md_docs_plans_phase3_index.html#autotoc_md425", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step10-boundary-stress.md \"P3-Step 10: Chunked/lazy boundary stress benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md426", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step11-toolchain-matrix.md \"P3-Step 11: Toolchain matrix automation\"", "md_docs_plans_phase3_index.html#autotoc_md427", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step12-reporting-and-repro.md \"P3-Step 12: Reporting, dashboard, and reproducibility pack\"", "md_docs_plans_phase3_index.html#autotoc_md428", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_plans_phase3_index.html#autotoc_md430", null ],
      [ "Agent Execution Model", "md_docs_plans_phase3_index.html#autotoc_md432", [
        [ "Worktree Discipline", "md_docs_plans_phase3_index.html#autotoc_md433", null ],
        [ "What to write in handoff-next.md", "md_docs_plans_phase3_index.html#autotoc_md434", null ],
        [ "Rules", "md_docs_plans_phase3_index.html#autotoc_md435", null ]
      ] ]
    ] ],
    [ "Phase 4: API Surface Gap Closure", "md_docs_plans_phase4_index.html", [
      [ "Context", "md_docs_plans_phase4_index.html#autotoc_md437", null ],
      [ "Step Index", "md_docs_plans_phase4_index.html#autotoc_md438", null ],
      [ "Standing Conventions", "md_docs_plans_phase4_index.html#autotoc_md439", null ]
    ] ],
    [ "Step 14: <tt>codec::replacement</tt> Decoder", "md_docs_plans_step14_replacement_codec.html", [
      [ "Goal", "md_docs_plans_step14_replacement_codec.html#autotoc_md442", null ],
      [ "WHATWG Specification", "md_docs_plans_step14_replacement_codec.html#autotoc_md443", null ],
      [ "Files Modified", "md_docs_plans_step14_replacement_codec.html#autotoc_md444", [
        [ "<tt>include/beman/transcode/whatwg_decode_view.hpp</tt>", "md_docs_plans_step14_replacement_codec.html#autotoc_md445", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode.test.cpp</tt>", "md_docs_plans_step14_replacement_codec.html#autotoc_md446", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode_or_error.test.cpp</tt>", "md_docs_plans_step14_replacement_codec.html#autotoc_md447", null ]
      ] ],
      [ "No New Files Created", "md_docs_plans_step14_replacement_codec.html#autotoc_md448", null ],
      [ "No Negative Compile Tests", "md_docs_plans_step14_replacement_codec.html#autotoc_md449", null ],
      [ "Verification", "md_docs_plans_step14_replacement_codec.html#autotoc_md450", null ]
    ] ],
    [ "Step 15: <tt>codec::x_user_defined</tt> Decoder", "md_docs_plans_step15_x_user_defined.html", [
      [ "Goal", "md_docs_plans_step15_x_user_defined.html#autotoc_md453", null ],
      [ "WHATWG Specification", "md_docs_plans_step15_x_user_defined.html#autotoc_md454", null ],
      [ "Files Created", "md_docs_plans_step15_x_user_defined.html#autotoc_md455", [
        [ "<tt>include/beman/transcode/detail/x_user_defined.hpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md456", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step15_x_user_defined.html#autotoc_md457", [
        [ "<tt>include/beman/transcode/whatwg_decode_view.hpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md458", null ],
        [ "<tt>include/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md459", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode.test.cpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md460", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode_or_error.test.cpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md461", null ]
      ] ],
      [ "Verification", "md_docs_plans_step15_x_user_defined.html#autotoc_md462", null ]
    ] ],
    [ "Step 16: Single-Byte Table Infrastructure + <tt>codec::windows_1252</tt>", "md_docs_plans_step16_single_byte_infra.html", [
      [ "Goal", "md_docs_plans_step16_single_byte_infra.html#autotoc_md465", null ],
      [ "WHATWG Specification — Single-Byte Decoder Algorithm", "md_docs_plans_step16_single_byte_infra.html#autotoc_md466", null ],
      [ "Files Created", "md_docs_plans_step16_single_byte_infra.html#autotoc_md467", [
        [ "<tt>include/beman/transcode/detail/single_byte.hpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md468", null ],
        [ "<tt>include/beman/transcode/detail/tables/windows_1252.hpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md469", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step16_single_byte_infra.html#autotoc_md470", [
        [ "<tt>include/beman/transcode/whatwg_decode_view.hpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md471", null ],
        [ "<tt>include/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md472", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode.test.cpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md473", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode_or_error.test.cpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md474", null ]
      ] ],
      [ "Verification", "md_docs_plans_step16_single_byte_infra.html#autotoc_md475", null ]
    ] ],
    [ "Step 17: Data Tooling — WHATWG Index Acquisition & Table Generation", "md_docs_plans_step17_iso_8859_tables.html", [
      [ "Goal", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md478", null ],
      [ "Why This Step Exists", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md479", null ],
      [ "WHATWG Index Data Sources", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md480", [
        [ "Single-byte indexes (one per codec)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md481", null ],
        [ "Multi-byte indexes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md482", null ],
        [ "Metadata", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md483", null ]
      ] ],
      [ "Directory Layout", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md484", [
        [ "<tt>docs/whatwg/</tt> — pristine upstream data", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md485", null ],
        [ "<tt>docs/wpt/</tt> — Web Platform Tests (future)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md486", null ],
        [ "<tt>data/</tt> — our derived/processed artifacts", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md487", null ]
      ] ],
      [ "Provenance: <tt>docs/whatwg/SOURCE.md</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md488", null ],
      [ "Provenance: <tt>docs/whatwg/source.bib</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md489", null ],
      [ "Python Quality Requirements", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md490", null ],
      [ "Deliverables", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md491", [
        [ "Script: <tt>tools/download_indexes.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md492", null ],
        [ "Script: <tt>tools/generate_tables.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md493", null ],
        [ "Tests: <tt>tools/tests/</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md494", null ],
        [ "Generated files (single-byte, 22 total)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md495", null ]
      ] ],
      [ "Procedure", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md496", null ],
      [ "Verification", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md497", null ],
      [ "Notes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md498", null ]
    ] ],
    [ "Step 18: UTF-8 Encoder — <tt>whatwg_encode_view<codec::utf_8></tt>", "md_docs_plans_step18_utf8_encoder.html", [
      [ "Goal", "md_docs_plans_step18_utf8_encoder.html#autotoc_md501", null ],
      [ "WHATWG Specification — UTF-8 Encoder", "md_docs_plans_step18_utf8_encoder.html#autotoc_md502", null ],
      [ "Files Created", "md_docs_plans_step18_utf8_encoder.html#autotoc_md503", [
        [ "<tt>include/beman/transcode/detail/utf8_encode.hpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md504", null ],
        [ "<tt>include/beman/transcode/whatwg_encode_view.hpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md505", null ],
        [ "<tt>tests/beman/transcode/whatwg_encode.test.cpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md506", null ],
        [ "<tt>tests/beman/transcode/whatwg_encode_or_error.test.cpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md507", null ],
        [ "Negative compile test: <tt>whatwg_encode_reject_char_range_fail.cpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md508", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step18_utf8_encoder.html#autotoc_md509", [
        [ "<tt>include/beman/transcode/detail/concepts.hpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md510", null ],
        [ "<tt>include/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md511", null ],
        [ "<tt>tests/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md512", null ]
      ] ],
      [ "Verification", "md_docs_plans_step18_utf8_encoder.html#autotoc_md513", null ]
    ] ],
    [ "Step 19: Round-Trip Composition Tests", "md_docs_plans_step19_roundtrip.html", [
      [ "Goal", "md_docs_plans_step19_roundtrip.html#autotoc_md516", null ],
      [ "Composition Syntax", "md_docs_plans_step19_roundtrip.html#autotoc_md517", null ],
      [ "Files Created", "md_docs_plans_step19_roundtrip.html#autotoc_md518", [
        [ "<tt>tests/beman/transcode/roundtrip.test.cpp</tt>", "md_docs_plans_step19_roundtrip.html#autotoc_md519", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step19_roundtrip.html#autotoc_md520", [
        [ "<tt>tests/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step19_roundtrip.html#autotoc_md521", null ]
      ] ],
      [ "Verification", "md_docs_plans_step19_roundtrip.html#autotoc_md522", null ],
      [ "Notes", "md_docs_plans_step19_roundtrip.html#autotoc_md523", null ]
    ] ],
    [ "WHATWG Encoding Standard — Index Data", "md_docs_whatwg_SOURCE.html", [
      [ "File Provenance", "md_docs_whatwg_SOURCE.html#autotoc_md525", null ],
      [ "Full Checksums", "md_docs_whatwg_SOURCE.html#autotoc_md526", null ]
    ] ],
    [ "The Story of Failing to Find a Way Out: Reverse-Parsing Legacy Encodings", "md_docs_why_forward_iterator.html", [
      [ "Act I: The Overlapping Abyss", "md_docs_why_forward_iterator.html#autotoc_md528", [
        [ "The Specific Example", "md_docs_why_forward_iterator.html#autotoc_md529", null ]
      ] ],
      [ "Act II: The Illusion of the Anchor", "md_docs_why_forward_iterator.html#autotoc_md531", [
        [ "The Specific Example", "md_docs_why_forward_iterator.html#autotoc_md532", null ]
      ] ],
      [ "Act III: State Machine Amnesia", "md_docs_why_forward_iterator.html#autotoc_md534", [
        [ "The Specific Example", "md_docs_why_forward_iterator.html#autotoc_md535", null ],
        [ "The Final Verdict", "md_docs_why_forward_iterator.html#autotoc_md536", null ]
      ] ]
    ] ],
    [ "Web Platform Tests — Encoding Test Vectors", "md_docs_wpt_SOURCE.html", [
      [ "File Provenance", "md_docs_wpt_SOURCE.html#autotoc_md538", null ],
      [ "Full Checksums", "md_docs_wpt_SOURCE.html#autotoc_md539", null ]
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
"classbeman_1_1transcoding_1_1encode__view_1_1iterator.html#a1959514af85afb54f413022c7f564bfd",
"classbeman_1_1transcoding_1_1random__access__decode__or__error__view.html#ac8898cab09ecafd3848229b46a99d874",
"classbeman_1_1transcoding_1_1random__access__whatwg__decode__view_1_1iterator.html#aa53dc46eeae6db4f5a39e2b45d709956",
"classbeman_1_1transcoding_1_1single__byte__encode__view_1_1iterator.html#a9b5b0c1fb874370d7d2281c4b06f4656",
"classbeman_1_1transcoding_1_1whatwg__encode__or__error__view_1_1iterator.html#a91ea895c81f92bd9b36f824488572292",
"iso__8859__8_8hpp_source.html",
"md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md267",
"namespaceanonymous__namespace_02custom__single__byte__decoder_8cpp_03.html#a8f50f955a599a74ab40ff93c21f57efb",
"structbeman_1_1transcoding_1_1single__byte__decode__closure.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';