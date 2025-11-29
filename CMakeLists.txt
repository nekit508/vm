cmake_minimum_required(VERSION 4.0)
project(virtual_machine)

set(CMAKE_CXX_STANDARD 23)

if (${NMISC_LOADED})
else ()
    set(NMISC_LOADED 1)
    message("Downloading nmisc lib")
    execute_process(COMMAND curl "https://raw.githubusercontent.com/nekit508/misc-cxx-stuff/main/functions.cmake" -s -o "${PROJECT_SOURCE_DIR}/lib.cmake")
    include("lib.cmake")
endif ()

add_executable(virtual_machine)

serve_target(virtual_machine "core/src" "core/include" CXX)
