#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define CS251_OS_STATIC_OBJECTS_ON
#include "cs251_os.h"
#include "utils.h"


volatile int global = 42;
volatile uint32_t controller_status = 0;
volatile uint32_t *CARTRIDGE = (volatile uint32_t *)(0x4000001C);
volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);

typedef void (*FunPtr)(void);
volatile uint32_t cartridgeFlag=0;
volatile int vip_seq = 1;
volatile int cmd_seq = 0;

void idleThread(void* param)
{
    int cnt = 0;
    while(1)
    {
        // VIDEO_MEMORY[0x40 * 6] = '0' + cnt++ % 10;
        line_printf(6, "ECS251 Group 1, OS version: 1.2.1;  cnt: %d", cnt++);
        cs251::thread_yield();
    }
}

void threadWaitingForCartridge(void* param)
{
    int cnt = 0;
    while(1){
        line_printf(3, "Firmware ready, please insert cartridge. seq: %d", cnt++);
        if(((*CARTRIDGE) & 0x1) && cartridgeFlag==0){
            cartridgeFlag =1;
            for(int i = 0; i < 0x40; i++) VIDEO_MEMORY[0x40 * 3 + i] = 0;
            ((FunPtr)((*CARTRIDGE) & 0xFFFFFFFC))();
        }
        // cs251::thread_yield();
    }
}

int main() {
    disable_interrupts();
    cs251::schedulerInstance().create(idleThread, nullptr);
    cs251::schedulerInstance().create(threadWaitingForCartridge, nullptr);
    cs251::schedulerInstance().launchFirstTask();

    while(1);
    
    return 0;
}
