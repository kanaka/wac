#include <stdlib.h>

#include "util.h"
#include "wa.h"
#include "thunk.h"

//
// Outbound Thunks (calling imported functions)
//

void thunk_out(Module *m, uint32_t fidx) {
    Block    *func = &m->functions[fidx];
    Type     *type = func->type;
    if (TRACE) {
        warn("  >>> thunk_out 0x%x(%d) %s.%s(",
             func->fidx, func->fidx,
             func->import_module, func->import_field);
        for (int p=type->param_count-1; p >= 0; p--) {
            warn("%s%s", value_repr(&m->stack[m->sp-p]), p ? " " : "");
        }
        warn("), %d results\n", type->result_count);
        debug("      mask: 0x%x\n", type->mask);
    }

    switch (type->mask) {
    case 0x800       : THUNK_OUT_0(m, func, 0);              break;
    case 0x8001      : THUNK_OUT_1(m, func, 0, i);           break;
    case 0x80011     : THUNK_OUT_2(m, func, 0, i,i);         break;
    case 0x8001111   : THUNK_OUT_4(m, func, 0, i,i,i,i);     break;
    case 0x810       : THUNK_OUT_0(m, func, i);              break;
    case 0x8101      : THUNK_OUT_1(m, func, i, i);           break;
    case 0x81011     : THUNK_OUT_2(m, func, i, i,i);         break;
    case 0x810111    : THUNK_OUT_3(m, func, i, i,i,i);       break;
    case 0x8101111   : THUNK_OUT_4(m, func, i, i,i,i,i);     break;
    case 0x81011111  : THUNK_OUT_5(m, func, i, i,i,i,i,i);   break;
    case 0x8003      : THUNK_OUT_1(m, func, 0, f);           break;
    case 0x80033     : THUNK_OUT_2(m, func, 0, f,f);         break;
    case 0x800333    : THUNK_OUT_3(m, func, 0, f,f,f);       break;
    case 0x8003333   : THUNK_OUT_4(m, func, 0, f,f,f,f);     break;
    case 0x8303      : THUNK_OUT_1(m, func, f, f);           break;
    case 0x8004      : THUNK_OUT_1(m, func, 0, F);           break;
    case 0x80044     : THUNK_OUT_2(m, func, 0, F,F);         break;
    case 0x800444    : THUNK_OUT_3(m, func, 0, F,F,F);       break;
    case 0x8004444   : THUNK_OUT_4(m, func, 0, F,F,F,F);     break;
    case 0x800444444 : THUNK_OUT_6(m, func, 0, F,F,F,F,F,F); break;
    case 0x8103      : THUNK_OUT_1(m, func, i, f);           break;
    case 0x8404      : THUNK_OUT_1(m, func, F, F);           break;
    default: FATAL("unsupported thunk_out mask 0x%llx\n", type->mask);
    }

    if (TRACE) {
        warn("  <<< thunk_out 0x%x(%d) %s.%s = %s\n",
             func->fidx, func->fidx, func->import_module, func->import_field,
             type->result_count > 0 ? value_repr(&m->stack[m->sp]) : "_");
    }
}


//
// Inbound Thunks (external calls into exported functions)
//

// TODO: global state, clean up somehow
// This global is used by setup_thunk_in since signal handlers don't have
// a way to pass arguments when they are setup.
Module * _wa_current_module_;

THUNK_IN_FN_0(m, 0)
THUNK_IN_FN_2(m, 0, i,i)
THUNK_IN_FN_1(m, 0, F)
THUNK_IN_FN_1(m, i, i)
THUNK_IN_FN_2(m, i, i,i)

// Push arguments
// return function pointer to thunk_in_* function
void (*setup_thunk_in(uint32_t fidx))() {
    Module   *m = _wa_current_module_; // TODO: global state, clean up somehow
    Block    *func = &m->functions[fidx];
    Type     *type = func->type;

    // Make space on the stack
    m->sp += type->param_count;

    if (TRACE) {
        warn("  {{}} setup_thunk_in '%s', mask: 0x%x, ARGS FOR '>>' ARE BOGUS\n",
             func->export_name, type->mask);
    }

    // Do normal function call setup. The fp will point to the start of stack
    // elements that were just added above
    setup_call(m, fidx);

    // Set the type of the unset stack elements
    for(uint32_t p=0; p<type->param_count; p++) {
        m->stack[m->fp+p].value_type = type->params[p];
    }

    // Return the thunk_in function
    void (*f)(void) = NULL;
    switch (type->mask) {
    case 0x800      : f = (void (*)(void)) thunk_in_0_0; break;
    case 0x8101     : f = (void (*)(void)) thunk_in_i_i; break;
    case 0x80011    : f = (void (*)(void)) thunk_in_0_ii; break;
    default: FATAL("unsupported thunk_in mask 0x%llx\n", type->mask);
    }

    return f;
}

void init_thunk_in(Module *m) {
    _wa_current_module_ = m; // TODO: global state, clean up somehow
}

