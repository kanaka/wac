#ifndef WAC_H
#define WAC_H

#include <stdint.h>

#define WA_MAGIC   0x6d736100
#define WA_VERSION 0x01

#define STACK_SIZE      0x10000  // 65536
#define BLOCKSTACK_SIZE 0x1000   // 4096
#define CALLSTACK_SIZE  0x1000   // 4096

#define I32       0x7f  // -0x01
#define I64       0x7e  // -0x02
#define F32       0x7d  // -0x03
#define F64       0x7c  // -0x04
#define ANYFUNC   0x70  // -0x10
#define FUNC      0x60  // -0x20
#define BLOCK     0x40  // -0x40

#define KIND_FUNCTION 0
#define KIND_TABLE    1
#define KIND_MEMORY   2
#define KIND_GLOBAL   3


typedef struct Type {
    uint8_t   form;
    uint32_t  param_count;
    uint32_t *params;
    uint32_t  result_count;
    uint32_t *results;
} Type;

typedef union FuncPtr {
    void   (*void_void)  ();
    void   (*void_i32)   (uint32_t);
    void   (*void_i64)   (uint64_t);
    void   (*void_f32)   (float);
    void   (*void_f64)   (double);

    double (*f64_f64)    (double);
} FuncPtr;

// A block or function
typedef struct Block {
    uint8_t    block_type;    // 0x00: function, 0x01: init_exp
                              // 0x02: block, 0x03: loop, 0x04: if
    uint32_t   fidx;          // function only (index)
    Type      *type;          // params/results type
    uint32_t   local_count;   // function only
    uint32_t  *locals;        // function only
    uint32_t   start_addr;
    uint32_t   end_addr;
    uint32_t   else_addr;     // if block only
    uint32_t   br_addr;       // blocks only
    char      *export_name;   // function only (exported)
    char      *import_module; // function only (imported)
    char      *import_field;  // function only (imported)
    void     (*func_ptr)();   // function only (imported)
} Block;

///

typedef struct StackValue {
    uint8_t    value_type;
    union {
        uint32_t   uint32;
        int32_t    int32;
        uint64_t   uint64;
        int64_t    int64;
        float      f32;
        double     f64;
    }          value;
} StackValue;

typedef struct Frame {
    Block      *block;
    // Saved state
    int         sp;   
    int         fp;
    uint32_t    ra;
} Frame;

///

typedef struct Module {
    char       *path;           // file path of the wasm module
    uint32_t    byte_count;     // number of bytes in the module
    uint8_t    *bytes;          // module content/bytes

    uint32_t    type_count;     // number of function types
    Type       *types;          // function types
    uint32_t    import_count;   // number of leading imports in functions
    uint32_t    function_count; // number of function (including imports)
    Block      *functions;      // imported and locally defined functions
    Block     **block_lookup;   // map of module byte position to Blocks
                                // same length as byte_count
    uint32_t    start_function; // function to run on module load

    uint32_t    table_initial;  // initial table size
    uint32_t    table_maximum;  // maximum table size
    uint32_t   *table;          // table entries

    uint32_t    global_count;   // number of globals
    StackValue *globals;        // globals

    uint32_t    memory_initial;  // 64K pages
    uint32_t    memory_maximum;  // 64K pages
    uint32_t    memory_pages;    // 64K pages
    uint8_t    *memory;          // memory area

    // Runtime state
    int         sp;                // operand stack pointer
    int         fp;                // current frame pointer into stack
    StackValue  stack[STACK_SIZE]; // main operand stack 
    int         csp;               // callstack pointer
    Frame       callstack[CALLSTACK_SIZE]; // callstack
} Module;

//
// Function declarations
//

Module *load_module(char *path);
uint32_t invoke(Module *m, int argc, char **argv);


#endif // of WAC_H
