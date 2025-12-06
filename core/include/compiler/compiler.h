#pragma once
#include "parser.h"

namespace compiler {
    struct frame_info {
        vm::utils::str_t name;
    };

    struct context_t {
        vm::utils::vector_t<char, vm::utils::heap_allocator_t<4096>> output;
        vm::utils::vector_t<char, vm::utils::heap_allocator_t<8192>> asm_output;

        frame_info *current_frame = nullptr;

        context_t() {
        }

        void compile_asm_block(parser::ast_asm_block_t *asm_block) {
            (void) 0;
        }

        void compile_statement(parser::ast_t *stmt) {
            if (const auto kind = stmt->kind; kind == parser::asm_block) {
                compile_asm_block(reinterpret_cast<parser::ast_asm_block_t *>(stmt));
            }
        }

        void compile_frame(parser::ast_frame_t *frame) {
            frame_info info;
            current_frame = &info;

            current_frame->name = frame->name;

            for (auto statement : frame->statements)
                compile_statement(statement);

            current_frame = nullptr;
        }

        void compile_root(parser::ast_root_t *root) {
            for (auto ast : root->list) {
                if (ast->kind == parser::frame)
                    compile_frame(parser::cast<parser::ast_frame_t>(ast));
            }
        }

        void write(FILE *fd) {
            fwrite(output.begin(), 1, output.size, fd);
        }

        void write_asm(FILE *fd) {
            fwrite(asm_output.begin(), 1, asm_output.size, fd);
        }
    };
}