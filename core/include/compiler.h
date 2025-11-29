#pragma once
#include <fstream>
#include <map>
#include <vector>

#include "ops.h"

/*
namespace compiler {
    typedef std::vector<char> bytes_t;

    struct context_t {
        bytes_t code, code_symbols, const_pool, symbols;
    };

    typedef void (*op_encoder)(context_t *ctx, std::string &line);

    inline std::map<std::string, op_encoder> encoders;

    std::string get_word(std::string &str);

    size_t write_block(std::fstream &stream, const bytes_t &out);

    void write(bytes_t &out, const char *ptr, const size_t n);

    void write_str(bytes_t &out, const std::string &name);

    void translate(const char *in, const char *out);

    template<typename T>
    void skip(bytes_t &out) {
        for (int i = 0; i < sizeof(T); i++)
            out.push_back(0);
    }

    template<typename T>
    void write(bytes_t &out, T data) {
        const auto ptr = reinterpret_cast<char *>(&data);
        out.insert(out.begin() + out.size(), ptr, ptr + sizeof(data));
    }

    template<typename T>
    void write(std::fstream &out, T data) {
        T *ptr = &data;
        out.write(reinterpret_cast<char *>(ptr), sizeof(T));
    }

    template<typename T>
    void param(bytes_t &out, bytes_t &sym, std::string &&param, T (*cons)(std::string &)) {
        if (param.starts_with('$')) {
            write(sym, out.size());
            write_str(sym, param.substr(1));
            skip<T>(out);
        } else {
            write<T>(out, cons(param));
        }
    }

    inline void opcode(context_t *ctx, const opcode_t opcode) {
        write(ctx->code, opcode);
    }

    inline void param_d64(context_t *ctx, std::string &line) {
        param<uint64_t>(ctx->code, ctx->code_symbols, get_word(line), [](std::string &str) { return std::stoul(get_word(str)); });
    }

    inline void param_s64(context_t *ctx, std::string &line) {
        write(ctx->code, std::stoul(get_word(line)));
    }

    inline void param_s32(context_t *ctx, std::string &line) {
        write(ctx->code, static_cast<uint32_t>(std::stoul(get_word(line))));
    }

    inline void param_s16(context_t *ctx, std::string &line) {
        write(ctx->code, static_cast<uint16_t>(std::stoul(get_word(line))));
    }

    inline void param_s8(context_t *ctx, std::string &line) {
        write(ctx->code, static_cast<uint8_t>(std::stoul(get_word(line))));
    }
}
*/