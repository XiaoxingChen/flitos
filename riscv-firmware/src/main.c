#include <stdint.h>
#include <stdlib.h>
#include <string.h>

volatile int global = 42;
volatile uint32_t controller_status = 0;
volatile uint32_t *CARTRIDGE = (volatile uint32_t *)(0x4000001C);
volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);

typedef void (*FunPtr)(void);
volatile uint32_t cartridgeFlag=0;
volatile uint32_t  counter3= 0;
volatile int vip_seq = 1;
volatile int cmd_seq = 0;
int main() {
    while(1){
        if(((*CARTRIDGE) & 0x1) && cartridgeFlag==0){
            counter3++;
            cartridgeFlag =1;
            ((FunPtr)((*CARTRIDGE) & 0xFFFFFFFC))();
        }
    }
    return 0;
}
