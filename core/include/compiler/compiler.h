#pragma once
#include "parser.h"

#include "instructions.h"

/*
There is some parts of memory:
addr_t

*/

// Instructions layout near frame
/*
ill
:frame_name
...
ret
ill
*/

namespace compiler {
    struct asm_line {
    };

    struct frame_info {
        utils::vector_t<char, utils::heap_allocator_t<4096> > code;
        utils::str_t name;
    };

    namespace writer {
#define write(name) \
template<typename A> requires utils::allocator_c<A> \
static constexpr void name(utils::vector_t<char, A> *vector)

#define writem(name, args...) \
template<typename A> requires utils::allocator_c<A> \
static constexpr void name(utils::vector_t<char, A> *vector, args)

        write(dbg_brk) {
            utils::write2vec(vector, instructions::dbg_brk);
        }

        write(nop) {
            utils::write2vec(vector, instructions::nop);
        }

        write(ret) {
            utils::write2vec(vector, instructions::ret);
        }

        write(ill) {
            utils::write2vec(vector, instructions::ill);
        }

        writem(jmp, s_addr_t to) {
            utils::write2vec(vector, instructions::jmp);
            utils::write2vec(vector, to);
        }

#undef write
    }

    struct context_t {
        utils::vector_t<char, utils::heap_allocator_t<4096>> output;
        utils::vector_t<char, utils::heap_allocator_t<8192>> asm_output;

        std::vector<char> v;

        utils::vector_t<asm_line> asm_lines;

        frame_info *current_frame = nullptr;

        context_t() {
        }

        void compile_asm_block(parser::ast_asm_block_t *asm_block) {
            for (auto instruction : asm_block->instructions) {
                auto parts = instruction->parts.as_slice();
                auto part = *parts[0];

                if (instructions::nop_literal == part->literal) {
                    writer::nop(&current_frame->code);
                } else if (instructions::ret_literal == part->literal) {
                    writer::ret(&current_frame->code);
                } else if (instructions::jmp_literal == part->literal) {
                    utils::str_t c((*parts[1])->literal.copy());
                    const auto d = c.emplace(0).data();
                    const auto addr = static_cast<s_addr_t>(std::stoul(d));
                    writer::jmp(&current_frame->code, addr);
                } else if (instructions::call_literal == part->literal) {
                    //writer::ca(&current_frame->code);
                } else if (instructions::dbg_brk_literal == part->literal) {
                    writer::dbg_brk(&current_frame->code);
                } else if (instructions::ill_literal == part->literal) {
                    writer::ill(&current_frame->code);
                }
            }
        }

        void compile_statement(parser::ast_t *stmt) {
            if (const auto kind = stmt->kind; kind == parser::asm_block)
                compile_asm_block(reinterpret_cast<parser::ast_asm_block_t *>(stmt));
        }

        void compile_frame(parser::ast_frame_t *frame) {
            frame_info info;
            // ReSharper disable once CppDFALocalValueEscapesFunction
            current_frame = &info;

            current_frame->name = frame->name;

            for (const auto statement: frame->statements)
                compile_statement(statement);

            writer::ret(&current_frame->code);

            output.push(current_frame->name).push(0);
            utils::write2vec(&output, current_frame->code.size);

            output.push(current_frame->code);
            current_frame = nullptr;
        }

        void compile_root(parser::ast_root_t *root) {
            for (const auto ast: root->list) {
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
