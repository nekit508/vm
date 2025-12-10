#include <chrono>

#include "utils.h"
#include "compiler/compiler.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/virtual_machine.h"

#include "tests/tests.h"

namespace lx = lexer;
namespace pr = parser;
namespace cp = compiler;
namespace vm = virtual_machine;

long long getCurrentTimeInMillis() {
    auto now = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    return milliseconds.count();
}

int main(int argc, char **argv) {
    lx::context_t lexer_context;
    lexer_context.stream = fopen("code.f", "r");

    auto tokens = lexer_context.parse();

    // print file as lex parts
    /*for (auto &token : tokens)
        fputs(token.literal.copy().emplace(' ').emplace(0).begin(), stdout);
    fputc('\n', stdout);*/

    pr::context_t parser_context{utils::vector_t(tokens)};

    utils::res_t root_res(std::move(parser_context.parse()));

    if (!root_res) {
        fputs("Error: ", stderr);
        fputs(root_res.error().message.copy().emplace(0).begin(), stderr);
        fputc('\n', stderr);
        return 69;
    }

    cp::context_t complier_context;
    complier_context.compile_root(root_res.value_m());

    const auto fd1 = fopen("code.bc", "w");
    complier_context.write(fd1);
    fclose(fd1);

    const auto fd2 = fopen("code.asm", "w");
    complier_context.write(fd2);
    fclose(fd2);


    utils::vector_t<char, utils::file_allocator_t<PROT_READ>> file_data{utils::file_allocator_t<PROT_READ>(fopen("code.bc", "r"))};
    file_data.size = file_data.allocator.capacity;

    vm::virtual_machine_context_t virtual_machine_context;
    virtual_machine_context.load(file_data);

    auto thread = vm::thread_t();

    auto *frame = new vm::frame_t();

    frame->stack_start = addr(malloc(1024));
    frame->info = virtual_machine_context.frames[0];

    frame->prev = thread.current_frame;
    thread.current_frame = frame;
    virtual_machine_context.threads.emplace(std::move(thread));

    while (virtual_machine_context.tick()) {}

    return 0;
}
