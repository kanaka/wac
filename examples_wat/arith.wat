(module
  (func $add (param i32 i32) (result i32)
    (i32.add
      (local.get 0)
      (local.get 1)))
  (func $sub (param i32 i32) (result i32)
    (i32.sub
      (local.get 0)
      (local.get 1)))
  (func $mul (param i32 i32) (result i32)
    (i32.mul
      (local.get 0)
      (local.get 1)))
  (func $div (param i32 i32) (result i32)
    (i32.div_u
      (local.get 0)
      (local.get 1)))
  (export "add" (func $add))
  (export "sub" (func $sub))
  (export "mul" (func $mul))
  (export "div" (func $div)))
