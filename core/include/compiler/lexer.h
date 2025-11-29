#pragma once

#include <algorithm>
#include <vector>

#include "memory.h"

namespace compiler {
    typedef vm::utils::vector_t<char> str_t;

    inline auto white_space = "\t\n\r ";
    inline auto ident_start = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_$";
    inline auto ident_body = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_$0123456789";

    struct eof_e {};

    enum token_kind_t {
        null = 0,

        ident, num, string, boolean,

        fun,

        eof
    };

    struct token_t {
        addr_t pos;
        token_kind_t kind;
        str_t literal;

        token_t() : pos(0), kind(null), literal(0) {
        }

        token_t(const addr_t pos, const token_kind_t kind, str_t &&literal)
            : pos(pos),
              kind(kind),
              literal(std::move(literal)) {
        }

        token_t(const token_t &other) {
            *this = other;
        }

        token_t(token_t &&other) noexcept {
            *this = std::move(other);
        }

        token_t & operator=(const token_t &other) {
            if (this == &other)
                return *this;
            pos = other.pos;
            kind = other.kind;
            literal = other.literal;
            return *this;
        }

        token_t & operator=(token_t &&other) noexcept {
            if (this == &other)
                return *this;
            pos = other.pos;
            kind = other.kind;
            literal = std::move(other.literal);
            return *this;
        }
    };

    typedef vm::utils::vector_t<token_t> tokens_t;

    struct context_t {
        vm::utils::stream_t<char> stream;

        bool in(const char c, const char *list) {
            for (const char *l = list; *l; l++)
                if (c == *l)
                    return true;
            return false;
        }

        char read_one() {
            char out;
            if (stream.read(&out, 1) == EOF) throw eof_e();
            return out;
        }

        void skip_all(const char *to_skip) {
            for (bool not_skipped = true; not_skipped;) {
                char c = read_one();

                not_skipped = false;
                for (const char *p = to_skip; *p; p++)
                    if (c == *p) {
                        not_skipped = false;
                        break;
                    }
            }

            stream << 1;
        }

        str_t read_until(const char *list) {
            str_t out;

            for (bool exit = false; !exit;) {
                char c = read_one();
                for (const char *p = list; *p; p++)
                    if (c == *p) {
                        exit = true;
                        out.push(c);
                        break;
                    }
            }

            stream << 1;

            return out;
        }

        str_t read_while(const char *list) {
            str_t out;

            for (bool exit = false; !exit;) {
                char c = read_one();
                exit = true;
                for (const char *p = list; *p; p++)
                    if (c == *p) {
                        exit = false;
                        out.push(c);
                        break;
                    }
            }

            stream << 1;

            return out;
        }

        void skip_white_spaces() {
            skip_all(white_space);
        }

        token_t read_ident_or_keyword() {
            token_kind_t kind = fun;
            str_t literal(std::move(read_while(ident_body)));
            literal.trim();
            return token_t{stream.pos(), kind, std::move(literal)};
        }

        tokens_t parse() {
            tokens_t out{};

            try {
                for (;;) {
                    skip_white_spaces();
                    char c = read_one();
                    stream << 1;

                    if (in(c, ident_start)) {
                        out.emplace(std::move(read_ident_or_keyword()));
                    }

                }
            } catch (const eof_e &e) {
                out.emplace(token_t(stream.pos(), eof, str_t(0)));
            }

            return out;
        }
    };
}
