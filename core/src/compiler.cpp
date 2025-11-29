#include "compiler.h"

#include <csignal>
#include <map>

#include <iostream>

#include "ops.h"

/*namespace compiler {
    std::string process_string(const std::string &str) {
        std::string out;
        bool shielded = false;
        for (const char c: str) {
            if (!shielded) {
                if (c == '\\')
                    shielded = true;
                else
                    out.push_back(c);
            } else {
                if (c == '\\')
                    out.push_back(c);
                else if (c == 'n')
                    out.push_back('\n');
                else {
                    std::cerr << "wrong escape '" << c << "'." << std::endl;
                    raise(SIGILL);
                }
                shielded = false;
            }
        }

        return out;
    }

    std::string get_word(std::string &str) {
        size_t ind = str.find(' ');

        if (ind == std::string::npos) ind = str.size();

        std::string out = str.substr(0, ind);
        str.erase(0, ind + 1);

        return out;
    }

    size_t write_block(std::fstream &stream, const std::vector<char> &out) {
        write(stream, (uint64_t) out.size());
        stream.write(out.data(), out.size());
        return out.size() + sizeof(uint64_t);
    }

    void write_str(std::vector<char> &out, const std::string &name) {
        write(out, name.c_str(), name.size());
        write(out, static_cast<uint8_t>('\000'));
    }

    void write(std::vector<char> &out, const char *ptr, const size_t n) {
        out.insert(out.begin() + out.size(), ptr, ptr + n);
    }

    void translate(const char *in, const char *out) {
        context_t ctx;

        std::fstream source_stream(in, std::ios_base::in);
        std::fstream compiled_file(out, std::ios_base::out | std::ios_base::binary);

        uint64_t line_counter = 0;
        // 0 - nothing
        // 1 - const pool
        // 2 - code
        uint8_t block = 0;
        for (std::string line{}; std::getline(source_stream, line);) {
            line_counter++;

            if (line.starts_with("//")) {
                // just skip comment
            } else if (line.starts_with('.')) {
                line = line.substr(1);
                if (line == "const-pool")
                    block = 1;
                else if (line == "code")
                    block = 2;
            } else if (line.size() != 0) {
                if (block == 1) {
                    auto name = get_word(line);
                    get_word(line); // skip =

                    size_t ptr = ctx.const_pool.size();

                    if (line.starts_with("\"")) {
                        // it's a string
                        auto string_value = line.substr(1, line.size() - 2);
                        string_value.push_back('\000'); // yes, this is null terminated string
                        string_value = process_string(string_value);
                        write(ctx.const_pool, string_value.c_str(), string_value.size());
                    }

                    write(ctx.symbols, name.c_str(), name.size());
                    write(ctx.symbols, static_cast<uint8_t>('\000'));
                    write<uint8_t>(ctx.symbols, block);
                    write<size_t>(ctx.symbols, ptr);
                } else if (block == 2) {
                    if (auto command = get_word(line); command.starts_with(':')) {
                        auto label_name = command.substr();
                        write(ctx.symbols, label_name.c_str(), label_name.size());
                        write(ctx.symbols, static_cast<uint8_t>('\000'));
                        write<uint8_t>(ctx.symbols, block);
                        write<size_t>(ctx.symbols, ctx.code.size());
                    } else {
                        if (!encoders.contains(command)) {
                            std::cerr << "unknown command " << command << " at line " << line_counter << std::endl;
                            raise(SIGILL);
                        } else {
                            encoders[command](&ctx, line);
                        }
                    }
                }
            }
        }

        {
            write_block(compiled_file, ctx.symbols);
            write_block(compiled_file, ctx.const_pool);
            write_block(compiled_file, ctx.code_symbols);
            write_block(compiled_file, ctx.code);
        }

        source_stream.close();
        compiled_file.close();
    }
}
*/