#include <stdint.h>

volatile int global = 42;


int main() {
    int a = 4;
    int b = 12;
    char * UART_MEMORY = (char*)(0x10000000);
    uint32_t * VIRT_TEST = (uint32_t*)(0x10'0000);

    char msg[] = "hello world\n";
    for(int i = 0; i < sizeof(msg); i++)
    {
        UART_MEMORY[0] = msg[i];
    }

    uint32_t exit_code = 0;

    VIRT_TEST[0] = (exit_code << 16) | 0x3333;

    return a + b;
}
