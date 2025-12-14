#pragma once
#include "defs.h"
#include "instructions.h"
#include "utils/vector_t.h"

namespace ins = instructions;

namespace virtual_machine {
    struct frame_info_t {
        addr_t id;
        utils::str_t name;
        addr_t code_start;

        frame_info_t() : id(0), code_start(0) {
        }

        frame_info_t(const frame_info_t &other) = delete;

        frame_info_t &operator=(const frame_info_t &other) = delete;

        frame_info_t(frame_info_t &&other) noexcept
            : id(other.id),
              name(std::move(other.name)),
              code_start(other.code_start) {
        }

        frame_info_t &operator=(frame_info_t &&other) noexcept {
            if (this == &other)
                return *this;
            id = other.id;
            name = std::move(other.name);
            code_start = other.code_start;
            return *this;
        }
    };

    struct frame_t {
        frame_info_t *info = nullptr;
        frame_t *prev = nullptr;

        addr_t stack_start = 0;
        addr_t code_pos = 0;

        ~frame_t() {
            if (stack_start != 0)
                free(ptr<void *>(stack_start));
        }
    };

    struct thread_t {
        frame_t *current_frame = nullptr;
    };

    struct virtual_machine_context_t {
        utils::vector_t<char, utils::heap_allocator_t<8192>> code;
        utils::vector_t<frame_info_t, utils::heap_allocator_t<4096>> frames;

        utils::vector_t<thread_t> threads;

        addr_t heap_start = 0;
        const size_t heap_size = 4096ul * 1024 * 1024;

        virtual_machine_context_t() {
            heap_start = addr(mmap(nullptr, heap_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0));
        }

        ~virtual_machine_context_t() {
            munmap(ptr<void>(heap_start), heap_size);
        }

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

        bool tick() {
            for (addr_t i = 0; i < threads.size; ++i)
                if (threads[i]->current_frame == nullptr)
                    threads.erase(i--);

            for (thread_t &thread: threads) {
                frame_t *frame = thread.current_frame;
                frame_info_t *info = frame->info;
                ins::opcode_t opcode;
                frame->code_pos += rptrs(code[info->code_start + frame->code_pos], opcode);

                switch (opcode) {
                    case ins::nop: // no operation
                        break;
                    case ins::ret: // return to caller
                        thread.current_frame = frame->prev;
                        // TODO exit frame correctly
                        break;
                    case ins::jmp: // jump to operation (frame start relative)
                        frame->code_pos = *as<addr_t>(code[info->code_start + frame->code_pos]);
                        break;
                    case ins::call: // call another frame
                        // TODO call instruction decode
                        break;
                    case ins::dbg_brk: // debug break
                        (void) 0;
                        break;
                    case ins::ill: // illegal operation
                        raise(SIGILL);
                        break;
                    default:
                        raise(SIGALRM);
                        break;
                }

                if (thread.current_frame != frame)
                    delete frame;
            }

            return threads.size != 0;
        }
    };
}
