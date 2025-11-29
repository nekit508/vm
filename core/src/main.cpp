
#include "memory.h"
#include "compiler/lexer.h"

namespace cp = compiler;

int main(int argc, char **argv) {
    cp::context_t compiler_context;
    compiler_context.stream = fopen("code.f", "r");

    auto tokens = compiler_context.parse();

    for (auto &token : tokens) {
        fputs(token.literal.emplace(' ').emplace(0).begin(), stdout);
    }

    return 0;
}
