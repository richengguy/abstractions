# abstractions
Generating abstract images with policy gradients.  This is a C++ implementation
of https://es-clip.github.io/ with some minor upgrades.

| Original | Abstraction |
|----------|-------------|
| ![original](docs/examples/original/yonge-dundas.jpg) | ![abstraction](docs/examples/yd-large.png) |

## Building

Building abstractions requires

* CMake 3.23 or higher
* Clang 19

The conda environment can get a supported version of CMake while the steps for
installing Clang are below.

### Getting Clang 19 (Ubuntu)

The easiest way to get Clang (and LLVM) 19 is with the setup script from the
LLVM project itself.  Adapted from https://apt.llvm.org/,

```shell
wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
sudo ./llvm.sh 19

# The line below is only needed if the default GCC version is less than 12
# (e.g., on Ubuntu 22.04).  Clang uses the libstdc++ headers GCC provides and
# versions older than 12 don't have the necessary C++23 headers.
sudo apt install g++-12
```

### Getting Clang 19 (macOS)

The easiest way to get Clang 19 is with [Homebrew](https://brew.sh/):

```bash
brew install llvm@19
```

The [macOS profile](./profiles/macos-arm64) is configured to use the
Homebrew-installed Clang instead of the system Clang.

### Environment Setup

'abstractions' uses [Conan](https://conan.io/) as the main software package
manager.  It needs to be installed before doing anything else.  There are two
ways to do this.

First, if you already have [conda](https://docs.conda.io/en/latest/) installed
then just run

```shell
conda env create
conda activate abstractions
```

to create the environment.

The other approach is just to use Python virtualenvs directly with

```shell
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### Running Conan and CMake

Run the commands below to pull all dependencies and build the project:

```shell
conan install . -pr:a profiles/$PROFILE --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
```

`$PROFILE` is one of the files in the `profiles` directory.  The file will
depend on the OS.

Add `-s build_type=Debug` to compile a debug build.  Tests are built by default
for both release and debug builds.  Add `-o "&:build_tests=False` to avoid
building tests.

There are some additional CMake variables that can be enabled/disabled once the
build folder is created:

| Option | Description | Default |
|--------|-------------|---------|
| `ABSTRACTIONS_BUILD_TESTS` | Build the unit and feature tests. | `OFF` |
| `ABSTRACTIONS_ASSERTS` | Enable the internal asserts system. | `ON` |
| `ABSTRACTIONS_ENABLE_ASAN` | Enable the Clang AddressSanitizer to catch memory leaks and other issues.  Off by default as it has a performance impact. | `OFF`|
| `ABSTRACTIONS_ENABLE_PROFILING` | Enables linking with gperftools to enable source-level profiling.  This adds a `--profile` option to some of the subcommands. | `OFF` |

## Licensing

All source code falls under the BSD-3 Clause license.  The documentation in the
[docs](docs/) falls under a [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) license.
