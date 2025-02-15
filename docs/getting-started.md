# Getting Started

:::{important}
'abstractions' is not supported on Windows.
:::

## Environment Setup

Compiling abstractions requires:

* [Conan](https://conan.io/)
* [CMake](https://cmake.org/) 3.23 or higher
* [Clang](https://clang.llvm.org/) 19
* Python 3.12 or higher

The provided [conda](https://docs.conda.io/en/latest/) environment will create
an `abstractions` environment with the necessary dependencies.  Clang is the
exception and requires a separate installation step.

Building the documentation also requires:

* [Doxygen](https://www.doxygen.nl/)

The easiest way to create the build environment is with [conda](https://docs.conda.io/en/latest/):

```shell
conda env create
conda activate abstractions
```

This will install the correct versions of CMake and Doxygen into the
`abstractions` environment.

It's also possible to install the Python dependencies directly with:

```shell
python -m venv .venv
source .venn/bin/activate
pip install -r requirements.txt
```

This is the method the GitHub workflow users.

Note that this requires ensuring CMake and, optionally, Doxygen are installed
using an alternate method.

### Getting Clang 19 on Ubuntu

The easiest way to get Clang 19 on an Ubuntu or Debian system is with setup
script provided by the LLVM project.  Adapted from https://apt.llvm.org/,

```shell
wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
sudo ./llvm.sh 19

# The line below is only needed if the default GCC version is less than 12
# (e.g., on Ubuntu 22.04).  Clang uses the libstdc++ headers GCC provides and
# versions older than 12 don't have the necessary C++23 headers.
sudo apt install g++-12
```

### Getting Clang 19 on macOS

The version of Clang that comes with Xcode, or the Xcode Command Line Tools, is
not supported.  Instead, use [Homebrew](https://brew.sh/) to get the correct
version:

```shell
brew install llvm@19
```

The [macOS profile](https://github.com/richengguy/abstractions/blob/main/profiles/macos-arm64)
is configured to the Homebrew version.

## Compiling

Creating a release build is done with

```shell
conan install . -pr:a profiles/$PROFILE --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
```

where `PROFILE` is `linux-x86_64` or `macos-arm64`.

Creating a debug build is done with

```shell
conan install . -pr:a profiles/$PROFILE --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
```

The unit and feature tests are compiled by default.  They can be disabled by
adding `-o "&:build_tests=False"`.

### CMake Variables

| Option | Description | Default |
|--------|-------------|---------|
| `ABSTRACTIONS_BUILD_DOCS` | Enables the `make docs` target that builds the documentation. See <project:#documentation> for how this works. |
| `ABSTRACTIONS_BUILD_TESTS` | Build the unit and feature tests. | `OFF` |
| `ABSTRACTIONS_ASSERTS` | Enable the internal asserts system. | `ON` |
| `ABSTRACTIONS_ENABLE_ASAN` | Enable the Clang AddressSanitizer to catch memory leaks and other issues.  Off by default as it has a performance impact. | `OFF`|
| `ABSTRACTIONS_ENABLE_PROFILING` | Enables linking with gperftools to enable source-level profiling.  This adds a `--profile` option to some of the subcommands. | `OFF` |

The variables above enable/disable certain compile-time features.  For example,
to enable profiling, run

```shell
cd $BUILD_DIR  # $BUILD_DIR will be something like 'build/Release'
cmake -DABSTRACTIONS_ENABLE_PROFILING=ON .
make -j
```

## `abstractions`

The `abstractions` binary can be found in `$BUILD_DIR/bin`.  The dependencies
are statically linked so the file can be moved to other locations on the same
system.  Help information can be found with

```shell
abstractions --help
```

See <project:abstracting-images.md> for details on how to generate an image abstraction.

## Tests

The test binaries are compiled to the `$BUILD_DIR/tests/` directory.  The
`library-tests` binary runs all of the library unit tests.  The remaining
binaries contain self-contained feature tests.

| Feature Test | Description |
|--------------|-------------|
| `assert-test` | Verifies that the assertion framework is working correctly. |
| `canvas-test` | Runs through a set of rendering operations that are provided by an internal canvas API. |
| `optimizer-test` | Tests the PGPE optimizer by attempting to optimize the [Rastrigin function](https://en.wikipedia.org/wiki/Rastrigin_function).  It adapts a test from the [Python pgpelib library](https://github.com/nnaisense/pgpelib/blob/release/examples/01-rastrigin.ipynb). |
| `renderer-test` | Runs through a set of rendering operations. |
| `threads-test` | Runs the internal thread pool API through some simple tasks. |

(documentation)=
## Documentation

The documentation isn't built by default.  This requires installing Doxygen and
a separate set of Python dependencies.  Please note that the conda environment
ensures Doxygen is available.  The minimal steps for building the docs are

```shell
conan install . -pr:a $PROFILE --build=missing -o "&:build_docs=True"
cmake --preset conan-release
cd build/Release
make docs
```
