# wac - WebAssembly in C

A Minimal WebAssembly interpreter written in C. Supports the
WebAssembly MVP (minimum viable product) version of the WebAssembly
specification.

There are two different builds of wac:

* **wac**: (WebAssembly in C) Minimal client with an interactive REPL
  mode. Designed to run standalone wasm files compiled with
  `wast2wasm` or `wasm-as`. Passes most spec tests apart from some
  multi-module import/export tests.
* **wace**: (WebAssembly in C with Emscripten) Client with host
  library/memory integration. Designed to run wasm code that has been
  built with Emscripten (using `-s SIDE_MODULE=1 -s LEGALIZE_JS_FFI=0`).

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
wasm-as tool from [Binaryen](https://github.com/WebAssembly/binaryen).
To compile C programs to wasm modules you will need a [patched version
of emscripten](https://github.com/kanaka/emscripten), the [incoming
branch of fastcomp](https://github.com/kripken/emscripten-fastcomp)
and the [master branch of
binaryen](https://github.com/WebAssembly/binaryen).

As an alternative to downloading and building the above tools, the
`kanaka/emscripten` docker image (1.7GB) has 32-bit gcc
compiler/libraries, patched emscripten, and binaryen preinstalled. The
docker image can be started with appropriate file mounts like this:

```
docker run -v `pwd`:/wac -w /wac -it kanaka/emscripten bash
```

All the build commands below can be run within the docker container.


## wac usage

Build wac:

```bash
$ make wac
```

Use `wasm-as` to compile a simple wast program to a wasm:

```bash
$ make examples_wast/arith.wasm
```

Now load the compiled wasm file and invoke some functions:

```bash
$./wac examples_wast/arith.wasm add 2 3
0x5:i32
$./wac examples_wast/arith.wasm mul 7 8
0x38:i32
```

wac also supports a very simple REPL (read-eval-print-loop) mode that
runs commands in the form of `FUNC ARG...`:

```
$ ./wac --repl examples_wast/arith.wasm
> sub 10 5
0x5:i32
> div 13 4
0x3:i32
```

## wace usage

Build wace:

```bash
$ make wace
```

Use emscripten/binaryen to compile some simple C programs and run them
using wace:

```bash
$ make examples_c/hello1.wasm
$ ./wace examples_c/hello1.wasm
hello world

$ make examples_c/hello2.wasm
$ ./wace examples_c/hello2.wasm
hello malloc people
```

Use emscripten/binaryen to compile some C SDL programs and run them
using wace:

```bash
$ make examples_c/hello_sdl.wasm
$ ./wace examples_c/hello_sdl.wasm
INFO: OpenGL shaders: ENABLED
INFO: Created renderer: opengl
# Blue Window displayed for 2 seconds
Done.

$ make examples_c/triangle.wasm
$ ./wace examples_c/triangle.wasm
# A colorfully shaded triangle is rendered

$ make examples_c/hello_owl/hello_owl.wasm
$ ./wace examples_c/hello_owl/hello_owl.wasm
# An Owl image displayed for 2 seconds
```

## Running WebAssembly spec tests

wac includes a `runtest.py` test driver which can be used for running
tests from the WebAssembly specification.

Check out the spec:

```
git clone https://github.com/WebAssembly/spec
```

You will need `wast2wasm` to compile the spec tests. Check-out and
build [wabt](https://github.com/WebAssembly/wabt) (wabbit):

```
git clone --recursive https://github.com/WebAssembly/wabt
make -C wabt gcc-release
```

Run the `func.wast` test file (to test function calls) from the spec:

```
./runtest.py --wast2wasm ./wabt/out/gcc/Release/wast2wasm --interpreter ./wac spec/test/core/func.wast
```

Run all the spec tests apart from a few that currently fail (mostly due to
`runtest.py` missing support for some syntax used in those test files):

```
BROKE_TESTS="comments exports imports linking names data elem inline-module"
for t in $(ls spec/test/core/*.wast | grep -Fv "${BROKE_TESTS// /$'\n'}"); do
    echo -e "\nTESTING ${t}"
    ./runtest.py ${t} || break
done
```


## Standalone Builds using Fooboot

wac and wace can be built to run as standalone bootable programs
using [fooboot](https://github.com/kanaka/fooboot):

```
cd wac
git clone https://github.com/kanaka/fooboot
make PLATFORM=fooboot clean
make PLATFORM=fooboot wac wace examples_wast/addTwo.wasm
```

The `fooboot/runfoo` script can be used to boot wac/wace with QEMU.
`fooboot/runfoo` also creates a connection on a serial port (COM2)
that allows files to be read from the host system:

```
fooboot/runfoo wac --repl examples_wast/addTwo.wasm
QEMU waiting for connection on: disconnected:tcp:localhost:21118,server
webassembly> addTwo 2 3
0x5:i32
```

The standalone wac/wace builds can also be built into an ISO image
that can boot directly on real hardware. You will need Grub 2 and the
Grub PC/BIOS binary files (grub-pc-bin) and the xorriso program to be
able to do this. Also, the wasm modules that you wish to run must be
built into the binary to become part of a simple in-memory
file-system:

```
echo "examples_wast/addTwo.wasm" > mem_fs_files
make PLATFORM=fooboot \
     FOO_TARGETS="wac" \
     FOO_CMDLINE="examples_wast/addTwo.wasm addTwo 3 4" \
     boot.iso
```

You can now boot the ISO with QEMU like this:

```
qemu-system-i386 -cdrom boot.iso
```

Or you can burn the ISO to a USB device and boot from it on real
hardware.  This will destroy any data on the USB device! Also, make
completely sure that /dev/MY\_USB\_DEVICE is really the USB device you
want to overwrite and not your hard drive. You have been warned!

```
sudo dd if=boot.iso of=/dev/MY_USB_DEVICE && sync
# Now boot you can boot from the USB device
```

## License

MPL-2.0 (see LICENSE).
