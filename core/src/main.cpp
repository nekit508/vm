#include "utils.h"
#include "compiler/compiler.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"

#include "tests/tests.h"

namespace lx = lexer;
namespace pr = parser;
namespace cp = compiler;

int main(int argc, char **argv) {
    lx::context_t lexer_context;
    lexer_context.stream = fopen("code.f", "r");

    auto tokens = lexer_context.parse();

    for (auto &token : tokens)
        fputs(token.literal.copy().emplace(' ').emplace(0).begin(), stdout);
    fputc('\n', stdout);

    pr::context_t parser_context{vm::utils::vector_t(tokens)};

    vm::utils::res_t root_res(std::move(parser_context.parse()));

    if (!root_res) {
        fputs("Error: ", stderr);
        fputs(root_res.error().message.copy().emplace(0).begin(), stderr);
        fputc('\n', stderr);
        return 69;
    }

    cp::context_t complier_context;
    complier_context.compile_root(root_res.value_m());

    auto fd1 = fopen("code.bc", "w");
    complier_context.write(fd1);
    fclose(fd1);

    auto fd2 = fopen("code.asm", "w");
    complier_context.write(fd2);
    fclose(fd2);

    return 0;
}
