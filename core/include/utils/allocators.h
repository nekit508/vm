#pragma once

#include "string.h"

namespace utils {
    template<typename T>
    concept allocator_c = requires(T o, const size_t s, void *vp, bool b)
    {
        { o.alloc(s) } -> std::same_as<void>;
        { o.alloc(s, b) } -> std::same_as<void>;
        { o.data() } -> std::same_as<void *>;
        { o.move(vp, vp, s) } -> std::same_as<void>;
    };

    template<int access_type = PROT_READ | PROT_WRITE>
    struct file_allocator_t {
        FILE *fd = nullptr;
        void *mem = nullptr;
        size_t capacity = 0;

        file_allocator_t() {
        }

        explicit file_allocator_t(FILE *fd) : fd(fd) {
            alloc(0, true);
        }

        file_allocator_t(file_allocator_t &&other) noexcept
            : fd(other.fd),
              mem(other.mem),
              capacity(other.capacity) {
            other.fd = nullptr;
            other.mem = nullptr;
        }

        file_allocator_t &operator=(file_allocator_t &&other) noexcept {
            if (this == &other)
                return *this;
            this->~file_allocator_t();

            fd = other.fd;
            mem = other.mem;
            capacity = other.capacity;

            other.fd = nullptr;
            other.mem = nullptr;

            return *this;
        }

        file_allocator_t(const file_allocator_t &other) = delete;

        file_allocator_t &operator=(const file_allocator_t &other) = delete;

        ~file_allocator_t() {
            if (mem) munmap(mem, capacity);
            if (fd) fclose(fd);
        }

        void alloc(const size_t size, const bool direct = false) {
            if (!mem) {
                fseek(fd, 0, SEEK_END);
                capacity = ftell(fd);
                mem = mmap(nullptr, capacity, access_type, MAP_SHARED | MADV_RANDOM, fileno(fd), 0);
            }
        }

        void *data() const {
            return mem;
        }

        void move(void *dest, void *src, const size_t num) {
            memmove(dest, src, num);
        }
    };

    constexpr size_t heap_allocator_block_size(64);

    template<size_t BS = heap_allocator_block_size> requires (BS != 0)
    struct heap_allocator_t {
        size_t capacity;
        void *mem = nullptr;
        bool del = true;

        heap_allocator_t(void *data, const size_t size, const bool del = true) : capacity(size),
            mem(static_cast<char *>(data)), del(del) {
        }

        heap_allocator_t() : capacity(0) {
        }

        heap_allocator_t(const heap_allocator_t &other)
            : capacity(other.capacity) {
            alloc(capacity, true);
        }

        heap_allocator_t &operator=(const heap_allocator_t &other) {
            if (this == &other)
                return *this;
            this->~heap_allocator_t();
            capacity = other.capacity;
            del = true;
            alloc(capacity, true);
            return *this;
        }

        heap_allocator_t(heap_allocator_t &&other) noexcept
            : capacity(other.capacity),
              mem(other.mem),
              del(other.del) {
            other.mem = nullptr;
        }

        heap_allocator_t &operator=(heap_allocator_t &&other) noexcept {
            if (this == &other)
                return *this;
            this->~heap_allocator_t();
            mem = other.mem;
            capacity = other.capacity;
            del = other.del;
            other.mem = nullptr;
            return *this;
        }

        void alloc(const size_t size, const bool direct = false) {
            if (direct) {
                capacity = size;
                mem = realloc(mem, capacity);
            } else if (size > capacity) {
                capacity = size % BS ? (size / BS + 1) * BS : size;
                mem = realloc(mem, capacity);
            }
        }

        void *data() const {
            return mem;
        }

        void move(void *dest, void *src, const size_t num) {
            memmove(dest, src, num);
        }

        ~heap_allocator_t() {
            if (del && mem) {
                free(mem);
                mem = nullptr;
            }
        }
    };
}
