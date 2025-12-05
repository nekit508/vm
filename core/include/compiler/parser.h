#pragma once
#include <algorithm>
#include <any>
#include <iostream>
#include <string>

#include "lexer.h"
#include "utils.h"

#define immutable(type) \
    type(type &&other) = delete; \
    type(const type &other) = delete; \
    type &operator=(type &&other) = delete; \
    type &operator=(const type &other) = delete;

namespace parser {
    struct parse_error_e {
        addr_t pos;
        vm::utils::str_t message;

        explicit parse_error_e(const addr_t pos, vm::utils::str_t &&message) : pos(pos), message(std::move(message)) {
        }
    };

    namespace ast {
        enum ast_kind_t {
            asm_part, asm_instruction, asm_block, parameter, frame, root
        };

        struct ast_t {
            ast_kind_t kind;

            immutable(ast_t);

            explicit ast_t(const ast_kind_t kind) : kind(kind) {
            }

            ~ast_t();

            void operator delete(void *ptr, size_t size);
        private:
            bool deleted = false;
        };

        template<typename T> requires __is_base_of(ast_t, T)
        constexpr T *cast(ast_t *ast) {
            return reinterpret_cast<T *>(ast);
        }

        struct ast_asm_part_t : ast_t {
            immutable(ast_asm_part_t)

            vm::utils::str_t literal;

            explicit ast_asm_part_t(vm::utils::str_t &&literal) : ast_t(asm_part),
                                                                  literal(std::move(literal)) {
            }
        };

        struct ast_asm_instruction_t : ast_t {
            immutable(ast_asm_instruction_t)

            vm::utils::vector_t<ast_asm_part_t *> parts;

            explicit ast_asm_instruction_t(vm::utils::vector_t<ast_asm_part_t *> &&parts) : ast_t(asm_instruction),
                parts(std::move(parts)) {
            }
        };

        struct ast_asm_block_t : ast_t {
            immutable(ast_asm_block_t)

            vm::utils::vector_t<ast_asm_block_t *> instructions;

            explicit ast_asm_block_t(vm::utils::vector_t<ast_asm_block_t *> &&instructions) : ast_t(asm_block),
                instructions(
                    std::move(instructions)) {
            }
        };

        struct ast_parameter_t : ast_t {
            immutable(ast_parameter_t)


            vm::utils::vector_t<char> type;
            vm::utils::vector_t<char> name;

            ast_parameter_t(vm::utils::vector_t<char> &&type,
                            vm::utils::vector_t<char> &&name) : ast_t(parameter), type(std::move(type)),
                                                                name(std::move(name)) {
            }
        };

        struct ast_frame_t : ast_t {
            immutable(ast_frame_t)

            vm::utils::str_t name;
            vm::utils::vector_t<ast_parameter_t *> parameters;

            explicit ast_frame_t(vm::utils::str_t &&name,
                                 vm::utils::vector_t<ast_parameter_t *> &&parameters) : ast_t(frame),
                name(std::move(name)),
                parameters(std::move(parameters)) {
            }
        };

        struct ast_root_t : ast_t {
            immutable(ast_root_t)

            vm::utils::vector_t<ast_t *> list;

            explicit ast_root_t(vm::utils::vector_t<ast_t *> &&list) : ast_t(root), list(std::move(list)) {
            }
        };

        inline ast_t::~ast_t() {
            if (deleted)
                return;
            deleted = true;
        }

        inline void ast_t::operator delete(void *ptr, const size_t size) {
            auto p = static_cast<ast_t *>(ptr);

            if (p->kind == asm_part)
                reinterpret_cast<ast_asm_part_t *>(p)->~ast_asm_part_t();
            else if (p->kind == asm_instruction)
                reinterpret_cast<ast_asm_instruction_t *>(p)->~ast_asm_instruction_t();
            else if (p->kind == asm_block)
                reinterpret_cast<ast_asm_block_t *>(p)->~ast_asm_block_t();
            else if (p->kind == parameter)
                reinterpret_cast<ast_parameter_t *>(p)->~ast_parameter_t();
            else if (p->kind == frame)
                reinterpret_cast<ast_frame_t *>(p)->~ast_frame_t();
            else if (p->kind == root)
                reinterpret_cast<ast_root_t *>(p)->~ast_root_t();

            ::operator delete(ptr);
        }
    }

    using namespace ast;

    struct context_t {
        vm::utils::vector_t<lexer::token_t> tokens;
        addr_t current_pos = -1;

        explicit context_t(vm::utils::vector_t<lexer::token_t> &&tokens) : tokens(std::move(tokens)) {
        }

        addr_t pos() const {
            return current_pos;
        }

        lexer::token_t *get() const {
            return tokens[current_pos];
        }

        lexer::token_t *get(const addr_t pos) const {
            return tokens[pos];
        }

        lexer::token_t *next() {
            auto *out = tokens[++current_pos];
            if (*out == lexer::eof) raise(SIGSEGV);
            return out;
        }

        void redo(const addr_t offset) {
            current_pos -= offset;
        }

        void redo() {
            redo(1);
        }

        vm::utils::res_t<lexer::token_t *, parse_error_e> accept(const lexer::token_kind_t target) {
            if (*next() == target)
                return get();
            return parse_error_e(pos(), std::move(
                                     vm::utils::cstr2strm_t("Expected ").emplace(
                                         vm::utils::cstr2strm_t(lexer::to_string(target))[0, 1, true])));
        }

        bool probe(const lexer::token_kind_t prober) {
            return *next() == prober;
        }

        bool probe_not(const lexer::token_kind_t prober) {
            return *next() != prober;
        }

        bool probe_and_redo(const lexer::token_kind_t prober) {
            const bool out = *next() == prober;
            current_pos--;
            return out;
        }

        bool probe_not_and_redo(const lexer::token_kind_t prober) {
            const bool out = *next() != prober;
            current_pos--;
            return out;
        }

        bool probe_and_redo_if(const lexer::token_kind_t prober, const bool v) {
            const bool out = *next() == prober;
            if (out == v) current_pos--;
            return out;
        }

        bool probe_not_and_redo_if(const lexer::token_kind_t prober, const bool v) {
            const bool out = *next() != prober;
            if (out == v) current_pos--;
            return out;
        }

        vm::utils::res_t<ast_parameter_t *, parse_error_e> parse_parameter() {
            auto type = accept(lexer::ident).disown();
            if (!type) return type.error();

            auto name = accept(lexer::ident).disown();
            if (!name) return name.error();

            return new ast_parameter_t(type.value()->literal.copy(), name.value()->literal.copy());
        }

        vm::utils::res_t<ast_frame_t *, parse_error_e> parse_frame() {
            if (auto fun = accept(lexer::fun).disown(); !fun)
                return fun.error();

            auto name = accept(lexer::ident).disown();
            if (!name)
                return name.error();

            if (auto lpr = accept(lexer::l_paren).disown(); !lpr)
                return lpr.error();

            vm::utils::vector_t<ast_parameter_t *> params;
            if (probe_not_and_redo_if(lexer::r_paren, true)) {
                do {
                    auto par = parse_parameter().disown();
                    if (!par)
                        return par.error();
                    params.push(par.value());
                } while (probe_and_redo_if(lexer::comma, false));
                if (auto rpr = accept(lexer::r_paren).disown(); !rpr)
                    return rpr.error();
            }

            return new ast_frame_t(name.value()->literal.copy(), std::move(params));
        }

        vm::utils::res_t<ast_root_t *, parse_error_e> parse() {
            auto out = vm::utils::vector_t<ast_t *>();

            auto res = parse_frame().disown();
            if (!res)
                return res.error();
            out.emplace(res.value());

            return new ast_root_t(std::move(out));
        }
    };
}
