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

_configuration_types ?= "RelWithDebInfo;Debug;Tsan;Asan;Gcov"

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

.PHONY: lint
lint: venv mypy
lint: ## Run all configured tools in pre-commit and mypy
	$(PRE_COMMIT) run -a

.PHONY: lint-manual
lint-manual: venv
lint-manual: ## Run all manual tools in pre-commit
	$(PRE_COMMIT) run --hook-stage manual -a

.PHONY: coverage
coverage: ## Build and run the tests with the GCOV profile and process the results
coverage: venv $(_build_path)/CMakeCache.txt
	$(CMAKE) --build $(_build_path) --config Gcov
	$(ACTIVATE) ctest --build-config Gcov --output-on-failure --test-dir $(_build_path)
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
bench-report: compile ## Run smoke benchmark (RelWithDebInfo) and generate Markdown throughput report
	@mkdir -p $(BENCH_RESULTS_DIR)
	$(_build_path)/benchmark/$(CONFIG)/beman.transcode.benchmarks.smoke "[smoke]" \
		--reporter xml --out $(BENCH_RESULTS_DIR)/smoke-$(CONFIG).xml
	$(UV) run python tools/process_benchmark_results.py \
		--corpus-dir benchmark/corpus \
		--manifest data/benchmarks/corpus_manifest.json \
		--label "$(CONFIG)" \
		$(BENCH_RESULTS_DIR)/smoke-$(CONFIG).xml

.PHONY: bench-report-lto
bench-report-lto: bench-lto-xml ## Run GCC LTO smoke benchmark and generate Markdown throughput report
	$(UV) run python tools/process_benchmark_results.py \
		--corpus-dir benchmark/corpus \
		--manifest data/benchmarks/corpus_manifest.json \
		--label "GCC -O3 -flto" \
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

.PHONY: docs
docs: ## Build the docs with Doxygen
	doxygen docs/Doxyfile

.PHONY: clean-docs
clean-docs: ## Delete generated Doxygen output
	-rm -rf docs/html docs/latex

clean: clean-docs

.PHONY: mrdocs
mrdocs: ## Build the docs with MrDocs
	-rm -rf docs/adoc
	cd docs && NO_COLOR=1 mrdocs mrdocs.yml 2>&1 | sed 's/\x1b\[[0-9;]*m//g'
	find docs/adoc -name '*.adoc' | xargs asciidoctor

.PHONY: clean-mrdocs
clean-mrdocs: ## Delete generated MrDocs output
	-rm -rf docs/adoc

clean: clean-mrdocs

.PHONY: testinstall
testinstall: install
testinstall: CONFIG=RelWithDebInfo
testinstall: ## Test the installed package
	-$(RM) -rf installtest/.build
	$(CMAKE) -S installtest -B installtest/.build 	-G "Ninja Multi-Config"
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
