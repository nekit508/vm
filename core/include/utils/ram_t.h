#pragma once

#include <cstdio>
#include <sys/mman.h>

#include "defs.h"

namespace utils {
    struct ram_file_t {
        FILE *fp;
        int fd;
        char *data;
        size_t size;

        explicit ram_file_t(FILE *fd) : fp(fd), fd(fileno(fp)) {
            fseek(fd, 0, SEEK_END);
            size = ftell(fd);
            data = static_cast<char *>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, 0));
        }

        ram_file_t(const ram_file_t &other) = delete;

        ram_file_t &operator=(const ram_file_t &other) = delete;

        ram_file_t(ram_file_t &&other) noexcept
            : fp(other.fp),
              fd(other.fd),
              data(other.data),
              size(other.size) {
            other.data = nullptr;
        }

        ram_file_t &operator=(ram_file_t &&other) noexcept {
            if (this == &other)
                return *this;
            fp = other.fp;
            fd = other.fd;
            data = other.data;
            size = other.size;
            other.data = nullptr;
            return *this;
        }

        ~ram_file_t() {
            if (data != nullptr) {
                munmap(data, size);
                fclose(fp);
                data = nullptr;
            }
        }

        char *operator[](addr_t pos) {
            return data + pos;
        }
    };
}