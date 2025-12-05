#pragma once
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
                ed = true;
            }
        };

        {
            auto res2 =  vm::utils::res_t<value, error>(error{});
            auto res1 =  vm::utils::res_t<value, error>(value{});
        }

        TEST(|___|___|___test_stack_destructor value, vd)
        TEST(|___|___|___test_stack_destructor error, ed)
    }

    inline void run() {
        MESSAGELN(|___Running res_t tests)

        test_stack_destructor();
    }
}
