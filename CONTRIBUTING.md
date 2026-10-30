<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

# Development

## Configure and Build the Project Using CMake Presets

The simplest way of configuring and building the project is to use [CMake
Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html). Appropriate
presets for major compilers have been included by default.  You can use `cmake
--list-presets=workflow` to see all available presets.

Here is an example of invoking the `gcc-debug` preset:

```shell
cmake --workflow --preset gcc-debug
```

Generally, there are two kinds of presets, `debug` and `release`.

The `debug` presets are designed to aid development, so they have debuginfo and sanitizers
enabled.

> [!NOTE]
>
> The sanitizers that are enabled vary from compiler to compiler.  See the toolchain files
> under ([`infra/cmake`](infra/cmake/)) to determine the exact configuration used for each
> preset.

The `release` presets are designed for production use, and
consequently have the highest optimization turned on (e.g. `O3`).

## Configure and Build Manually

If the presets are not suitable for your use case, a traditional CMake invocation will
provide more configurability.

To configure, build and test the project manually, you can run this set of commands. Note
that this requires Catch2 3 to be installed.

```bash
cmake \
  -B build \
  -S . \
  -DCMAKE_CXX_STANDARD=23 \
  # Your extra arguments here.
cmake --build build
ctest --test-dir build
```

> [!IMPORTANT]
>
> Beman projects are [passive projects](
> https://github.com/bemanproject/beman/blob/main/docs/beman_standard.md#cmakepassive_projects),
> so you need to specify the C++ version via `CMAKE_CXX_STANDARD` when manually
> configuring the project.

## Dependency Management

### vcpkg

The best way to install the project's dependencies is to use the vcpkg workflow.

To do so, make sure vcpkg is installed and `VCPKG_ROOT` is defined in your environment,
then specify
`-DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"`. Vcpkg will handle
the project's dependencies, including Catch2.

Example commands:

```shell
cmake \
  -B build \
  -S . \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build
ctest --test-dir build
```

The file `./vcpkg.json` configures the list of dependencies that will be configured by
vcpkg.

### FetchContent

Instead of installing the project's dependencies via a package manager, you can optionally
configure beman.transcode to fetch them automatically via CMake FetchContent.

To do so, specify
`-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake`. This will
bring in Catch2 automatically along with any other dependency the project may require.

Example commands:

```shell
cmake \
  -B build \
  -S . \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./infra/cmake/use-fetch-content.cmake
cmake --build build
ctest --test-dir build
```

The file `./lockfile.json` configures the list of dependencies and versions that will be
acquired by FetchContent.


## Project-specific configure arguments

Project-specific options are prefixed with `BEMAN_TRANSCODE`.
You can see the list of available options with:

```bash
cmake -LH -S . -B build | grep "BEMAN_TRANSCODE" -C 2
```

<details>

<summary>Some project-specific configure arguments</summary>

### `BEMAN_TRANSCODE_BUILD_TESTS`

Enable building tests and test infrastructure. Default: `ON`.
Values: `{ ON, OFF }`.

### `BEMAN_TRANSCODE_BUILD_EXAMPLES`

Enable building examples. Default: `ON`. Values: `{ ON, OFF }`.

### `BEMAN_TRANSCODE_INSTALL_CONFIG_FILE_PACKAGE`

Enable installing the CMake config file package. Default: `ON`.
Values: `{ ON, OFF }`.

This is required so that users of `beman.transcode` can use
`find_package(beman.transcode)` to locate the library.

</details>

## Building the Documentation

The documentation site at <https://steve-downey.github.io/transcode/> is built
by `.github/workflows/docs-pages.yml` on every push to `main` and assembled by
`tools/build_site.py`.  It holds the API reference as rendered by Doxygen and
by MrDocs, the paper, and every Markdown document under `docs/` and `papers/`,
all reachable from the index page.  To build it locally:

```shell
make docs        # Doxygen, into .build/docs/doxygen/html
make mrdocs      # MrDocs, into .build/docs/mrdocs
make paper       # the paper, into papers/generated/
make site        # all three, then the pages and the index, into .build/docs/site
make view-site   # open it
```

`make docs` needs `doxygen`; `make mrdocs` needs
[MrDocs](https://www.mrdocs.com/) on the `PATH` (the workflow pins the release
it installs); `make paper` needs a TeX installation with `xelatex` and
downloads its own pandoc.  `make site-pages` renders just the Markdown pages
and the index, with whichever of the three is already built, and needs only
the system `pandoc`.  The landing page's authored text is
`docs/site/index.md`; the generated sections below it list what was built.
