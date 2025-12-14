#pragma once
#include <algorithm>

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
    struct label_pos_t {
        constant(label_pos_t);

        addr_t pos;
        std::string label;

        label_pos_t(const addr_t pos, std::string &&label) : pos(pos), label(std::move(label)) {
        }

        label_pos_t(label_pos_t &&other) noexcept : pos(other.pos), label(std::move(other.label)) {
        }
    };

    struct frame_info_t {
        utils::vector_t<char, utils::heap_allocator_t<4096> > code;
        utils::str_t name;

        std::map<std::string, addr_t> labels;
        utils::vector_t<label_pos_t> labels_positions;
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

        writem(jmp, addr_t to) {
            utils::write2vec(vector, instructions::jmp);
            utils::write2vec(vector, to);
        }

        template<typename A> requires utils::allocator_c<A>
        static constexpr addr_t jmp(utils::vector_t<char, A> *vector) {
            utils::write2vec(vector, instructions::jmp);
            const addr_t out = vector->size;
            utils::write2vec(vector, static_cast<addr_t>(0));
            return out;
        }

        writem(calln, addr_t id) {
            utils::write2vec(vector, instructions::calln);
            utils::write2vec(vector, id);
        }

#undef write
    }

    template<typename T>
    constexpr T parse_number(const utils::str_t &str) {
        utils::str_t c(str);
        const auto d = c.emplace(0).data();
        const T addr = static_cast<T>(std::stoul(d));
        return addr;
    }

    struct context_t {
        utils::vector_t<char, utils::heap_allocator_t<4096> > output;
        utils::vector_t<char, utils::heap_allocator_t<8192> > asm_output;

        std::vector<char> v;

        frame_info_t *current_frame = nullptr;

        context_t() {
        }

        void compile_asm_block(parser::ast_asm_block_t *asm_block) {
            for (auto instruction: asm_block->instructions) {
                auto parts = instruction->parts.as_slice();
                auto part = *parts[0];

                if (part->kind == lexer::colon) {
                    current_frame->labels[std::string(parts.get(1)->literal.copy().emplace(0).data())] = current_frame->code.size;
                } else if (instructions::nop_literal == part->literal) {
                    writer::nop(&current_frame->code);
                } else if (instructions::ret_literal == part->literal) {
                    writer::ret(&current_frame->code);
                } else if (instructions::jmp_literal == part->literal) {
                    if (parts.get(1)->kind == lexer::num) {
                        const auto addr = parse_number<addr_t>(parts.get(1)->literal);
                        writer::jmp(&current_frame->code, addr);
                    } else if (parts.get(1)->kind == lexer::ident) {
                        const auto addr = writer::jmp(&current_frame->code);
                        auto pos = label_pos_t(addr, std::string(parts.get(1)->literal.copy().emplace(0).data()));
                        current_frame->labels_positions.emplace(std::move(pos));
                    } else
                        raise(SIGABRT);
                } else if (instructions::call_literal == part->literal) {
                    // TODO call instruction compilation
                } else if (instructions::dbg_brk_literal == part->literal) {
                    writer::dbg_brk(&current_frame->code);
                } else if (instructions::ill_literal == part->literal) {
                    writer::ill(&current_frame->code);
                } else if (instructions::calln_literal == part->literal) {
                    writer::calln(&current_frame->code, 0);
                }
            }
        }

        void compile_statement(parser::ast_t *stmt) {
            if (const auto kind = stmt->kind; kind == parser::asm_block)
                compile_asm_block(reinterpret_cast<parser::ast_asm_block_t *>(stmt));
        }

        void compile_frame(parser::ast_frame_t *frame) {
            frame_info_t info;
            // ReSharper disable once CppDFALocalValueEscapesFunction
            current_frame = &info;

            current_frame->name = frame->name;

            for (const auto statement: frame->statements)
                compile_statement(statement);

            writer::ret(&current_frame->code);

            // replace all label dummies
            for (const auto &[pos, label]: current_frame->labels_positions)
                *as<s_addr_t>(current_frame->code[pos]) = current_frame->labels[label];

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
