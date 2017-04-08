# wac - WebAssembly in C

A Minimal WebAssembly interpreter written in C.

## Usage

Currently supports the WebAssembly MVP (minimum viable product)
version of the spec.

```
wast2wasm foo.wast -o foo.wasm
make wac
./wac foo.wasm myfunc 10
```

## License

MPL-2.0
