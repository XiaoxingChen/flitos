cmake_minimum_required(VERSION 3.10)

project(flitos LANGUAGES C CXX ASM)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/os_kernel/src )

set(CMAKE_C_COMPILER ${CMAKE_CXX_COMPILER})

add_subdirectory(riscv_qemu_hello_world)
add_subdirectory(os_test_qemu)



