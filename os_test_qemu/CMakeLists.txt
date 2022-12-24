cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_FLAGS "\
  -O0 \
  -g \
  -Wall \
  -ffreestanding \
  -nostartfiles \
  -fno-exceptions \
")

set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_ASM_FLAGS ${CMAKE_CXX_FLAGS})

set(TARGET_NAME test_main)

set(OS_KERNEL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../os_kernel/src)

set(QEMU_TEST_SRC 
  src/main.c 
  src/startup.c 
  src/crt0.s 
  src/interrupt.s 
  src/uart_printf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../cartridge_baseline/src/printf.c
  ${OS_KERNEL_SRC_DIR}/utils.c
  ${OS_KERNEL_SRC_DIR}/context_switch.s
  ${OS_KERNEL_SRC_DIR}/new_delete_overloading.c)

add_definitions(-DVIRT_CLINT)

add_executable(${TARGET_NAME}.elf ${QEMU_TEST_SRC})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../cartridge_baseline/src)

# target_link_libraries(${TARGET_NAME}.elf PUBLIC nosys)
SET(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/riscv32-console.ld")
set_target_properties(${TARGET_NAME}.elf PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
set_target_properties(${TARGET_NAME}.elf PROPERTIES LINKER_LANGUAGE CXX)


SET(CMAKE_EXE_LINKER_FLAGS  
"${CMAKE_EXE_LINKER_FLAGS} \
--specs=nosys.specs \
-lc \
-lnosys \
-lgcc \
-lstdc++ \
-Wl,--gc-sections \
-Wl,-T,${LINKER_SCRIPT} \
")
