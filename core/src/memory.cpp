#include "memory.h"

vm::utils::str_t vm::utils::cstr2str_t(const char *str) {
    const size_t len = strlen(str);
    return str_t(const_cast<char *>(str), len, false);
}

vm::utils::str_t vm::utils::cstr2strm_t(const char *str) {
    const size_t len = strlen(str);
    return str_t(const_cast<char *>(str), len, false).copy();
}
