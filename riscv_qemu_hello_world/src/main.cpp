#include <stdint.h>

volatile int global = 42;


int main() {
    int a = 4;
    int b = 12;
    char * UART_MEMORY = (char*)(0x10000000);

    char msg[] = "hello world\n";
    for(int i = 0; i < sizeof(msg); i++)
    {
        UART_MEMORY[0] = msg[i];
    }

    return a + b;
}
