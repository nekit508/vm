#pragma once

#include <csignal>
#include <cstdio>

#include "defs.h"

namespace vm::utils {
    /** Owns stream if COD is true. */
    template<typename T, bool COD = true>
    struct stream_t {
        FILE *stream;
        addr_t current_pos;
        bool should_update_pos = true;

        stream_t() : stream(nullptr), current_pos(0) {
        }

        explicit stream_t(FILE *stream) : stream(stream), current_pos(0) {
        }

        int operator>>(const size_t n) {
            invalidate_pos();
            return fseek(stream, n, SEEK_CUR);
        }

        int operator<<(const size_t n) {
            invalidate_pos();
            return fseek(stream, -n, SEEK_CUR);
        }

        stream_t &operator=(FILE *new_stream) {
            if (new_stream == nullptr) raise(SIGSEGV);
            if_debug_memory(if (stream != nullptr) memory_error("Unable to reassign stream"))
            stream = new_stream;
            invalidate_pos();

            return *this;
        }

        addr_t pos() {
            if (should_update_pos) {
                should_update_pos = false;
                current_pos = ftell(stream);
            }

            return current_pos;
        }

        void invalidate_pos() {
            should_update_pos = true;
        }

        stream_t(const stream_t &other) = delete;

        stream_t(stream_t &&other) = delete;

        ~stream_t() {
            if constexpr (COD)
                if (stream != nullptr)
                    fclose(stream);
        }

        size_t read(T *data, const size_t n) {
            invalidate_pos();
            return fread(data, sizeof(T), n, stream);
        }

        size_t write(T *data, const size_t n) {
            invalidate_pos();
            return fwrite(data, sizeof(T), n, stream);
        }
    };
}
