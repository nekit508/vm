#pragma once

#include <csignal>

namespace instructions {
    enum opcode_t {
        nop, ill, ret, dbg_brk,

        jmp, call,
    };

    inline utils::str_t nop_literal = utils::cstr2str("nop");
    inline utils::str_t ill_literal = utils::cstr2str("ill");
    inline utils::str_t ret_literal = utils::cstr2str("ret");
    inline utils::str_t dbg_brk_literal = utils::cstr2str("dbg_brk");
    inline utils::str_t jmp_literal = utils::cstr2str("jmp");
    inline utils::str_t call_literal = utils::cstr2str("call");

    inline void tick(const opcode_t opcode, char *data) {
        switch (opcode) {
            case nop: // no operation
                break;
            case ret: // return to caller
                break;
            case jmp: // jump to operation (frame start relative)
                // TODO jmp instruction decode
                break;
            case call: // call another frame
                // TODO call instruction decode
                break;
            case dbg_brk: // debug break
                (void)0;
                break;
            case ill: // illegal operation
                raise(SIGILL);
                break;
            default:
                raise(SIGALRM);
                break;
        }
    }
}
