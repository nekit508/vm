#include "bc_decoder.h"

#include <cstdint>
#include <iostream>
#include <stack>

namespace vm::bcd {
    std::string read_str(utils::buffer &buffer) {
        std::string out;
        uint8_t c;
        while (true) {
            buffer >> c;
            if (c == '\000')
                break;
            out.push_back(c);
        }
        return out;
    }

    void load(context *ctx, const char *file) {
        std::fstream byte_code(file, std::ios_base::binary | std::ios_base::in | std::ios_base::ate);
        const size_t file_size = byte_code.tellg();
        char *ptr = static_cast<char *>(malloc(file_size));
        byte_code.seekg(0);
        byte_code.readsome(ptr, file_size);
        utils::buffer buffer{ptr, file_size, ptr};

        std::vector<size_t> code_symbols_positions;
        std::vector<std::string> code_symbols;

    symbol_table: {
            // read symbol table into temporary map
            size_t const_pool_offset = reinterpret_cast<size_t>(ctx->const_pool.data) + ctx->const_pool.write_pos;
            size_t code_offset = reinterpret_cast<size_t>(ctx->code.data) + ctx->code.write_pos;

            size_t block_size;
            buffer >> block_size;
            char *block_end = buffer.pos + block_size;

            uint8_t block_id;
            size_t in_block_pos;
            size_t offset = 0;
            while (buffer.pos < block_end) {
                auto str = std::move(read_str(buffer));
                buffer >> block_id;
                buffer >> in_block_pos;

            offset: {
                    if (block_id == 1) {
                        offset = const_pool_offset;
                    } else if (block_id == 2) {
                        offset = code_offset;
                    } else {
                        std::cerr << "unknown block id " << block_id << std::endl;
                        raise(SIGILL);
                    }
                }

                ctx->symbol_table[str] = in_block_pos + offset;
            }
        }

    const_data: {
            // read const data
            size_t block_size;
            buffer >> block_size;
            for (size_t i = 0; i < block_size; i++) {
                buffer >> *(ctx->const_pool.data + ctx->const_pool.write_pos++);
            }
        }

    code_symbols: {
            size_t block_size;
            buffer >> block_size;
            char *block_end = buffer.pos + block_size;
            size_t pos;
            while (buffer.pos < block_end) {
                buffer >> pos;
                code_symbols_positions.push_back(pos);
                code_symbols.push_back(read_str(buffer));
            }
        }

    code: {
            size_t code_symbols_pos = 0;
            // read code
            size_t block_size, code_symbols_size = code_symbols_positions.size();
            buffer >> block_size;
            for (size_t i = 0; i < block_size; i++) {
                if (code_symbols_pos < code_symbols_size && i == code_symbols_positions[code_symbols_pos]) {
                    auto symbol = code_symbols[code_symbols_pos];
                    if (!ctx->symbol_table.contains(symbol)) {
                        std::cerr << "Symbol " << symbol << " was not founded in current context!" << std::endl;
                        raise(SIGILL);
                    }
                    *reinterpret_cast<size_t *>(ctx->code.data + ctx->code.write_pos) = ctx->symbol_table[symbol];
                    code_symbols_pos++;
                    // skip bytes
                    i += sizeof(size_t) - 1;
                    ctx->code.write_pos += sizeof(size_t);
                    size_t _;
                    buffer >> _;
                } else {
                    buffer >> *(ctx->code.data + ctx->code.write_pos++);
                }
            }
        }

        return;
    }
}
