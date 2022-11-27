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


struct position{
    uint32_t x;
    uint32_t y;
    mutex_id_t mtx;
};

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
void gravityThread(void* param);

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
    // fetch bird position struct.
    struct position flappyBird;
    flappyBird.y=30;
    flappyBird.x=30;
    flappyBird.mtx = mutexInit();
    thread_id_t th1 = threadCreate(idleThread, NULL);
    thread_id_t th2 = threadCreate(threadGraphics, &flappyBird);
    thread_id_t th3 = threadCreate(threadCommandButtonMonitor, NULL);

    thread_id_t th4 = threadCreate(gravityThread, &flappyBird);

    thread_id_t th5 = threadCreate(movePillarThread, NULL);



    // threadJoin(th1);
    // threadJoin(th2);
    // threadJoin(th3);
    
    while(1) threadYield();
    return 0;
}

void threadGraphics(void* param)
{
    struct position *flappyBird = (struct position* ) param;
    int x_pos = 18;
    int sprite_inc_x;
    int sprite_inc_y;
    int move_speed = 5;
    uint32_t time_counter = 0;
    uint32_t last_time = 0;
    while (1) {
        global = getTicks();
        sprite_inc_x = 0;
        sprite_inc_y = 0;
        if(global != last_time){
            linePrintf(0, "video interrupt: %d", getVideoInterruptSeq());
            controller_status = getStatus();
            if(controller_status){
                // VIDEO_MEMORY[x_pos] = ' ';
                if(controller_status & 0x1){
                    sprite_inc_x = -move_speed;
                    if(x_pos & 0x3F){
                        x_pos--;
                    }
                }
                if(controller_status & 0x2){
                    sprite_inc_y = -move_speed;
                    if(x_pos >= 0x40){
                        x_pos -= 0x40;
                    }
                }
                if(controller_status & 0x4){
                    sprite_inc_y = move_speed;
                    if(x_pos < 0x8C0){
                        x_pos += 0x40;
                    }
                }
                if(controller_status & 0x8){
                    sprite_inc_x = move_speed;
                    if((x_pos & 0x3F) != 0x3F){
                        x_pos++;
                    }
                }
                // setLargeSpriteControl(0, 64, 64, sprite_x, sprite_y, 1);
                // VIDEO_MEMORY[x_pos] = 'X';


            }
            mutexLock(flappyBird->mtx);
            flappyBird->x += sprite_inc_x;
            flappyBird->y +=sprite_inc_y;
            mutexUnlock(flappyBird->mtx);
            for(int i = 0; i < 3; i++)
            {
                setLargeSpriteControl(i, 64, 64, flappyBird->x, flappyBird->y, i == global % 3);
            }
            
            last_time = global;
        } //global != last_time
        threadYield();
        // uint32_t g_ptr = getGlobalPointer();
        // *(uint32_t*)VIDEO_MEMORY = g_ptr;
    } // while(1)
}

void gravityThread(void *param) {
    int move_frequency =3;
    uint32_t last_time = 0;
    int count111=0;
    int global2=42;
    struct position *flappyBird = (struct position*) param;
    while (1) {
        global2 = getTicks();
        if(global2 - last_time >=move_frequency){
            linePrintf(11+count111, "bird y=%d", flappyBird->y);
            // count111++;
            mutexLock(flappyBird->mtx);
            uint32_t sprite_x = flappyBird->x;
            uint32_t sprite_y = flappyBird->y;
            sprite_y +=3;
            flappyBird->y=sprite_y;
            mutexUnlock(flappyBird->mtx);
            for(int i = 0; i < 3; i++)
            {
                setLargeSpriteControl(i, 64, 64, sprite_x, sprite_y, i == global2 % 3);
            }
            
            
            last_time=global2;
        }
        
        threadYield();
    }
}
