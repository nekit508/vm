#pragma once

#include <vector>

#include "defs.h"

namespace vm::utils {
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
                const size_t s = size / BS;
                capacity = (size % BS ? s + 1 : s) * BS;
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

    template<typename T>
    concept allocator_c = requires(T o, const size_t s, void *vp, bool b)
    {
        { o.alloc(s) } -> std::same_as<void>;
        { o.alloc(s, b) } -> std::same_as<void>;
        { o.data() } -> std::same_as<void *>;
        { o.move(vp, vp, s) } -> std::same_as<void>;
    };

    template<typename T, typename A = heap_allocator_t<> >
        requires (allocator_c<A>)
    struct vector_t;

    typedef vector_t<char> str_t;

    /** Slices shouldn't own memory. [start;end) */
    template<typename T>
    struct slice_t {
        T *data;

        addr_t start, end;
        size_t size;

        slice_t(T *data, const addr_t start, const addr_t end) : data(data), start(start), end(end) {
            size = end - start;
        }

        slice_t(const slice_t &other) = default;

        slice_t(slice_t &&other) = delete;

        T *operator[](addr_t index) {
            if_debug_memory(
                if (start + index >= end) {
                memory_error("index bounds out of slice")
                }
            )
            return data + start + index;
        }
    };

    /** There are three type of insert functions:
     * - emplace - creates new object in new cell with a move constructor
     * - push - creates new object in new cell with a copy constructor
     * - place - creates new object in old cell with a move constructor
     */
    template<typename T, typename A> requires (allocator_c<A>)
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

        vector_t() : allocator(alloc_type()) {
        }

        vector_t(const alloc_type &alloc) : allocator(alloc),
                                            size(alloc.capacity) {
        }

        vector_t(alloc_type &&alloc) : allocator(std::move(alloc)),
                                       size(alloc.capacity) {
        }

        slice_t<data_type> operator[](const addr_t from, const addr_t to, bool back = false) {
            if (back)
                return slice_t<data_type>(data(), from, size - to + 1);
            return slice_t<data_type>(data(), from, to);
        }

        data_type *operator[](const addr_t ind) const {
            return data() + ind;
        }

        vector_t(const vector_t &other)
            : allocator(other.allocator),
              size(other.size) {
            for (int i = 0; i < other.size; ++i) place(i, T(*(other.data() + i)));
        }

        vector_t &operator=(const vector_t &other) {
            if (this == &other) return *this;

            this->~vector_t();

            allocator = other.allocator;
            size = other.size;
            for (int i = 0; i < size; ++i) place(i, T(*(other.data() + i)));

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
            if (const size_t m = size - slice.end; m > 0)
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
    };

    constexpr str_t::alloc_type heap_allocator_from_str(const char *str) {
        const size_t len = strlen(str);
        return str_t::alloc_type(const_cast<char *>(str), len, false);
    }

    inline str_t cstr2str_t(const char *str) {
        return str_t(heap_allocator_from_str(str));
    }

    inline str_t cstr2strm_t(const char *str) {
        return str_t(heap_allocator_from_str(str)).copy();
    }
}
