#pragma once

#include <string>
#include <vector>

#include "allocators.h"
#include "defs.h"

namespace utils {
    /** Slices shouldn't own memory. [start;end) */
    template<typename T>
    struct slice_t {
        T *data;

        addr_t data_start, data_end;
        size_t size;

        slice_t(T *data, const addr_t start, const addr_t end) : data(data), data_start(start), data_end(end), size(end - start) {
        }

        slice_t(T *data, const size_t size) : slice_t(data, 0, size) {
        }

        slice_t(const slice_t &other) = default;

        slice_t(slice_t &&other) = delete;

        T *begin() {
           return data + data_start;
        }

        T *end() {
           return data + data_end ;
        }

        T *operator[](addr_t index) {
            if_debug_memory(
                if (data_start + index >= data_end) {
                memory_error("index bounds out of slice")
                }
            )
            return data + data_start + index;
        }

        slice_t operator[](const addr_t from, const addr_t to, const bool back = false) {
            if (back)
                return slice_t(data, from, size - to + 1);
            return slice_t(data, from, to);
        }
    };

    /** There are three type of insert functions:
     * - emplace - creates new object in new cell with a move constructor
     * - push - creates new object in new cell with a copy constructor
     * - place - creates new object in old cell with a move constructor
     */
    template<typename T, typename A = heap_allocator_t<>> requires allocator_c<A>
    struct vector_t {
        typedef T data_type;
        typedef A alloc_type;

        alloc_type allocator;
        size_t size = 0;

        data_type *data() const {
            return reinterpret_cast<data_type *>(allocator.data());
        }

        ~vector_t() {
            if (data())
                erase(operator[](0, size));
        }

        vector_t copy() {
            return vector_t(*this);
        }

        vector_t() : allocator() {
        }

        vector_t(const alloc_type &alloc, const size_t size) : allocator(alloc), size(size) {
        }

        vector_t(alloc_type &&alloc, const size_t size) : allocator(std::move(alloc)), size(size) {
        }

        vector_t(const alloc_type &alloc) : vector_t(alloc, 0) {
        }

        vector_t(alloc_type &&alloc) : vector_t(std::move(alloc), 0) {
        }

        slice_t<data_type> operator[](const addr_t from, const addr_t to, const bool back = false) {
            if (back)
                return slice_t<data_type>(data(), from, size - to + 1);
            return slice_t<data_type>(data(), from, to);
        }

        data_type *operator[](const addr_t ind) const {
            return data() + ind;
        }

        slice_t<data_type> as_slice() {
            return slice_t(data(), 0, size);
        }

        operator slice_t<data_type>() {
            return as_slice();
        }

        vector_t(const vector_t &other)
            : allocator(other.allocator),
              size(other.size) {
            for (int i = 0; i < other.size; i++) place(i, T(*(other.data() + i)));
        }

        vector_t &operator=(const vector_t &other) {
            if (this == &other) return *this;

            this->~vector_t();

            allocator = other.allocator;
            size = other.size;
            for (int i = 0; i < size; i++) place(i, T(*(other.data() + i)));

            return *this;
        }

        vector_t(vector_t &&other) noexcept
            : allocator(std::move(other.allocator)),
              size(other.size) {
        }

        vector_t &operator=(vector_t &&other) noexcept {
            if (this == &other) return *this;

            this->~vector_t();

            allocator = std::move(other.allocator);
            size = other.size;

            return *this;
        }

        vector_t &&move() {
            return std::move(*this);
        }

        data_type *begin() {
            return data();
        }

        data_type *end() {
            return data() + size;
        }

        vector_t &trim() {
            allocator.alloc(size, true);
            return *this;
        }

        vector_t &push(const data_type &t) {
            set_size(size + 1);
            new(static_cast<void *>(data() + size++)) data_type(t);
            return *this;
        }

        vector_t &push(addr_t ind, const data_type &t) {
            set_size(size + 1);
            if (size - ind > 0)
                allocator.move(data() + ind + 1, data() + ind, size++ - ind);
            new(static_cast<void *>(data() + ind)) data_type(t);
            return *this;
        }

        vector_t &push(slice_t<data_type> slice) {
            set_size(size + slice.size);
            for (addr_t i = 0; i < slice.size; i++)
                new(static_cast<void *>(data() + size++)) data_type(*slice[i]);
            return *this;
        }

        vector_t &emplace(data_type &&t) {
            set_size(size + 1);
            new(static_cast<void *>(data() + size++)) data_type(std::move(t));
            return *this;
        }

        vector_t &emplace(addr_t ind, data_type &&t) {
            set_size(size + 1);
            if (size - ind > 0)
                allocator.move(data() + ind + 1, data() + ind, size++ - ind);
            new(static_cast<void *>(data() + ind)) data_type(std::move(t));
            return *this;
        }

        vector_t &emplace(slice_t<data_type> slice) {
            set_size(size + slice.size);
            for (addr_t i = 0; i < slice.size; i++)
                new(static_cast<void *>(data() + size++)) data_type(std::move(*slice[i]));
            return *this;
        }

        vector_t &place(addr_t ind, data_type &&t) {
            new(static_cast<void *>(data() + ind)) data_type(std::move(t));
            return *this;
        }

        vector_t &place(addr_t ind, slice_t<data_type> slice) {
            for (addr_t i = 0; i < slice.size; i++)
                new(static_cast<void *>(data() + ind + i)) data_type(std::move(slice[i]));
            return *this;
        }

        vector_t &set_size(const size_t desired_size) {
            allocator.alloc(sizeof(data_type) * desired_size);
            return *this;
        }

        vector_t &erase(addr_t ind) {
            data_type *p = data() + ind;
            delete_value(p, data_type)
            if (const size_t m = size-- - ind; m > 0)
                memmove(p, p + 1, m);
            return *this;
        }

        vector_t &erase(slice_t<data_type> slice) {
            for (addr_t i = 0; i < slice.size; i++) {
                if constexpr (__is_pointer(data_type))
                    delete *slice[i];
                else slice[i]->~data_type();
            }
            if (const size_t m = size - slice.data_end; m > 0)
                memmove(slice[0], slice[slice.size - 1] + 1, m);
            size -= slice.size;
            return *this;
        }

        bool operator==(const vector_t<data_type> &other) const {
            if (other.size != size)
                return false;

            for (addr_t i = 0; i < size; ++i)
                if (*operator[](i) != *other[i])
                    return false;

            return true;
        }

        bool operator==(const slice_t<data_type> &other) const {
            if (other.size != size)
                return false;

            for (addr_t i = 0; i < size; ++i)
                if (*operator[](i) != *other[i])
                    return false;

            return true;
        }
    };

    typedef vector_t<char> str_t;

    template<typename T> concept is_trivially_to_string = requires(T o)
    {
        { std::to_string(o) } -> std::same_as<std::string>;
    };

    template<typename T, std::string (*parser)(T) = nullptr> requires
        (std::is_same_v<T, char *> || std::is_same_v<T, const char *> || is_trivially_to_string<T> || parser != nullptr)
    constexpr str_t cstr2str(T value) {
        if constexpr (std::is_same_v<T, char *> || std::is_same_v<T, const char *>)
            return str_t().push(slice_t(const_cast<char *>(value), strlen(value)));
        else {
            std::string s;
            if constexpr (parser == nullptr)
                s = std::to_string(value);
            else
                s = parser(value);
            char *str = const_cast<char *>(s.c_str());
            return str_t().push(slice_t(str, 0, strlen(str)));
        }
    }

    template<typename A, typename V> requires allocator_c<A>
    constexpr void write2vec(vector_t<char, A> *vec, V *val) {
        vec->push(slice_t(reinterpret_cast<char *>(val), sizeof(V)));
    }

    template<typename A, typename V> requires allocator_c<A>
    constexpr void write2vec(vector_t<char, A> *vec, V val) {
        vec->push(slice_t(reinterpret_cast<char *>(&val), sizeof(V)));
    }
}
