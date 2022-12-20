#include <stdint.h>

volatile int global = 42;


int main() {
    int a = 4;
    int b = 12;
    char * UART_MEMORY = (char*)(0x10000000);
    uint16_t * VIRT_TEST = (uint16_t*)(0x10'0000);

    char msg[] = "hello world\n";
    for(int i = 0; i < sizeof(msg); i++)
    {
        UART_MEMORY[0] = msg[i];
    }

    VIRT_TEST[0] = 0x5555;
    // VIRT_TEST[0] = 0x7777;

    return a + b;
}
