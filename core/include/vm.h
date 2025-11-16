#pragma once

#include <map>

#include "buffer.h"
#include "ops.h"

#define to_ptr(type, ptr) reinterpret_cast<type *>(ptr)
#define code_at(ctx, addr) ctx->code.read_pos = reinterpret_cast<char *>(addr) - ctx->code.data

namespace vm {
    struct context;

    typedef void(*bindable)(context *ctx);
    typedef void(*op_handler)(context *ctx);

    struct context {
        utils::straight_buffer code{1024 * 1024};
        utils::straight_buffer const_pool{1024 * 1024};
        utils::buffer stack;
        char *memory;
        // name : ptr
        std::map<std::string, size_t> symbol_table;
        bool stop = false;
    };

    inline std::map<opcode_t, op_handler> handlers;
    inline context *C;

    void allocate_stack(context *ctx);

    void init(context *ctx);

    void dispose(context *ctx);

    void exec_operation(context *ctx);
}
