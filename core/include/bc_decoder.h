#pragma once
#include "vm.h"

namespace vm::bcd {
    std::string read_str(utils::buffer &buffer);

    void load(context *ctx, const char* file);
}
