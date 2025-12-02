#pragma once

#include "defs.h"

#define VECTOR_T_DEFAULT_BLOCK_SIZE 64

namespace vm::utils {
    template<typename T, size_t block_size = VECTOR_T_DEFAULT_BLOCK_SIZE>
    struct vector_t;
    typedef vector_t<char> str_t;

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
     * - emplace - creates new object in new cell with a move constructor
     * - push - creates new object in new cell with a copy constructor
     * - place - creates new object in old cell with a move constructor
     */
    template<typename T, size_t block_size>
    struct vector_t {
        size_t size = 0, capacity;
        T *data;

        /** Whether data be erased and freed. */
        bool del = true;

        ~vector_t() {
            if (del && data != nullptr) {
                erase(operator[](0, size));
                free(data);
                data = nullptr;
            }
        }

        vector_t copy() {
            return vector_t(*this);
        }

        vector_t(T *data, const size_t len, const bool del = false) : size(len), capacity(len), data(data), del(del) {
        }

        explicit vector_t(const size_t capacity) : capacity(capacity),
                                                   data(nullptr) {
            resize(capacity);
        }

        vector_t() : vector_t(block_size) {
        }

        slice_t<T> operator[](const addr_t from, const addr_t to, bool back = false) {
            if (back)
                return slice_t<T>(this, from, size - to + 1);
            return slice_t<T>(this, from, to);
        }

        T *operator[](const addr_t ind) const {
            return data + ind;
        }

        vector_t(const vector_t &other)
            : size(other.size),
              capacity(other.size),
              data(static_cast<T *>(malloc(sizeof(T) * capacity))),
              del(other.del) {
            for (int i = 0; i < other.size; ++i) place(i, T(*(other.data + i)));
        }

        vector_t(vector_t &&other) noexcept
            : size(other.size),
              capacity(other.size),
              data(other.data),
              del(other.del) {
            other.data = nullptr;
        }

        vector_t &operator=(vector_t &&other) noexcept {
            if (this == &other) return *this;

            del = other.del;
            size = other.size;
            capacity = other.size;

            this->~vector_t();
            data = other.data;
            other.data = nullptr;
            return *this;
        }

        vector_t &operator=(const vector_t &other) {
            if (this == &other) return *this;

            del = other.del;
            size = other.size;
            capacity = other.capacity;

            this->~vector_t();
            data = static_cast<T *>(malloc(sizeof(T) * capacity));
            for (int i = 0; i < capacity; ++i) place(i, T(*(other.data + i)));

            return *this;
        }

        vector_t &dont_del() {
            del = false;
            return *this;
        }

        vector_t &do_del() {
            del = true;
            return *this;
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
            delete_value(p, T)
            if (const size_t m = size-- - ind; m > 0)
                memmove(p, p + 1, m);
            return *this;
        }

        vector_t &erase(slice_t<T> slice) {
            for (addr_t i = 0; i < slice.size; i++)
                delete_value(slice[i], T)
            if (const size_t m = size - slice.end; m > 0)
                memmove(slice[0], slice[slice.size - 1] + 1, m);
            size -= slice.size;
            return *this;
        }

        bool operator==(const vector_t<T> &other) const {
            if (other.size != size)
                return false;

            for (addr_t i = 0; i < size; ++i)
                if (*operator[](i) != *other[i])
                    return false;

            return true;
        }
    };

    inline str_t cstr2str_t(const char *str) {
        const size_t len = strlen(str);
        return str_t(const_cast<char *>(str), len, false);
    }

    inline str_t cstr2strm_t(const char *str) {
        const size_t len = strlen(str);
        return str_t(const_cast<char *>(str), len, false).copy();
    }


    template<typename T>
    T *slice_t<T>::operator[](const addr_t index) {
        if_debug_memory(
            if (start + index >= end) {
            memory_error("index bounds out of slice")
            }
        )
        return parent->data + start + index;
    }
}
