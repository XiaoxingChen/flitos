cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_FLAGS "\
  -O0 \
  -g \
  -Wall \
  -ffreestanding \
  -nostartfiles \
  -fno-exceptions \
")

set(TARGET_NAME os_kernel)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../cartridge_baseline/src)
  
add_library(${TARGET_NAME}.elf STATIC src/context_switch.s src/utils.c src/new_delete_overloading.c)



