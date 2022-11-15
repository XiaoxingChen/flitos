#include <stdint.h>
#include <string.h>
#include "video_api.h"
#include "thread_api.h"

volatile int global = 42;
volatile uint32_t controller_status = 0;
uint32_t getTicks(void);

uint32_t getStatus(void);
uint32_t getVideoInterruptSeq(void);
uint32_t getCmdInterruptSeq(void);
void initVideoSetting();

/**
 * interrupt handlers
 */
uint32_t registerHandler(uint32_t code);

uint32_t myHandler(uint32_t code);
uint32_t myHandler2(uint32_t code);

volatile int invokingFlag = 0;

extern uint8_t bird_img_0[64*64];
extern uint8_t bird_img_1[64*64];
extern uint8_t bird_img_2[64*64];

volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);
volatile char *MODE_CONTROL_REG = (volatile char *)(0x50000000 + 0xFF414);

uint32_t hookFunction(uint32_t func_id);

FuncWriteTargetMem writeTargetMem;
FuncWriteTarget writeTarget;

void threadGraphics(void* param);

int main() {
    registerHandler((uint32_t) myHandler);
    registerHandler((uint32_t) myHandler2);
    invokingFlag = 1;
    // hold the invoking, otherwise the main will be invoked repeatedly.

    int a = 4;
    int b = 12;
    int last_global = 42;
    int x_pos = 18;

    writeTargetMem = (FuncWriteTargetMem)hookFunction(1);
    writeTarget = (FuncWriteTarget)hookFunction(2);

    char video_cnt_str[]= "video interrupt:";
    char cmd_cnt_str[]= "CMD interrupt:";
    char timer_cnt_str[]= "timer interrupt:";
    memcpy((void*)&VIDEO_MEMORY[0x40*0], video_cnt_str, sizeof(video_cnt_str));
    memcpy((void*)&VIDEO_MEMORY[0x40*1], cmd_cnt_str, sizeof(cmd_cnt_str));
    memcpy((void*)&VIDEO_MEMORY[0x40*2], timer_cnt_str, sizeof(timer_cnt_str));

    initVideoSetting();
    threadGraphics(NULL);
    return 0;
}

void threadGraphics(void* param)
{
    int x_pos = 18;
    uint32_t sprite_x = 30;
    uint32_t sprite_y = 30;
    int move_speed = 5;
    uint32_t time_counter = 0;
    uint32_t last_time = 0;
    while (1) {
        global = getTicks();
        if(global != last_time){
            VIDEO_MEMORY[17] = '0' + (getVideoInterruptSeq() % 10);
            
            int cmdSeq = getCmdInterruptSeq();
            VIDEO_MEMORY[15 +  + 0x40] = '0' + cmdSeq % 10;
            
            setDisplayMode(cmdSeq ^ 1);
            controller_status = getStatus();
            if(controller_status){
                VIDEO_MEMORY[x_pos] = ' ';
                if(controller_status & 0x1){
                    sprite_x -= move_speed;
                    if(x_pos & 0x3F){
                        x_pos--;
                    }
                }
                if(controller_status & 0x2){
                    sprite_y -= move_speed;
                    if(x_pos >= 0x40){
                        x_pos -= 0x40;
                    }
                }
                if(controller_status & 0x4){
                    sprite_y += move_speed;
                    if(x_pos < 0x8C0){
                        x_pos += 0x40;
                    }
                }
                if(controller_status & 0x8){
                    sprite_x += move_speed;
                    if((x_pos & 0x3F) != 0x3F){
                        x_pos++;
                    }
                }
                // setLargeSpriteControl(0, 64, 64, sprite_x, sprite_y, 1);
                VIDEO_MEMORY[x_pos] = 'X';
            }
            for(int i = 0; i < 3; i++)
            {
                setLargeSpriteControl(i, 64, 64, sprite_x, sprite_y, i == global % 3);
            }
            last_time = global;
        } //global != last_time
        // threadYield();
    } // while(1)
}


uint32_t myHandler(uint32_t code) {
    // TODO: fix upcall with global_pointer
    char* VIDEO_MEMORY_LOCAL = (char *)(0x50000000 + 0xFE800);
    VIDEO_MEMORY_LOCAL[0x40 * 2 + 17] = '0' + code % 10;
    return 0;
}

uint32_t myHandler2(uint32_t code) {
    // TODO: fix upcall with global_pointer
    char* VIDEO_MEMORY_LOCAL = (char *)(0x50000000 + 0xFE800);
    VIDEO_MEMORY_LOCAL[0x40 * 2 + 18] = '0' + code % 10;
    return 0;
}
