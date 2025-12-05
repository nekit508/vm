#pragma once
#include "parser.h"

namespace compiler {
    struct context_t {
        //vm::utils::vector_t<char, vm::utils::file_virtual_memory_allocator> output;

        context_t(const char *file) {
            //output.allocator = vm::utils::file_virtual_memory_allocator(fopen(file, "w"));

            //output.emplace(87);
        }

        void compile_frame(parser::ast_frame_t *frame) {

        }

        void compile_root(parser::ast_root_t *root) {
            for (auto ast : root->list) {
                if (ast->kind == parser::frame)
                    compile_frame(parser::cast<parser::ast_frame_t>(ast));
            }
        }
    };
}