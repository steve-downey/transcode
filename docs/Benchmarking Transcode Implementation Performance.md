# **Methodological Framework for Benchmarking C++29 Transcoding Pipelines: Evaluating P2728 and the Beman utf\_view Implementation**

## **1\. Architectural Context and the Standardization of C++ Transcoding**

The evolution of text processing within the C++ standard library has historically been constrained by legacy abstractions, platform-specific inconsistencies, and suboptimal memory management paradigms. The proposal for C++29 to integrate standard Unicode transcoding facilities—encapsulated in proposals P2728 (Unicode in the Library, Part 1: UTF Transcoding) and P3705 (A Sentinel for Null-Terminated Strings)—aims to rectify these historical deficiencies.1 The reference implementation for this proposal, initially developed under steve-downey/transcode and subsequently transitioning to the Beman Project ecosystem as bemanproject/utf\_view, establishes a modern, ranges-based architecture for Unicode text manipulation.2  
To justify the inclusion of utf\_view into the C++ standard and to convince the SG16 (Unicode) and SG9 (Ranges) study groups of its viability, its performance characteristics must be rigorously quantified.3 Benchmarking a C++ ranges-based transcoding pipeline presents distinct, highly complex challenges. Unlike traditional eager algorithms that process entire continuous memory buffers monolithically in a single function call, ranges utilize lazy evaluation. This approach generates complex iterator pipelines that rely heavily on the compiler's intermediate representation (IR) optimization passes to collapse abstraction overhead.5  
This report presents an exhaustive methodological framework for benchmarking the Beman utf\_view library implementation. It delineates a comparative matrix against traditional POSIX mechanisms (iconv), deprecated standard library components (std::locale and \<codecvt\>), and state-of-the-art third-party implementations (encoding\_rs and simdutf). Furthermore, it defines standard corpora selection based on the Text Encoding Initiative (TEI), evaluates best- and worst-case execution paths based on algorithmic complexity and memory locality, and outlines the precise integration of Catch2 version 3 as the primary microbenchmarking harness. Finally, it critically factors in compiler toolchain configurations, particularly the impact of link-time optimizations and auto-vectorization.

### **1.1 Structural Constraints of the Proposed Architecture**

Before establishing performance metrics, the benchmarking methodology must account for the strict, safety-oriented architectural invariants imposed by the Beman utf\_view implementation. These constraints introduce baseline computational overhead in exchange for type safety and memory correctness, which traditional C libraries bypass.

* **The byte\_like Output Concept and Strict Aliasing:** To prevent strict-aliasing pessimizations associated with native char and std::byte types, the encoding pipelines output to custom enumeration-backed types (e.g., enum class octet : unsigned char {}).3 The detail::byte\_like\<T\> concept constrains input and output to strictly valid byte representations, preventing the compiler from making unsafe assumptions about memory overlap during pipeline execution.3  
* **The unicode\_scalar\_range Input Constraint:** The WHATWG encoding standard, which forms the core algorithmic basis of the proposal, strictly requires fully resolved Unicode code points.3 Passing char8\_t or char16\_t arrays directly will fail at compile time; the API demands an std::ranges::input\_range whose range\_value\_t is strictly char32\_t.3 This mandates a reification of an intermediate scalar, potentially bottlenecking memory bandwidth compared to direct byte-to-byte transcoding if not optimized effectively by the compiler.  
* **Array Decay Prohibition:** All byte ingestion pipelines enforce a static\_assert against taking const char\[N\] (C-style strings) directly.3 Users must wrap string literals via C++26's std::views::null\_term (P3705R2).2 Benchmarking must factor in the cycle cost of calculating sentinels and advancing iterators over null-terminated spans versus sized extents.

## **2\. The Competitive Landscape: Baseline Transcoding Implementations**

To contextualize the mechanical throughput of the proposed C++29 transcoding facilities, the evaluation must measure the implementation against four distinct paradigms of text encoding libraries. Each represents a specific evolutionary stage in systems programming and memory manipulation.

### **2.1 The Negative Baseline: std::locale and \<codecvt\>**

The C++11 \<codecvt\> header and its associated std::locale facets represent the historical approach to standard library transcoding. This implementation is widely recognized as fundamentally flawed in both API design and runtime performance, leading to its official deprecation in C++17.9  
The std::codecvt API forces heavy virtualization, relying on polymorphic virtual function calls for character conversion. This design destroys instruction cache locality and severely hinders the branch predictor.9 Furthermore, implementations across the major standard libraries (LLVM libc++, GNU libstdc++, and MSVC STL) exhibit systemic bugs, particularly regarding UTF-16 code unit evaluation and error handling.9 In the proposed benchmarking suite, \<codecvt\> serves strictly as a negative baseline to demonstrate the absolute magnitude of performance improvement achieved by shifting to the utf\_view proposal. It provides the empirical proof required to finalize the removal of \<codecvt\> from modern codebases.

### **2.2 The Legacy POSIX Pivot: iconv**

The POSIX iconv API remains the ubiquitous C-level abstraction for character set conversion on UNIX-like operating systems.3 Unlike standard Unicode-centric libraries, iconv supports a massive matrix of legacy encodings, acting as the foundational layer for countless enterprise systems.11

* **Architectural Limitations:** iconv operates via a highly stateful, opaque handle (iconv\_t). It frequently relies on an internal "pivot" or "substrate" encoding—typically UCS-4 or UTF-8. Consequently, transcoding from one legacy encoding (e.g., Shift\_JIS) to another (e.g., Windows-1251) may incur a hidden double-transcoding memory allocation penalty within the C library.11  
* **Performance Profile:** Execution performance varies drastically depending on the underlying OS implementation (e.g., GNU libiconv, FreeBSD Citrus, or musl libc).3 While highly versatile, its performance is often bottlenecked by function call overhead on every buffer submission, opaque internal buffer allocations, and branch-heavy state machines that cannot be inlined by the caller's compiler.9  
* **Integration in Beman:** The utf\_view proposal uniquely addresses this legacy by integrating an iconv\_transcode\_view.3 This view wraps iconv in a zero-cost, move-only iterator abstraction, allowing direct legacy-to-legacy streaming while completely bypassing the WHATWG char32\_t scalar constraint.3 Benchmarking must clearly separate the overhead of the iconv C API itself from the overhead of the C++ iconv\_transcode\_view iterator adapter to prove the zero-cost abstraction claim.

### **2.3 The Web Platform Standard: encoding\_rs**

Developed by Mozilla for the Gecko browser engine, encoding\_rs is a highly optimized, Rust-based implementation of the WHATWG Encoding Standard.13 It serves as the primary modern comparative baseline for utf\_view, as both libraries target the same algorithmic specification.3

* **Architectural Strengths:** encoding\_rs explicitly replaces legacy complexity with a strictly bounded set of web-compatible encodings.13 It utilizes highly optimized, table-driven approaches for single-byte encodings and avoids unnecessary heap allocations by leveraging Rust's Cow (Copy-on-Write) semantics, effectively executing zero-cost passes when inputs are strict ASCII subsets.14  
* **Optimization Strategies:** The library employs SIMD instructions (traditionally integrated via the packed\_simd crate or core intrinsics) to aggressively accelerate processing, particularly for ASCII fast-paths.14 It heavily favors streaming scenarios and assumes valid UTF-8/UTF-16 internal representations, precisely matching the core architectural use-cases of utf\_view.8 Comparing utf\_view against encoding\_rs provides a direct measure of C++ standard ranges overhead versus Rust's zero-cost iterator and unsafe memory block abstractions.18

### **2.4 The Modern Performance Ceiling: simdutf**

The simdutf library represents the absolute state-of-the-art in hardware-accelerated text transcoding.19 Written in modern C++, it relies extensively on Single Instruction, Multiple Data (SIMD) extensions including ARM NEON, Intel SSE, AVX2, AVX-512, and RISC-V Vector Extensions.20

* **Architectural Strengths:** By processing multiple characters per CPU cycle across wide vector registers, simdutf bypasses traditional scalar state machines, achieving throughputs frequently exceeding one billion characters per second on modern hardware architectures.19 It is strictly exception-free, strictly non-allocating, and provides highly tuned fast-paths for ASCII data.19  
* **Benchmarking Context:** simdutf acts as the performance ceiling for the evaluation. Because the proposed utf\_view relies primarily on scalar standard C++ (relying on compiler auto-vectorization rather than manually invoking architecture-specific SIMD intrinsics), it is physically incapable of matching simdutf in raw, unconstrained throughput.19 However, the benchmark suite must quantify the exact delta between the generic, composable ranges-based approach of utf\_view and the hardware-specific, tightly coupled implementation of simdutf. This comparison informs library users of the critical trade-offs between standard C++ range composability and raw mechanical sympathy.

| Transcoding Implementation | Core Programming Paradigm | Primary Target Domain | API / Specification Conformance | Expected Scalability |
| :---- | :---- | :---- | :---- | :---- |
| std::codecvt | Virtualized Polymorphic Facets | Legacy C++ Applications | ISO C++11 (Deprecated) | Low (Scalar bottlenecked) |
| iconv | Stateful Opaque C API | OS Systems Layer | POSIX Standard | Moderate (Function call overhead) |
| beman::utf\_view | Lazy C++ Range Adapters | Modern C++29 Library | WHATWG / ISO C++ | High (Compiler dependent) |
| encoding\_rs | Table-Driven Rust FFI | Web Browsers (Gecko) | WHATWG Standard | High (SIMD optimized) |
| simdutf | Manual Hardware Intrinsics | High-Performance Data | Unicode Standard | Extreme (AVX/NEON saturated) |

## **3\. Analysis of Existing Comparative Benchmarks**

To establish a baseline hypothesis for the performance of the Beman utf\_view, it is necessary to analyze existing comparative benchmarks across the text-processing ecosystem. Software making high-performance claims, such as simdutf, encoding\_rs, and specialized iconv wrappers, have published extensive datasets that reveal the physical limits of current hardware.

### **3.1 Transcoding Throughput Claims and Industry Metrics**

Research into modern database indexing systems, specifically the SWUFE-DB-Group's Janus project (A Two-Level Character Validation Framework for Legacy CJK Encodings), provides empirical benchmarking over legacy encodings.24 When processing GB2312 (a complex multi-byte Chinese encoding) on a Linux machine equipped with an Intel i9-12900K processor and 64GB of memory, the Janus benchmarks reported the following throughput limits for various implementations:

* Standard iconv: 0.65 GiB/s  
* Mozilla encoding\_rs: 0.58 GiB/s  
* Range-based implementations: 1.04 GiB/s  
* Table-lookup implementations: 3.36 GiB/s  
* Hardware SIMD implementations: 9.01 GiB/s 24

These figures establish a critical expectation: when dealing with stateful CJK encodings, raw iconv and encoding\_rs exhibit roughly equivalent throughput, bottlenecked by the necessary memory lookups and branch density. If the C++ ranges approach of utf\_view can achieve the \~1.04 GiB/s range capability observed in the Janus test, it will represent a highly competitive standard library feature.

### **3.2 SIMD Dominance and the Abstraction Delta**

The simdutf library provides extensive benchmark data verifying its claims of extreme performance. The integration of simdutf into the Node.js JavaScript runtime yielded a 364% improvement in UTF-8 decoding speed compared to the previous Node.js 16 implementation.19 Furthermore, when benchmarking Arabic text (a highly complex mix of single-byte and double-byte UTF-8 sequences) against the universally utilized ICU library, simdutf routinely operates 3 to 10 times faster, and up to 20 times faster in specific pathological cases.19  
Benchmarks ported to Go (charlievieth/simdutf) utilizing an AMD Ryzen 9 9950X demonstrate UTF-8 validation and processing speeds scaling from 3,376 MB/s on small 10-byte inputs up to an astonishing 146,231 MB/s on 16K purely ASCII buffers, highlighting the extreme efficiency of SIMD fast-paths.26 The Beman utf\_view benchmarks must monitor its own ASCII fast-paths to see if compiler auto-vectorization can capture any fraction of this SIMD dominance.

### **3.3 The Iconv Wrapper Variances**

While raw iconv is considered a known quantity, the manner in which it is invoked significantly alters performance. Benchmarks of the @fengmk2/iconv Rust wrapper (comparing iconv-rust against the JavaScript iconv-lite) demonstrate that optimized FFI boundary crossings can yield up to a 17.6x speedup for Windows-1251 long-string decoding compared to native unoptimized calls.12 This suggests that the iconv\_transcode\_view within the Beman architecture has the potential to match or exceed raw C performance if the iterator boundary is minimized and the standard library implementation optimizes the data span transitions effectively.3

| Benchmark Source | Evaluated Library | Operation Profile | Reported Throughput / Speedup |
| :---- | :---- | :---- | :---- |
| Janus Framework 24 | iconv | GB2312 Decoding | 0.65 GiB/s |
| Janus Framework 24 | encoding\_rs | GB2312 Decoding | 0.58 GiB/s |
| Node.js Runtime 19 | simdutf | General UTF-8 Decoding | 364% Speedup vs Node 16 |
| Lemire / Muła 23 | simdutf | Arabic UTF-8 to UTF-16 | 4x to 10x Faster than ICU |
| fengmk2/iconv 12 | iconv-rust | Windows-1251 Decoding | 17.6x Faster than iconv-lite |

## **4\. Bounding the Performance Envelope: Pathological and Optimal Execution Pathways**

A robust benchmarking methodology must explicitly isolate the algorithmic extremes of the proposed library. The Beman utf\_view architecture will exhibit vastly different performance profiles—ranging from near memory-bandwidth saturation to severe CPU cycle stalling—depending on the chosen WHATWG algorithmic category.3

### **4.1 The Optimal Execution Paths (Best-Case Scenarios)**

The highest theoretical throughput in scalar transcoding is achieved when memory lookups are strictly constrained to CPU registers or the L1 instruction/data cache, and conditional branching logic is entirely eliminated.

#### **Category B: Table-Driven Single-Byte Encoders**

Encodings such as iso\_8859\_2 through iso\_8859\_16, windows\_1252, and koi8\_r map a single legacy byte directly to a single Unicode scalar code point.3

* **Architectural Mechanism:** The utf\_view build scripts execute during the CMake build step, parsing the WHATWG indexes.json to generate constexpr std::array\<char32\_t, 128\> arrays corresponding to the upper block (0x80–0xFF) of each encoding.3  
* **Benchmark Expectation:** This architecture provides guaranteed ![][image1] constant-time lookup performance. Because the array size is exactly 512 bytes (![][image2]), the entire lookup table fits effortlessly within a single cache line of modern L1 data caches. The benchmark evaluation should demonstrate that whatwg\_decode\_view for Category B encodings operates at near memory-bandwidth limits, bottlenecked only by the bounds-checking mechanics of the underlying C++ range adapter.

#### **Category A: The Algorithmic UTF Family**

For the core standard encodings—utf\_8, utf\_16le, and utf\_16be—no external lookup tables are utilized.3

* **Architectural Mechanism:** Transcoding is executed purely via bit-shifting, bit-masking, and continuation-byte arithmetic.  
* **Benchmark Expectation:** Modern C++ compilers (GCC, Clang) utilizing \-O3 optimization flags are highly adept at auto-vectorizing these predictable bitwise operations.27 While it will not reach the explicit AVX-512 throughput of simdutf, the algorithmic UTF path should heavily outperform any stateful iconv invocation and closely approximate the non-SIMD throughput metrics of encoding\_rs. The benchmark methodology must strictly evaluate the ASCII fast-path, where leading zero-bits in UTF-8 allow the pipeline to bypass complex multi-byte continuation logic entirely.

### **4.2 The Pathological Execution Paths (Worst-Case Scenarios)**

The poorest performance will reliably occur when the transcoding algorithm is forced to manage shifting state machines, perform multi-level memory index lookups, or interact with volatile C++ iterator boundary conditions.

#### **Category C: Multi-Byte CJK Complex Indexing**

Encodings such as shift\_jis, big5, gb18030, and iso\_2022\_jp represent the most computationally expensive and memory-hostile pathways in the WHATWG standard.3

* **Architectural Mechanism:** These encodings require multi-level tries (segmented lookup arrays) due to their massive character sets. iso\_2022\_jp is exceptionally hostile to pipeline performance because it is highly stateful; the interpretation of any given byte depends entirely on previously encountered escape sequences (shift states).3  
* **Benchmark Expectation:** Branch misprediction rates will inevitably spike. The multi-level trie lookups guarantee that the data structures will frequently spill from the L1 cache into L2 or L3, introducing severe memory latency stalls. The benchmark harness must measure the "Cycles Per Instruction" (CPI) during CJK decoding to accurately quantify this memory stall overhead against simpler encodings.

#### **Lazy Pipeline Boundary Interruptions**

Because utf\_view functions as a lazy C++ range adapter, it translates standard range iteration into segmented buffer processing behind the scenes.5 The absolute worst-case scenario for a range adaptor occurs when the underlying data source is chunked (e.g., reading from asynchronous network sockets or file buffers via std::views::chunk) and multi-byte characters are sliced across chunk boundaries.5

* **The EINVAL Split Sequence:** If a 4-byte UTF-8 sequence representing an emoji or complex glyph (e.g., the Musical Symbol G Clef U+1D11E, encoded as 0xF0 0x9D 0x84 0x9E) is sliced exactly in half at the end of a buffer chunk, the internal iconv or WHATWG state machine will halt and return EINVAL (incomplete sequence).5  
* **The E2BIG Buffer Exhaustion:** When piping massive input ranges into constrained output spans, the underlying system (particularly iconv) will return E2BIG, forcing the C++ iterator to pause execution, yield the converted characters, and seamlessly resume state.5  
* **Benchmark Expectation:** The computational cost of caching leftover bytes, gracefully advancing iterators, and restarting the underlying C-level state machine upon the next operator++ call represents the ultimate stress test of the C++ range abstraction penalty. The benchmarking suite must intentionally feed the view via std::views::chunk to trigger EINVAL conditions across arbitrary boundaries, artificially inducing maximum pipeline stall to measure worst-case latency.5

## **5\. Linguistic Entropy and Standard Corpora Selection**

Benchmarking text processing algorithms on synthetic, highly repetitive strings (e.g., generating a buffer containing the letter 'A' repeated one million times) yields heavily distorted and ultimately meaningless metrics. Modern CPU branch predictors and instruction caches artificially inflate performance on uniform data, failing to represent the chaos of real-world text. Therefore, the benchmarking methodology requires a meticulously curated corpus that reflects actual linguistic entropy, structural variety, and standard encoding edge cases.

### **5.1 The Text Encoding Initiative (TEI) and Linguistic Reality**

The Text Encoding Initiative (TEI) guidelines dictate that language corpora must not be arbitrary or randomized collections of data.28 Instead, they must be composite texts that provide a dual perspective: statistical representation of linguistic varieties and systematic contextual documentation.28 Standardized corpora, such as those formulated by the EAGLES guidelines or the ACL Data Collection Initiative, emphasize the necessity of diverse genre, register, and factual continuity.29  
When constructing the benchmark input buffers, this variance is mandatory. Transcoder branch predictors behave vastly differently when processing highly structured technical documentation (which is primarily ASCII with sparse UTF-8 multibyte characters) versus conversational text containing dense multilinguistic clusters or high-frequency emoji usage.

### **5.2 The "Wikipedia Mars" Heuristic**

Following the empirical precedent established by both the Mozilla encoding\_rs project (via the encoding\_bench suite) and the simdutf performance testing framework, raw Wikipedia article dumps serve as the ideal real-world testing data.23 Specifically, the Wikipedia page for the planet "Mars" is utilized as the canonical benchmark artifact.31

* **Rationale for Selection:** The article on Mars is of substantial length across numerous languages and contains a natural mix of scientific data, numerical formatting, and continuous narrative prose. This ensures that the benchmark processes a high volume of complex bytes without repeating cache-friendly loops.23  
* **Linguistic Diversity Target Matrix:** To test the diverse encoding paths outlined in the WHATWG specification, the "Mars" article must be extracted in multiple languages to force the transcoder down specific execution paths:  
  * *English/French:* Tests the ASCII fast-path and minimal-continuation UTF-8 decoding logic.  
  * *Czech/Russian:* Specifically targets Category B (Single-Byte Legacy) encodings like Windows-1251 or KOI8-R to test ![][image1] table lookups.3  
  * *Arabic:* Arabic UTF-8 is highly complex due to the constant intermingling of one-byte (spaces/punctuation) and two-byte (Arabic glyphs) characters. This constant oscillation heavily taxes CPU branch predictors and serves as an excellent stress test for scalar algorithms.23  
  * *Chinese/Japanese/Vietnamese (CJKV):* Targets Category C multi-byte complex index encodings (e.g., Shift\_JIS, GB18030). These languages demand extensive multi-level trie lookups and shift-state transitions.3 Furthermore, Vietnamese introduces unique overhead regarding NFD (Normalization Form Canonical Decomposition) versus NFC orthographic decomposition, testing the limits of scalar processing.31

### **5.3 Synthetic Boundary and Fuzzing Data**

While real-world corpora measure average-case throughput, synthetic datasets are required to measure the true cost of boundary resilience and error recovery. The Beman project already utilizes out-of-tree "Clean Room" testing architectures to validate iconv conformance without violating GPL licensing.3 The benchmarking methodology must reuse these fuzzed vectors to test the CPU cycle cost of error recovery.

* **Web Platform Tests (WPT):** The W3C provides extensive JSON-based test vectors for the WHATWG encoding standard.3 Benchmarking the iteration through these arrays will accurately quantify the overhead of handling gb18030 fallbacks, stateful BOM (Byte Order Mark) sniffing, and the x-user-defined private use area mapping paths.3  
* **Random Data Generation:** Utilizing structural fuzzer outputs will test the \_or\_error failure paths of utf\_view. By intentionally triggering EILSEQ (invalid byte sequence) errors at high frequencies, the benchmark measures the cost of generating and bubbling up std::expected types versus standard fallback substitution.3

| Corpus Profile | Target Language / Data Type | Primary Algorithmic Stress Vector | Expected Bottleneck |
| :---- | :---- | :---- | :---- |
| Wikipedia Mars | English / French | ASCII Fast-Path Validation | Memory Bandwidth |
| Wikipedia Mars | Czech / Russian | Single-Byte Table Lookup (![][image1]) | Cache Line Boundaries |
| Wikipedia Mars | Arabic | Mixed 1-byte/2-byte UTF-8 Oscillation | CPU Branch Prediction |
| Wikipedia Mars | Japanese / Chinese | Multi-Level Trie & Shift States | L2/L3 Cache Misses |
| WPT JSON Vectors | Synthetic Edge Cases | EILSEQ / Fallback Processing | Error Handling Logic |

## **6\. Harnessing Catch2 for Micro-Architectural Evaluation**

The Beman project natively utilizes Catch2 as its foundational unit-testing framework.3 Therefore, it is highly advantageous to leverage Catch2 version 3, which provides an advanced micro-benchmarking suite that is significantly more sophisticated than simple loop profiling, for the performance evaluation.35 Leveraging Catch2 effectively requires understanding its statistical sampling methods and utilizing its advanced macros to actively prevent compiler interference.

### **6.1 Dynamic Estimation and Statistical Sampling**

Unlike traditional benchmarking scripts that execute a hardcoded loop (e.g., for (int i \= 0; i \< 1000000; \++i)), Catch2 performs active environmental probing.37

* **Architectural Mechanism:** Before any user code is executed, Catch2 evaluates the absolute resolution of the system clock and quantifies the overhead of the timing function itself.37 It then executes the user kernel dynamically to estimate the optimal number of iterations required per sample to achieve statistical significance.35  
* **Statistical Advantage:** This dynamic sampling mitigates the risk of fringe cases, background operating system interrupts, or CPU thermal throttling biasing the final metrics.35 It ensures that fast-executing operations (like single-byte table lookups) are run tens of thousands of times per sample, while slow operations (like GB18030 fallback parsing over chunked arrays) are scaled down to prevent benchmark execution timeouts.

### **6.2 Combating the Compiler Optimizer: BENCHMARK\_ADVANCED**

The primary danger when benchmarking modern C++ standard ranges is that aggressive compilers (via Dead Code Elimination passes) will detect that the output of a transcoding loop is ultimately unused and optimize the entire kernel away, resulting in zero-nanosecond execution times.35 To prevent this, developers frequently introduce volatile variables or write outputs to external sinks. Catch2 mitigates this through its macro design, but careful implementation is required.  
Furthermore, to benchmark the raw algorithmic cost of transcoding without factoring in the unpredictable latency of memory allocation (e.g., allocating a std::string or expanding a std::vector on the heap), the benchmark must separate object construction from the measurement phase. This is achieved using the BENCHMARK\_ADVANCED macro in conjunction with Catch::Benchmark::Chronometer and the storage\_for utilities.36

C++  
// Example architectural implementation for evaluating Catch2 integration  
BENCHMARK\_ADVANCED("Transcode UTF-8 to UTF-16 (Lazy View)")(Catch::Benchmark::Chronometer meter) {  
    // Phase 1: Setup and Environment Probing (Excluded from benchmark time)  
    std::string utf8\_corpus \= load\_wikipedia\_mars\_corpus("en");  
    std::vector\<char16\_t\> output\_buffer;  
    output\_buffer.reserve(utf8\_corpus.size()); // Pre-allocate to prevent heap fragmentation

    // Phase 2: Core Measurement Execution  
    meter.measure(\[&\](int i) {  
        // Evaluate the Beman utf\_view lazily over the corpus  
        auto transcoded\_view \= utf8\_corpus | beman::utf\_view::to\_utf16;  
          
        // Force eager evaluation into the pre-allocated buffer  
        output\_buffer.assign(transcoded\_view.begin(), transcoded\_view.end());  
          
        // Return value ensures the compiler does not optimize away the iteration loop  
        return output\_buffer.size();   
    });  
};

By forcing the evaluation into a pre-allocated vector and returning the resulting size, the meter.measure lambda guarantees that the CPU cycles measured reflect purely the execution of the utf\_view state machine and the iterator increments, fully isolating the algorithmic performance from memory allocator interference.37

### **6.3 Limitations and Augmentations of Catch2 Throughput Reporting**

While Catch2 provides excellent statistical timing in nanoseconds and microseconds, it natively lacks throughput reporting capabilities (e.g., natively reporting in Megabytes or Gigabytes per second).40 Because string processing performance is universally communicated in MB/s or GB/s across the industry (as evidenced by encoding\_rs, simdutf, and iconv reporting metrics), the standard Catch2 output is insufficient for comparative analysis.19  
The proposed methodology dictates augmenting the Catch2 execution block. This can be achieved by utilizing custom reporters or by parsing the XML/JSON outputs generated by Catch2 (via automated CLI arguments) and piping them into visualization tools like Vega-Lite.42 A post-processing script will cross-reference the execution time in nanoseconds against the pre-calculated byte size of the input corpus to output the definitive GiB/s metric.

### **6.4 Mocking the Substrate: Deterministic State Machine Validation**

Beyond measuring raw throughput over massive continuous corpora, the Beman transcoding proposal must be benchmarked for its structural resilience at boundaries. If the system's underlying iconv implementation handles invalid sequences poorly, the C++ iconv\_transcode\_view will transparently inherit those performance cliffs.3  
To benchmark the *view logic* independently of the host operating system's C library (which varies wildly between macOS, FreeBSD, musl, and GNU libc), the methodology requires the injection of a mock iconv wrapper.5 By replacing the POSIX iconv signature with a mocked C++ implementation during the Catch2 microbenchmarks, the framework can deterministically trigger boundary states.5

* **Simulating E2BIG (Buffer Exhaustion):** The mock API can be hardcoded to return (size\_t)-1 and set errno \= E2BIG exactly every 16 bytes. This forces the C++ iterator to execute its state-saving and buffer-flushing logic repeatedly, measuring the latency of context switching in a lazy pipeline.5  
* **Simulating EINVAL (Incomplete Sequences):** By feeding the mock API a sequence chunked exactly down the middle of a multi-byte code point, the benchmark measures the cycle cost of the view prepending leftover bytes without dropping data.5

## **7\. The Compiler Optimizer as a Critical Variable**

A benchmarking proposal for a modern C++ ranges library is fundamentally incomplete—and likely invalid—without a rigorous specification of the compiler toolchain and its active flags. The Beman utf\_view relies heavily on C++20 concepts, constrained template parameters, and deeply nested iterator adapters.2 In a debug build, or without the proper optimization flags, the abstraction penalty of C++ ranges will make utf\_view appear orders of magnitude slower than a raw iconv C pointer loop, providing a false negative on the proposal's viability.7

### **7.1 Maximum Optimization (-O3) and Auto-Vectorization**

The \-O3 flag (or \-Ofast, though \-ffast-math is generally irrelevant for integer-based text transcoding) is strictly required for the benchmark suite.27

* **Optimization Impact:** \-O3 enables aggressive loop unrolling, deep function inlining, and most importantly, auto-vectorization.44 Modern compilers (GCC 14+, Clang 18+, MSVC) are increasingly capable of recognizing bit-masking loops within scalar C++ code and automatically substituting SSE or AVX instructions without manual intrinsics. The benchmark must be run explicitly under \-O3 to determine how closely standard, scalar C++29 ranges code can approximate the manually hand-tuned intrinsics of the simdutf library over the ASCII fast-path.

### **7.2 Link Time Optimization (-flto)**

The absolute most critical compiler flag for benchmarking C++ ranges against legacy C libraries is Link Time Optimization (-flto).27

* **The Ranges Abstraction Penalty:** Range adapters often generate complex, highly templated intermediate types (std::ranges::transform\_view, std::views::filter, beman::utf\_view::to\_utf16) that are passed across function boundaries.2 Without LTO enabled, the compiler's optimization algorithms are strictly bound by the translation unit (TU). It cannot effectively inline a view definition located in performance\_sensitive.cpp into the benchmarking harness located in main.cpp.7  
* **The LTO Resolution:** Compiling with \-flto (and \-ffat-lto-objects where necessary for static library compatibility) delays final machine code generation until the linking phase. This allows the compiler to view the entire program's intermediate representation at once.43 Consequently, the compiler can mathematically flatten the deeply nested C++29 iterator layers of utf\_view down into tightly packed, contiguous machine code instructions.7 Benchmarks conducted without \-flto will exhibit a false "abstraction penalty" that does not reflect modern production environments. The suite must report metrics for both \-O3 and \-O3 \-flto to quantify this exact delta.

## **8\. The Proposed Execution Matrix and Synthesized Implications**

To produce exhaustive, academically sound, and comparative data that addresses the rigorous concerns of the WG21 C++ standard committee, the benchmarking methodology demands the execution of a highly structured multidimensional matrix.

### **8.1 The Execution Matrix Topology**

**Dimension 1: The Implementations (The Baselines)**

1. Beman utf\_view (The WHATWG scalar pipeline proposal).  
2. Beman iconv\_transcode\_view (The direct legacy streaming proposal).  
3. System iconv (The raw POSIX C API baseline).  
4. std::codecvt (The deprecated C++11 standard baseline).  
5. encoding\_rs (The Rust FFI wrapper for WHATWG parity).  
6. simdutf (The C++ AVX/NEON hardware performance ceiling).

**Dimension 2: The Corpora (The Data)**

1. *Wikipedia Mars (English):* Tests UTF-8 / ASCII fast paths.  
2. *Wikipedia Mars (Arabic):* Tests complex UTF-8 multibyte branch prediction.  
3. *Wikipedia Mars (Czech):* Tests windows\_1251 / iso\_8859\_2 ![][image1] single-byte table lookups.  
4. *Wikipedia Mars (Japanese):* Tests shift\_jis / iso\_2022\_jp stateful multi-level trie indexing.  
5. *Synthetic WPT Vectors:* Tests error-replacement and EILSEQ fallback overhead.

**Dimension 3: Pipeline Topologies (The Environment)**

1. **Monolithic/Eager Evaluation:** Processing the entire input buffer into a std::vector\<char32\_t\> in a single pass (evaluating raw mechanical throughput).  
2. **Chunked/Lazy Evaluation:** Processing the input buffer wrapped tightly in std::views::chunk(1024). This artificially forces iterator state-saving at boundary edges to evaluate the true C++ range abstraction overhead.

**Dimension 4: Compiler Toolchains (The Optimizer)**

1. GCC (Latest Trunk) \-O3.  
2. GCC (Latest Trunk) \-O3 \-flto.  
3. Clang (Latest Trunk) \-O3 \-flto (Required to compare LLVM's auto-vectorization heuristics against GCC's).

### **8.2 Synthesized Analytical Insights**

The implementation of this methodological framework will uncover several critical second- and third-order insights regarding the standardization of text encoding in C++29.  
**The Illusion of Zero-Cost Abstractions in Chunked Text Processing:** While C++ ranges continually promise zero-cost abstractions, text transcoding inherently defies this principle in chunked or asynchronous environments. The Beman architecture's strict adherence to safe constraints—such as structurally banning array decay via std::views::null\_term and avoiding strict aliasing via the octet / byte\_like concept—introduces irrefutable type-safety at compile time.3 However, at runtime, the necessity to safely handle state shifts (e.g., iso\_2022\_jp) across chunk boundaries mandates that the iterators carry heavy internal state. The proposed benchmark matrix will reliably reveal that while monolithic, eager operations approach C-level speeds, heavily chunked lazy views will incur a measurable state-management penalty. This realization makes Link Time Optimization (-flto) not merely a suggestion, but an absolute necessity for modern string processing.5  
**The Mechanical Sympathy Gap and Hardware Intrinsics:** The comparison between utf\_view and simdutf will highlight a fundamental philosophical divergence in standard library design. The simdutf library achieves its extreme, billion-character-per-second throughput by aggressively abandoning generic, character-by-character iteration in favor of loading massive 64-byte chunks directly into AVX-512 registers.19 The Beman utf\_view, adhering to standard C++ range semantics, iterates character by character. The execution of this benchmark suite will mathematically demonstrate that unless compiler auto-vectorizers become significantly more advanced, a purely standard-compliant, iterator-based C++ transcoding pipeline cannot physically match the throughput of manual SIMD intrinsics. This resulting data justifies the Beman proposal's design choice: allowing ecosystem composability and memory safety to take precedence over extreme, hardware-coupled optimization.  
**The Direct OS Bypass Advantage:** The most revealing and arguably most important metric for enterprise adoption will emerge from the iconv\_transcode\_view. By explicitly bypassing the WHATWG requirement to reify intermediate char32\_t scalars, this specific view pipes legacy bytes directly into legacy bytes.3 If the Catch2 benchmarks confirm that the Beman iterator can wrap the POSIX iconv handle without adding measurable cycle overhead, it will empirically prove that the C++29 standard can seamlessly absorb legacy OS capabilities without forcing developers to pay the massive memory bandwidth cost of expanding every single byte into a 4-byte scalar before downcasting it again.3  
By executing this exhaustive methodology—incorporating dynamic Catch2 sampling, rigorous compiler optimization controls, real-world linguistic corpora, and deterministic state-machine mocking—the performance, viability, and necessity of the C++29 utf\_view transcoding proposal can be definitively proven to the standardization committee.

#### **Works cited**

1. SG16: Unicode meeting summaries 2024-03-13 through 2024-05-08 \- Open Standards, accessed May 25, 2026, [https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3302r0.html](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3302r0.html)  
2. bemanproject/utf\_view: Implementation of the UTF transcoding views proposal, P2728 \- GitHub, accessed May 25, 2026, [https://github.com/bemanproject/utf\_view](https://github.com/bemanproject/utf_view)  
3. OK print out as fenced \`\`\`markdown for ease of pu...  
4. r/wg21 \- Unicode in the Library, Part 1: UTF Transcoding \- The C++ Alliance, accessed May 25, 2026, [https://cppalliance.org/r/wg21/p2728r11-reddit.html](https://cppalliance.org/r/wg21/p2728r11-reddit.html)  
5. Print out in a quad fenced-\`\`\`\` markdown block, s...  
6. Unicode in the Library, Part 2: Normalization \- Open Standards, accessed May 25, 2026, [https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2729r0.html](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2729r0.html)  
7. Header files are killing your performance, don't bully your compiler : r/cpp \- Reddit, accessed May 25, 2026, [https://www.reddit.com/r/cpp/comments/17v440e/header\_files\_are\_killing\_your\_performance\_dont/](https://www.reddit.com/r/cpp/comments/17v440e/header_files_are_killing_your_performance_dont/)  
8. hsivonen/encoding\_rs: A Gecko-oriented implementation of the Encoding Standard in Rust \- GitHub, accessed May 25, 2026, [https://github.com/hsivonen/encoding\_rs](https://github.com/hsivonen/encoding_rs)  
9. The Wonderfully Terrible World of C and C++ Encoding APIs (with Some Rust) \- The Pasture, accessed May 25, 2026, [https://thephd.dev/the-c-c++-rust-string-text-encoding-api-landscape](https://thephd.dev/the-c-c++-rust-string-text-encoding-api-landscape)  
10. sg16-meetings/README-2023.md at master \- GitHub, accessed May 25, 2026, [https://github.com/sg16-unicode/sg16-meetings/blob/master/README-2023.md](https://github.com/sg16-unicode/sg16-meetings/blob/master/README-2023.md)  
11. cuneicode, and the Future of Text in C \- The Pasture, accessed May 25, 2026, [https://thephd.dev/cuneicode-and-the-future-of-text-in-c](https://thephd.dev/cuneicode-and-the-future-of-text-in-c)  
12. fengmk2/iconv: Port iconv-lite API on Rust \- GitHub, accessed May 25, 2026, [https://github.com/fengmk2/iconv](https://github.com/fengmk2/iconv)  
13. encoding\_rs \- Rust \- Docs.rs, accessed May 25, 2026, [https://docs.rs/encoding\_rs/](https://docs.rs/encoding_rs/)  
14. encoding\_rs: a Web-Compatible Character Encoding Library in Rust \- Henri Sivonen, accessed May 25, 2026, [https://hsivonen.fi/encoding\_rs/](https://hsivonen.fi/encoding_rs/)  
15. Yore \- library for decoding/encoding character sets according to OEM code pages : r/rust, accessed May 25, 2026, [https://www.reddit.com/r/rust/comments/od2nqb/yore\_library\_for\_decodingencoding\_character\_sets/](https://www.reddit.com/r/rust/comments/od2nqb/yore_library_for_decodingencoding_character_sets/)  
16. Switch from simd to packed\_simd · Issue \#23 · hsivonen/encoding\_rs \- GitHub, accessed May 25, 2026, [https://github.com/hsivonen/encoding\_rs/issues/23](https://github.com/hsivonen/encoding_rs/issues/23)  
17. The Wonderfully Terrible World of C and C++ Encoding APIs (with Some Rust) \- Reddit, accessed May 25, 2026, [https://www.reddit.com/r/cpp/comments/y4dx3z/the\_wonderfully\_terrible\_world\_of\_c\_and\_c/](https://www.reddit.com/r/cpp/comments/y4dx3z/the_wonderfully_terrible_world_of_c_and_c/)  
18. hsivonen/safe\_encoding\_rs\_mem: Reference implementation of the encoding\_rs::mem API using the standard library and no unsafe \- GitHub, accessed May 25, 2026, [https://github.com/hsivonen/safe\_encoding\_rs\_mem](https://github.com/hsivonen/safe_encoding_rs_mem)  
19. simdutf: Text processing at billions of characters per second \- GitHub, accessed May 25, 2026, [https://github.com/simdutf/simdutf](https://github.com/simdutf/simdutf)  
20. simdutf: Unicode validation and transcoding at billions of characters per second \- Google Git, accessed May 25, 2026, [https://chromium.googlesource.com/external/github.com/simdutf/simdutf/+/refs/tags/upstream/v3.2.0](https://chromium.googlesource.com/external/github.com/simdutf/simdutf/+/refs/tags/upstream/v3.2.0)  
21. simdutf repositories \- GitHub, accessed May 25, 2026, [https://github.com/orgs/simdutf/repositories](https://github.com/orgs/simdutf/repositories)  
22. simdutf \- Ravenports, accessed May 25, 2026, [https://www.ravenports.com/catalog/bucket\_FF/simdutf/std/](https://www.ravenports.com/catalog/bucket_FF/simdutf/std/)  
23. Transcoding Billions of Unicode Characters per Second with SIMD Instructions | R \-libre \- TeluQ, accessed May 25, 2026, [https://r-libre.teluq.ca/2400/3/Transcoding%20Billions%20of%20Unicode%20Characters%20per%20Second%20with%20SIMD%20Instructions.pdf](https://r-libre.teluq.ca/2400/3/Transcoding%20Billions%20of%20Unicode%20Characters%20per%20Second%20with%20SIMD%20Instructions.pdf)  
24. SWUFE-DB-Group/Janus: A Two-Level Character Validation Framework for Legacy CJK Encodings and Short-Texts \- GitHub, accessed May 25, 2026, [https://github.com/SWUFE-DB-Group/Janus](https://github.com/SWUFE-DB-Group/Janus)  
25. simdutf: Text processing at billions of characters per second \- GitHub Pages, accessed May 25, 2026, [https://simdutf.github.io/simdutf/](https://simdutf.github.io/simdutf/)  
26. simdutf package \- github.com/charlievieth/simdutf \- Go Packages \- Golang, accessed May 25, 2026, [https://pkg.go.dev/github.com/charlievieth/simdutf](https://pkg.go.dev/github.com/charlievieth/simdutf)  
27. Optimize Options (Using the GNU Compiler Collection (GCC)), accessed May 25, 2026, [https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)  
28. Language corpora and Text Encoding Initiative(TEI) \- GFOSS, accessed May 25, 2026, [https://gfoss.eu/language-corpora-and-text-encoding-initiativetei/](https://gfoss.eu/language-corpora-and-text-encoding-initiativetei/)  
29. Corpus encoding \- CNR-ILC, accessed May 25, 2026, [https://www.ilc.cnr.it/EAGLES96/corpintr/node18.html](https://www.ilc.cnr.it/EAGLES96/corpintr/node18.html)  
30. Encoding Standards for Linguistic Corpora \- Computer Science | Vassar College, accessed May 25, 2026, [https://www.cs.vassar.edu/\~ide/papers/tihany.tei.pdf](https://www.cs.vassar.edu/~ide/papers/tihany.tei.pdf)  
31. Perform normalization performance evaluation between Rust and ICU \#93 \- GitHub, accessed May 25, 2026, [https://github.com/unicode-org/icu4x/issues/93](https://github.com/unicode-org/icu4x/issues/93)  
32. hsivonen/encoding\_bench: Performance testing framework ... \- GitHub, accessed May 25, 2026, [https://github.com/hsivonen/encoding\_bench](https://github.com/hsivonen/encoding_bench)  
33. Month: January 2023 \- Daniel Lemire's blog, accessed May 25, 2026, [https://lemire.me/blog/2023/01/](https://lemire.me/blog/2023/01/)  
34. catchorg/Catch2: A modern, C++-native, test framework for unit-tests, TDD and BDD \- using C++14, C++17 and later (C++11 support is in v2.x branch, and C++03 on the Catch1.x branch) \- GitHub, accessed May 25, 2026, [https://github.com/catchorg/Catch2](https://github.com/catchorg/Catch2)  
35. A Microbenchmark Framework for Performance Evaluation of OpenMP Target Offloading, accessed May 25, 2026, [https://arxiv.org/html/2503.00408v1](https://arxiv.org/html/2503.00408v1)  
36. Catch2 documentation, accessed May 25, 2026, [https://catch2-temp.readthedocs.io/](https://catch2-temp.readthedocs.io/)  
37. Catch2/docs/benchmarks.md at devel \- GitHub, accessed May 25, 2026, [https://github.com/catchorg/Catch2/blob/devel/docs/benchmarks.md](https://github.com/catchorg/Catch2/blob/devel/docs/benchmarks.md)  
38. Catch2/tests/SelfTest/UsageTests/Benchmark.tests.cpp at devel \- GitHub, accessed May 25, 2026, [https://github.com/catchorg/Catch2/blob/devel/tests/SelfTest/UsageTests/Benchmark.tests.cpp](https://github.com/catchorg/Catch2/blob/devel/tests/SelfTest/UsageTests/Benchmark.tests.cpp)  
39. The Little Things: Testing with Catch2 \- The Coding Nest, accessed May 25, 2026, [https://codingnest.com/the-little-things-testing-with-catch-2/](https://codingnest.com/the-little-things-testing-with-catch-2/)  
40. Optional (k/M/G)B/s Benchmark result · Issue \#1839 · catchorg/Catch2 \- GitHub, accessed May 25, 2026, [https://github.com/catchorg/Catch2/issues/1839](https://github.com/catchorg/Catch2/issues/1839)  
41. Benchmark Dashboard · Issue \#25 · Nugine/simd \- GitHub, accessed May 25, 2026, [https://github.com/Nugine/simd/issues/25](https://github.com/Nugine/simd/issues/25)  
42. Visualize Catch2 benchmarks with Vega-Lite | Johtizen, accessed May 25, 2026, [https://joht.github.io/johtizen/data/2022/09/05/visualize-catch2-benchmarks-with-vega-lite.html](https://joht.github.io/johtizen/data/2022/09/05/visualize-catch2-benchmarks-with-vega-lite.html)  
43. Using the GNU Compiler Collection (GCC): Optimize Options, accessed May 25, 2026, [https://gcc.gnu.org/onlinedocs/gcc-5.5.0/gcc/Optimize-Options.html](https://gcc.gnu.org/onlinedocs/gcc-5.5.0/gcc/Optimize-Options.html)  
44. When C++ O3 is Slower than O2 \- Abutalib (Barish) Namazov, accessed May 25, 2026, [https://barish.me/blog/cpp-o3-slower/](https://barish.me/blog/cpp-o3-slower/)  
45. Beman Libraries, accessed May 25, 2026, [https://bemanproject.org/libraries/](https://bemanproject.org/libraries/)  
46. Link Time Optimizations: New Way to Do Compiler Optimizations \- Johnny's Software Lab, accessed May 25, 2026, [https://johnnysswlab.com/link-time-optimizations-new-way-to-do-compiler-optimizations/](https://johnnysswlab.com/link-time-optimizations-new-way-to-do-compiler-optimizations/)

[image1]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACsAAAAaCAYAAAAue6XIAAAB5klEQVR4Xu2XPyhHURTHj1BEFCJGMZBNTEZGBhbFqBhMDKJMdkkmWUxKkrIYDL9MMpOZRAYjA/lzvp33q/u+3ffe/dUPv+F96tuvvue8e89977x7308kJ6fsNKsOVcscSKFLdaWq4kAp4GJMeqI6V63Hw16Qt6uq5UAGw6opNkNoUX2pjslvUH2LFeQDk42xGYGFb6reVY0UK3IqdoeD6VFdq1ZV1RQDGAwF8yNrF7uunnzwKHbNZ/SbVCwWipwg0G8Y7IIDDiOqD9U8+WeqB/KYfUkvFqyoptlkcKc2xFaW9CjBoOpVtUc+vCPymJBicTMOVDUccNkRG2iGA8S4WB4mdoGHWBohxeK9wMJxUxLBI8RA3Rwg1sTyth0PE7yphhzPR0ixIHPhSICyeBLL63O8TtVd9JtGKcWidxMJLbaY5/ZURRY7KZazRX7FFVvcR7GtYYtzaVO9SLw1fJRS7BKbLgtiSb5tCxs9YrcccEA89aWQsGKDdgOc5TjT7yX+VveLnVqYqMnxGRSBncJHh2pAdSOWN6fqVdW5SRHIRR6eViYYYFSscGxPrfFwIpeSfYKFgF6dYLPcLEp6z4eA9iiIvR+/zqz4ez4UfHWlfZeUFfQ9+ju0dZhnse/aP+Pf/ink5FQiP/Ojc5TBNIS5AAAAAElFTkSuQmCC>

[image2]: <data:image/png;base64,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>