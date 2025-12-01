#pragma once
#include <algorithm>

#include "lexer.h"
#include "memory.h"

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
            frame,
            parameter
        };

        struct ast_parameter_t {
            immutable(ast_parameter_t)

            ast_kind_t kind;
            vm::utils::vector_t<char> type;
            vm::utils::vector_t<char> name;

            ast_parameter_t(const ast_kind_t kind, vm::utils::vector_t<char> &&type,
                            vm::utils::vector_t<char> &&name) : kind(kind), type(std::move(type)),
                                                                name(std::move(name)) {
            }
        };

        struct ast_frame_t {
            immutable(ast_frame_t)

            ast_kind_t kind;
            vm::utils::str_t name;
            vm::utils::vector_t<ast_parameter_t *> parameters;

            explicit ast_frame_t(const ast_kind_t kind, vm::utils::str_t &&name,
                                 vm::utils::vector_t<ast_parameter_t *> &&parameters) : kind(kind),
                name(std::move(name)), parameters(std::move(parameters)) {
            }
        };
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
            return parse_error_e(pos(), std::move(vm::utils::cstr2strm_t("Expected ").emplace(vm::utils::cstr2strm_t(lexer::to_string(target))[0, 1, true])));
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

            return new ast_parameter_t(parameter, type.value()->literal.copy(), name.value()->literal.copy());
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

            return new ast_frame_t(frame, name.value()->literal.copy(), std::move(params));
        }
    };
}
