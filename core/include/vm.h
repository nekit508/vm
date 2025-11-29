#pragma once

#include <vector>

#include "memory.h"

namespace vm {
    struct environment_t {

    };

    struct sym_table_t {

    };

    struct frame_prototype_t {
        utils::memory_t<> code;
    };

    struct frame_t {
        frame_prototype_t *prototype;
        addr_t code_pos;

        frame_t *parent;
    };

    struct thread_t {
        frame_t *frame;
    };

    struct context_t {
        utils::memory_t<> heap;
        utils::memory_t<> ro_data;

        sym_table_t sym_table;
        std::vector<frame_prototype_t> frame_prototypes;
        std::vector<thread_t> threads;
    };
}
