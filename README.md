# wac - WebAssembly in C

A Minimal WebAssembly interpreter written in C. Supports the
WebAssembly MVP (minimum viable product) version of the WebAssembly
specification.

There are two different builds of wac:

* **wac**: (WebAssembly in C) Minimal client with an interactive REPL
  mode. Designed to run standalone wasm files compiled with wast2wasm.
  Passes most spec tests apart from some multi-module import/export
  tests.
* **wace**: (WebAssembly in C and Emscripten) Client that host
  library/memory integration. Designed to run wasm code that has been
  built using Emscripten.


## wac usage

```
make wac
wast2wasm foo.wast -o foo.wasm
./wac foo.wasm myfunc 10
./wac --repl foo.wasm
> myfunc 10
```

## wace usage

The `kanaka/webassembly` docker image has emscripten, 32-bit gcc
compiler/libraries and wabt (wast2wasm).

```
docker run -v `pwd`:/wac -w /wac -it kanaka/webassembly make wace

docker run -v `pwd`:/wac -w /wac -it kanaka/webassembly make test/hello1.wasm
./wace test/hello1.wasm

docker run -v `pwd`:/wac -w /wac -it kanaka/webassembly make test/hello2.wasm
./wace test/hello2.wasm

docker run -v `pwd`:/wac -w /wac -it kanaka/webassembly make test/hello_sdl.wasm
./wace test/hello_sdl.wasm  # does not work yet
```

## License

MPL-2.0
