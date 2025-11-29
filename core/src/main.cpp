
#include <cstring>
#include <fstream>
#include <iostream>

#include "compiler.h"
#include "memory.h"
#include "compiler/lexer.h"

#define reg(id, encoder, handler) register_op(#id, id, [](compiler::context_t *ctx, std::string &line)encoder, [](vm::context_t *ctx)handler)

namespace cp = compiler;

int main(int argc, char *argv[]) {
    /*reg(nop, { compiler::opcode(ctx, nop); }, {});
    reg(end, { compiler::opcode(ctx, end); }, { ctx->stop = true; });
    reg(load64c, { compiler::opcode(ctx, load64c); compiler::param_d64(ctx, line); }, { uint64_t d; ctx->code >> d; ctx->stack << d; });
    reg(load8c, { compiler::opcode(ctx, load8c); compiler::param_s8(ctx, line); }, { uint8_t d; ctx->code >> d; ctx->stack << d; });
    reg(write64s64s, { compiler::opcode(ctx, write64s64s); }, { uint64_t addr; ctx->stack >> addr; uint64_t value; ctx->stack >> value; *as<uint64_t>(ctx->memory[addr]) = value; });
    reg(calln64s, { compiler::opcode(ctx, calln64s); }, { uint64_t addr; ctx->stack >> addr; reinterpret_cast<vm::bindable>(addr)(ctx); });
    reg(read6464s, { compiler::opcode(ctx, read6464s); }, { uint64_t addr; ctx->stack >> addr; ctx->stack << *as<uint64_t>(ctx->memory[addr]); });
    reg(read864s, { compiler::opcode(ctx, read864s); }, { uint64_t addr; ctx->stack >> addr; ctx->stack << *as<uint8_t>(ctx->memory[addr]); });
    reg(write64s8s, { compiler::opcode(ctx, write64s8s); }, { uint64_t addr; ctx->stack >> addr; uint8_t value; ctx->stack >> value; *as<uint8_t>(ctx->memory[addr]) = value; });
    reg(add64c64s, { compiler::opcode(ctx, add64c64s); compiler::param_d64(ctx, line); }, { uint64_t a; ctx->code >> a; uint64_t b; ctx->stack >> b; ctx->stack << (a + b); });
    reg(sub64c64s, { compiler::opcode(ctx, sub64c64s); compiler::param_d64(ctx, line); }, { uint64_t a; ctx->code >> a; uint64_t b; ctx->stack >> b; ctx->stack << (b - a); });
    reg(mapmem6464s, { compiler::opcode(ctx, mapmem6464s); }, { uint64_t addr; ctx->stack >> addr; ctx->stack << as<uint64_t>(ctx->memory[addr]); });
    reg(cmpeq8s8s, { compiler::opcode(ctx, cmpeq8s8s); }, { uint8_t a; ctx->stack >> a; uint8_t b; ctx->stack >> b; ctx->stack << static_cast<uint8_t>(a == b); });
    reg(cmpeq64s64s, { compiler::opcode(ctx, cmpeq64s64s); }, { uint64_t a; ctx->stack >> a; uint64_t b; ctx->stack >> b; ctx->stack << static_cast<uint8_t>(a == b); });
    reg(jmpt8s64c, { compiler::opcode(ctx, jmpt8s64c); compiler::param_d64(ctx, line); }, { uint64_t addr; ctx->code >> addr; uint8_t value; ctx->stack >> value; if (value) code_pos(ctx, addr); });
    reg(jmpf8s64c, { compiler::opcode(ctx, jmpf8s64c); compiler::param_d64(ctx, line); }, { uint64_t addr; ctx->code >> addr; uint8_t value; ctx->stack >> value; if (!value) code_pos(ctx, addr); });
    reg(jmp64c, { compiler::opcode(ctx, jmp64c); compiler::param_d64(ctx, line); }, { uint64_t addr; ctx->code >> addr; code_pos(ctx, addr); });
    reg(ret, { compiler::opcode(ctx, ret); }, { size_t addr; ctx->call_stack >> addr; code_pos(ctx, addr); });
    reg(call64c, { compiler::opcode(ctx, call64c); compiler::param_d64(ctx, line); }, { uint64_t addr; ctx->code >> addr; ctx->call_stack << code_pos(ctx); code_pos(ctx, addr); });*/

    cp::context_t compiler_context;
    compiler_context.stream = fopen("code.f", "r");

    cp::parse(&compiler_context);

    return 0;
}
