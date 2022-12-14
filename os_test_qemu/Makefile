TOOLPREFIX=riscv32-unknown-elf-
AR=$(TOOLPREFIX)ar
AS=$(TOOLPREFIX)as
CC=$(TOOLPREFIX)gcc
CPP=$(TOOLPREFIX)cpp
CXX=$(TOOLPREFIX)g++
LD=$(TOOLPREFIX)ld
OBJCOPY=$(TOOLPREFIX)objcopy
OBJDUMP=$(TOOLPREFIX)objdump
STRIP=$(TOOLPREFIX)strip

INC_DIR = ./src
SRC_DIR = ./src
OBJ_DIR = ./obj
OS_KERNEL_INC_DIR = ../os_kernel/src
OS_KERNEL_SRC_DIR = ../os_kernel/src
BASELINE_INC_DIR = ../cartridge_baseline/src
BASELINE_SRC_DIR = ../cartridge_baseline/src
BIN_DIR = ./bin

DEBUG_MODE=TRUE

ifdef DEBUG_MODE
DEFINES  += -DDEBUG -DVIRT_CLINT
CFLAGS   += -O0 -g -ggdb
#LDFLAGS  += -Wl,-no_pie
else
CFLAGS   += -O3
endif

LDFILE   = riscv32-console.ld
INCLUDE  += -I $(INC_DIR) -I $(OS_KERNEL_INC_DIR) -I $(BASELINE_INC_DIR)
CFLAGS   += -ffreestanding  -nostartfiles -fno-exceptions
LDFLAGS  += -lc -lnosys -lgcc -lstdc++
LDFLAGS  += -Wl,--gc-sections -Wl,-T,$(LDFILE)

TARGET_NAME = os_test_qemu

TARGET_OBJS =   $(OBJ_DIR)/main.o               \
				$(OBJ_DIR)/new_delete_overloading.o               \
                $(OBJ_DIR)/crt0.o               \
                $(OBJ_DIR)/interrupt.o          \
				$(OBJ_DIR)/context_switch.o          \
				$(OBJ_DIR)/printf.o          \
				$(OBJ_DIR)/uart_printf.o          \
                $(OBJ_DIR)/startup.o			\
				$(OBJ_DIR)/utils.o			\


all: directories $(BIN_DIR)/$(TARGET_NAME)

$(BIN_DIR)/$(TARGET_NAME): $(TARGET_OBJS)
	$(CC) $(TARGET_OBJS) -o $(BIN_DIR)/$(TARGET_NAME) $(CFLAGS) $(DEFINES) $(LDFLAGS)
	$(STRIP) $(BIN_DIR)/$(TARGET_NAME) -o $(BIN_DIR)/$(TARGET_NAME).strip

$(OBJ_DIR)/crt0.o: $(SRC_DIR)/crt0.s
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@
$(OBJ_DIR)/interrupt.o: $(SRC_DIR)/interrupt.s
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@
$(OBJ_DIR)/context_switch.o: $(OS_KERNEL_SRC_DIR)/context_switch.s
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c
	$(CXX) $(CFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@
$(OBJ_DIR)/uart_printf.o: $(SRC_DIR)/uart_printf.c
	$(CXX) $(CFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@

$(OBJ_DIR)/new_delete_overloading.o: $(OS_KERNEL_SRC_DIR)/new_delete_overloading.c
	$(CXX) $(CFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@
$(OBJ_DIR)/startup.o: $(SRC_DIR)/startup.c
	$(CXX) $(CFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@
$(OBJ_DIR)/utils.o: $(OS_KERNEL_SRC_DIR)/utils.c
	$(CXX) $(CFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@

$(OBJ_DIR)/printf.o: $(BASELINE_SRC_DIR)/printf.c
	$(CXX) $(CFLAGS) $(DEFINES) $(INCLUDE) -c $< -o $@

directories: $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)