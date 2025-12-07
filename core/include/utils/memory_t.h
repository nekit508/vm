#pragma once

#include "defs.h"

namespace utils {
    /** Wraps raw bytes data and provides access to it as input buffer, stack or output buffer. */
    template<bool ro = false>
    struct memory_t {
        memory_t &read(const r_addr_t offset) {
            read_pos += offset;
            return *this;
        }

        memory_t &write(const r_addr_t offset) {
            write_pos += offset;
            return *this;
        }

        memory_t &stack(const r_addr_t offset) {
            stack_pos += offset;
            return *this;
        }

        template<typename T>
        T *read() {
            return reinterpret_cast<T *>(data + read_pos);
        }

        template<typename T>
        T *stack() {
            return reinterpret_cast<T *>(data + stack_pos - sizeof(T));
        }

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

        size_t size = 0;
        char *data = nullptr;
        addr_t read_pos = 0, write_pos = 0, stack_pos = 0;

        template<typename T>
        memory_t(T *data, const size_t size) : size(size), data(reinterpret_cast<char *>(data)) {
        }

        explicit memory_t(const size_t size) : size(size), data(static_cast<char *>(malloc(this->size))) {
        }

        memory_t(const memory_t &other)
            : size(other.size),
              data(static_cast<char *>(malloc(size))),
              read_pos(other.read_pos),
              write_pos(other.write_pos),
              stack_pos(other.stack_pos) {
            memcpy(data, other.data, size);
        }

        memory_t(memory_t &&other) noexcept
            : size(other.size),
              data(other.data),
              read_pos(other.read_pos),
              write_pos(other.write_pos),
              stack_pos(other.stack_pos) {
            other.data = nullptr;
        }

        memory_t &operator=(const memory_t &other) {
            if (this == &other)
                return *this;
            size = other.size;
            read_pos = other.read_pos;
            write_pos = other.write_pos;
            stack_pos = other.stack_pos;

            this->~memory_t();
            data = other.data;
            memcpy(data, other.data, size);

            return *this;
        }

        memory_t &operator=(memory_t &&other) noexcept {
            if (this == &other)
                return *this;

            size = other.size;
            read_pos = other.read_pos;
            write_pos = other.write_pos;
            stack_pos = other.stack_pos;

            this->~memory_t();
            data = other.data;
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
            if_debug_memory(
            if (read_pos > size - sizeof(T)) {
                memory_error("unable to read from buffer")
            }
            )
            value = *reinterpret_cast<T *>(data + read_pos);
            read_pos += sizeof(T);
            return *this;
        }

        /** Buffer input. */
        template<typename T>
        memory_t &operator<<(const T &value) {
            static_assert(!ro, "Writing to read only memory");
            if_debug_memory(
            if (write_pos > size - sizeof(T)) {
                memory_error("unable write to buffer")
            }
            )
            *reinterpret_cast<T *>(data + write_pos) = value;
            write_pos += sizeof(T);
            return *this;
        }

        /** Stack output. */
        template<typename T>
        memory_t &operator>>=(T &value) {
            if_debug_memory(
            if (stack_pos < sizeof(T)) {
                memory_error("unable to read from stack");
            }
            )
            stack_pos -= sizeof(T);
            value = *reinterpret_cast<T *>(data + stack_pos);
            return *this;
        }

        /** Stack input. */
        template<typename T>
        memory_t &operator<<=(const T &value) {
            static_assert(!ro, "Writing to read only memory");
            if_debug_memory(
            if (stack_pos > size - sizeof(T)) {
                memory_error("unable to write to stack")
            }
            )
            *reinterpret_cast<T *>(data + stack_pos) = value;
            stack_pos += sizeof(T);
            return *this;
        }
    };
}
