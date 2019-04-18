(module
  (import "wasi_unstable" "fd_write" (func $fd_write (param i32 i32 i32 i32) (result i32)))
  (import "wasi_unstable" "fd_read" (func $fd_read (param i32 i32 i32 i32) (result i32)))

  (memory 10)
  (export "memory" (memory 0))

  ;; prompt __wasi_ciovec_t struct
  (data (i32.const 0) "\08\00\00\00") ;; buf: pointer to prompt string
  (data (i32.const 4) "\02\00\00\00") ;; buf_len: 2 characters
  ;; string
  (data (i32.const 8) "> ")

  ;; read buf __wasi_ciovec_t struct
  (data (i32.const 16) "\18\00\00\00") ;; buf: pointer to string
  (data (i32.const 20) "\64\00\00\00") ;; buf_len: 100 characters max
  ;; string
  (data (i32.const 24) "\00") ;; buf (of 100 characters) to hold read in string

  (func $main
    (local $ret i32)
    (block $done
      (loop $loop
        (drop (call $fd_write (i32.const 1) (i32.const 0) (i32.const 1) (i32.const 256)))
        (local.set $ret (call $fd_read (i32.const 0) (i32.const 16) (i32.const 1) (i32.const 256)))
        (if (i32.eqz (i32.load (i32.const 256)) (i32.const 0)) (br $done))
        (drop (call $fd_write (i32.const 0) (i32.const 16) (i32.const 1) (i32.const 256)))
        (br $loop)
      )
    )
  )

  (export "_start" $main)
)
