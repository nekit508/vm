#include "vm.h"

#include <cstdint>
#include <iostream>

void vm::allocate_stack(context *ctx) {
    size_t size = 1024;
    ctx->stack = utils::buffer(static_cast<char *>(malloc(size)), size);
}

void vm::init(context *ctx) {
    allocate_stack(ctx);
    ctx->memory = static_cast<char *>(malloc(1024 * 1024));
}

void vm::dispose(context *ctx) {
    // TODO here is memory and stack snapshot
    std::fstream stream("memory.dump", std::ios_base::out | std::ios_base::binary);
    stream.write(ctx->memory, 1024 * 1024);
    stream.close();
    ctx->stack.dump("stack.dump");

    free(ctx->memory);
}

void vm::exec_operation(context *ctx) {
    uint16_t opcode;
    ctx->code >> opcode;

    std::cout << "opcode: " << opcode << std::endl;

    if (!handlers.contains(opcode)) {
        std::cerr << "opcode " << opcode << " was not processed" << std::endl;
        raise(SIGILL);
    } else {
        handlers[opcode](ctx);
    }
}
