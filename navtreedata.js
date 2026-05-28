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
    [ "Benchmark Platform Notes", "md_docs_benchmarks_PLATFORM_NOTES.html", [
      [ "<tt>simdutf</tt> ceiling baseline (optional — requires explicit CMake opt-in)", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md36", [
        [ "Enabling", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md37", null ],
        [ "Files", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md38", null ],
        [ "Platform availability", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md39", null ],
        [ "Benchmarks provided", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md40", null ],
        [ "Expected comparison", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md41", null ]
      ] ],
      [ "<tt>encoding_rs</tt> baseline (optional — requires Rust/Cargo)", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md43", [
        [ "Enabling", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md44", null ],
        [ "Files", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md45", null ],
        [ "Platform availability", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md46", null ],
        [ "Benchmarks provided", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md47", null ],
        [ "Results on this machine (Asan build — indicative only)", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md48", null ]
      ] ],
      [ "<tt>std::codecvt</tt> / <tt>std::wstring_convert</tt> (negative baseline)", "md_docs_benchmarks_PLATFORM_NOTES.html#autotoc_md50", null ]
    ] ],
    [ "Benchmarking Guide", "md_docs_benchmarks_README.html", [
      [ "Overview", "md_docs_benchmarks_README.html#autotoc_md53", null ],
      [ "Quick Start", "md_docs_benchmarks_README.html#autotoc_md55", null ],
      [ "Corpus Files", "md_docs_benchmarks_README.html#autotoc_md57", [
        [ "Small Corpus (checked-in)", "md_docs_benchmarks_README.html#autotoc_md58", null ],
        [ "Large Corpus (downloaded)", "md_docs_benchmarks_README.html#autotoc_md59", null ]
      ] ],
      [ "Generating Reports", "md_docs_benchmarks_README.html#autotoc_md61", [
        [ "From a Single XML File", "md_docs_benchmarks_README.html#autotoc_md62", null ],
        [ "With Large Corpus Sizes", "md_docs_benchmarks_README.html#autotoc_md63", null ],
        [ "With a Vega-Lite Chart", "md_docs_benchmarks_README.html#autotoc_md64", null ],
        [ "Comparing Multiple Toolchains", "md_docs_benchmarks_README.html#autotoc_md65", null ]
      ] ],
      [ "Throughput Formula", "md_docs_benchmarks_README.html#autotoc_md67", null ],
      [ "Reproducing Published Results", "md_docs_benchmarks_README.html#autotoc_md69", null ],
      [ "Benchmark Suites", "md_docs_benchmarks_README.html#autotoc_md71", null ]
    ] ],
    [ "Running Benchmarks", "md_docs_benchmarks_RUNNING.html", [
      [ "Quick Smoke Run (Default Build)", "md_docs_benchmarks_RUNNING.html#autotoc_md73", null ],
      [ "Optimized Builds via CMake Presets", "md_docs_benchmarks_RUNNING.html#autotoc_md74", [
        [ "GCC -O3 (smoke)", "md_docs_benchmarks_RUNNING.html#autotoc_md75", null ],
        [ "GCC -O3 -flto (smoke)", "md_docs_benchmarks_RUNNING.html#autotoc_md76", null ],
        [ "Clang -O3 -flto (smoke)", "md_docs_benchmarks_RUNNING.html#autotoc_md77", null ]
      ] ],
      [ "Full Compiler Matrix", "md_docs_benchmarks_RUNNING.html#autotoc_md78", null ],
      [ "Capturing Results", "md_docs_benchmarks_RUNNING.html#autotoc_md79", null ],
      [ "Environment Metadata", "md_docs_benchmarks_RUNNING.html#autotoc_md80", null ],
      [ "Manually Configuring a Preset", "md_docs_benchmarks_RUNNING.html#autotoc_md81", null ],
      [ "Matrix Slices", "md_docs_benchmarks_RUNNING.html#autotoc_md82", null ],
      [ "Notes", "md_docs_benchmarks_RUNNING.html#autotoc_md83", null ]
    ] ],
    [ "Benchmark Corpora — Wikipedia Mars Article", "md_docs_benchmarks_SOURCE.html", [
      [ "Purpose", "md_docs_benchmarks_SOURCE.html#autotoc_md85", null ],
      [ "Source", "md_docs_benchmarks_SOURCE.html#autotoc_md86", null ],
      [ "License", "md_docs_benchmarks_SOURCE.html#autotoc_md87", [
        [ "BibTeX", "md_docs_benchmarks_SOURCE.html#autotoc_md88", null ]
      ] ],
      [ "Encoding Conversions", "md_docs_benchmarks_SOURCE.html#autotoc_md89", null ],
      [ "Fallback Corpus", "md_docs_benchmarks_SOURCE.html#autotoc_md90", null ],
      [ "Manifest", "md_docs_benchmarks_SOURCE.html#autotoc_md91", null ]
    ] ],
    [ "C++29 Transcoding Architecture", "md_docs_C__29_Transcoding_Architecture.html", [
      [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md92", [
        [ "<strong>Executive Summary</strong>", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md93", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md94", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md95", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md96", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md97", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md98", null ],
        [ "—", "md_docs_C__29_Transcoding_Architecture.html#autotoc_md99", null ]
      ] ]
    ] ],
    [ "Testing C++ Range Adapters over <tt>iconv</tt>", "md_docs_iconv_testing.html", [
      [ "1. The <tt>iconv</tt> View Boundary Conditions", "md_docs_iconv_testing.html#autotoc_md101", [
        [ "A. Buffer Exhaustion (<tt>E2BIG</tt>)", "md_docs_iconv_testing.html#autotoc_md102", null ],
        [ "B. The Split Multi-byte Sequence (<tt>EINVAL</tt>)", "md_docs_iconv_testing.html#autotoc_md103", null ],
        [ "C. Invalid Byte Sequences (<tt>EILSEQ</tt>)", "md_docs_iconv_testing.html#autotoc_md104", null ],
        [ "D. End-of-Range Stateful Flush", "md_docs_iconv_testing.html#autotoc_md105", null ]
      ] ],
      [ "2. Mocking <tt>iconv</tt> for Deterministic View Testing", "md_docs_iconv_testing.html#autotoc_md107", null ],
      [ "3. Testing Execution Plan", "md_docs_iconv_testing.html#autotoc_md109", null ]
    ] ],
    [ "Handoff: Phase 3 Complete", "md_docs_plans_handoff_next.html", [
      [ "Completed", "md_docs_plans_handoff_next.html#autotoc_md111", null ],
      [ "What was done", "md_docs_plans_handoff_next.html#autotoc_md112", null ],
      [ "Sample Output", "md_docs_plans_handoff_next.html#autotoc_md113", null ],
      [ "Files created", "md_docs_plans_handoff_next.html#autotoc_md114", null ],
      [ "Files modified", "md_docs_plans_handoff_next.html#autotoc_md115", null ],
      [ "Implementation notes", "md_docs_plans_handoff_next.html#autotoc_md116", null ],
      [ "Phase 3 Status: COMPLETE", "md_docs_plans_handoff_next.html#autotoc_md117", null ],
      [ "Current State", "md_docs_plans_handoff_next.html#autotoc_md118", null ],
      [ "Branch State", "md_docs_plans_handoff_next.html#autotoc_md119", null ]
    ] ],
    [ "P3-Step 1: Benchmark Harness Scaffolding", "md_docs_plans_p3_step1_benchmark_harness.html", [
      [ "Goal", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md122", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md123", [
        [ "Key files to reference", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md124", null ],
        [ "Build conventions", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md125", null ],
        [ "Library usage in benchmark code", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md126", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md127", null ],
      [ "Procedure", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md128", null ],
      [ "Makefile Target Pattern", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md129", null ],
      [ "Verification", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md130", null ],
      [ "Handoff to Step 2", "md_docs_plans_p3_step1_benchmark_harness.html#autotoc_md131", null ]
    ] ],
    [ "P3-Step 10: Chunked/Lazy Boundary Stress Benchmarks", "md_docs_plans_p3_step10_boundary_stress.html", [
      [ "Goal", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md134", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md135", null ],
      [ "Deliverables", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md136", null ],
      [ "Constraints", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md137", null ],
      [ "Procedure", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md138", null ],
      [ "Verification", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md139", null ],
      [ "Notes", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md140", null ],
      [ "Handoff to Step 11", "md_docs_plans_p3_step10_boundary_stress.html#autotoc_md141", null ]
    ] ],
    [ "P3-Step 11: Toolchain Matrix Automation", "md_docs_plans_p3_step11_toolchain_matrix.html", [
      [ "Goal", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md144", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md145", null ],
      [ "Matrix To Support", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md146", null ],
      [ "Deliverables", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md147", null ],
      [ "Constraints", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md148", null ],
      [ "Procedure", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md149", null ],
      [ "Verification", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md150", null ],
      [ "Notes", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md151", null ],
      [ "Handoff to Step 12", "md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md152", null ]
    ] ],
    [ "P3-Step 12: Reporting, Dashboard, and Reproducibility Pack", "md_docs_plans_p3_step12_reporting_and_repro.html", [
      [ "Goal", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md155", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md156", null ],
      [ "Deliverables", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md157", null ],
      [ "Files Expected", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md158", null ],
      [ "Constraints", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md159", null ],
      [ "Procedure", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md160", null ],
      [ "Verification", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md161", null ],
      [ "Notes", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md162", null ],
      [ "Handoff (Phase 3 Complete)", "md_docs_plans_p3_step12_reporting_and_repro.html#autotoc_md163", null ]
    ] ],
    [ "P3-Step 2: Benchmark Corpus Acquisition and Provenance", "md_docs_plans_p3_step2_benchmark_data.html", [
      [ "Goal", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md166", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md167", [
        [ "MediaWiki API for extraction", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md168", null ],
        [ "Python encoding conversion", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md169", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md170", null ],
      [ "Corpus Manifest Schema", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md171", null ],
      [ "Fallback Corpus (checked in)", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md172", null ],
      [ "Procedure", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md173", null ],
      [ "Verification", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md174", null ],
      [ "Handoff to Step 3", "md_docs_plans_p3_step2_benchmark_data.html#autotoc_md175", null ]
    ] ],
    [ "P3-Step 3: Benchmark Fixture Library and Result Schema", "md_docs_plans_p3_step3_benchmark_fixtures.html", [
      [ "Goal", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md178", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md179", null ],
      [ "Deliverables", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md180", null ],
      [ "API Design", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md181", [
        [ "benchmark_fixture.hpp", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md182", null ],
        [ "benchmark_sink.hpp", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md183", null ]
      ] ],
      [ "Procedure", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md184", null ],
      [ "Verification", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md185", null ],
      [ "Handoff to Step 4", "md_docs_plans_p3_step3_benchmark_fixtures.html#autotoc_md186", null ]
    ] ],
    [ "P3-Step 4: UTF-Family and ASCII Fast-Path Benchmarks", "md_docs_plans_p3_step4_utf_benchmarks.html", [
      [ "Goal", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md189", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md190", null ],
      [ "Deliverables", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md191", null ],
      [ "Benchmark Cases", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md192", null ],
      [ "Procedure", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md193", null ],
      [ "Verification", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md194", null ],
      [ "Handoff to Step 5", "md_docs_plans_p3_step4_utf_benchmarks.html#autotoc_md195", null ]
    ] ],
    [ "P3-Step 5: Legacy WHATWG Codec Benchmarks", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html", [
      [ "Goal", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md198", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md199", null ],
      [ "Deliverables", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md200", null ],
      [ "Benchmark Cases", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md201", [
        [ "whatwg_benchmarks.bench.cpp", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md202", null ],
        [ "pluggable_codec_benchmarks.bench.cpp", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md203", null ]
      ] ],
      [ "Procedure", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md204", null ],
      [ "Verification", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md205", null ],
      [ "Handoff to Step 6", "md_docs_plans_p3_step5_legacy_whatwg_benchmarks.html#autotoc_md206", null ]
    ] ],
    [ "P3-Step 6: <tt>iconv</tt> Baselines", "md_docs_plans_p3_step6_iconv_baselines.html", [
      [ "Goal", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md209", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md210", null ],
      [ "Deliverables", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md211", null ],
      [ "Benchmark Cases", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md212", null ],
      [ "Constraints", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md213", null ],
      [ "Procedure", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md214", null ],
      [ "Verification", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md215", null ],
      [ "Handoff to Step 7", "md_docs_plans_p3_step6_iconv_baselines.html#autotoc_md216", null ]
    ] ],
    [ "P3-Step 7: <tt>std::codecvt</tt> Negative Baseline", "md_docs_plans_p3_step7_codecvt_baseline.html", [
      [ "Goal", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md219", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md220", null ],
      [ "Deliverables", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md221", null ],
      [ "Constraints", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md222", null ],
      [ "Procedure", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md223", null ],
      [ "Verification", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md224", null ],
      [ "Notes", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md225", null ],
      [ "Handoff to Step 8", "md_docs_plans_p3_step7_codecvt_baseline.html#autotoc_md226", null ]
    ] ],
    [ "P3-Step 8: <tt>encoding_rs</tt> Baseline", "md_docs_plans_p3_step8_encoding_rs_baseline.html", [
      [ "Goal", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md229", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md230", null ],
      [ "Deliverables", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md231", null ],
      [ "Constraints", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md232", null ],
      [ "Procedure", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md233", null ],
      [ "Verification", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md234", null ],
      [ "Notes", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md235", null ],
      [ "Handoff to Step 9", "md_docs_plans_p3_step8_encoding_rs_baseline.html#autotoc_md236", null ]
    ] ],
    [ "P3-Step 9: <tt>simdutf</tt> Ceiling Baseline", "md_docs_plans_p3_step9_simdutf_baseline.html", [
      [ "Goal", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md239", null ],
      [ "Context for Executing Agent", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md240", null ],
      [ "Deliverables", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md241", null ],
      [ "Constraints", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md242", null ],
      [ "Procedure", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md243", null ],
      [ "Verification", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md244", null ],
      [ "Notes", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md245", null ],
      [ "Handoff to Step 10", "md_docs_plans_p3_step9_simdutf_baseline.html#autotoc_md246", null ]
    ] ],
    [ "P4-Step 1: Pluggable Encode View", "md_docs_plans_p4_step1_pluggable_encode.html", [
      [ "Goal", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md249", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md250", [
        [ "Key files to reference", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md251", null ],
        [ "Key differences from decode_view", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md252", null ],
        [ "The encode_codec concept", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md253", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md254", null ],
      [ "Constraints", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md255", null ],
      [ "Procedure", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md256", null ],
      [ "Verification", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md257", null ],
      [ "Handoff to Step 2", "md_docs_plans_p4_step1_pluggable_encode.html#autotoc_md258", null ]
    ] ],
    [ "P4-Step 2: Pluggable Bulk Operations", "md_docs_plans_p4_step2_pluggable_bulk.html", [
      [ "Goal", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md261", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md262", [
        [ "Signature difference", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md263", null ],
        [ "Fast path opportunity", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md264", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md265", null ],
      [ "Procedure", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md266", null ],
      [ "Verification", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md267", null ],
      [ "Handoff to Step 3", "md_docs_plans_p4_step2_pluggable_bulk.html#autotoc_md268", null ]
    ] ],
    [ "P4-Step 3: Pluggable Transcode Pipeline", "md_docs_plans_p4_step3_pluggable_transcode.html", [
      [ "Goal", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md271", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md272", [
        [ "Implementation", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md273", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md274", null ],
      [ "Procedure", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md275", null ],
      [ "Verification", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md276", null ],
      [ "Handoff", "md_docs_plans_p4_step3_pluggable_transcode.html#autotoc_md277", null ]
    ] ],
    [ "P4-Step 4: iconv Bulk Operations", "md_docs_plans_p4_step4_iconv_bulk.html", [
      [ "Goal", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md280", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md281", [
        [ "Dependency injection", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md282", null ],
        [ "Key files to reference", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md283", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md284", null ],
      [ "Constraints", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md285", null ],
      [ "Procedure", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md286", null ],
      [ "Verification", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md287", null ],
      [ "Handoff to Step 5", "md_docs_plans_p4_step4_iconv_bulk.html#autotoc_md288", null ]
    ] ],
    [ "P4-Step 5: iconv Null-Terminated Input", "md_docs_plans_p4_step5_iconv_null_term.html", [
      [ "Goal", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md291", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md292", [
        [ "Possible outcomes", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md293", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md294", null ],
      [ "Procedure", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md295", null ],
      [ "Verification", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md296", null ],
      [ "Handoff to Step 6", "md_docs_plans_p4_step5_iconv_null_term.html#autotoc_md297", null ]
    ] ],
    [ "P4-Step 6: Error Enum Coherence", "md_docs_plans_p4_step6_error_coherence.html", [
      [ "Goal", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md300", null ],
      [ "Context for Executing Agent", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md301", [
        [ "whatwg_error (include/beman/transcode/detail/error.hpp)", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md302", null ],
        [ "decode_error (include/beman/transcode/detail/codec_result.hpp)", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md303", null ],
        [ "iconv_error (include/beman/transcode/detail/error.hpp)", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md304", null ],
        [ "Why they differ", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md305", null ],
        [ "The problem", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md306", null ]
      ] ],
      [ "Deliverables", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md307", null ],
      [ "Procedure", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md308", null ],
      [ "Verification", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md309", null ],
      [ "Handoff (Phase 4 Complete)", "md_docs_plans_p4_step6_error_coherence.html#autotoc_md310", null ]
    ] ],
    [ "Phase 2 Progress Checklist", "md_docs_plans_phase2_checklist.html", [
      [ "Step 14: <tt>codec::replacement</tt> (<tt>step14-replacement-codec</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md313", null ],
      [ "Step 15: <tt>codec::x_user_defined</tt> (<tt>step15-x-user-defined</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md314", null ],
      [ "Step 16: Single-byte infra + <tt>windows_1252</tt> (<tt>step16-single-byte-infra</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md315", null ],
      [ "Step 17: Data tooling (<tt>step17-data-tooling</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md316", null ],
      [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md317", null ],
      [ "Step 19: Single-byte encoder infra + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md318", null ],
      [ "Step 20: UTF-8 encoder (<tt>step20-utf8-encoder</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md319", null ],
      [ "Step 21: UTF-16 decode + encode (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md320", null ],
      [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md321", null ],
      [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md322", null ],
      [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md323", null ],
      [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md324", null ],
      [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md325", null ],
      [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md326", null ],
      [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md327", null ],
      [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md328", null ],
      [ "Step 30: WPT test vector integration (<tt>step30-wpt-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md330", null ],
      [ "Step 31: WPT ISO-2022-JP + single-byte exhaustive (<tt>step31-wpt-iso2022jp-singlebyte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md331", null ],
      [ "Step 32: ISO-2022-JP full WHATWG conformance (<tt>step32-iso2022jp-conformance</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md332", null ],
      [ "Step 33: WPT UTF-16 surrogate conformance (<tt>step33-wpt-utf16-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md333", null ],
      [ "Step 34: WPT fatal mode vectors (<tt>step34-wpt-fatal-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md334", null ],
      [ "Step 35: WPT BOM stripping conformance (<tt>step35-wpt-bom-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md335", null ],
      [ "Step 37: WPT EOF vectors + Big5 conformance fix (<tt>step37-wpt-eof-vectors</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md337", null ],
      [ "Step 36: WPT fatal single-byte (<tt>step36-wpt-fatal-single-byte</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md339", null ],
      [ "Step 38: WPT UTF-8 surrogate encode vectors (<tt>step38-wpt-surrogates-utf8</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md340", null ],
      [ "Upcoming", "md_docs_plans_phase2_checklist.html#autotoc_md342", null ],
      [ "Step 39: WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md343", [
        [ "WPT TextEncoder UTF-16 surrogate round-trip (<tt>step39-wpt-encoder-surrogates</tt>) — DONE", "md_docs_plans_phase2_checklist.html#autotoc_md344", null ],
        [ "Label lookup API (<tt>step39-label-lookup</tt> or <tt>step40-label-lookup</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md345", null ]
      ] ],
      [ "Step 41: <tt>sniff_encoding</tt> BOM detection (<tt>step41-sniff-encoding</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md346", null ],
      [ "Step 42: Umbrella header (<tt>step42-umbrella-header</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md347", null ],
      [ "Step 43: <tt>transcode_string</tt> one-shot function (<tt>step43-transcode-string</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md348", null ],
      [ "Step 44: Coverage audit of <tt>transcode_string</tt> (<tt>step44-coverage-audit</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md349", null ],
      [ "Step 45: <tt>transcode_string</tt> label overload (<tt>step45-transcode-string-label</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md350", null ],
      [ "Step 46: <tt>transcode_view</tt> pipe composition helper (<tt>step46-transcode-view</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md351", null ],
      [ "Step 47: <tt>transcode_view</tt> negative compile test + consteval test (<tt>step47-transcode-view-negative-consteval</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md352", null ],
      [ "Step 48: <tt>sniff_encoding</tt> negative compile test + Python lint fix (<tt>step48-sniff-negative-lint-fix</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md353", null ],
      [ "Step 49: <tt>whatwg_decode_view</tt> coverage improvement (<tt>step49-decode-view-coverage</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md354", null ],
      [ "Step 50: iconv view boundary-condition tests (<tt>step50-iconv-boundary-tests</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md355", null ],
      [ "Step 51: iconv stateful flush (<tt>step51-iconv-stateful-flush</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md356", null ],
      [ "Step 52: real-iconv ISO-2022-JP flush integration tests (<tt>step52-iconv-iso2022jp-flush-test</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md357", null ],
      [ "Step 53: coverage improvements (<tt>step53-coverage-improvements</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md358", null ],
      [ "Step 53b: unreachable code annotations (<tt>step53b-unreachable-annotations</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md359", null ],
      [ "Step 54: C++23 module support audit (<tt>step54-module-audit</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md360", null ],
      [ "Step 55: Module integration and smoke tests (<tt>step55-module-tests</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md361", null ],
      [ "Step 56: Coverage analysis — whatwg_decode_view.hpp (<tt>step56-decode-coverage</tt>)", "md_docs_plans_phase2_checklist.html#autotoc_md362", null ],
      [ "Notes", "md_docs_plans_phase2_checklist.html#autotoc_md364", null ]
    ] ],
    [ "Phase 2: Full WHATWG Codec Implementation", "md_docs_plans_phase2_index.html", [
      [ "Overview", "md_docs_plans_phase2_index.html#autotoc_md366", null ],
      [ "WHATWG Encoding Inventory", "md_docs_plans_phase2_index.html#autotoc_md368", [
        [ "Already implemented", "md_docs_plans_phase2_index.html#autotoc_md369", null ],
        [ "Single-byte (22 total, 1 done)", "md_docs_plans_phase2_index.html#autotoc_md370", null ],
        [ "Multi-byte (7 codecs, each with unique algorithm)", "md_docs_plans_phase2_index.html#autotoc_md371", null ],
        [ "Other", "md_docs_plans_phase2_index.html#autotoc_md372", null ]
      ] ],
      [ "Steps", "md_docs_plans_phase2_index.html#autotoc_md374", [
        [ "Completed", "md_docs_plans_phase2_index.html#autotoc_md375", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step14-replacement-codec.md \"Step 14: `codec::replacement`\"", "md_docs_plans_phase2_index.html#autotoc_md376", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step15-x-user-defined.md \"Step 15: `codec::x_user_defined`\"", "md_docs_plans_phase2_index.html#autotoc_md377", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step16-single-byte-infra.md \"Step 16: Single-byte infrastructure + `windows_1252`\"", "md_docs_plans_phase2_index.html#autotoc_md378", null ]
        ] ],
        [ "Data tooling", "md_docs_plans_phase2_index.html#autotoc_md379", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step17-iso-8859-tables.md \"Step 17: Data tooling — WHATWG index acquisition & table generation\"", "md_docs_plans_phase2_index.html#autotoc_md380", null ]
        ] ],
        [ "Single-byte codec loop", "md_docs_plans_phase2_index.html#autotoc_md381", [
          [ "Step 18: All single-byte decoders (<tt>step18-all-single-byte-decode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md382", null ],
          [ "Step 19: Single-byte encoder infrastructure + all encoders (<tt>step19-single-byte-encode</tt>)", "md_docs_plans_phase2_index.html#autotoc_md383", null ]
        ] ],
        [ "UTF-8 encoder", "md_docs_plans_phase2_index.html#autotoc_md384", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/step18-utf8-encoder.md \"Step 20: UTF-8 encoder\"", "md_docs_plans_phase2_index.html#autotoc_md385", null ]
        ] ],
        [ "UTF-16", "md_docs_plans_phase2_index.html#autotoc_md386", [
          [ "Step 21: UTF-16 decoder + encoder (<tt>step21-utf16</tt>)", "md_docs_plans_phase2_index.html#autotoc_md387", null ]
        ] ],
        [ "Multi-byte codecs (decode + encode per codec)", "md_docs_plans_phase2_index.html#autotoc_md388", [
          [ "Step 22: GBK decode + encode (<tt>step22-gbk</tt>)", "md_docs_plans_phase2_index.html#autotoc_md389", null ],
          [ "Step 23: gb18030 decode + encode (<tt>step23-gb18030</tt>)", "md_docs_plans_phase2_index.html#autotoc_md390", null ],
          [ "Step 24: Big5 decode + encode (<tt>step24-big5</tt>)", "md_docs_plans_phase2_index.html#autotoc_md391", null ],
          [ "Step 25: Shift_JIS decode + encode (<tt>step25-shift-jis</tt>)", "md_docs_plans_phase2_index.html#autotoc_md392", null ],
          [ "Step 26: EUC-JP decode + encode (<tt>step26-euc-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md393", null ],
          [ "Step 27: ISO-2022-JP decode + encode (<tt>step27-iso-2022-jp</tt>)", "md_docs_plans_phase2_index.html#autotoc_md394", null ],
          [ "Step 28: EUC-KR decode + encode (<tt>step28-euc-kr</tt>)", "md_docs_plans_phase2_index.html#autotoc_md395", null ]
        ] ],
        [ "Composition", "md_docs_plans_phase2_index.html#autotoc_md396", [
          [ "Step 29: Round-trip composition (<tt>step29-roundtrip</tt>)", "md_docs_plans_phase2_index.html#autotoc_md397", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_plans_phase2_index.html#autotoc_md399", null ]
    ] ],
    [ "Phase 3 Progress Checklist", "md_docs_plans_phase3_checklist.html", [
      [ "P3-Step 1: Benchmark harness scaffolding (<tt>p3-step1-benchmark-harness</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md402", null ],
      [ "P3-Step 2: Benchmark corpus acquisition and provenance (<tt>p3-step2-benchmark-data</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md403", null ],
      [ "P3-Step 3: Benchmark fixture library and result schema (<tt>p3-step3-benchmark-fixtures</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md404", null ],
      [ "P3-Step 4: UTF-family and ASCII fast-path benchmarks (<tt>p3-step4-utf-benchmarks</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md405", null ],
      [ "P3-Step 5: Legacy WHATWG codec benchmarks (<tt>p3-step5-legacy-whatwg-benchmarks</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md406", null ],
      [ "P3-Step 6: <tt>iconv</tt> baselines (<tt>p3-step6-iconv-baselines</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md407", null ],
      [ "P3-Step 7: <tt>std::codecvt</tt> negative baseline (<tt>p3-step7-codecvt-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md408", null ],
      [ "P3-Step 8: <tt>encoding_rs</tt> baseline (<tt>p3-step8-encoding-rs-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md409", null ],
      [ "P3-Step 9: <tt>simdutf</tt> ceiling baseline (<tt>p3-step9-simdutf-baseline</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md410", null ],
      [ "P3-Step 10: Chunked/lazy boundary stress benchmarks (<tt>p3-step10-boundary-stress</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md411", null ],
      [ "P3-Step 11: Toolchain matrix automation (<tt>p3-step11-toolchain-matrix</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md412", null ],
      [ "P3-Step 12: Reporting, dashboard, and reproducibility pack (<tt>p3-step12-reporting-and-repro</tt>)", "md_docs_plans_phase3_checklist.html#autotoc_md413", null ],
      [ "Notes", "md_docs_plans_phase3_checklist.html#autotoc_md415", null ]
    ] ],
    [ "Phase 3 General Handoff — Read This First", "md_docs_plans_phase3_handoff.html", [
      [ "Project Identity", "md_docs_plans_phase3_handoff.html#autotoc_md418", null ],
      [ "Directory Layout", "md_docs_plans_phase3_handoff.html#autotoc_md419", null ],
      [ "Key Make Targets", "md_docs_plans_phase3_handoff.html#autotoc_md420", null ],
      [ "Coding Rules (Enforced)", "md_docs_plans_phase3_handoff.html#autotoc_md421", null ],
      [ "Library API Surface (Current)", "md_docs_plans_phase3_handoff.html#autotoc_md422", [
        [ "WhatWG codec views (enum-based, closed set)", "md_docs_plans_phase3_handoff.html#autotoc_md423", null ],
        [ "Pluggable codec protocol (type-based, open for extension)", "md_docs_plans_phase3_handoff.html#autotoc_md424", null ],
        [ "iconv views", "md_docs_plans_phase3_handoff.html#autotoc_md425", null ],
        [ "Bulk operations", "md_docs_plans_phase3_handoff.html#autotoc_md426", null ],
        [ "Runtime dispatch", "md_docs_plans_phase3_handoff.html#autotoc_md427", null ]
      ] ],
      [ "Catch2 Benchmark Macros", "md_docs_plans_phase3_handoff.html#autotoc_md428", null ],
      [ "iconv Mock Library", "md_docs_plans_phase3_handoff.html#autotoc_md429", null ],
      [ "Branch and Commit Discipline", "md_docs_plans_phase3_handoff.html#autotoc_md430", null ],
      [ "What to Write in handoff-next.md", "md_docs_plans_phase3_handoff.html#autotoc_md431", null ]
    ] ],
    [ "Phase 3: Benchmarking and Performance Characterization", "md_docs_plans_phase3_index.html", [
      [ "Overview", "md_docs_plans_phase3_index.html#autotoc_md433", null ],
      [ "Why Phase 3 Is Separate", "md_docs_plans_phase3_index.html#autotoc_md435", null ],
      [ "Phase Scope", "md_docs_plans_phase3_index.html#autotoc_md437", null ],
      [ "P3 Steps", "md_docs_plans_phase3_index.html#autotoc_md439", [
        [ "Harness and data foundations", "md_docs_plans_phase3_index.html#autotoc_md440", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step1-benchmark-harness.md \"P3-Step 1: Benchmark harness scaffolding\"", "md_docs_plans_phase3_index.html#autotoc_md441", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step2-benchmark-data.md \"P3-Step 2: Benchmark corpus acquisition and provenance\"", "md_docs_plans_phase3_index.html#autotoc_md442", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step3-benchmark-fixtures.md \"P3-Step 3: Benchmark fixture library and result schema\"", "md_docs_plans_phase3_index.html#autotoc_md443", null ]
        ] ],
        [ "Native <tt>beman::transcode</tt> measurement", "md_docs_plans_phase3_index.html#autotoc_md444", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step4-utf-benchmarks.md \"P3-Step 4: UTF-family and ASCII fast-path benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md445", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step5-legacy-whatwg-benchmarks.md \"P3-Step 5: Legacy WHATWG codec benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md446", null ]
        ] ],
        [ "Baseline implementations", "md_docs_plans_phase3_index.html#autotoc_md447", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step6-iconv-baselines.md \"P3-Step 6: `iconv` baselines\"", "md_docs_plans_phase3_index.html#autotoc_md448", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step7-codecvt-baseline.md \"P3-Step 7: `std::codecvt` negative baseline\"", "md_docs_plans_phase3_index.html#autotoc_md449", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step8-encoding-rs-baseline.md \"P3-Step 8: `encoding_rs` baseline\"", "md_docs_plans_phase3_index.html#autotoc_md450", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step9-simdutf-baseline.md \"P3-Step 9: `simdutf` ceiling baseline\"", "md_docs_plans_phase3_index.html#autotoc_md451", null ]
        ] ],
        [ "Stress paths and final reporting", "md_docs_plans_phase3_index.html#autotoc_md452", [
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step10-boundary-stress.md \"P3-Step 10: Chunked/lazy boundary stress benchmarks\"", "md_docs_plans_phase3_index.html#autotoc_md453", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step11-toolchain-matrix.md \"P3-Step 11: Toolchain matrix automation\"", "md_docs_plans_phase3_index.html#autotoc_md454", null ],
          [ "@ref /home/runner/work/transcode/transcode/docs/plans/p3-step12-reporting-and-repro.md \"P3-Step 12: Reporting, dashboard, and reproducibility pack\"", "md_docs_plans_phase3_index.html#autotoc_md455", null ]
        ] ]
      ] ],
      [ "Key Architecture Decisions", "md_docs_plans_phase3_index.html#autotoc_md457", null ],
      [ "Agent Execution Model", "md_docs_plans_phase3_index.html#autotoc_md459", [
        [ "Worktree Discipline", "md_docs_plans_phase3_index.html#autotoc_md460", null ],
        [ "What to write in handoff-next.md", "md_docs_plans_phase3_index.html#autotoc_md461", null ],
        [ "Rules", "md_docs_plans_phase3_index.html#autotoc_md462", null ]
      ] ]
    ] ],
    [ "Phase 4: API Surface Gap Closure", "md_docs_plans_phase4_index.html", [
      [ "Context", "md_docs_plans_phase4_index.html#autotoc_md464", null ],
      [ "Step Index", "md_docs_plans_phase4_index.html#autotoc_md465", null ],
      [ "Standing Conventions", "md_docs_plans_phase4_index.html#autotoc_md466", null ]
    ] ],
    [ "Step 14: <tt>codec::replacement</tt> Decoder", "md_docs_plans_step14_replacement_codec.html", [
      [ "Goal", "md_docs_plans_step14_replacement_codec.html#autotoc_md469", null ],
      [ "WHATWG Specification", "md_docs_plans_step14_replacement_codec.html#autotoc_md470", null ],
      [ "Files Modified", "md_docs_plans_step14_replacement_codec.html#autotoc_md471", [
        [ "<tt>include/beman/transcode/whatwg_decode_view.hpp</tt>", "md_docs_plans_step14_replacement_codec.html#autotoc_md472", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode.test.cpp</tt>", "md_docs_plans_step14_replacement_codec.html#autotoc_md473", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode_or_error.test.cpp</tt>", "md_docs_plans_step14_replacement_codec.html#autotoc_md474", null ]
      ] ],
      [ "No New Files Created", "md_docs_plans_step14_replacement_codec.html#autotoc_md475", null ],
      [ "No Negative Compile Tests", "md_docs_plans_step14_replacement_codec.html#autotoc_md476", null ],
      [ "Verification", "md_docs_plans_step14_replacement_codec.html#autotoc_md477", null ]
    ] ],
    [ "Step 15: <tt>codec::x_user_defined</tt> Decoder", "md_docs_plans_step15_x_user_defined.html", [
      [ "Goal", "md_docs_plans_step15_x_user_defined.html#autotoc_md480", null ],
      [ "WHATWG Specification", "md_docs_plans_step15_x_user_defined.html#autotoc_md481", null ],
      [ "Files Created", "md_docs_plans_step15_x_user_defined.html#autotoc_md482", [
        [ "<tt>include/beman/transcode/detail/x_user_defined.hpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md483", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step15_x_user_defined.html#autotoc_md484", [
        [ "<tt>include/beman/transcode/whatwg_decode_view.hpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md485", null ],
        [ "<tt>include/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md486", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode.test.cpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md487", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode_or_error.test.cpp</tt>", "md_docs_plans_step15_x_user_defined.html#autotoc_md488", null ]
      ] ],
      [ "Verification", "md_docs_plans_step15_x_user_defined.html#autotoc_md489", null ]
    ] ],
    [ "Step 16: Single-Byte Table Infrastructure + <tt>codec::windows_1252</tt>", "md_docs_plans_step16_single_byte_infra.html", [
      [ "Goal", "md_docs_plans_step16_single_byte_infra.html#autotoc_md492", null ],
      [ "WHATWG Specification — Single-Byte Decoder Algorithm", "md_docs_plans_step16_single_byte_infra.html#autotoc_md493", null ],
      [ "Files Created", "md_docs_plans_step16_single_byte_infra.html#autotoc_md494", [
        [ "<tt>include/beman/transcode/detail/single_byte.hpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md495", null ],
        [ "<tt>include/beman/transcode/detail/tables/windows_1252.hpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md496", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step16_single_byte_infra.html#autotoc_md497", [
        [ "<tt>include/beman/transcode/whatwg_decode_view.hpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md498", null ],
        [ "<tt>include/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md499", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode.test.cpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md500", null ],
        [ "<tt>tests/beman/transcode/whatwg_decode_or_error.test.cpp</tt>", "md_docs_plans_step16_single_byte_infra.html#autotoc_md501", null ]
      ] ],
      [ "Verification", "md_docs_plans_step16_single_byte_infra.html#autotoc_md502", null ]
    ] ],
    [ "Step 17: Data Tooling — WHATWG Index Acquisition & Table Generation", "md_docs_plans_step17_iso_8859_tables.html", [
      [ "Goal", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md505", null ],
      [ "Why This Step Exists", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md506", null ],
      [ "WHATWG Index Data Sources", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md507", [
        [ "Single-byte indexes (one per codec)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md508", null ],
        [ "Multi-byte indexes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md509", null ],
        [ "Metadata", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md510", null ]
      ] ],
      [ "Directory Layout", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md511", [
        [ "<tt>docs/whatwg/</tt> — pristine upstream data", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md512", null ],
        [ "<tt>docs/wpt/</tt> — Web Platform Tests (future)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md513", null ],
        [ "<tt>data/</tt> — our derived/processed artifacts", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md514", null ]
      ] ],
      [ "Provenance: <tt>docs/whatwg/SOURCE.md</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md515", null ],
      [ "Provenance: <tt>docs/whatwg/source.bib</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md516", null ],
      [ "Python Quality Requirements", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md517", null ],
      [ "Deliverables", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md518", [
        [ "Script: <tt>tools/download_indexes.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md519", null ],
        [ "Script: <tt>tools/generate_tables.py</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md520", null ],
        [ "Tests: <tt>tools/tests/</tt>", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md521", null ],
        [ "Generated files (single-byte, 22 total)", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md522", null ]
      ] ],
      [ "Procedure", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md523", null ],
      [ "Verification", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md524", null ],
      [ "Notes", "md_docs_plans_step17_iso_8859_tables.html#autotoc_md525", null ]
    ] ],
    [ "Step 18: UTF-8 Encoder — <tt>whatwg_encode_view<codec::utf_8></tt>", "md_docs_plans_step18_utf8_encoder.html", [
      [ "Goal", "md_docs_plans_step18_utf8_encoder.html#autotoc_md528", null ],
      [ "WHATWG Specification — UTF-8 Encoder", "md_docs_plans_step18_utf8_encoder.html#autotoc_md529", null ],
      [ "Files Created", "md_docs_plans_step18_utf8_encoder.html#autotoc_md530", [
        [ "<tt>include/beman/transcode/detail/utf8_encode.hpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md531", null ],
        [ "<tt>include/beman/transcode/whatwg_encode_view.hpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md532", null ],
        [ "<tt>tests/beman/transcode/whatwg_encode.test.cpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md533", null ],
        [ "<tt>tests/beman/transcode/whatwg_encode_or_error.test.cpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md534", null ],
        [ "Negative compile test: <tt>whatwg_encode_reject_char_range_fail.cpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md535", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step18_utf8_encoder.html#autotoc_md536", [
        [ "<tt>include/beman/transcode/detail/concepts.hpp</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md537", null ],
        [ "<tt>include/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md538", null ],
        [ "<tt>tests/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step18_utf8_encoder.html#autotoc_md539", null ]
      ] ],
      [ "Verification", "md_docs_plans_step18_utf8_encoder.html#autotoc_md540", null ]
    ] ],
    [ "Step 19: Round-Trip Composition Tests", "md_docs_plans_step19_roundtrip.html", [
      [ "Goal", "md_docs_plans_step19_roundtrip.html#autotoc_md543", null ],
      [ "Composition Syntax", "md_docs_plans_step19_roundtrip.html#autotoc_md544", null ],
      [ "Files Created", "md_docs_plans_step19_roundtrip.html#autotoc_md545", [
        [ "<tt>tests/beman/transcode/roundtrip.test.cpp</tt>", "md_docs_plans_step19_roundtrip.html#autotoc_md546", null ]
      ] ],
      [ "Files Modified", "md_docs_plans_step19_roundtrip.html#autotoc_md547", [
        [ "<tt>tests/beman/transcode/CMakeLists.txt</tt>", "md_docs_plans_step19_roundtrip.html#autotoc_md548", null ]
      ] ],
      [ "Verification", "md_docs_plans_step19_roundtrip.html#autotoc_md549", null ],
      [ "Notes", "md_docs_plans_step19_roundtrip.html#autotoc_md550", null ]
    ] ],
    [ "<strong>System Architecture and Implementation Strategies for C++ Transcoding: Integrating encoding_rs as an Oracle for P1439-compliant Interfaces</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html", [
      [ "<strong>The Standardization Landscape and the Crisis in Text Processing</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md552", null ],
      [ "<strong>Deconstructing the P1439 Transcoding Model</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md553", [
        [ "<strong>The Imperative of Range-Based Composability</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md554", null ]
      ] ],
      [ "<strong>The Internal Engine: Profiling encoding_rs and WHATWG Compliance</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md555", [
        [ "<strong>WHATWG Encoding Standard Compliance</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md556", null ],
        [ "<strong>Streaming versus Non-Streaming Execution Modes</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md557", null ]
      ] ],
      [ "<strong>Bridging the FFI Boundary: The Constraints of encoding_c</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md558", [
        [ "<strong>The Panic=Abort Imperative and Exception Safety</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md559", null ],
        [ "<strong>Type Impedance and the Guideline Support Library (GSL)</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md560", null ]
      ] ],
      [ "<strong>Implementation Strategy: Mapping Ranges to Rust Decoders</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md561", [
        [ "<strong>Bypassing Iterator Overhead via Contiguous Specialization</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md562", null ],
        [ "<strong>Managing Non-Contiguous State and Pipeline Eagerness</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md563", null ]
      ] ],
      [ "<strong>Theoretical Foundations: Coinductive Types, Trees, and Traversables</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md564", null ],
      [ "<strong>Monadic Error Handling: Integration with views::maybe and std::optional</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md565", [
        [ "<strong>The Role of views::maybe in Transcoding Pipelines</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md566", null ],
        [ "<strong>std::optional and Monadic Continuations</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md567", null ]
      ] ],
      [ "<strong>The Oracle Paradigm: Fuzzing and Implementer Relief</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md568", [
        [ "<strong>Defining the Testing Oracle</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md569", null ],
        [ "<strong>Differential Fuzzing Strategy</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md570", null ]
      ] ],
      [ "<strong>Managing Mojibake, Identifier Normalization, and Security</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md571", null ],
      [ "<strong>Navigating Build Systems and Ecosystem Integration</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md572", null ],
      [ "<strong>Conclusion</strong>", "md_docs_Rust_Encoding_for_C___Transcode.html#autotoc_md573", null ]
    ] ],
    [ "Review of transcode as a C++29 Library Candidate and Beman Inclusion Candidate", "md_docs_transcode_as_a_C__29_Library_Candidate.html", [
      [ "Executive summary", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md576", [
        [ "Audit summary", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md577", null ]
      ] ],
      [ "Repository status and engineering quality", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md578", null ],
      [ "API design and code-level behavior", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md579", [
        [ "Design issues and concrete defects", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md580", null ],
        [ "Targeted code suggestions", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md581", null ]
      ] ],
      [ "Single-byte support and the feasibility of random-access iteration or indexing", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md582", [
        [ "Feasibility matrix for random access", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md583", null ]
      ] ],
      [ "Relationship to P2728", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md584", [
        [ "Current behavior versus P2728-like expectations", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md585", null ],
        [ "Suggested architectural split", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md586", null ]
      ] ],
      [ "Concrete changes, tests to add, and documentation fixes", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md587", null ],
      [ "Acceptance checklist for Beman inclusion and proposal readiness", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md588", [
        [ "Bottom-line recommendation", "md_docs_transcode_as_a_C__29_Library_Candidate.html#autotoc_md589", null ]
      ] ]
    ] ],
    [ "Execution Plan: C++29 Transcoding Architecture (Beman Project)", "md_docs_transcoding_plan_part2.html", [
      [ "1. Type Architecture & I/O Interfaces", "md_docs_transcoding_plan_part2.html#autotoc_md591", null ],
      [ "2. WHATWG Codecs: Symmetric Pipelines & Concrete Roster", "md_docs_transcoding_plan_part2.html#autotoc_md592", [
        [ "The Interfaces", "md_docs_transcoding_plan_part2.html#autotoc_md593", null ],
        [ "The Codec Roster & Algorithm/Table Sources", "md_docs_transcoding_plan_part2.html#autotoc_md594", null ]
      ] ],
      [ "3. POSIX <tt>iconv</tt>: Direct Legacy-to-Legacy Abstraction", "md_docs_transcoding_plan_part2.html#autotoc_md595", [
        [ "The Interface", "md_docs_transcoding_plan_part2.html#autotoc_md596", null ]
      ] ],
      [ "4. Validation Architecture", "md_docs_transcoding_plan_part2.html#autotoc_md597", [
        [ "A. In-Tree WHATWG Conformance (W3C Web Platform Tests)", "md_docs_transcoding_plan_part2.html#autotoc_md598", null ],
        [ "B. Out-of-Tree Iconv Conformance (\"Clean Room\" Testing)", "md_docs_transcoding_plan_part2.html#autotoc_md599", null ]
      ] ],
      [ "5. Next Steps & Standardization", "md_docs_transcoding_plan_part2.html#autotoc_md600", null ]
    ] ],
    [ "WHATWG Encoding Standard — Index Data", "md_docs_whatwg_SOURCE.html", [
      [ "File Provenance", "md_docs_whatwg_SOURCE.html#autotoc_md602", null ],
      [ "Full Checksums", "md_docs_whatwg_SOURCE.html#autotoc_md603", null ]
    ] ],
    [ "The Story of Failing to Find a Way Out: Reverse-Parsing Legacy Encodings", "md_docs_why_forward_iterator.html", [
      [ "Act I: The Overlapping Abyss", "md_docs_why_forward_iterator.html#autotoc_md605", [
        [ "The Specific Example", "md_docs_why_forward_iterator.html#autotoc_md606", null ]
      ] ],
      [ "Act II: The Illusion of the Anchor", "md_docs_why_forward_iterator.html#autotoc_md608", [
        [ "The Specific Example", "md_docs_why_forward_iterator.html#autotoc_md609", null ]
      ] ],
      [ "Act III: State Machine Amnesia", "md_docs_why_forward_iterator.html#autotoc_md611", [
        [ "The Specific Example", "md_docs_why_forward_iterator.html#autotoc_md612", null ],
        [ "The Final Verdict", "md_docs_why_forward_iterator.html#autotoc_md613", null ]
      ] ]
    ] ],
    [ "Web Platform Tests — Encoding Test Vectors", "md_docs_wpt_SOURCE.html", [
      [ "File Provenance", "md_docs_wpt_SOURCE.html#autotoc_md615", null ],
      [ "Full Checksums", "md_docs_wpt_SOURCE.html#autotoc_md616", null ]
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
"classbeman_1_1transcoding_1_1iconv__transcode__view.html#a4a0e580a320a3aa781a7e8e9030f2b91",
"classbeman_1_1transcoding_1_1random__access__whatwg__decode__or__error__view.html#acfec19dcbe2fb5f8974b5412c63587bc",
"classbeman_1_1transcoding_1_1random__access__whatwg__encode__view_1_1iterator.html#adc1c339ec2a3b592fd1cc186a8fc8fd5",
"classbeman_1_1transcoding_1_1whatwg__decode__view_1_1iterator.html#a32bd825ca112aac1071ca77ed034f553",
"error_8hpp.html#a3823b84e4fc7b689f0b2b38741b30baba41a389ed570cf4bd17e392adceb1bf31",
"md_docs_plans_p3_step11_toolchain_matrix.html#autotoc_md147",
"md_docs_plans_phase3_checklist.html#autotoc_md411",
"namespacebeman_1_1transcoding_1_1views.html#aaa8c7a22aa85348f2156d62b50097952",
"whatwg__decode__view_8hpp.html#a20dfb324ebd26c6127d014520f93005aa8034dde2b07847e0eef3470fb0865816"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';