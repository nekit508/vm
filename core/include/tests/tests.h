#pragma once

#include "res_t.h"
#include "tests_def.h"

#pragma once
namespace tests {

#ifndef RELEASE
    // TODO tests for all utils data structures
    inline void run() {
        MESSAGELN(Running tests)
        res_t::run();
    }
#else
    inline void run() {
    }
#endif

}
