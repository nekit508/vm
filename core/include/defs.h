#pragma once
#include <cstdint>
#include <stddef.h>

#define delete_value(value, T) {if constexpr (__is_pointer(T)) delete *value; else value->~T();}

#define debug_memory true
#define if_debug_memory(code) \
if constexpr (debug_memory) { \
code \
}
#define memory_error(message) {fputs(message, stderr);fputs("\n", stderr);raise(SIGSEGV);}

typedef uint64_t addr_t;
typedef int64_t r_addr_t;

inline constexpr size_t word_size(sizeof(void *));

struct frame_meta {
    char *name;
    size_t local_variables_size;
    addr_t code_start;
    addr_t code_end;
};

template<typename T>
constexpr char *to(T *ptr) {
    return reinterpret_cast<char *>(ptr);
}

template<typename T>
constexpr T *as(char *ptr) {
    return reinterpret_cast<T *>(ptr);
}

template<typename T>
constexpr T *ptr(addr_t addr) {
    return reinterpret_cast<T *>(addr);
}

template<typename T>
constexpr addr_t addr(T *ptr) {
    return reinterpret_cast<addr_t>(ptr);
}

template<typename T>
    constexpr void* tv(T *ptr) {
    return  static_cast<void *>(ptr);
}