#include "vm.h"


#include <iostream>

/*void vm::frame_t::tick() {

}

void vm::context_t::tick() {
    uint16_t opcode;
    code >> opcode;

    if (!handlers.contains(opcode)) {
        std::cerr << "opcode " << opcode << " was not processed" << std::endl;
        raise(SIGILL);
    } else {
        handlers[opcode](this);
    }
}

void vm::context_t::next_frame(frame_t *frame) {
    frame->previous_frame = frame;
    cf = frame;

    cf->return_addr = code.read_pos + 1;
}

void vm::context_t::prev_frame() {
    const frame_t *old_frame = cf;
    cf = old_frame->previous_frame;

    code.read_pos = cf->return_addr;

    delete old_frame;
}

vm::context_t::~context_t() {
    delete cf;
}
*/