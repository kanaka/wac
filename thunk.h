#ifndef _THUNK_H
#define _THUNK_H

#include "wa.h"

#define TH_C_0 void
#define TH_C_i uint32_t
#define TH_C_I uint64_t
#define TH_C_f float
#define TH_C_F double
#define TH_RES_0
#define TH_RES_i uint32_t res =
#define TH_RES_I uint64_t res =
#define TH_RES_f float res =
#define TH_RES_F double res =
#define TH_RCNT_0 0
#define TH_RCNT_i 1
#define TH_RCNT_I 1
#define TH_RCNT_f 1
#define TH_RCNT_F 1
#define TH_ATTR_i uint32
#define TH_ATTR_I uint64
#define TH_ATTR_f f32
#define TH_ATTR_F f64
#define TH_WA_i I32
#define TH_WA_I I64
#define TH_WA_f F32
#define TH_WA_F F64
#define TH_SP(M,OP,T) M->stack[M->sp OP].value.TH_ATTR_##T
#define TH_FP(M,OP,T) M->stack[M->fp OP].value.TH_ATTR_##T
#define TH_OUT_RET_(M,T) \
    M->stack[M->sp].value_type = TH_WA_##T; \
    TH_SP(M,,T) = res;
#define TH_OUT_RET_0(M)
#define TH_OUT_RET_i(M) TH_OUT_RET_(M,i)
#define TH_OUT_RET_I(M) TH_OUT_RET_(M,I)
#define TH_OUT_RET_f(M) TH_OUT_RET_(M,f)
#define TH_OUT_RET_F(M) TH_OUT_RET_(M,F)
#define TH_IN_RET_0(M)
#define TH_IN_RET_i(M) TH_SP(M,--,i)
#define TH_IN_RET_I(M) TH_SP(M,--,I)
#define TH_IN_RET_f(M) TH_SP(M,--,f)
#define TH_IN_RET_F(M) TH_SP(M,--,F)


#define THUNK_OUT_0(M,FN,R) { \
    TH_RES_##R ((TH_C_##R (*)())FN->func_ptr)(); \
    M->sp += 0 + TH_RCNT_##R; TH_OUT_RET_##R(M); \
}
#define THUNK_OUT_1(M,FN,R,A) { \
    TH_RES_##R ((TH_C_##R (*)(TH_C_##A))FN->func_ptr)(TH_SP(M,+0,A)); \
    M->sp += -1 + TH_RCNT_##R; TH_OUT_RET_##R(M); \
}
#define THUNK_OUT_2(M,FN,R,A,B) { \
    TH_RES_##R ((TH_C_##R (*)(TH_C_##A,TH_C_##B))FN->func_ptr)(TH_SP(M,-1,A),TH_SP(M,+0,B)); \
    M->sp += -2 + TH_RCNT_##R; TH_OUT_RET_##R(M); \
}
#define THUNK_OUT_3(M,FN,R,A,B,C) { \
    TH_RES_##R ((TH_C_##R (*)(TH_C_##A,TH_C_##B,TH_C_##C))FN->func_ptr)(TH_SP(M,-2,A),TH_SP(M,-1,B),TH_SP(M,+0,C)); \
    M->sp += -3 + TH_RCNT_##R; TH_OUT_RET_##R(M); \
}
#define THUNK_OUT_4(M,FN,R,A,B,C,D) { \
    TH_RES_##R ((TH_C_##R (*)(TH_C_##A,TH_C_##B,TH_C_##C,TH_C_##D))FN->func_ptr)(TH_SP(M,-3,A),TH_SP(M,-2,B),TH_SP(M,-1,C),TH_SP(M,+0,D)); \
    M->sp += -4 + TH_RCNT_##R; TH_OUT_RET_##R(M); \
}
#define THUNK_OUT_5(M,FN,R,A,B,C,D,E) { \
    TH_RES_##R ((TH_C_##R (*)(TH_C_##A,TH_C_##B,TH_C_##C,TH_C_##D,TH_C_##E))FN->func_ptr)(TH_SP(M,-4,A),TH_SP(M,-3,B),TH_SP(M,-2,C),TH_SP(M,-1,D),TH_SP(M,+0,E)); \
    M->sp += -5 + TH_RCNT_##R; TH_OUT_RET_##R(M); \
}
#define THUNK_OUT_6(M,FN,R,A,B,C,D,E,F) { \
    TH_RES_##R ((TH_C_##R (*)(TH_C_##A,TH_C_##B,TH_C_##C,TH_C_##D,TH_C_##E,TH_C_##F))FN->func_ptr)(TH_SP(M,-5,A),TH_SP(M,-4,B),TH_SP(M,-3,C),TH_SP(M,-2,D),TH_SP(M,-1,E),TH_SP(M,+0,F)); \
    M->sp += -6 + TH_RCNT_##R; TH_OUT_RET_##R(M); \
}

#define THUNK_IN_FN_0(M,R) \
    TH_C_##R thunk_in_##R##_0() { \
        Module *m = _wa_current_module_; \
        interpret(M); return TH_IN_RET_##R(M); \
    }
#define THUNK_IN_FN_1(M,R,A) \
    TH_C_##R thunk_in_##R##_##A(TH_C_##A a) { \
        Module *m = _wa_current_module_; \
        TH_FP(M,+0,A) = a;  \
        interpret(M); return TH_IN_RET_##R(M); \
    }
#define THUNK_IN_FN_2(M,R,A,B) \
    TH_C_##R thunk_in_##R##_##A##B(TH_C_##A a, TH_C_##B b) { \
        Module *m = _wa_current_module_; \
        TH_FP(M,+0,A) = a; TH_FP(M,+1,B) = b;  \
        interpret(M); return TH_IN_RET_##R(M); \
    }
#define THUNK_IN_FN_3(M,R,A,B,C) \
    TH_C_##R thunk_in_##R##_##A##B##C(TH_C_##A a, TH_C_##B b, TH_C_##C c) { \
        Module *m = _wa_current_module_; \
        TH_FP(M,+0,A) = a; TH_FP(M,+1,B) = b; TH_FP(M,+2,C) = c;  \
        interpret(M); return TH_IN_RET_##R(M); \
    }


void thunk_out(Module *m, uint32_t fidx);

void init_thunk_in(Module *m);

#endif // _THUNK_H
