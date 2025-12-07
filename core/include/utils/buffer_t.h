#pragma once

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "defs.h"

namespace utils {
    struct buffer_t {
        char *data;
        size_t size;
        addr_t read_pos, write_pos;

        buffer_t(const char *src, const size_t len) : data(static_cast<char *>(malloc(len))), size(len), read_pos(0),
                                                      write_pos(0) {
            memcpy(data, src, len);
        }

        buffer_t(const buffer_t &other) = delete;

        explicit buffer_t(const size_t size) : data(static_cast<char *>(malloc(size))), size(size), read_pos(0),
                                               write_pos(0) {
        }

        buffer_t(buffer_t &&other) noexcept : data(other.data), size(other.size),
                                              read_pos(other.read_pos), write_pos(other.write_pos) {
            other.data = nullptr;
        }

        ~buffer_t() {
            free(data);
        }

        template<typename T>
        buffer_t &operator>>(T &other) {
            if (!validate_pos())
                raise(SIGSEGV);

            other = *reinterpret_cast<T *>(data + read_pos);
            read_pos += sizeof(other);
            return *this;
        }

        buffer_t &operator<<(const addr_t new_pos) {
            read_pos = new_pos;
            return *this;
        }

        bool validate_pos() const {
            return read_pos <= size;
        }

        void dump(const char *file) const {
            const auto f = fopen(file, "w");
            fwrite(data, sizeof(char), size, f);
            fclose(f);
        }
    };
}