#include <cstdint>
#include <fstream>
#include <iostream>

#include "asm2bc.h"
#include "bc_decoder.h"
#include "buffer.h"
#include "vm.h"

void std_cout_cstr(vm::context *ctx) {
    uint64_t ptr; ctx->stack >> ptr;
    std::cout << reinterpret_cast<char *>(ptr);
}

void std_cin_byte(vm::context *ctx) {
    char c;
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
    c.symbol_table["std_cout_cstr"] = reinterpret_cast<size_t>(std_cout_cstr);
    c.symbol_table["std_cin_byte"] = reinterpret_cast<size_t>(std_cin_byte);

    reg(nop, {}, {});
    reg(end, {}, { ctx->stop = true; });
    reg(jmp64c, { param_d64(); }, {
        uint64_t ptr; ctx->code >> ptr;
        ctx->code.read_pos = reinterpret_cast<char *>(ptr) - ctx->code.data;
    });

    vm::tr::translate("code.asm", "code.bc");

    vm::bcd::load(vm::C, "code.bc");

    c.const_pool.dump("const_pool.dump");
    c.code.dump("code.dump");

    vm::init(vm::C);

    while (!vm::C->stop)
        vm::exec_operation(vm::C);

    vm::dispose(vm::C);
}
