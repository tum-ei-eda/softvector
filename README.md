# Softvector

This project targets functional simulation of Vector Processing Units (VPUs). It wraps highly abstracted vector arithmetic functions around a target-specific interface.

Softvector mainly focuses on the RISC-V Vector (RVV) instruction set and was built as a support library for the [Extendable Translating Instruction Set Simulator (ETISS)](https://github.com/tum-ei-eda/etiss).

The CMake project builds a shared library containing generic RISC-V helper/soft functions for the RVV instruction set, i.e. either:

- `soft`: A high-level implementation including unit test support for SoftVector verification.
- `ETISS`: Support functions that allow for more efficient and convenient vector computations to be used with the ETISS.

# Getting Started

## Requirements

- CMake >= v3.15
- Graphviz + Doxygen (for documentation generation)

## Build:

Create Build directory:

```
mkdir build
```

Change to the created directory:

```
cd build
```

Configure the build system. Replace `<target>` with either `ETISS` or `soft`. Statements in `[ .. ]` are optional:

```
cmake .. -DTARGET_NAME=<target> -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=`pwd`/installed [-DTEST_BUILD=1]
```

Build the project:

```
make
```

(Build the source file documentation):

```
make doc
```

## Test

If `-DTEST_BUILD=1` was specified and the target supports unit tests (currently only RISC-V `soft`), the test can be executed with

```
make test
```

This tests softvector against a set of 'golden' inputs for the specified target. For RVV `soft` these are pre-configured vector register files and sample memory vectors which are altered by a SoftVector RVV instruction. The output is compared against the expected states specified in the 'golden' file.


## Licensing and Copyright

See the separate LICENSE file to determine your rights and responsibilities for using SoftVector.

## User Documentation

The documentation is optionally built with SoftVector and can be found in `build/doc/html/index.html`.
