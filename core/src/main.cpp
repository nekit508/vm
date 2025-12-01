#include "memory.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"

namespace lx = lexer;
namespace pr = parser;

int main(int argc, char **argv) {
    lx::context_t compiler_context;
    compiler_context.stream = fopen("code.f", "r");

    auto tokens = compiler_context.parse();

    for (auto &token : tokens)
        fputs(token.literal.copy().emplace(' ').emplace(0).begin(), stdout);

    pr::context_t parser_context{vm::utils::vector_t(tokens)};

    auto root = std::move(parser_context.parse_frame());

    if (!root) {
        fputs(root.error().message.copy().emplace(0).begin(), stderr);
        fputc('\n', stderr);
    }

    return 0;
}
