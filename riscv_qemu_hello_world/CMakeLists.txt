cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_FLAGS "\
  -O0 \
  -g \
  -Wall \
  -ffreestanding \
  -nostartfiles \
  -nostdlib \
  -nodefaultlibs \
")

set(TARGET_NAME riscv_qemu_hello_world)
add_executable(${TARGET_NAME}.elf src/main.cpp src/startup.c src/crt0.s src/interrupt.s)
SET(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/riscv32-console.ld")
set_target_properties(${TARGET_NAME}.elf PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
set_target_properties(${TARGET_NAME}.elf PROPERTIES LINKER_LANGUAGE CXX)

# target_link_options(${TARGET_NAME}.elf PRIVATE "LINKER:-T,${LINKER_SCRIPT}")


# foreach (SRC_MODULE main startup)
#   add_custom_command(TARGET ${TARGET_NAME}.elf 
#                      PRE_LINK
#                      COMMAND ${CMAKE_OBJDUMP} -S CMakeFiles/${TARGET}.elf.dir/${SRC_MODULE}.c.obj > ${SRC_MODULE}.s
#                      COMMENT "Invoking: Disassemble ( CMakeFiles/${TARGET}.elf.dir/${SRC_MODULE}.c.obj)")
# endforeach()

SET(CMAKE_EXE_LINKER_FLAGS  
"${CMAKE_EXE_LINKER_FLAGS} \
-Xlinker \
-T ${LINKER_SCRIPT} \
-Wl,-Map=${TARGET_NAME}.map")



