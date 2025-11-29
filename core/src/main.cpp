
#include "memory.h"
#include "compiler/lexer.h"

namespace cp = compiler;

int main(int argc, char **argv) {
    cp::context_t compiler_context;
    compiler_context.stream = fopen("code.f", "r");

    auto token = compiler_context.parse();

    return 0;
}
