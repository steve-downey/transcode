# Makefile                                                       -*-makefile-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

export
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

NO_COLOR:=1

INSTALL_PREFIX ?= .install/
BUILD_DIR ?= .build
DEST ?= $(INSTALL_PREFIX)
CMAKE_FLAGS ?=

PYEXECPATH ?= $(shell which python3.13 || which python3.12 || which python3.11 || which python3.10 || which python3.9 || which python3.8 || which python3)
PYTHON ?= $(notdir $(PYEXECPATH))
VENV := .venv
UV := $(shell command -v uv 2> /dev/null)
ACTIVATE := $(UV) run
PYEXEC := $(UV) run python
MARKER = .initialized.venv.stamp

PRE_COMMIT := $(UV) run pre-commit

EMACS := $(shell command -v emacs 2> /dev/null)

.update-submodules:
	git submodule update --init --recursive
	touch .update-submodules

.gitmodules: .update-submodules

CONFIG ?= Asan

export

ifeq ($(strip $(TOOLCHAIN)),)
	_build_name?=build-system/
	_build_dir?=.build/
	_local_toolchain?=$(CURDIR)/etc/toolchain.cmake
else
	_build_name?=build-$(TOOLCHAIN)
	_build_dir?=.build/
	_local_toolchain?=$(CURDIR)/etc/$(TOOLCHAIN)-toolchain.cmake
endif

_configuration_types ?= "RelWithDebInfo;Debug;Tsan;Asan;Gcov;Perf;Tidy"

_build_path ?= $(_build_dir)/$(_build_name)
_build_path := $(subst //,/,$(_build_path))
_build_path := $(patsubst %/,%,$(_build_path))

VCPKG ?= $(shell command -v vcpkg 2> /dev/null)

ifeq ($(VCPKG),)
	_cmake_top_level?="infra/cmake/use-fetch-content.cmake"
	_toolchain:=$(_local_toolchain)
	_args=
else
	_vcpkg_toolchain:=$(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
	_cmake_top_level?=$(_vcpkg_toolchain)
	export PROJECT_VCPKG_TOOLCHAIN=$(_local_toolchain)
	_toolchain:=$(_local_toolchain)
	_args=-DVCPKG_OVERLAY_TRIPLETS=$(CURDIR)/cmake -DVCPKG_TARGET_TRIPLET=x64-linux-custom
	# for debugging add 	-DVCPKG_INSTALL_OPTIONS="--debug"
endif

CMAKE ?= $(UV) run cmake
CTEST ?= $(UV) run ctest
define run_cmake =
	$(CMAKE) \
	-G "Ninja Multi-Config" \
	-DCMAKE_CONFIGURATION_TYPES=$(_configuration_types) \
	-DCMAKE_INSTALL_PREFIX=$(abspath $(INSTALL_PREFIX)) \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
	-DCMAKE_PREFIX_PATH=$(CURDIR)/infra/cmake \
	-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=$(_cmake_top_level) \
	-DCMAKE_C_COMPILER_LAUNCHER=ccache \
	-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
	-DCMAKE_TOOLCHAIN_FILE=$(_toolchain) \
	$(_args) \
	$(_cmake_args) \
	$(CURDIR)
endef

default: test
.PHONY: default

$(_build_path):
	mkdir -p $(_build_path)

$(_build_path)/CMakeCache.txt: | $(_build_path) .gitmodules $(VENV)
	cd $(_build_path) && $(run_cmake)

$(_build_path)/compile_commands.json: $(_build_path)/CMakeCache.txt

.PHONY: compile_commands.json
compile_commands.json: $(_build_path)/compile_commands.json
compile_commands.json: ## symlink the current compile commands db
	if [ "$(shell readlink compile_commands.json)" != "$(_build_path)/compile_commands.json" ] ; then \
		ln -sf $(_build_path)/compile_commands.json ; \
	fi

.PHONY: clean-compile-commands
clean-compile-commands: ## Delete the compile commands symlink
	-rm -f compile_commands.json

clean: clean-compile-commands

.PHONY: compile
compile: $(_build_path)/CMakeCache.txt compile_commands.json
compile: ## Compile the project
	$(CMAKE) --build $(_build_path)  --config $(CONFIG) --target all -- -k 0

.PHONY: compile-headers
compile-headers: $(_build_path)/CMakeCache.txt ## Compile the headers
	 $(CMAKE) --build $(_build_path)  --config $(CONFIG) --target all_verify_interface_header_sets -- -k 0

.PHONY: install
install: $(_build_path)/CMakeCache.txt compile ## Install the project
	$(CMAKE) --install $(_build_path) --config $(CONFIG) --component transcode_Development --verbose

.PHONY: clean-install
clean-install:
	-rm -rf $(INSTALL_PREFIX)

clean: clean-install

.PHONY: realclean
realclean: clean clean-install

.PHONY: ctest
ctest: $(_build_path)/CMakeCache.txt ## Run CTest on current build
	$(CTEST) --test-dir $(_build_path) --output-on-failure -C $(CONFIG)

.PHONY: ctest_
ctest_: compile
	$(CTEST) --test-dir $(_build_path) --output-on-failure -C $(CONFIG)

.PHONY: pytest
pytest: venv ## Run Python tool tests
	$(UV) run pytest tools/tests/

.PHONY: mypy
mypy: venv ## Run mypy type checker on Python tools
	$(UV) run mypy tools/

.PHONY: test
test: ctest_ pytest ## Rebuild and run all tests (C++ and Python)

.PHONY: cmake
cmake: |  $(_build_path)
	cd $(_build_path) && ${run_cmake}

.PHONY: clean
clean: ## Clean the build artifacts without recreating the build tree
	if [ -f $(_build_path)/CMakeCache.txt ] ; then \
		$(CMAKE) --build $(_build_path) --config $(CONFIG) --target clean ; \
	fi

.PHONY: clean-reconf
clean-reconf: ## Delete the current configured build tree
	rm -rf $(_build_path)

.PHONY: reconf
reconf: clean-reconf cmake ## Recreate the current configured build tree

.PHONY: realclean
realclean: ## Delete the generated build infrastructure
	rm -rf $(_build_dir) build

.PHONY: env
env:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))

.PHONY: all
all: compile

.PHONY: venv
venv: ## Create python virtual env
venv: $(VENV)/$(MARKER)

.PHONY: clean-venv
clean-venv: ## Delete python virtual env
	-rm -rf $(VENV)

.PHONY: clean-uv-lock
clean-uv-lock: ## Delete the generated uv lockfile
	-rm -f uv.lock

realclean: clean-venv
realclean: clean-uv-lock

.PHONY: show-venv
show-venv: venv
show-venv: ## Debugging target - show venv details
	$(PYEXEC) -c "import sys; print('Python ' + sys.version.replace('\n',''))"
	@echo venv: $(VENV)

uv.lock: pyproject.toml
	$(UV) lock

$(VENV):
	$(UV) venv --python $(PYTHON)

$(VENV)/$(MARKER): uv.lock | $(VENV)
	$(UV) sync
	touch $(VENV)/$(MARKER)

.PHONY: dev-shell
dev-shell: venv
dev-shell: ## Shell with the venv activated
	$(ACTIVATE) $(notdir $(SHELL))

.PHONY: bash zsh
bash zsh: venv
bash zsh: ## Run bash or zsh with the venv activated
	$(ACTIVATE) $@

# What lives in papers/wording/ without being generated into it.  `wording-check`
# regenerates into a scratch directory and diffs, and a scratch directory holds
# only output, so every authored file here has to be named or the check reports
# it missing and calls the fragments stale.  The list is generate.sh's, because
# its cleanup needs the same one and two copies would drift.
WORDING_AUTHORED := $(shell papers/wording/generate.sh --authored)
WORDING_BUILD_INCLUDE := $(abspath $(_build_path))/include
WORDING_GCC_TOOLCHAIN := $(SPECGEN_GCC_TOOLCHAIN)
ifeq ($(strip $(WORDING_GCC_TOOLCHAIN)),)
ifneq ($(filter gcc-%,$(TOOLCHAIN)),)
WORDING_GXX := $(shell command -v g++-$(patsubst gcc-%,%,$(TOOLCHAIN)) 2>/dev/null)
WORDING_GCC_TOOLCHAIN := $(shell \
	libstdcpp=`$(WORDING_GXX) -print-file-name=libstdc++.so 2>/dev/null`; \
	libstdcpp=`readlink -f "$$libstdcpp"`; \
	libdir=`dirname "$$libstdcpp"`; \
	dirname "$$libdir")
endif
endif
WORDING_ENV = BEMAN_TRANSCODE_BUILD_INCLUDE="$(WORDING_BUILD_INCLUDE)" \
	$(if $(WORDING_GCC_TOOLCHAIN),SPECGEN_GCC_TOOLCHAIN="$(WORDING_GCC_TOOLCHAIN)")

.PHONY: wording
wording: $(_build_path)/CMakeCache.txt ## Regenerate the paper's wording fragments from the header markup
	$(WORDING_ENV) papers/wording/generate.sh

.PHONY: wording-inputs-check
wording-inputs-check: ## Fail if a spec-facing header changed without `make wording`
	papers/wording/inputs-check.sh

.PHONY: wording-pending-check
wording-pending-check: ## Fail if any regeneration is still deferred
	@[ -r papers/wording/PENDING ] || { \
		echo "papers/wording/PENDING is missing or unreadable" >&2; \
		exit 2; \
	}; \
	entries=$$(sed -e 's/#.*//' -e 's/[[:space:]]*$$//' papers/wording/PENDING | awk 'NF'); \
	if [ -n "$$entries" ]; then \
		echo "" >&2; \
		echo "papers/wording/PENDING is not empty:" >&2; \
		echo "" >&2; \
		printf '    %s\n' $$entries >&2; \
		echo "" >&2; \
		echo "A deferred regeneration is fine mid-series and is not fine here." >&2; \
		echo "Run 'make wording' and commit the result before tagging or" >&2; \
		echo "publishing a paper revision." >&2; \
		exit 1; \
	else \
		echo "no wording regeneration is deferred"; \
	fi

.PHONY: wording-check
wording-check: $(_build_path)/CMakeCache.txt ## Fail if the committed wording fragments are not what the headers generate
	@scratch=$$(mktemp -d); \
	trap 'rm -rf "$$scratch"' EXIT; \
	$(WORDING_ENV) papers/wording/generate.sh --out "$$scratch/wording" || { \
		echo "wording-check: generate.sh failed; see above" >&2; \
		exit 2; \
	}; \
	if diff -ru $(WORDING_AUTHORED:%=--exclude=%) papers/wording "$$scratch/wording"; then \
		echo "wording fragments are up to date"; \
	else \
		echo "wording fragments are stale: run 'make wording' and commit the result" >&2; \
		exit 1; \
	fi

.PHONY: wording-validate
wording-validate: $(_build_path)/CMakeCache.txt ## Report specgen's wording validation findings for each spec-facing header
	$(WORDING_ENV) papers/wording/generate.sh --validate

.PHONY: lint
lint: venv mypy clang-tidy
lint: ## Run all configured tools in pre-commit and mypy
	$(PRE_COMMIT) run -a

.PHONY: lint-manual
lint-manual: venv
# No hook in .pre-commit-config.yaml declares `stages: [manual]` today, so this
# currently runs the same set as `lint`.  It is kept as the entry point for the
# manual stage, not because it adds anything yet.
lint-manual: ## Run all manual tools in pre-commit
	$(PRE_COMMIT) run --hook-stage manual -a

.PHONY: clang-tidy
clang-tidy: $(_build_path)/CMakeCache.txt ## Run clang-tidy on project headers via Tidy config
	$(CMAKE) --build $(_build_path) --config Tidy --target all_verify_interface_header_sets -- -k 0

.PHONY: coverage
coverage: ## Build and run the tests with the GCOV profile and process the results
coverage: venv $(_build_path)/CMakeCache.txt
	$(CMAKE) --build $(_build_path) --config Gcov
	$(ACTIVATE) ctest --build-config Gcov --output-on-failure --test-dir $(_build_path)
	$(CMAKE) --build $(_build_path) --config Gcov --target process_coverage

.PHONY: bench-coverage
bench-coverage: venv $(_build_path)/CMakeCache.txt ## Run benchmarks under Gcov and generate coverage report
	$(CMAKE) --build $(_build_path) --config Gcov
	-$(_build_path)/benchmark/Gcov/beman.transcode.benchmarks.smoke "[smoke]"
	-$(_build_path)/benchmark/Gcov/beman.transcode.benchmarks.utf "[benchmark][utf]"
	-$(_build_path)/benchmark/Gcov/beman.transcode.benchmarks.whatwg "[benchmark][whatwg]"
	-$(_build_path)/benchmark/Gcov/beman.transcode.benchmarks.pluggable "[benchmark][pluggable]"
	-$(_build_path)/benchmark/Gcov/beman.transcode.benchmarks.iconv "[benchmark][iconv]"
	-$(_build_path)/benchmark/Gcov/beman.transcode.benchmarks.boundary "[benchmark][boundary]"
	$(CMAKE) --build $(_build_path) --config Gcov --target process_coverage

.PHONY: clean-coverage
clean-coverage: ## Delete generated coverage reports
	-rm -rf $(_build_path)/coverage
	-rm -f .coverage .coverage.* coverage.xml

clean: clean-coverage

.PHONY: view-coverage
view-coverage: ## View the coverage report
	sensible-browser $(_build_path)/coverage/coverage.html

.PHONY: bench
bench: compile ## Run benchmark smoke
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.smoke "[smoke]"

.PHONY: bench-utf
bench-utf: compile ## Run UTF-family benchmarks
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.utf "[benchmark][utf]"

.PHONY: bench-whatwg
bench-whatwg: compile ## Run WHATWG legacy codec benchmarks
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.whatwg "[benchmark][whatwg]"

.PHONY: bench-pluggable
bench-pluggable: compile ## Run pluggable codec benchmarks
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.pluggable "[benchmark][pluggable]"

.PHONY: bench-iconv
bench-iconv: compile ## Run iconv baseline benchmarks
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.iconv "[benchmark][iconv]"

.PHONY: bench-codecvt
bench-codecvt: compile ## Run std::codecvt negative baseline benchmarks (skips if <codecvt> absent)
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.codecvt "[benchmark][codecvt]"

.PHONY: bench-encoding-rs
bench-encoding-rs: compile ## Run encoding_rs baseline benchmarks (requires cargo; skips build if absent)
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.encoding_rs "[benchmark][encoding_rs]"

.PHONY: bench-simdutf
bench-simdutf: compile ## Run simdutf ceiling baseline benchmarks (requires cmake -DBEMAN_TRANSCODE_BENCHMARK_SIMDUTF=ON)
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.simdutf "[benchmark][simdutf]"

.PHONY: bench-boundary
bench-boundary: compile ## Run boundary stress benchmarks (chunked + mock-iconv EINVAL/E2BIG)
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.boundary "[benchmark][boundary]"

.PHONY: bench-env
bench-env: ## Print environment metadata (compiler versions, CPU, OS)
	@echo "# TIMESTAMP: $$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
	@echo "# HOSTNAME: $$(hostname)"
	@echo "# OS: $$(uname -srm)"
	@if [ -r /etc/os-release ]; then . /etc/os-release; echo "# DISTRO: $${PRETTY_NAME:-unknown}"; fi
	@if [ -r /proc/cpuinfo ]; then echo "# CPU: $$(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | xargs)"; echo "# CPU_CORES: $$(nproc)"; fi
	@if command -v gcc >/dev/null 2>&1; then echo "# GCC: $$(gcc --version | head -1)"; else echo "# GCC: not found"; fi
	@if command -v clang >/dev/null 2>&1; then echo "# CLANG: $$(clang --version | head -1)"; else echo "# CLANG: not found"; fi

.PHONY: bench-lto
bench-lto: ## Configure (if needed), build, and run GCC LTO benchmark smoke
	@if ! [ -f build/gcc-release-lto/CMakeCache.txt ]; then \
		echo "Configuring gcc-release-lto preset..."; \
		$(CMAKE) --preset gcc-release-lto; \
	fi
	$(CMAKE) --build build/gcc-release-lto --target beman.transcode.benchmarks.smoke
	build/gcc-release-lto/benchmark/beman.transcode.benchmarks.smoke "[smoke]"

.PHONY: bench-matrix-gcc
bench-matrix-gcc: ## Configure (if needed), build, and run GCC -O3 benchmark smoke
	@if ! [ -f build/gcc-release/CMakeCache.txt ]; then \
		echo "Configuring gcc-release preset..."; \
		$(CMAKE) --preset gcc-release; \
	fi
	$(CMAKE) --build build/gcc-release --target beman.transcode.benchmarks.smoke
	build/gcc-release/benchmark/beman.transcode.benchmarks.smoke "[smoke]"

.PHONY: bench-matrix-gcc-lto
bench-matrix-gcc-lto: bench-lto ## Alias for bench-lto (GCC -O3 -flto smoke)

.PHONY: bench-matrix-llvm-lto
bench-matrix-llvm-lto: ## Configure (if needed), build, and run Clang LTO benchmark smoke (skips if clang absent)
	@if ! command -v clang >/dev/null 2>&1; then \
		echo "SKIP: clang not found"; \
		exit 0; \
	fi
	@if ! [ -f build/llvm-release-lto/CMakeCache.txt ]; then \
		echo "Configuring llvm-release-lto preset..."; \
		$(CMAKE) --preset llvm-release-lto; \
	fi
	$(CMAKE) --build build/llvm-release-lto --target beman.transcode.benchmarks.smoke
	build/llvm-release-lto/benchmark/beman.transcode.benchmarks.smoke "[smoke]"

.PHONY: bench-matrix
bench-matrix: ## Run the full compiler/optimization matrix (skips unavailable slices)
	infra/scripts/bench-matrix.sh "[smoke]"

.PHONY: bench-matrix-full
bench-matrix-full: ## Run the full compiler/optimization matrix with all benchmarks
	infra/scripts/bench-matrix.sh "[benchmark]"

BENCH_RESULTS_DIR ?= data/benchmarks/results

.PHONY: bench-report
bench-report: CONFIG=RelWithDebInfo
bench-report: compile ## Run smoke benchmark (RelWithDebInfo) and generate Markdown + HTML throughput report
	@mkdir -p $(BENCH_RESULTS_DIR)
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.smoke "[smoke]" \
		--reporter xml --out $(BENCH_RESULTS_DIR)/smoke-$(CONFIG).xml
	$(UV) run python tools/process_benchmark_results.py \
		--label "$(CONFIG)" \
		--markdown $(BENCH_RESULTS_DIR)/smoke-$(CONFIG).md \
		--html $(BENCH_RESULTS_DIR)/smoke-$(CONFIG).html \
		$(BENCH_RESULTS_DIR)/smoke-$(CONFIG).xml

.PHONY: bench-report-lto
bench-report-lto: bench-lto-xml ## Run GCC LTO smoke benchmark and generate Markdown + HTML throughput report
	$(UV) run python tools/process_benchmark_results.py \
		--label "GCC -O3 -flto" \
		--markdown $(BENCH_RESULTS_DIR)/smoke-lto.md \
		--html $(BENCH_RESULTS_DIR)/smoke-lto.html \
		--vegalite $(BENCH_RESULTS_DIR)/smoke-lto.vl.json \
		$(BENCH_RESULTS_DIR)/smoke-lto.xml

.PHONY: bench-lto-xml
bench-lto-xml: ## Configure (if needed), build GCC LTO benchmarks, save XML results
	@if ! [ -f build/gcc-release-lto/CMakeCache.txt ]; then \
		echo "Configuring gcc-release-lto preset..."; \
		$(CMAKE) --preset gcc-release-lto; \
	fi
	$(CMAKE) --build build/gcc-release-lto --target beman.transcode.benchmarks.smoke
	@mkdir -p $(BENCH_RESULTS_DIR)
	build/gcc-release-lto/benchmark/beman.transcode.benchmarks.smoke "[smoke]" \
		--reporter xml --out $(BENCH_RESULTS_DIR)/smoke-lto.xml

_PERF_BUILD = .build/build-perf
_PERF_BENCH = $(_PERF_BUILD)/benchmark/Perf

$(_PERF_BUILD)/CMakeCache.txt: | .gitmodules $(VENV)
	mkdir -p $(_PERF_BUILD)
	cd $(_PERF_BUILD) && $(CMAKE) \
		-G "Ninja Multi-Config" \
		-DCMAKE_CONFIGURATION_TYPES=$(_configuration_types) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-DCMAKE_PREFIX_PATH=$(CURDIR)/infra/cmake \
		-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=$(_cmake_top_level) \
		-DCMAKE_C_COMPILER_LAUNCHER=ccache \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_TOOLCHAIN_FILE=$(CURDIR)/etc/perf-toolchain.cmake \
		$(CURDIR)

.PHONY: bench-perf
bench-perf: $(_PERF_BUILD)/CMakeCache.txt ## Build GCC-16 Perf config and run smoke benchmark
	$(CMAKE) --build $(_PERF_BUILD) --config Perf --target beman.transcode.benchmarks.smoke -- -k 0
	$(_PERF_BENCH)/beman.transcode.benchmarks.smoke "[smoke]"

.PHONY: bench-perf-all
bench-perf-all: $(_PERF_BUILD)/CMakeCache.txt ## Build GCC-16 Perf config and run all core benchmarks
	$(CMAKE) --build $(_PERF_BUILD) --config Perf -- -k 0
	$(_PERF_BENCH)/beman.transcode.benchmarks.smoke "[smoke]"
	$(_PERF_BENCH)/beman.transcode.benchmarks.utf "[benchmark][utf]"
	$(_PERF_BENCH)/beman.transcode.benchmarks.whatwg "[benchmark][whatwg]"
	$(_PERF_BENCH)/beman.transcode.benchmarks.pluggable "[benchmark][pluggable]"
	$(_PERF_BENCH)/beman.transcode.benchmarks.iconv "[benchmark][iconv]"
	$(_PERF_BENCH)/beman.transcode.benchmarks.boundary "[benchmark][boundary]"

.PHONY: bench-perf-report
bench-perf-report: $(_PERF_BUILD)/CMakeCache.txt ## Build GCC-16 Perf, run core benchmarks, generate Markdown, HTML and SVG report
	$(CMAKE) --build $(_PERF_BUILD) --config Perf -- -k 0
	@mkdir -p $(BENCH_RESULTS_DIR)
	$(_PERF_BENCH)/beman.transcode.benchmarks.smoke "[smoke]" \
		--reporter xml --out $(BENCH_RESULTS_DIR)/smoke-Perf.xml
	$(_PERF_BENCH)/beman.transcode.benchmarks.utf "[benchmark][utf]" \
		--reporter xml --out $(BENCH_RESULTS_DIR)/utf-Perf.xml
	$(_PERF_BENCH)/beman.transcode.benchmarks.whatwg "[benchmark][whatwg]" \
		--reporter xml --out $(BENCH_RESULTS_DIR)/whatwg-Perf.xml
	$(_PERF_BENCH)/beman.transcode.benchmarks.pluggable "[benchmark][pluggable]" \
		--reporter xml --out $(BENCH_RESULTS_DIR)/pluggable-Perf.xml
	$(_PERF_BENCH)/beman.transcode.benchmarks.iconv "[benchmark][iconv]" \
		--reporter xml --out $(BENCH_RESULTS_DIR)/iconv-Perf.xml
	$(_PERF_BENCH)/beman.transcode.benchmarks.boundary "[benchmark][boundary]" \
		--reporter xml --out $(BENCH_RESULTS_DIR)/boundary-Perf.xml
	$(UV) run python tools/process_benchmark_results.py \
		--label "GCC-16 Perf (native)" \
		--markdown $(BENCH_RESULTS_DIR)/perf.md \
		--html $(BENCH_RESULTS_DIR)/perf.html \
		--svg-dir $(BENCH_RESULTS_DIR)/perf-svg \
		--vegalite $(BENCH_RESULTS_DIR)/perf.vl.json \
		$(BENCH_RESULTS_DIR)/smoke-Perf.xml \
		$(BENCH_RESULTS_DIR)/utf-Perf.xml \
		$(BENCH_RESULTS_DIR)/whatwg-Perf.xml \
		$(BENCH_RESULTS_DIR)/pluggable-Perf.xml \
		$(BENCH_RESULTS_DIR)/iconv-Perf.xml \
		$(BENCH_RESULTS_DIR)/boundary-Perf.xml

# Run the core suites BENCH_REPEATS times into one directory, as SUITE.N.xml
# with SUITE.N.meta.json.  Each repeat runs every suite before the next repeat
# starts, so the libraries in one repeat see much the same machine state.  The
# report combines the repeats of each benchmark; see docs/benchmarks/README.md.
BENCH_REPEATS ?= 5
BENCH_REPEAT_SUITES ?= smoke utf whatwg pluggable iconv boundary
BENCH_REPEAT_DIR ?= $(BENCH_RESULTS_DIR)/repeats

.PHONY: bench-repeat
bench-repeat: $(_PERF_BUILD)/CMakeCache.txt ## Build GCC-16 Perf, run the core suites BENCH_REPEATS times, report on the repeats
	@if ls $(BENCH_REPEAT_DIR)/*.xml >/dev/null 2>&1; then \
		echo "$(BENCH_REPEAT_DIR) already holds results; move them or set BENCH_REPEAT_DIR"; \
		exit 2; \
	fi
	$(CMAKE) --build $(_PERF_BUILD) --config Perf -- -k 0
	@mkdir -p $(BENCH_REPEAT_DIR)
	@for i in $$(seq 1 $(BENCH_REPEATS)); do \
		for s in $(BENCH_REPEAT_SUITES); do \
			if [ "$$s" = smoke ]; then tag="[smoke]"; else tag="[benchmark][$$s]"; fi; \
			echo "repeat $$i of $(BENCH_REPEATS): $$s"; \
			$(_PERF_BENCH)/beman.transcode.benchmarks.$$s "$$tag" \
				--reporter xml --out $(BENCH_REPEAT_DIR)/$$s.$$i.xml >/dev/null \
				|| echo "warning: $$s exited with status $$?"; \
		done; \
	done
	$(UV) run python tools/process_benchmark_results.py \
		--label "GCC-16 Perf, $(BENCH_REPEATS) repeats" \
		--markdown $(BENCH_REPEAT_DIR)/report.md \
		--html $(BENCH_REPEAT_DIR)/report.html \
		--svg-dir $(BENCH_REPEAT_DIR)/svg \
		--vegalite $(BENCH_REPEAT_DIR)/report.vl.json \
		$(BENCH_REPEAT_DIR)

# Count instructions instead of timing: run each core suite once under
# callgrind with the fixture in count mode, which runs every benchmark body
# BENCH_COUNT_RUNS times with collection on and dumps the counts under the
# benchmark's name.  Catch2's own settings are cut to the minimum, since its
# timings under Valgrind mean nothing.  The report divides each count by the
# runs and the input size.  Pass this directory beside a timing run's to see
# instructions per byte next to the times.
BENCH_COUNT_RUNS ?= 3
BENCH_INSTRUCTION_SUITES ?= $(BENCH_REPEAT_SUITES)
BENCH_INSTRUCTION_DIR ?= $(BENCH_RESULTS_DIR)/instructions

# $(call bench_count_suites,BIN_DIR,OUT_DIR): count each of
# BENCH_INSTRUCTION_SUITES from the binaries in BIN_DIR into OUT_DIR.
define bench_count_suites
for s in $(BENCH_INSTRUCTION_SUITES); do \
	if [ "$$s" = smoke ]; then tag="[smoke]"; else tag="[benchmark][$$s]"; fi; \
	echo "counting: $$s"; \
	BEMAN_TRANSCODE_BENCH_COUNT=$(BENCH_COUNT_RUNS) \
		valgrind --tool=callgrind --collect-atstart=no \
		--callgrind-out-file=$(2)/$$s.callgrind.%p \
		$(1)/beman.transcode.benchmarks.$$s "$$tag" \
		--benchmark-samples 1 --benchmark-resamples 1 --benchmark-warmup-time 0 \
		--reporter xml --out $(2)/$$s.xml >/dev/null 2>&1 \
		|| echo "warning: $$s exited with status $$?"; \
done
endef

.PHONY: bench-instructions
bench-instructions: $(_PERF_BUILD)/CMakeCache.txt ## Build GCC-16 Perf, count instructions of the core suites under callgrind, report
	@command -v valgrind >/dev/null || { echo "bench-instructions needs valgrind"; exit 2; }
	@if ls $(BENCH_INSTRUCTION_DIR)/*.xml >/dev/null 2>&1; then \
		echo "$(BENCH_INSTRUCTION_DIR) already holds results; move them or set BENCH_INSTRUCTION_DIR"; \
		exit 2; \
	fi
	$(CMAKE) --build $(_PERF_BUILD) --config Perf -- -k 0
	@mkdir -p $(BENCH_INSTRUCTION_DIR)
	@$(call bench_count_suites,$(_PERF_BENCH),$(BENCH_INSTRUCTION_DIR))
	$(UV) run python tools/process_benchmark_results.py \
		--label "GCC-16 Perf, instruction counts" \
		--markdown $(BENCH_INSTRUCTION_DIR)/report.md \
		--html $(BENCH_INSTRUCTION_DIR)/report.html \
		--svg-dir $(BENCH_INSTRUCTION_DIR)/svg \
		--vegalite $(BENCH_INSTRUCTION_DIR)/report.vl.json \
		$(BENCH_INSTRUCTION_DIR)

# One job of the runner-stability workflow
# (.github/workflows/bench-runner-stability.yml): run the core suites once
# timed, into BENCH_JOB_DIR/timing, then once counted, into
# BENCH_JOB_DIR/instructions, from binaries already built in BENCH_BIN_DIR.
# It builds nothing and writes no report; tools/runner_stability.py reports on
# all the jobs' directories together.
BENCH_JOB_DIR ?= $(BENCH_RESULTS_DIR)/stability-job

.PHONY: bench-stability-job
bench-stability-job: ## Run the core suites timed, then counted, from prebuilt BENCH_BIN_DIR into BENCH_JOB_DIR
	@if [ -z "$(BENCH_BIN_DIR)" ]; then \
		echo "usage: make bench-stability-job BENCH_BIN_DIR=DIR [BENCH_JOB_DIR=DIR]"; \
		exit 2; \
	fi
	@command -v valgrind >/dev/null || { echo "bench-stability-job needs valgrind"; exit 2; }
	@if ls $(BENCH_JOB_DIR)/*/*.xml >/dev/null 2>&1; then \
		echo "$(BENCH_JOB_DIR) already holds results; move them or set BENCH_JOB_DIR"; \
		exit 2; \
	fi
	@mkdir -p $(BENCH_JOB_DIR)/timing $(BENCH_JOB_DIR)/instructions
	@for s in $(BENCH_REPEAT_SUITES); do \
		if [ "$$s" = smoke ]; then tag="[smoke]"; else tag="[benchmark][$$s]"; fi; \
		echo "timing: $$s"; \
		$(BENCH_BIN_DIR)/beman.transcode.benchmarks.$$s "$$tag" \
			--reporter xml --out $(BENCH_JOB_DIR)/timing/$$s.xml >/dev/null \
			|| echo "warning: $$s exited with status $$?"; \
	done
	@$(call bench_count_suites,$(BENCH_BIN_DIR),$(BENCH_JOB_DIR)/instructions)

# Compare two result directories, each holding the XML and .meta.json files of
# one run, e.g. a copy of $(BENCH_RESULTS_DIR) taken before a change.  An empty
# label is filled in from what tells the runs apart: compiler, commit, host....
BENCH_BASELINE_LABEL ?=
BENCH_CANDIDATE_LABEL ?=

.PHONY: bench-compare
bench-compare: ## Compare runs: make bench-compare BENCH_BASELINE=DIR BENCH_CANDIDATE=DIR
	@if [ -z "$(BENCH_BASELINE)" ] || [ -z "$(BENCH_CANDIDATE)" ]; then \
		echo "usage: make bench-compare BENCH_BASELINE=DIR BENCH_CANDIDATE=DIR"; \
		exit 2; \
	fi
	$(UV) run python tools/process_benchmark_results.py \
		--run "$(BENCH_BASELINE_LABEL)" $(BENCH_BASELINE) \
		--run "$(BENCH_CANDIDATE_LABEL)" $(BENCH_CANDIDATE) \
		--markdown $(BENCH_RESULTS_DIR)/compare.md \
		--html $(BENCH_RESULTS_DIR)/compare.html \
		--vegalite $(BENCH_RESULTS_DIR)/compare.vl.json

# The documentation site.  Each generator writes under $(DOCS_BUILD_DIR);
# tools/build_site.py assembles them, the rendered Markdown of docs/ and
# papers/, and an index into $(SITE_DIR), which is what the Pages workflow
# publishes (.github/workflows/docs-pages.yml).  docs/Doxyfile and
# docs/mrdocs.yml name the same directories; change them together.
DOCS_BUILD_DIR ?= .build/docs
DOXYGEN_HTML_DIR := $(DOCS_BUILD_DIR)/doxygen/html
MRDOCS_HTML_DIR := $(DOCS_BUILD_DIR)/mrdocs
PAPER_DIR := papers/generated
SITE_DIR ?= $(DOCS_BUILD_DIR)/site
SITE_REVISION ?= $(shell git rev-parse --short HEAD 2>/dev/null)

.PHONY: docs
docs: ## Build the API reference with Doxygen
	mkdir -p $(DOCS_BUILD_DIR)/doxygen
	doxygen docs/Doxyfile

.PHONY: clean-docs
clean-docs: ## Delete generated Doxygen output
	-rm -rf $(DOCS_BUILD_DIR)/doxygen docs/html docs/latex

clean: clean-docs

.PHONY: mrdocs
mrdocs: ## Build the API reference with MrDocs (needs mrdocs on PATH)
	-rm -rf $(MRDOCS_HTML_DIR)
	cd docs && $(UV) run mrdocs --config=mrdocs.yml

.PHONY: clean-mrdocs
clean-mrdocs: ## Delete generated MrDocs output
	-rm -rf $(MRDOCS_HTML_DIR) $(DOCS_BUILD_DIR)/mrdocs-cmake docs/adoc

clean: clean-mrdocs

.PHONY: paper
paper: ## Build the paper, HTML and PDF, with the mpark/wg21 framework
	$(MAKE) -C papers transcode-view.html transcode-view.pdf

.PHONY: clean-paper
clean-paper: ## Delete the generated paper
	-rm -rf $(PAPER_DIR)

clean: clean-paper

.PHONY: site
site: docs mrdocs paper ## Build the whole documentation site into $(SITE_DIR)
	$(MAKE) site-pages

# The pages alone, with whichever generated trees are already built.  What
# `make site` assembles is what the workflow publishes; this is for working on
# the Markdown or the landing page without Doxygen, MrDocs or TeX installed.
.PHONY: site-pages
site-pages: venv ## Render the Markdown pages and the index from what is built
	$(UV) run python tools/build_site.py --out $(SITE_DIR) \
		--revision "$(SITE_REVISION)" \
		$(if $(wildcard $(DOXYGEN_HTML_DIR)/index.html),--doxygen $(DOXYGEN_HTML_DIR)) \
		$(if $(wildcard $(MRDOCS_HTML_DIR)/index.html),--mrdocs $(MRDOCS_HTML_DIR)) \
		$(if $(wildcard $(PAPER_DIR)/transcode-view.pdf),--paper $(PAPER_DIR))

.PHONY: clean-site
clean-site: ## Delete the assembled site
	-rm -rf $(SITE_DIR)

clean: clean-site

.PHONY: view-site
view-site: ## Open the assembled site in a browser
	sensible-browser $(SITE_DIR)/index.html

.PHONY: testinstall
testinstall: install
testinstall: CONFIG=RelWithDebInfo
testinstall: ## Test the installed package
	-$(RM) -rf installtest/.build
	$(CMAKE) -S installtest -B installtest/.build -G "Ninja Multi-Config" \
		-DCMAKE_PREFIX_PATH=$(abspath $(INSTALL_PREFIX)) \
		-DCMAKE_TOOLCHAIN_FILE=$(_local_toolchain)
	$(CMAKE) --build  installtest/.build --target test --config="RelWithDebInfo"

.PHONY: clean-testinstall
clean-testinstall:
	-rm -rf installtest/.build

clean: clean-testinstall

realclean: clean-testinstall

ifeq ($(UV),)
define install_uv_cmd
pipx install uv
endef

define uv_error_message

'uv' command not found.
Please install uv or set the UV variable to the path of the uv binary.
The makefile target "install-uv" will run ``$(install_uv_cmd)''
endef

$(warn "$(uv_error_message)")
endif

.PHONY: install-uv
install-uv: ## install uv via `pipx install uv`
	$(install_uv_cmd)

ORGFILES := $(wildcard *.org)

%.html : %.org
	$(EMACS) --init-directory=.emacs.d/ \
	--batch --load .emacs.d/init.el  \
	-f package-initialize \
	--eval "(setq enable-local-variables :all)" \
	--visit $< \
	--eval "(org-transclusion-mode t)" \
	--eval "(org-export-to-file 'html \"$@\")"
	echo $@ : \\ > $@.deps
	echo "  $<" \\ >> $@.deps
	sed -n "s/^.*\[\[file:\(\S*\)::.*$$/\1/p" < $<  | sort -u | xargs printf "  %s \\\\\\n" >> $@.deps

-include $(ORGFILES:%.org=%.html.deps)

%-slides.html : %.org
	$(EMACS) --init-directory=.emacs.d/ \
	--batch --load .emacs.d/init.el  \
	-f package-initialize \
	--eval "(setq enable-local-variables :all)" \
	--visit $< \
	--eval "(org-transclusion-mode t)" \
	--eval "(org-export-to-file 're-reveal \"$@\")"
	echo $@ : \\ > $@.deps
	echo "  $<" \\ >> $@.deps
	sed -n "s/^.*\[\[file:\(\S*\)::.*$$/\1/p" < $<  | sort -u | xargs printf "  %s \\\\\\n" >> $@.deps

-include $(ORGFILES:%.org=%-slides.html.deps)


.PHONY: clean-emacs.d
clean-emacs.d:
	-rm -rf .emacs.d/eln-cache
	-rm -rf .emacs.d/elpa*

realclean: clean-emacs.d

.PHONY: clean-org-deps
clean-org-deps:
	-rm -f $(ORGFILES:%.org=%.org.deps)
clean: clean-org-deps

.PHONY: clean-org-html
clean-org-html:
	-rm -f $(ORGFILES:%.org=%.html) $(ORGFILES:%.org=%-slides.html)
clean: clean-org-html

.PHONY: clean-python-artifacts
clean-python-artifacts: ## Delete Python test and lint caches
	-rm -rf .cache .mypy_cache .pytest_cache .ruff_cache .tox .nox htmlcov cover
	-find tools -type d -name __pycache__ -prune -exec rm -rf {} +

clean: clean-python-artifacts

.PHONY: clean-submodules
clean-submodules: ## Delete the submodule update stamp
	-rm -f .update-submodules

realclean: clean-submodules

.PHONY: presentation
presentation: test
presentation: $(ORGFILES:%.org=%.html)
presentation: $(ORGFILES:%.org=%-slides.html)

.PHONY: elpa
elpa:
	$(EMACS) --init-directory=.emacs.d/ --batch --load .emacs.d/init.el

.PHONY: refresh
refresh:
	$(EMACS) --init-directory=.emacs.d/ --batch --load .emacs.d/init.el -f package-upgrade-all

# Help target
.PHONY: help
help: ## Show this help.
	@awk 'BEGIN {FS = ":.*?## "} /^[.a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'  $(MAKEFILE_LIST) | sort
