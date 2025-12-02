#pragma once

#include <csignal>
#include <cstdio>
#include <cstdlib>

namespace vm::utils {
    struct stack_t {
        char *start, *end, *pos;

        explicit stack_t(const size_t size) : stack_t(static_cast<char *>(malloc(size)), size) {
        }

        stack_t(char *start, const size_t size) : stack_t(start, size, start + size) {
        }

        stack_t(char *start, const size_t size, char *pos) : start(start), end(start + size), pos(pos) {
        }

        stack_t(stack_t &&other) noexcept : start(other.start), end(other.end), pos(other.pos) {
        }

        stack_t(const stack_t &other) = delete;

        ~stack_t() {
            free(start);
        }

        stack_t &operator=(stack_t &&other) noexcept {
            start = other.start;
            end = other.end;
            pos = other.pos;
            other.start = nullptr;

            return *this;
        }

        template<typename T>
        stack_t &operator<<(const T &other) {
            if (!validate_pos(sizeof(T)))
                raise(SIGSEGV);

            pos -= sizeof(T);
            *reinterpret_cast<T *>(pos) = other;

            return *this;
        }

        template<typename T>
        stack_t &operator<<=(const T &other) {
            if (!validate_pos(sizeof(T)))
                raise(SIGSEGV);

            *reinterpret_cast<T *>(pos) = other;
            return *this;
        }

        template<typename T>
        stack_t &operator>>(T &other) {
            if (!validate_pos(0, sizeof(T)))
                raise(SIGSEGV);

            other = *reinterpret_cast<T *>(pos);
            pos += sizeof(other);

            return *this;
        }

        template<typename T>
        stack_t &operator>>=(T &other) {
            if (!validate_pos(0, sizeof(T)))
                raise(SIGSEGV);

            other = *reinterpret_cast<T *>(pos);
            return *this;
        }

        bool validate_pos(const size_t left_offset = 0, const size_t right_offset = 0) const {
            return pos >= start + left_offset && pos <= end - right_offset;
        }

        void dump(const char *file) const {
            const auto f = fopen(file, "w");
            fwrite(start, sizeof(char), start - end, f);
            fclose(f);
        }
    };
}
