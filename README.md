# wac - WebAssembly in C

A Minimal WebAssembly interpreter written in C. Supports the
WebAssembly MVP (minimum viable product) version of the WebAssembly
specification.

There are two different builds of wac:

* **wac**: (WebAssembly in C) Minimal client with an interactive REPL
  mode. Designed to run standalone wasm files compiled with wast2wasm.
  Passes most spec tests apart from some multi-module import/export
  tests.
* **wace**: (WebAssembly in C and Emscripten) Client with host
  library/memory integration. Designed to run wasm code that has been
  built using Emscripten.

## Prerequisites

To build wac and wace you need a 32-bit version of gcc and 32-bit
versions of SDL2 and libedit. On 64-bit Ubuntu/Debian these can be
installed like this:

```
dpkg --add-architecture i386
apt-get update
apt-get install lib32gcc-4.9-dev libSDL2-dev:i386 libedit-dev:i386
```

To compile wast source files to binary wasm modules you will need the
wast2wasm tool from [wabt](https://github.com/WebAssembly/wabt). To
compile C programs to wasm modules you will need
[emscripten](https://github.com/kripken/emscripten) with
[binaryen](https://github.com/WebAssembly/binaryen). In addition
emscripten must be patched using `library.js.patch` to generate wace
compatible wasm files.

As an alternative to downloading and building the above tools, the
`kanaka/webassembly` docker image (1.5GB) has 32-bit gcc
compiler/libraries, emscripten, binaryen, and wabt (wast2wasm)
preinstalled. The docker image can be started with appropriate file
mounts like this:

```
docker run -v `pwd`:/wac -w /wac -it kanaka/webassembly bash
```

The build commands below can be run within the docker container.


## wac usage

Build wac:

```bash
make wac
```

Use `wast2wasm` to compile a simple wast program to a wasm:

```bash
wast2wasm test/arith.wast -o test/arith.wasm
```

Now load the compiled wasm file and invoke some functions:

```bash
./wac test/arith.wasm add 2 3
./wac test/arith.wasm mul 7 8
```

wac also supports a very simple REPL (read-eval-print-loop) mode that
runs commands in the form of `FUNC ARG...`:

```
./wac --repl test/arith.wasm
> sub 10 5
> div 13 4
```

## wace usage

Build wace:

```bash
make wace
```

Use emscripten/binaryen to compile some simple C programs and run them
using wace:

```bash
make test/hello1.wasm
./wace test/hello1.wasm

make test/hello2.wasm
./wace test/hello2.wasm

# this fails: Could not create GLES window surface
make test/hello_sdl.wasm
./wace test/hello_sdl.wasm
```

## Running WebAssembly spec tests

wac includes a `runtest.py` test driver which can be used for running
tests from the WebAssembly specification.

Check out the spec:

```
git clone https://github.com/WebAssembly/spec
```

Run the function test file from the spec:

```
./runtest.py --wast2wasm /path/to/wast2wasm --interpreter ./wac spec/test/core/block.wast
```


## License

MPL-2.0
