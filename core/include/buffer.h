#pragma once

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <vector>

template<typename _Tp>
std::remove_reference_t<_Tp> &&move(_Tp &&__t) { return std::move(__t); }

namespace vm::utils {
    struct straight_buffer {
        char *data;
        size_t size, read_pos, write_pos;

        straight_buffer(const straight_buffer &other) = delete;

        explicit straight_buffer(const size_t size) : data(static_cast<char *>(malloc(size))), size(size), read_pos(0),
                                                      write_pos(0) {
        }

        straight_buffer(straight_buffer &&other) noexcept : data(other.data), size(other.size),
                                                            read_pos(other.read_pos), write_pos(other.write_pos) {
            other.data = nullptr;
        }

        ~straight_buffer() {
            free(data);
        }

        template<typename T>
        straight_buffer &operator>>(T &other) {
            if (!validate_pos())
                raise(SIGSEGV);

            other = *reinterpret_cast<T *>(data + read_pos);
            read_pos += sizeof(other);
            return *this;
        }

        straight_buffer &operator<<(const size_t new_pos) {
            read_pos = new_pos;
            return *this;
        }

        bool validate_pos() const {
            return read_pos <= size;
        }

        void dump(const char *file) const {
            std::fstream stream(file, std::ios_base::out | std::ios_base::binary);
            stream.write(data, size);
            stream.close();
        }
    };

    struct buffer {
        char *start, *end, *pos;

        buffer() : start(nullptr), end(nullptr), pos(nullptr) {
        }

        buffer(char *start, const size_t size) : buffer(start, size, start + size) {
        }

        buffer(char *start, const size_t size, char *pos) : start(start), end(start + size), pos(pos) {
        }

        buffer(buffer &&other) noexcept : start(other.start), end(other.end), pos(other.pos) {
        }

        buffer(const buffer &other) = delete;

        ~buffer() {
            free(start);
        }

        buffer &operator=(buffer &&other) noexcept {
            start = other.start;
            end = other.end;
            pos = other.pos;
            other.start = nullptr;

            return *this;
        }

        template<typename T>
        buffer &operator<<(const T &other) {
            if (!validate_pos(sizeof(T)))
                raise(SIGSEGV);

            pos -= sizeof(T);
            *reinterpret_cast<T *>(pos) = other;

            return *this;
        }

        template<typename T>
        buffer &operator<<=(const T &other) {
            if (!validate_pos(sizeof(T)))
                raise(SIGSEGV);

            *reinterpret_cast<T *>(pos) = other;
            return *this;
        }

        template<typename T>
        buffer &operator>>(T &other) {
            if (!validate_pos(0, sizeof(T)))
                raise(SIGSEGV);

            other = *reinterpret_cast<T *>(pos);
            pos += sizeof(other);

            return *this;
        }

        template<typename T>
        buffer &operator>>=(T &other) {
            if (!validate_pos(0, sizeof(T)))
                raise(SIGSEGV);

            other = *reinterpret_cast<T *>(pos);
            return *this;
        }

        bool validate_pos(const size_t left_offset = 0, const size_t right_offset = 0) const {
            return pos >= start + left_offset && pos <= end - right_offset;
        }

        void dump(const char *file) const {
            std::fstream stream(file, std::ios_base::out | std::ios_base::binary);
            stream.write(start, end - start);
            stream.close();
        }
    };
}
