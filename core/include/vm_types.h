#pragma once
#include <cstdint>

typedef uint64_t addr_t;

inline constexpr size_t word_size (sizeof(void*));

struct frame_meta {
    char* name;
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