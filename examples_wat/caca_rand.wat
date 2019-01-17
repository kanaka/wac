(module
  (import "libcaca.so.0" "caca_rand"
    (func $caca_rand (param i32 i32) (result i32)))
  (func $doRand (param i32 i32) (result i32)
    (call $caca_rand
      (local.get 0) (local.get 1)))
  (export "rand" (func $doRand)))
