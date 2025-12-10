#pragma once
#include <algorithm>

#include "bits/move.h"
#include <iostream>

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
        utils::str_t message;

        parse_error_e(const parse_error_e &other) : pos(other.pos), message(other.message) {
        }

        parse_error_e(parse_error_e &&other) noexcept : pos(other.pos), message(std::move(other.message)) {
        }

        explicit parse_error_e(const addr_t pos, utils::str_t &&message) : pos(pos), message(std::move(message)) {
            0;
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

            utils::str_t literal;

            explicit ast_asm_part_t(utils::str_t &&literal) : ast_t(asm_part),
                                                              literal(std::move(literal)) {
            }
        };

        struct ast_asm_instruction_t : ast_t {
            immutable(ast_asm_instruction_t)

            utils::vector_t<ast_asm_part_t *> parts;

            explicit ast_asm_instruction_t(utils::vector_t<ast_asm_part_t *> &&parts) : ast_t(asm_instruction),
                parts(std::move(parts)) {
            }
        };

        struct ast_asm_block_t : ast_t {
            immutable(ast_asm_block_t)

            utils::vector_t<ast_asm_instruction_t *> instructions;

            explicit ast_asm_block_t(utils::vector_t<ast_asm_instruction_t *> &&instructions) : ast_t(asm_block),
                instructions(std::move(instructions)) {
            }
        };

        struct ast_parameter_t : ast_t {
            immutable(ast_parameter_t)


            utils::vector_t<char> type;
            utils::vector_t<char> name;

            ast_parameter_t(utils::vector_t<char> &&type,
                            utils::vector_t<char> &&name) : ast_t(parameter), type(std::move(type)),
                                                            name(std::move(name)) {
            }
        };

        struct ast_frame_t : ast_t {
            immutable(ast_frame_t)

            utils::str_t name;
            utils::vector_t<ast_parameter_t *> parameters;
            utils::vector_t<ast_t *> statements;

            explicit ast_frame_t(utils::str_t &&name,
                                 utils::vector_t<ast_parameter_t *> &&parameters,
                                 utils::vector_t<ast_t *> &&statements) : ast_t(frame),
                                                                          name(std::move(name)),
                                                                          parameters(std::move(parameters)),
                                                                          statements(std::move(statements)) {
            }
        };

        struct ast_root_t : ast_t {
            immutable(ast_root_t)

            utils::vector_t<ast_t *> list;

            explicit ast_root_t(utils::vector_t<ast_t *> &&list) : ast_t(root), list(std::move(list)) {
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
        utils::vector_t<lexer::token_t> tokens;
        addr_t current_pos = -1;

        explicit context_t(utils::vector_t<lexer::token_t> &&tokens) : tokens(std::move(tokens)) {
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
            return out;
        }

        void redo(const addr_t offset) {
            current_pos -= offset;
        }

        void redo() {
            redo(1);
        }

        utils::res_t<lexer::token_t *, parse_error_e> accept(const lexer::token_kind_t target) {
            if (*next() == target)
                return get();
            return parse_error_e(pos(), utils::str_t()
                                 .push(utils::obj2str("Expected "))
                                 .push(utils::obj2str(lexer::to_string(target)))
                                 .push(utils::obj2str(" at "))
                                 .push(utils::obj2str(pos()))
                                 .push(utils::obj2str(" but "))
                                 .push(utils::obj2str(lexer::to_string(get()->kind)))
                                 .push(utils::obj2str(" provided."))
                                 .trim().move());
        }

        utils::res_t<lexer::token_t *, parse_error_e> accept(const std::initializer_list<lexer::token_kind_t> any) {
            auto *token = next();

            for (const auto &target: any) {
                if (*token == target)
                    return token;
            }

            utils::str_t error;
            error.push(utils::obj2str("Expected any of: [ "));
            for (const auto &target: any)
                error
                        .push(utils::obj2str<lexer::token_kind_t, const char *, lexer::to_string>(target))
                        .push(utils::obj2str(" "));
            error.push(utils::obj2str("] at "))
                    .push(utils::obj2str(pos()))
                    .push(utils::obj2str(" but "))
                    .push(utils::obj2str(lexer::to_string(get()->kind)))
                    .push(utils::obj2str(" provided."))
                    .trim();

            return parse_error_e(pos(), error.move());
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

        utils::res_t<ast_asm_instruction_t *, parse_error_e> parse_asm_instruction() {
            utils::vector_t<ast_asm_part_t *> parts;

            do {
                auto res = accept({lexer::ident, lexer::num}).disown().move();
                if (!res)
                    return res.error_m();
                parts.push(new ast_asm_part_t(res.value()->literal.copy()));
            } while (probe_not_and_redo_if(lexer::semicolon, true));

            return new ast_asm_instruction_t(std::move(parts));
        }

        utils::res_t<ast_asm_block_t *, parse_error_e> parse_asm_block() {
            utils::vector_t<ast_asm_instruction_t *> instructions;

            auto asmm = accept(lexer::asmm).disown().move();
            if (!asmm)
                return asmm.error_m();

            auto lbr = accept(lexer::l_brace).disown().move();
            if (!asmm)
                return asmm.error_m();

            while (probe_not_and_redo_if(lexer::r_brace, true)) {
                auto res = parse_asm_instruction().disown().move();
                if (!res)
                    return res.error_m();

                instructions.push(res.value());
            }

            return new ast_asm_block_t(std::move(instructions));
        }

        utils::res_t<ast_t *, parse_error_e> parse_statement() {
            if (probe_and_redo(lexer::asmm)) {
                return parse_asm_block().disown().move();
            }

            return parse_error_e(pos(), utils::str_t()
                                 .push(utils::obj2str("Expected asm but "))
                                 .push(utils::obj2str(lexer::to_string(get()->kind)))
                                 .push(utils::obj2str(" provided."))
                                 .move());
        }

        utils::res_t<ast_parameter_t *, parse_error_e> parse_parameter() {
            auto type = std::move(accept(lexer::ident).disown());
            if (!type) return type.error_m();

            auto name = std::move(accept(lexer::ident).disown());
            if (!name) return name.error_m();

            return new ast_parameter_t(type.value()->literal.copy(), name.value()->literal.copy());
        }

        utils::res_t<ast_frame_t *, parse_error_e> parse_frame() {
            if (auto fun = std::move(accept(lexer::fun).disown()); !fun)
                return fun.error_m();

            auto name = std::move(accept(lexer::ident).disown());
            if (!name)
                return name.own().error_m();

            if (auto lpr = std::move(accept(lexer::l_paren).disown()); !lpr)
                return lpr.error_m();

            utils::vector_t<ast_parameter_t *> params;
            if (probe_not_and_redo_if(lexer::r_paren, true)) {
                do {
                    auto par = std::move(parse_parameter().disown());
                    if (!par)
                        return par.error_m();
                    params.push(par.value());
                } while (probe_and_redo_if(lexer::comma, false));
                if (auto rpr = std::move(accept(lexer::r_paren).disown()); !rpr)
                    return rpr.error_m();
            }

            if (auto lbr = accept(lexer::l_brace).disown().move(); !lbr)
                return lbr.error_m();

            utils::vector_t<ast_t *> statements;
            while (probe_not_and_redo_if(lexer::r_brace, true)) {
                auto stmt = parse_statement().disown().move();
                if (!stmt)
                    return stmt.error_m();

                statements.push(stmt.value());
            }

            return new ast_frame_t(name.value()->literal.copy(), std::move(params), statements.move());
        }

        utils::res_t<ast_root_t *, parse_error_e> parse() {
            auto out = utils::vector_t<ast_t *>();

            while (probe_not_and_redo_if(lexer::eof, true)) {
                auto res = parse_frame().disown().move();
                if (!res)
                    return res.error_m();
                out.push(res.value());
            }

            return new ast_root_t(std::move(out));
        }
    };
}
