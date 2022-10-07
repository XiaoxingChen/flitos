#include <stdint.h>
#include "snake.h"
volatile int global = 42;
volatile uint32_t controller_status = 0;

volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);
int main() {
    int a = 4;
    int b = 12;
    int last_global = 42;
    int x_pos = 12;

    Snake snake;
    for(size_t i = 0; i < snake.size(); i++)
    {
        VIDEO_MEMORY[snake.bodyCoord(i)] = '#';
    }

    std::array<int16_t, 2> cmd_dir;
    while (1) {
        int c = a + b + global;
        if(global != last_global){
            if(controller_status){
                if(controller_status & 0x1){
                    cmd_dir = std::array<int16_t, 2>{0,-1};
                }
                if(controller_status & 0x2){
                    cmd_dir = std::array<int16_t, 2>{-1,0};
                }
                if(controller_status & 0x4){
                    cmd_dir = std::array<int16_t, 2>{1,0};
                }
                if(controller_status & 0x8){
                    cmd_dir = std::array<int16_t, 2>{0,1};
                }
            }
            snake.updatePhysics(cmd_dir);
            cmd_dir = std::array<int16_t, 2>{0,0};
            StaticVector<uint16_t, 20> to_erase = snake.offsetsToErase();
            for(size_t i = 0; i < to_erase.size(); i++)
            {
                VIDEO_MEMORY[to_erase.at(i)] = ' ';
            }
            VIDEO_MEMORY[snake.newHeadCoord()] = '#';
            VIDEO_MEMORY[snake.foodCoord()] = '*';
            last_global = global;
        }
    }
    return 0;
}
