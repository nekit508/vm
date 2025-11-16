#pragma once

#include <cstdint>

/** Operation encoding
 *
 * [opcode 16b][data...]
 */
typedef uint16_t opcode_t;

/**
 * Names:
 * actual function name
 * param size (8, 16, 32, 64)
 * param type (c - constant, s - stack)
 * @param name name
 * @param id id
 */
#define new_opcode(name, id) inline constexpr opcode_t name (id)

new_opcode(nop, 0);
new_opcode(end, 1);

// const to satck
new_opcode(load64c, 2);
new_opcode(load32c, 3);
new_opcode(load16c, 4);
new_opcode(load8c, 5);

// read from mem to stack
new_opcode(read6464s, 6);
new_opcode(read3264s, 7);
new_opcode(read1664s, 8);
new_opcode(read864s, 9);

// write to mem from stack
new_opcode(write64s64s, 10);
new_opcode(write64s32s, 11);
new_opcode(write64s16s, 12);
new_opcode(write64s8s, 13);

new_opcode(calln64s, 14);

new_opcode(add64s64s, 15);
new_opcode(add64c64s, 16);
new_opcode(sub64s64s, 24);
new_opcode(sub64c64s, 25); // b - a

new_opcode(mapmem64s, 17);

new_opcode(cmpeq64s64s, 18);
new_opcode(cmpeq32s32s, 19);
new_opcode(cmpeq16s16s, 20);
new_opcode(cmpeq8s8s, 21);

new_opcode(jmp64c, 28);
new_opcode(jmp64s, 29);
new_opcode(jmpt8s64c, 23);
new_opcode(jmpf8s64c, 26);

new_opcode(shrink64c64s, 27);

#undef new_opcode
