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



/**
 * No operation.
 */
new_opcode(nop, 0);
/**
 * Stop code execution.
 */
new_opcode(end, 1);
/**
 * Jump to param
 */
new_opcode(jmp64c, 2);
/**
 * Call native function
 */
new_opcode(calln64c, 3);
/**
 * Call native function
 */
new_opcode(calln64s, 4);

#undef new_opcode
