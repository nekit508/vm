#pragma once

#include <algorithm>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "vm_types.h"

#define debug_memory true
#define if_debug_memory(code) \
    if constexpr (debug_memory) { \
        code \
    }
#define memory_error(message) {fputs(message, stderr);fputs("\n", stderr);raise(SIGSEGV);}

namespace vm::utils {
    template<typename T, size_t block_size = 64>
    struct vector_t;

    /** Slices shouldn't own memory. [start;end) */
    template<typename T>
    struct slice_t {
        vector_t<T> *parent;

        addr_t start, end;
        size_t size;

        slice_t(vector_t<T> *parent, const addr_t start, const addr_t end) : parent(parent), start(start), end(end) {
            size = end - start;
        }

        slice_t(const slice_t &other) = default;

        slice_t(slice_t &&other) = delete;

        T *operator[](addr_t index);
    };

    /** There are three type of insert functions:
     * - emplace - creates new object in new cell at index and calls a move constructor
     * - push - creates new object in new cell at index and calls a copy constructor
     * - place - creates new object in old cell and calls a move constructor
     */
    template<typename T, size_t block_size>
    struct vector_t {
        size_t size = 0, capacity;
        T *data;

        explicit vector_t(const size_t capacity) : capacity(capacity),
                                                   data(nullptr) {
            resize(capacity);
        }

        vector_t() : vector_t(block_size) {
        }

        slice_t<T> operator[](const addr_t from, const addr_t to) {
            return slice_t<T>(this, from, to);
        }

        T &operator[](const addr_t ind) {
            return *(data + ind);
        }

        vector_t(const vector_t &other)
            : size(other.size),
              capacity(other.size),
              data(static_cast<T *>(malloc(sizeof(T) * capacity))) {
            for (int i = 0; i < other.size; ++i) place(i, T(*(other.data + i)));
        }

        vector_t(vector_t &&other) noexcept
            : size(other.size),
              capacity(other.size),
              data(other.data) {
            other.data = nullptr;
        }

        vector_t & operator=(vector_t &&other) noexcept {
            if (this == &other)
                return *this;
            size = other.size;
            capacity = other.size;

            free(data);
            data = other.data;
            other.data = nullptr;
            return *this;
        }

        vector_t & operator=(const vector_t &other) {
            if (this == &other)
                return *this;
            size = other.size;
            capacity = other.capacity;

            free(data);
            data = static_cast<T *>(malloc(sizeof(T) * capacity));
            for (int i = 0; i < capacity; ++i) place(i, T(*(other.data + i)));

            return *this;
        }

        ~vector_t() {
            if (data != nullptr) {
                erase(this->operator[](0, size));
                free(data);
            }
        }

        /** Deattaches array. */
        T *unbind() {
            capacity = 0;
            size = 0;
            auto *out = data;
            data = nullptr;
            return out;
        }

        T *begin() {
            return data;
        }

        T *end() {
            return data + size;
        }

        vector_t &resize(size_t new_capacity) {
            if (new_capacity == 0)
                new_capacity = 1;
            capacity = new_capacity;
            data = static_cast<T *>(realloc(data, sizeof(T) * capacity));
            return *this;
        }

        vector_t &trim() {
            resize(size);
            return *this;
        }

        vector_t &push(const T &t) {
            shrink_if_needed(size + 1, block_size);
            new(static_cast<void *>(data + size++)) T(t);
            return *this;
        }

        vector_t &push(addr_t ind, const T &t) {
            shrink_if_needed(size + 1, block_size);
            if (size - ind > 0)
                memmove(data + ind + 1, data + ind, size++ - ind);
            new(static_cast<void *>(data + ind)) T(t);
            return *this;
        }

        vector_t &push(slice_t<T> slice) {
            shrink_if_needed(size + slice.size, block_size);
            for (addr_t i = 0; i < slice.size; i++)
                new(static_cast<void *>(data + size++)) T(*slice[i]);
            return *this;
        }

        vector_t &emplace(T &&t) {
            shrink_if_needed(size + 1, block_size);
            new(static_cast<void *>(data + size++)) T(std::move(t));
            return *this;
        }

        vector_t &emplace(addr_t ind, T &&t) {
            shrink_if_needed(size + 1, block_size);
            if (size - ind > 0)
                memmove(data + ind + 1, data + ind, size++ - ind);
            new(static_cast<void *>(data + ind)) T(std::move(t));
            return *this;
        }

        vector_t &emplace(slice_t<T> slice) {
            shrink_if_needed(size + slice.size, block_size);
            for (addr_t i = 0; i < slice.size; i++)
                new(static_cast<void *>(data + size++)) T(std::move(*slice[i]));
            return *this;
        }

        vector_t &place(addr_t ind, T &&t) {
            new(static_cast<void *>(data + ind)) T(std::move(t));
            return *this;
        }

        vector_t &shrink_if_needed(const size_t desired_size, const size_t add_size) {
            if (desired_size > capacity)
                resize((desired_size / add_size + (desired_size % add_size != 0 ? 1 : 0)) * add_size);
            return *this;
        }

        vector_t &erase(addr_t ind) {
            T *p = data + ind;
            p->~T();
            if (const size_t m = size-- - ind; m > 0)
                memmove(p, p + 1, m);
            return *this;
        }

        vector_t &erase(slice_t<T> slice) {
            for (addr_t i = 0; i < slice.size; i++)
                slice[i]->~T();
            if (const size_t m = size - slice.end; m > 0)
                memmove(slice[0], slice[slice.size - 1] + 1, m);
            size -= slice.size;
            return *this;
        }

        bool operator==(const vector_t &other) const {
            if (other.size != size)
                return false;

            for (addr_t i = 0; i < size; ++i)
                if (operator[](i) != other[i])
                    return false;

            return true;
        }
    };

    template<typename T>
    T *slice_t<T>::operator[](const addr_t index) {
        if_debug_memory(if (start + index >= end) {
            memory_error("index bounds out of slice")
            })
        return parent->data + start + index;
    }

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

    /** Wraps data and provides access to it as input buffer, stack or output buffer. */
    template<bool ro = false>
    struct memory_t {
        addr_t get_read_pos() const {
            return read_pos;
        }

        void set_read_pos(const addr_t read_pos) {
            this->read_pos = read_pos;
        }

        addr_t get_write_pos() const {
            return write_pos;
        }

        void set_write_pos(const addr_t write_pos) {
            this->write_pos = write_pos;
        }

        addr_t get_stack_pos() const {
            return stack_pos;
        }

        void set_stack_pos(const addr_t stack_pos) {
            this->stack_pos = stack_pos;
        }

        char *data = nullptr;

        size_t size = 0;
        addr_t read_pos = 0, write_pos = 0, stack_pos = 0;

        explicit memory_t(const size_t size) : data(static_cast<char *>(malloc(size))), size(size) {
        }

        explicit memory_t(const memory_t &other) : size(other.size) {
            if constexpr (ro)
                data = other.data;
            else {
                data = static_cast<char *>(malloc(size));
                memcpy(data, other.data, size);
            }
        }

        memory_t(memory_t &&other) noexcept {
            *this = std::move(other);
        }

        memory_t &operator=(memory_t &&other) noexcept {
            size = other.size;
            data = other.data;

            read_pos = other.read_pos;
            write_pos = other.write_pos;
            stack_pos = other.stack_pos;

            other.data = nullptr;
            return *this;
        }

        ~memory_t() {
            if constexpr (!ro)
                if (data != nullptr)
                    free(data);
        }

        const char *operator[](const addr_t pos) const {
            return data + pos;
        }

        /** Buffer output. */
        template<typename T>
        memory_t &operator>>(T &value) {
            if_debug_memory(if (read_pos > size - sizeof(T)) {
                memory_error("unable to read from buffer")
                })
            value = *reinterpret_cast<T *>(data + read_pos);
            read_pos += sizeof(T);
            return *this;
        }

        /** Buffer input. */
        template<typename T>
        memory_t &operator<<(const T &value) {
            static_assert(!ro, "Writing to read only memory");
            if_debug_memory(if (write_pos > size - sizeof(T)) {
                memory_error("unable write to buffer")
                })
            *reinterpret_cast<T *>(data + write_pos) = value;
            write_pos += sizeof(T);
            return *this;
        }

        /** Stack output. */
        template<typename T>
        memory_t &operator>>=(T &value) {
            if_debug_memory(if (stack_pos < sizeof(T)) {
                memory_error("unable to read from stack");
                })
            stack_pos -= sizeof(T);
            value = *reinterpret_cast<T *>(data + stack_pos);
            return *this;
        }

        /** Stack input. */
        template<typename T>
        memory_t &operator<<=(const T &value) {
            static_assert(!ro, "Writing to read only memory");
            if_debug_memory(if (stack_pos > size - sizeof(T)) {
                memory_error("unable to write to stack")
                })
            *reinterpret_cast<T *>(data + stack_pos) = value;
            stack_pos += sizeof(T);
            return *this;
        }
    };

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
