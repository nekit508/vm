#include <cstdint>
#include <fstream>
#include <iostream>

#include "asm2bc.h"
#include "bc_decoder.h"
#include "buffer.h"
#include "vm.h"

void std_cout_str(vm::context *ctx) {
    uint64_t ptr; ctx->stack >> ptr;
    std::cout << reinterpret_cast<char *>(ptr);
}

void std_cin_char(vm::context *ctx) {
    uint8_t c;
    read(STDIN_FILENO, &c, 1);
    ctx->stack << c;
}

void register_op(const std::string &name, const opcode_t id, const vm::tr::op_encoder encoder, const vm::op_handler handler) {
    vm::tr::encoders[name] = encoder;
    vm::handlers[id] = handler;
}

#define reg(id, encoder, handler) register_op(#id, id, [](std::vector<char> &code, std::vector<char> &code_symbols, std::string &line)encoder, [](vm::context *ctx)handler)

int main(int argc, char *argv[]) {
    vm::context c{};
    vm::C = &c;
    c.symbol_table["std_cin_char"] = reinterpret_cast<size_t>(std_cin_char);
    c.symbol_table["std_cout_str"] = reinterpret_cast<size_t>(std_cout_str);

    reg(nop, { opcode(nop); }, {});
    reg(end, { opcode(end); }, { ctx->stop = true; });
    reg(load64c, { opcode(load64c); param_d64(); }, { uint64_t d; ctx->code >> d; ctx->stack << d; });
    reg(load8c, { opcode(load8c); param_s8(); }, { uint8_t d; ctx->code >> d; ctx->stack << d; });
    reg(write64s64s, { opcode(write64s64s); }, { uint64_t addr; ctx->stack >> addr; uint64_t value; ctx->stack >> value; *to_ptr(uint64_t, ctx->memory + addr) = value; });
    reg(calln64s, { opcode(calln64s); }, { uint64_t addr; ctx->stack >> addr; reinterpret_cast<vm::bindable>(addr)(ctx); });
    reg(read6464s, { opcode(read6464s); }, { uint64_t addr; ctx->stack >> addr; ctx->stack << *to_ptr(uint64_t, ctx->memory + addr); });
    reg(read864s, { opcode(read864s); }, { uint64_t addr; ctx->stack >> addr; ctx->stack << *to_ptr(uint8_t, ctx->memory + addr); });
    reg(write64s8s, { opcode(write64s8s); }, { uint64_t addr; ctx->stack >> addr; uint8_t value; ctx->stack >> value; *to_ptr(uint8_t, ctx->memory + addr) = value; });
    reg(add64c64s, { opcode(add64c64s); param_d64(); }, { uint64_t a; ctx->code >> a; uint64_t b; ctx->stack >> b; ctx->stack << (a + b); });
    reg(sub64c64s, { opcode(sub64c64s); param_d64(); }, { uint64_t a; ctx->code >> a; uint64_t b; ctx->stack >> b; ctx->stack << (b - a); });
    reg(mapmem64s, { opcode(mapmem64s); }, { uint64_t addr; ctx->stack >> addr; ctx->stack << reinterpret_cast<uint64_t>(ctx->memory + addr); });
    reg(cmpeq8s8s, { opcode(cmpeq8s8s); }, { uint8_t a; ctx->stack >> a; uint8_t b; ctx->stack >> b; ctx->stack << static_cast<uint8_t>(a == b); });
    reg(cmpeq64s64s, { opcode(cmpeq64s64s); }, { uint64_t a; ctx->stack >> a; uint64_t b; ctx->stack >> b; ctx->stack << static_cast<uint8_t>(a == b); });
    reg(jmpt8s64c, { opcode(jmpt8s64c); param_d64(); }, { uint64_t addr; ctx->code >> addr; uint8_t value; ctx->stack >> value; if (value) code_at(ctx, addr); });
    reg(jmpf8s64c, { opcode(jmpf8s64c); param_d64(); }, { uint64_t addr; ctx->code >> addr; uint8_t value; ctx->stack >> value; if (!value) code_at(ctx, addr); });
    reg(jmp64c, { opcode(jmp64c); param_d64(); }, { uint64_t addr; ctx->code >> addr; code_at(ctx, addr); });

    vm::tr::translate("code.asm", "code.bc");

    vm::bcd::load(vm::C, "code.bc");

    c.const_pool.dump("const_pool.dump");
    c.code.dump("code.dump");

    vm::init(vm::C);

    while (!vm::C->stop)
        vm::exec_operation(vm::C);

    vm::dispose(vm::C);
}
