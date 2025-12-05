#pragma once

#include "utils.h"

namespace lexer {
    inline auto white_space = "\t\n\r ";
    inline auto ident_start = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_$";
    inline auto ident_body = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM_$0123456789";

    inline auto number = "0123456789";

    struct eof_e {};

    enum token_kind_t {
        null = 0,

        ident, num, string,
        falsee, truee,

        fun,
        iff, asmm,

        l_brace, r_brace,
        l_paren, r_paren,
        l_bracket, r_bracket,

        dot, comma, semicolon, colon,

        eof
    };

    const char *to_string(token_kind_t e);

    struct keyword_t {
        vm::utils::str_t name;
        token_kind_t kind;
    };

    inline keyword_t keywords_arr[] = {
        keyword_t(vm::utils::cstr2str_t("fun"), fun),

        keyword_t(vm::utils::cstr2str_t("false"), falsee),
        keyword_t(vm::utils::cstr2str_t("true"), truee),

        keyword_t(vm::utils::cstr2str_t("if"), iff),
        keyword_t(vm::utils::cstr2str_t("asm"), asmm)
    };

    inline vm::utils::vector_t<keyword_t> keywords(vm::utils::heap_allocator_t(keywords_arr, sizeof(keywords_arr) / sizeof(keyword_t), false));

    const char *to_string(token_kind_t e);

    struct token_t {
        addr_t pos;
        token_kind_t kind;
        vm::utils::str_t literal;

        token_t();

        token_t(addr_t pos, token_kind_t kind, vm::utils::str_t &&literal);

        token_t(const token_t &other);

        token_t(token_t &&other) noexcept;

        token_t & operator=(const token_t &other);

        token_t & operator=(token_t &&other) noexcept;

        bool operator==(const token_kind_t &other) const;

        bool operator!=(const token_kind_t &other) const;
    };

    typedef vm::utils::vector_t<token_t> tokens_t;

    struct context_t {
        vm::utils::stream_t<char> stream;

        bool in(char c, const char *list);

        char read_one();

        void skip_until(const char *list);

        void skip_while(const char *list);

        vm::utils::str_t read_until(const char *list);

        vm::utils::str_t read_while(const char *list);

        void skip_white_spaces();

        token_t parse_ident_or_keyword();

        token_t parse_number();

        tokens_t parse();
    };
}
