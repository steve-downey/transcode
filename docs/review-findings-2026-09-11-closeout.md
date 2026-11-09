<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# Repository review findings — 2026-09-11 closeout

This closes the findings in
[`review-findings-2026-09-11.md`](review-findings-2026-09-11.md) against the
integrated Phase 6 tree.  The review contained 27 findings.  Planning
and implementation found three more ([utf32-validation-not-uniform](plans/p6-step6-utf32-validation.md#utf32-validation-not-uniform), [gitleaks-hook](plans/p6-step9-docs-build-ci.md#gitleaks-hook) and [encode-error-question-mark-claim](plans/p6-step6-utf32-validation.md#encode-error-question-mark-claim)), so the accounting
below has 30 rows.

"Documented" means the behavior is intentional and is now stated accurately;
it does not mean that SG16 has made the design decision.  In particular, [optional-iconv-story](review-findings-2026-09-11.md#optional-iconv-story),
[posix-type-in-wording](review-findings-2026-09-11.md#posix-type-in-wording) and [fffd-error-sentinel](review-findings-2026-09-11.md#fffd-error-sentinel) remain questions for SG16 rather than accidental omissions in the
implementation or proposal.

## Finding accounting

The review and the Phase 6 plans first named the findings by serial (`C-01`,
`D-12`, and so on), and the commit subjects that closed them still do.  The
last column maps each serial to its finding, so `git log` stays readable.

| Finding | Disposition | Commit(s) | What closed it | Serial in commit subjects |
| --- | --- | --- | --- | --- |
| [sniff-consumes-single-pass](review-findings-2026-09-11.md#sniff-consumes-single-pass) | Closed | `3fd83a60` | `sniff_encoding` now requires a forward range, with positive and negative constraint tests. | `C-01` |
| [iconv-small-buffer-loses-input](review-findings-2026-09-11.md#iconv-small-buffer-loses-input) | Closed | `c24f9183` | The iconv views specify and enforce a minimum working-buffer size; boundary tests cover the former silent-empty result. | `C-02` |
| [iconv-flush-once](review-findings-2026-09-11.md#iconv-flush-once) | Closed | `c24f9183` | Flush state is resumable across `E2BIG`; mock tests require multiple flush calls and cover flush failures. | `C-03` |
| [iconv-lossy-replacement-corrupts](review-findings-2026-09-11.md#iconv-lossy-replacement-corrupts) | Closed | `063b380d` | Lossy eager iconv conversion skips rejected input instead of appending an encoding-independent `0x3f`.  UTF-16 and eager/lazy parity tests cover the policy. | `C-04` |
| [iconv-error-conflation](review-findings-2026-09-11.md#iconv-error-conflation) | Closed | `c24f9183`, `75e52502` | `open_failed` and `system_error` are explicit `iconv_error` values and unexpected platform failures are no longer misclassified; focused tests cover every flush-error classification. | `C-05` |
| [null-term-reachable-terminator](review-findings-2026-09-11.md#null-term-reachable-terminator) | Closed | `3e1f07d2` | `null_term_view` states its reachable-terminator and lifetime preconditions; its stored iterator is value-initialized and the singular state is documented. | `C-06` |
| [empty-output-as-unsupported-target](review-findings-2026-09-11.md#empty-output-as-unsupported-target) | Closed | `51d33e0e` | Runtime codec and label overloads return `optional<string>` and reject encoder-less targets before decoding, so empty success is unambiguous. | `C-07` |
| [utf32-validation-not-uniform](plans/p6-step6-utf32-validation.md#utf32-validation-not-uniform) | Closed | `621d12c6` | The newly found non-UTF-8 inconsistency is removed by validating UTF-32 input once, before codec dispatch, for all 40 codecs. | `C-08` |
| [optional-iconv-story](review-findings-2026-09-11.md#optional-iconv-story) | Documented | `b3d69e53` | The README and paper now say that the WHATWG implementation is portable header-only source while the reference package currently requires platform iconv. | `P-01` |
| [posix-type-in-wording](review-findings-2026-09-11.md#posix-type-in-wording) | Documented | `b3d69e53` | The iconv facility and `iconv_t` handle are explicitly implementation-defined, and the paper asks SG16 whether the POSIX adaptor should progress separately. | `P-02` |
| [invalid-char32-contract](review-findings-2026-09-11.md#invalid-char32-contract) | Closed | `621d12c6`, `b3d69e53` | The scalar-value precondition was removed.  Surrogates and out-of-range `char32_t` values are uniformly replaced or reported before encoding. | `S-01` |
| [iconv-lossy-recovery-policy](review-findings-2026-09-11.md#iconv-lossy-recovery-policy) | Closed | `063b380d` | Lazy and eager lossy iconv APIs now share the documented skip policy. | `S-02` |
| [fffd-error-sentinel](review-findings-2026-09-11.md#fffd-error-sentinel) | Documented | `37ff0fd1`, `b3d69e53` | U+FFFD is explicitly reserved as the indexed decode sentinel; the extensibility cost and possible replacement designs are referred to SG16. | `S-03` |
| [synopsis-free-standing-prose](review-findings-2026-09-11.md#synopsis-free-standing-prose) | Closed | `b3d69e53` | The wording outline records that declaration-level descriptions beside synopsis declarations are intentional specgen output; validation accepts the placement. | `S-04` |
| [unsubstantiated-claims](review-findings-2026-09-11.md#unsubstantiated-claims) | Closed | `b3d69e53` | Platform, performance, browser-engine and WHATWG codec-set claims were narrowed to what the repository and cited benchmark support. | `S-05` |
| [duplicate-author-email](review-findings-2026-09-11.md#duplicate-author-email) | Closed | `b3d69e53` | The duplicate author email key was removed. | `D-01` |
| [feature-test-macro-placeholders](review-findings-2026-09-11.md#feature-test-macro-placeholders) | Closed | `b3d69e53` | Feature-test macro placeholders now consistently use `202XXXL`. | `D-02` |
| [utf32-bom-sniffing-claim](review-findings-2026-09-11.md#utf32-bom-sniffing-claim) | Closed | `b3d69e53` | README and paper text now limit BOM sniffing to UTF-8, UTF-16BE and UTF-16LE. | `D-03` |
| [revision-drift](review-findings-2026-09-11.md#revision-drift) | Closed | `b3d69e53` | D4246 and P2728 revision references were reconciled; generic P2728 labels are used where an exact revision is not material. | `D-04` |
| [build-instructions-framework-and-standard](review-findings-2026-09-11.md#build-instructions-framework-and-standard) | Closed | `15a61282` | Contributor documentation names Catch2 3 and requests C++23 in every manual configuration example. | `D-05` |
| [compiler-support-vs-cmake](review-findings-2026-09-11.md#compiler-support-vs-cmake) | Closed | `c20d92c2`, `35ca56e7` | CMake probes the required library capabilities rather than a Clang version, and the README distinguishes tested configurations from capability-gated ones. | `D-06` |
| [table-codec-documented-as-public](review-findings-2026-09-11.md#table-codec-documented-as-public) | Closed | `41fbf060` | The custom codec example implements the public codec protocol and uses `decode`/`decode_or_error`; README no longer presents `detail::table_codec` as public API. | `D-07` |
| [icu-iconv-claim](review-findings-2026-09-11.md#icu-iconv-claim) | Closed | `07a547b3` | ICU is described as a separate backend requiring an adaptor, not as a provider of the POSIX iconv API. | `D-08` |
| [root-file-named-dash](review-findings-2026-09-11.md#root-file-named-dash) | Closed | `32d331d3` | The tracked root file named `-` was removed and a filename hygiene hook prevents recurrence. | `D-09` |
| [stale-handoff-guide](review-findings-2026-09-11.md#stale-handoff-guide) | Closed | `d6ccff14` | The Phase 4 handoff is prominently dated and marked historical while retaining its useful coverage audit. | `D-10` |
| [manual-lint-duplicate](review-findings-2026-09-11.md#manual-lint-duplicate) | Closed | `f62c3af5` | The duplicate CI lint step was removed; `lint-manual` remains as the future manual-stage entry point. | `D-11` |
| [module-install-consumer-test](review-findings-2026-09-11.md#module-install-consumer-test) | Closed | `364eac09`, `e4db58b7`, `cf87e1cd` | The module unit is licensed, its include graph is installed, CMake 4.4 import-std activation is correct, and clean installed-package consumers pass in header and module modes. | `D-12` |
| [telemetry-shebang](review-findings-2026-09-11.md#telemetry-shebang) | Closed | `43c7d9ef` | The telemetry script's shebang is now its first line. | `D-13` |
| [gitleaks-hook](plans/p6-step9-docs-build-ci.md#gitleaks-hook) | Closed | `6e94f7af` | The newly found gitleaks hook selects the installed CLI spelling and reports unsupported versions clearly. | `D-14` |
| [encode-error-question-mark-claim](plans/p6-step6-utf32-validation.md#encode-error-question-mark-claim) | Closed | `621d12c6`, `b3d69e53` | The newly found encode-error prose now describes U+FFFD substitution rather than a raw question mark. | `D-15` |

## Verification results

The supported project matrix is native GCC 16.2.1 and Clang 22.1.8 through the
repository toolchain files.  Containers and external images are strictly off
limits and were not used.  The review's request for a second iconv
implementation is out of scope; the focused iconv regressions run against the
supported system implementation.

| Check | Result |
| --- | --- |
| GCC 16 ASan | 750/750 C++ tests and 265/265 Python tests passed. |
| GCC 16 Debug | The current tree built and 750/750 C++ tests passed. |
| GCC 16 RelWithDebInfo | The current tree built successfully. |
| Clang 22 ASan | 750/750 C++ tests and 265/265 Python tests passed. |
| Clang 22 Debug | The current tree built and 750/750 C++ tests passed; 265/265 Python tests also passed. |
| Clang 22 RelWithDebInfo | The current tree built successfully. |
| GCC 16 coverage | 750/750 tests passed; coverage was 72.9% of lines, 89.8% of functions and 42.1% of branches. |
| Module modes | The non-module suites above passed, and the module integration test passed 1/1. |
| Installed consumers | The header consumer passed 1/1; the module and header-shim consumers passed 2/2. |
| Examples | All seven example programs built and ran successfully. |
| Wording | `wording-check`, `wording-inputs-check` and `wording-validate` passed with no pending regeneration.  The full check used the installed `specgen`. |
| Lint | `make lint` and `make lint-manual` passed, including codespell, mypy, clang-tidy and the configured pre-commit hooks. |

The optimized GCC build emitted a `-Warray-bounds` diagnostic in inlined
libstdc++ vector-growth code; inspection found no project defect behind it.  It
also emitted `-Wdangling-pointer` for `benchmark/benchmark_sink.hpp` storing an
argument address in `volatile_sink` beyond the argument's lifetime.  That
genuine warning is confined to the benchmark-only dead-code-elimination helper,
which never dereferences the stored pointer.  Reworking that helper is
explicitly deferred as a benchmark-harness follow-up and does not affect the
installed library or the verification results above.
