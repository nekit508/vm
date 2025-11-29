#pragma once

#include <vector>

#include "memory.h"

namespace compiler {
    inline auto white_space = "\t\n\r ";
    inline auto ident_start = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_$";
    inline auto ident_body = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_$0123456789";

    struct context_t {
        vm::utils::stream_t<char> stream;
    };

    inline char read_one(context_t *ctx) {
        char out;
        ctx->stream.read(&out, 1);
        return out;
    }

    inline void skip_all(context_t *ctx, const char *to_skip) {
        for (bool not_skipped = true; not_skipped;) {
            char c = read_one(ctx);

            not_skipped = false;
            for (const char *p = to_skip; *p; p++)
                if (c == *p) {
                    not_skipped = false;
                    break;
                }
        }

        ctx->stream << 1;
    }

    inline bool contained(context_t *ctx, char c, const char *list) {
        for (const char *l = list; *l; l++)
            if (c == *l)
                return true;
        return false;
    }

    inline void skip_white_spaces(context_t *ctx) {
        skip_all(ctx, white_space);
    }

    inline char* read_until(context_t *ctx, const char *list) {
        vm::utils::vector_t<char> out;

        for (bool exit = false; !exit;) {
            char c = read_one(ctx);
            for (const char *p = list; *p; p++) {
                if (c == *p) {
                    exit = true;
                    out.push(c);
                    break;
                }
            }
        }

        ctx->stream << 1;

        return out.unbind();
    }

    inline vm::utils::vector_t<char> read_while(context_t *ctx, const char *list) {
        vm::utils::vector_t<char> out;

        for (bool exit = false; !exit;) {
            char c = read_one(ctx);
            exit = true;
            for (const char *p = list; *p; p++) {
                if (c == *p) {
                    exit = false;
                    out.push(c);
                    break;
                }
            }
        }

        ctx->stream << 1;

        return out;
    }

    inline void parse(context_t *ctx) {
        skip_white_spaces(ctx);

        if (char c = read_one(ctx); contained(ctx, c, ident_start)) {
            vm::utils::vector_t vector(move(read_while(ctx, ident_body)));

            vector.push(0, c);
            vector.trim();

            if (!strcmp(vector.data, "fun")) {
                fputs("sdafasdf", stdout);
            }
        }
    }
}
