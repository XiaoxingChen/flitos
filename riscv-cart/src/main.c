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

// extern uint8_t bird_img_0[64*64];
// extern uint8_t bird_img_1[64*64];
// extern uint8_t bird_img_2[64*64];

// volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);
// volatile char *MODE_CONTROL_REG = (volatile char *)(0x50000000 + 0xFF414);

void threadGraphics(void* param);
void movePillarThread(void* param);

void idleThread(void* param)
{
    int cnt = 0;
    while (1)
    {
        linePrintf(7, "app idle thread cnt: %d", cnt++);
        // threadYield();
    }
    
}

void threadCommandButtonMonitor(void* param)
{
    while(1)
    {
        int cmdSeq = getCmdInterruptSeq();
        linePrintf(1, "cmd interrupt: %d", cmdSeq);
        
        setDisplayMode(cmdSeq ^ 1);
        threadYield();
    }
    
}


void createAPairOfPipe();

int main() {

    initVideoSetting();
    thread_id_t th1 = threadCreate(idleThread, NULL);
    thread_id_t th2 = threadCreate(threadGraphics, NULL);
    thread_id_t th3 = threadCreate(threadCommandButtonMonitor, NULL);


    thread_id_t th4 = threadCreate(movePillarThread, NULL);



    // threadJoin(th1);
    // threadJoin(th2);
    // threadJoin(th3);
    
    while(1) threadYield();
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
            linePrintf(0, "video interrupt: %d", getVideoInterruptSeq());
            controller_status = getStatus();
            if(controller_status){
                // VIDEO_MEMORY[x_pos] = ' ';
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
                // VIDEO_MEMORY[x_pos] = 'X';
            }
            for(int i = 0; i < 3; i++)
            {
                setLargeSpriteControl(i, 64, 64, sprite_x, sprite_y, i == global % 3);
            }
            last_time = global;
        } //global != last_time
        threadYield();
        // uint32_t g_ptr = getGlobalPointer();
        // *(uint32_t*)VIDEO_MEMORY = g_ptr;
    } // while(1)
}

