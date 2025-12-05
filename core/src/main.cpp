#include "utils.h"
#include "compiler/compiler.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"

#include "tests/tests.h"

namespace lx = lexer;
namespace pr = parser;
namespace cp = compiler;

int main(int argc, char **argv) {
    //auto vec = vm::utils::cstr2str_t("dfasgdfsg").copy();
    tests::run();

    lx::context_t lexer_context;
    lexer_context.stream = fopen("code.f", "r");

    auto tokens = lexer_context.parse();

    for (auto &token : tokens)
        fputs(token.literal.copy().emplace(' ').emplace(0).begin(), stdout);
    fputc('\n', stdout);

    pr::context_t parser_context{vm::utils::vector_t(tokens)};

    vm::utils::res_t root(std::move(parser_context.parse()));

    if (!root) {
        fputs(root.error().message.copy().emplace(0).begin(), stderr);
        fputc('\n', stderr);
    }

    cp::context_t complier_context("code.bc");

    return 0;
}
