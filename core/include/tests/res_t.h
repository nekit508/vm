#pragma once
#include <assert.h>
#include "utils/res_t.h"

#include "tests_def.h"

namespace tests::res_t {
    inline void test_stack_destructor() {
        MESSAGELN(|___|___test_stack_destructor start)
        static bool vd = false, ed = false;

        struct value {
            ~value() {
                vd = true;
            }
        };

        struct error {
            ~error() {
                ed = false;
            }
        };

        vm::utils::res_t<value, error> res1(value{});
        vm::utils::res_t<value, error> res2(value{});

        res1.~res_t();
        res2.~res_t();

        TEST(|___|___|___test_stack_destructor value, vd);
        TEST(|___|___|___test_stack_destructor error, ed);
    }

    inline void run() {
        MESSAGELN(|___Running res_t tests)

        test_stack_destructor();
    }
}
