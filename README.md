TODO

Full runnable examples can be found in [`examples/`](examples/).

## Dependencies

### Build Environment

This project requires at least the following to build:

* A C++ compiler that conforms to the C++20 standard or greater
* CMake 3.30 or later
* (Test Only) GoogleTest

You can disable building tests by setting CMake option `BEMAN_TRANSCODE_BUILD_TESTS` to
`OFF` when configuring the project.

### Supported Platforms

| Compiler   | Version | C++ Standards | Standard Library  |
|------------|---------|---------------|-------------------|
| GCC        | 16-13   | C++26-C++17   | libstdc++         |
| GCC        | 12-11   | C++23-C++17   | libstdc++         |
| Clang      | 22-19   | C++26-C++17   | libstdc++, libc++ |
| Clang      | 18      | C++26-C++17   | libc++            |
| Clang      | 18      | C++23-C++17   | libstdc++         |
| Clang      | 17      | C++26-C++17   | libc++            |
| Clang      | 17      | C++20, C++17  | libstdc++         |
| AppleClang | latest  | C++26-C++17   | libc++            |
| MSVC       | latest  | C++23         | MSVC STL          |

## Development

See the [Contributing Guidelines](CONTRIBUTING.md).

## Integrate beman.transcode into your project

### Build

You can build transcode using a CMake workflow preset:

```bash
cmake --workflow --preset gcc-release
```

To list available workflow presets, you can invoke:

```bash
cmake --list-presets=workflow
```

For details on building beman.transcode without using a CMake preset, refer to the
[Contributing Guidelines](CONTRIBUTING.md).

### Installation

#### Vcpkg

The preferred way to install transcode is via vcpkg. To do so, after installing vcpkg
itself, you need to add support for the Beman project's [vcpkg
registry](https://github.com/bemanproject/vcpkg-registry) by configuring a
`vcpkg-configuration.json` file (which transcode [provides](vcpkg-configuration.json)).

Then, simply run `vcpkg install beman-transcode`.

#### Manual

To install beman.transcode globally after building with the `gcc-release` preset, you can
run:

```bash
sudo cmake --install build/gcc-release
```

Alternatively, to install to a prefix, for example `/opt/beman`, you can run:

```bash
sudo cmake --install build/gcc-release --prefix /opt/beman
```

This will generate the following directory structure:

```txt
/opt/beman
├── include
│   └── beman
│       └── transcode
│           ├── transcode.hpp
│           └── ...
└── lib
    └── cmake
        └── beman.transcode
            ├── beman.transcode-config-version.cmake
            ├── beman.transcode-config.cmake
            └── beman.transcode-targets.cmake
```

### CMake Configuration

If you installed beman.transcode to a prefix, you can specify that prefix to your CMake
project using `CMAKE_PREFIX_PATH`; for example, `-DCMAKE_PREFIX_PATH=/opt/beman`.

You need to bring in the `beman.transcode` package to define the `beman::transcode` CMake
target:

```cmake
find_package(beman.transcode REQUIRED)
```

You will then need to add `beman::transcode` to the link libraries of any libraries or
executables that include `beman.transcode` headers.

```cmake
target_link_libraries(yourlib PUBLIC beman::transcode)
```

### Using beman.transcode

To use `beman.transcode` in your C++ project,
include an appropriate `beman.transcode` header from your source code.

```c++
#include <beman/transcode/transcode.hpp>
```

> [!NOTE]
>
> `beman.transcode` headers are to be included with the `beman/transcode/` prefix.
> Altering include search paths to spell the include target another way (e.g.
> `#include <transcode.hpp>`) is unsupported.
