#pragma once

namespace instructions {
    enum opcode_t {
        nop = 0,
        ill,
        ret,
        dbg_brk,

        // 16b
        jmp,
        call,
    };

    inline utils::str_t nop_literal = utils::obj2str("nop");
    inline utils::str_t ill_literal = utils::obj2str("ill");
    inline utils::str_t ret_literal = utils::obj2str("ret");
    inline utils::str_t dbg_brk_literal = utils::obj2str("dbg_brk");
    inline utils::str_t jmp_literal = utils::obj2str("jmp");
    inline utils::str_t call_literal = utils::obj2str("call");
}
