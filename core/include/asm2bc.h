#pragma once
#include <fstream>
#include <map>
#include <vector>

#define opcode(c) vm::tr::write(code, c)
#define param_d64() vm::tr::param<uint64_t>(code, code_symbols, vm::tr::get_word(line), [](const std::string &str) { return std::stoul(str); })
#define param_s64() vm::tr::write(code, static_cast<uint64_t>(std::stoul(vm::tr::get_word(line))))
#define param_s32() vm::tr::write(code, static_cast<uint32_t>(std::stoul(vm::tr::get_word(line))))
#define param_s16() vm::tr::write(code, static_cast<uint16_t>(std::stoul(vm::tr::get_word(line))))
#define param_s8() vm::tr::write(code, static_cast<uint8_t>(std::stoul(vm::tr::get_word(line))))

namespace vm::tr {
    typedef void(*op_encoder)(std::vector<char> &code, std::vector<char> &code_symbols, std::string &line);

    inline std::map<std::string, op_encoder> encoders;

    std::string get_word(std::string &str);

    size_t write_block(std::fstream &stream, const std::vector<char> &out);

    void write(std::vector<char> &out, const char *ptr, const size_t n);

    void write_str(std::vector<char> &out, const std::string &name);

    void translate(const char *in, const char *out);

    template<typename T>
    void write(std::vector<char> &out) {
        for (int i = 0; i < sizeof(T); i++)
            out.push_back(0);
    }

    template<typename T>
    void write(std::vector<char> &out, T data) {
        const auto ptr = reinterpret_cast<char *>(&data);
        out.insert(out.begin() + out.size(), ptr, ptr + sizeof(data));
    }

    template<typename T>
    void write(std::fstream &out, T data) {
        T *ptr = &data;
        out.write(reinterpret_cast<char *>(ptr), sizeof(T));
    }

    template<typename T>
    void param(std::vector<char> &out, std::vector<char> &sym, std::string &&param, T(*cons)(const std::string&)) {
        if (param.starts_with('$')) {
            write(sym, (size_t) out.size());
            write_str(sym, param.substr(1));
            write<T>(out);
        } else {
            write(out, cons(param));
        }
    }
}
