#pragma once
#include "defs.h"
#include "instructions.h"
#include "utils/vector_t.h"

namespace virtual_machine {
    struct frame_info_t {
        addr_t id;
        utils::str_t name;
        addr_t code_start;

        frame_info_t() : id(0), code_start(0) {}

        frame_info_t(const frame_info_t &other) = delete;

        frame_info_t & operator=(const frame_info_t &other) = delete;

        frame_info_t(frame_info_t &&other) noexcept
            : id(other.id),
              name(std::move(other.name)),
              code_start(other.code_start) {
        }

        frame_info_t & operator=(frame_info_t &&other) noexcept {
            if (this == &other)
                return *this;
            id = other.id;
            name = std::move(other.name);
            code_start = other.code_start;
            return *this;
        }
    };

    struct virtual_machine_context_t {
        utils::vector_t<char, utils::heap_allocator_t<8192>> code;
        utils::vector_t<frame_info_t, utils::heap_allocator_t<4096>> frames;
        addr_t heap_start;

        size_t load_frame(utils::slice_t<char> data) {
            frame_info_t info;
            info.code_start = code.size;
            info.id = frames.size;

            size_t read = 0;

            for (char c; (c = *data[read++]);)
                info.name.push(c);
            info.name.trim();

            size_t code_size;
            read += rptrs(data[read], code_size);
            const auto code_data = data[read, read + code_size];
            code.push(code_data);

            read += code_size;

            frames.emplace(std::move(info));

            return read;
        }

        void load(utils::slice_t<char> data) {
            addr_t pos = 0;
            while (pos < data.size) {
                pos += load_frame(data[pos, 1, true]);
            }
        }
    };

    struct frame_t {
        frame_info_t *info;
        virtual_machine_context_t *virtual_machine_context;

        addr_t stack_start;

        frame_t *prev;
    };

    struct thread_t {
        frame_t *current_frame;


    };
}

namespace ins = instructions;