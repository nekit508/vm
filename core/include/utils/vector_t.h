#pragma once

#include <string>

#include "allocators.h"
#include "defs.h"

namespace utils {
    template<typename T, bool owns = true>
    struct container_t {
        static_assert(owns || std::is_pointer_v<T>, "Value contained objects must be owned!");
        T data;

        operator T() {
            return data;
        }

        T operator*() {
            return data;
        }

        container_t() : data(T()) {
        }

        container_t(const T &data) : data(data) {
        }

        container_t(T &&data) : data(std::move(data)) {
        }

        container_t(const container_t &other)
            : data(other.data) {
            static_assert(not (owns && std::is_pointer_v<T>), "Owned pointer cannot be copied.");
        }

        container_t(container_t &&other) noexcept
            : data(std::move(other.data)) {
            if constexpr (std::is_pointer_v<T>)
                other.data = nullptr;
        }

        container_t &operator=(const container_t &other) {
            static_assert(not (owns && std::is_pointer_v<T>), "Owned pointer cannot be copied.");

            if (this == &other)
                return *this;

            this->~container_t();

            data = other.data;
            return *this;
        }

        container_t & operator=(container_t &&other) noexcept {
            if (this == &other)
                return *this;

            this->~container_t();

            data = std::move(other.data);
            if constexpr (std::is_pointer_v<T>)
                other.data = nullptr;

            return *this;
        }

        ~container_t() {
            if constexpr (owns && std::is_pointer_v<T>)
                if (data != nullptr)
                    delete data;
        }

        container_t &&move() {
            return std::move(*this);
        }
    };

    /** Slices shouldn't own memory. [start;end) */
    template<typename T>
    struct slice_t {
        typedef T data_type;
        data_type *data;

        addr_t data_start, data_end;
        size_t size;

        slice_t(data_type *data, const addr_t start, const addr_t end) : data(data), data_start(start), data_end(end),
                                                                 size(end - start) {
        }

        slice_t(data_type *data, const size_t size) : slice_t(data, 0, size) {
        }

        slice_t(const slice_t &other) = default;

        slice_t(slice_t &&other) = delete;

        data_type *begin() {
            return data + data_start;
        }

        data_type *end() {
            return data + data_end;
        }

        data_type *operator[](addr_t index) {
            if_debug_memory(
                if (data_start + index >= data_end) {
                memory_error("index bounds out of slice")
                }
            )
            return data + data_start + index;
        }

        data_type &get(addr_t index) {
            return *(data + data_start + index);
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
    template<typename T, typename A = heap_allocator_t<>, typename C = container_t<T> > requires allocator_c<A>
    struct vector_t {
        static_assert(sizeof(C) == sizeof(T));

        typedef T data_type;
        typedef C container_type;

        static constexpr container_type *container(data_type *data) {
            return reinterpret_cast<container_type *>(data);
        }
        static constexpr size_t data_size = sizeof(T);

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

        data_type &get(const addr_t ind) {
            return *(data() + ind);
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
            for (int i = 0; i < other.size; i++) place(i, container_type(*other[i]));
        }

        vector_t &operator=(const vector_t &other) {
            if (this == &other) return *this;

            this->~vector_t();

            allocator = other.allocator;
            size = other.size;
            for (int i = 0; i < size; i++) place(i, container_type(*other[i]));

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
            new(static_cast<void *>(data() + size++)) container_type(t);
            return *this;
        }

        vector_t &push(addr_t ind, const data_type &t) {
            set_size(size + 1);
            if (size - ind > 0)
                allocator.move(data() + ind + 1, data() + ind, size++ - ind);
            new(static_cast<void *>(data() + ind)) container_type(t);
            return *this;
        }

        vector_t &push(slice_t<data_type> slice) {
            set_size(size + slice.size);
            for (addr_t i = 0; i < slice.size; i++)
                new(static_cast<void *>(data() + size++)) container_type(*slice[i]);
            return *this;
        }

        vector_t &emplace(data_type &&t) {
            set_size(size + 1);
            new(static_cast<void *>(data() + size++)) container_type(std::move(t));
            return *this;
        }

        vector_t &emplace(addr_t ind, data_type &&t) {
            set_size(size + 1);
            if (size - ind > 0)
                allocator.move(data() + ind + 1, data() + ind, size++ - ind);
            new(static_cast<void *>(data() + ind)) container_type(std::move(t));
            return *this;
        }

        vector_t &emplace(slice_t<data_type> slice) {
            set_size(size + slice.size);
            for (addr_t i = 0; i < slice.size; i++)
                new(static_cast<void *>(data() + size++)) container_type(std::move(*slice[i]));
            return *this;
        }

        vector_t &place(addr_t ind, data_type &&t) {
            new(static_cast<void *>(data() + ind)) container_type(std::move(t));
            return *this;
        }

        vector_t &place(addr_t ind, slice_t<data_type> slice) {
            for (addr_t i = 0; i < slice.size; i++)
                new(static_cast<void *>(data() + ind + i)) container_type(std::move(slice[i]));
            return *this;
        }

        vector_t &set_size(const size_t desired_size) {
            allocator.alloc(data_size * desired_size);
            return *this;
        }

        vector_t &erase(const addr_t ind) {
            data_type *ptr = this->operator[](ind);
            container(ptr)->~container_type();
            if (const size_t m = size-- - ind; m > 0)
                memmove(ptr, ptr + 1, m);
            return *this;
        }

        vector_t &erase(slice_t<data_type> slice) {
            for (addr_t i = 0; i < slice.size; i++) {
                container(slice[i])->~container_type();
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

    template<typename T, typename V = std::string, V (*parser)(T) = nullptr> requires
        (std::is_same_v<T, char *> || std::is_same_v<T, const char *> || is_trivially_to_string<T> || parser != nullptr)
        && (std::is_same_v<V, const char *> || std::is_same_v<V, std::string>)
    constexpr str_t obj2str(T value) {
        if constexpr (std::is_same_v<T, char *> || std::is_same_v<T, const char *>)
            return str_t().push(slice_t(const_cast<char *>(value), strlen(value))).trim();
        else {
            char *str = nullptr;
            if constexpr (std::is_same_v<V, std::string>) {
                std::string s;
                if constexpr (parser == nullptr)
                    s = std::to_string(value);
                else
                    s = parser(value);
                str = const_cast<char *>(s.c_str());
            } else if constexpr (std::is_same_v<V, const char *>) {
                str = const_cast<char *>(parser(value));
            } else {
                raise(SIGABRT);
            }
            return str_t().push(slice_t(str, 0, strlen(str))).trim();
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
