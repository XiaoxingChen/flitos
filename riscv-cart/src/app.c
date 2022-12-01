#include "video_api.h"
#include <string.h>
#include "thread_api.h"


extern uint8_t bird_color_palette[888];
// extern uint8_t bird_background_color_palette[4 * 16];
extern uint8_t bird_img_0[64 * 64];
extern uint8_t bird_img_1[64 * 64];
extern uint8_t bird_img_2[64 * 64];
extern uint8_t bird_background_img[288 * 512];

extern uint8_t pillar_palette[4*135];
extern uint8_t pillar_img_head_up[64*64];
extern uint8_t pillar_img_head_down[64*64];
extern uint8_t pillar_img_body_up[64*64];
extern uint8_t pillar_img_body_down[64*64];

extern uint8_t gameover_img_01[4096];
extern uint8_t gameover_img_02[4096];

extern volatile uint32_t running_flag;

struct pillarBlock {
    /**
     * x and y represent the top-left point of the pillar
     */
    int x;
    int y;
    /**
     * height represents the height in unit of pixel
     */
    int height;

    int controlIndex;
};
struct pillar {
    /**
     * x and y represent the top-left point of the pillar
     */
    int x;
    int y;
    /**
     * height represents the height in unit of pixel
     */
    int height;

    int block_number;
    /**
     *
     */
    struct pillarBlock blocks[3];
};
volatile struct pillar globalPillars[6];

volatile mutex_id_t pillarMutex;

// extern FuncWriteTargetMem writeTargetMem;

void initVideoSetting() {
    setDisplayMode(DISPLAY_MODE_GRAPHICS);
    initBackgroundPalette(0, bird_color_palette, 888);
    // initPaletteBirdBackground(BACKGROUND_PALETTE_0);
    initSpritePalette(1, bird_color_palette, 888);
    initTransparentSpritePalette(0);

    setBackgroundControl(0, 0, 0, 0, 0);
    //  setBackgroundControl0(BACKGROUND_CONTROL_0);
    setLargeSpriteControl(0, 64, 64, 30, 30, 1);
    setLargeSpriteControl(1, 64, 64, 0, 0, 0);
    setLargeSpriteControl(2, 64, 64, 0, 0, 0);
    setBackgroundDataImage(0, bird_background_img);

    // writeIndexedTarget((uint32_t)0x000, 0, 0, bird_background_img, 0x24000); // doesn't work
    // writeTargetMem((uint32_t)0x00, (uint32_t)bird_background_img, 512*288); //work
    // wrapCall((uint32_t)0x00, (uint32_t)bird_background_img, 512*288); // doesn't work
    setLargeSpriteDataImage(0, bird_img_0);
    // writeTargetMem(0xB4000, (uint32_t)img, 64*64);
    // memcpy((void*)(0x50000000 + 0xB4000) + 64*32, bird_img_0, 64);
    setLargeSpriteDataImage(1, bird_img_1);
    setLargeSpriteDataImage(2, bird_img_2);

    // setDisplayMode(DISPLAY_MODE_GRAPHICS);
    // setDisplayMode(DISPLAY_MODE_TEXT);

    // volatile char *MODE_CONTROL_REG = (volatile char *)(0x50000000 + 0xFF414);
    // *MODE_CONTROL_REG = 0x1;
    // *MODE_CONTROL_REG = 0x0;

    // setLargeSpriteControl(0, 64, 64, 30, 30, 1);

}


int countXPosition(int i, int width) {
    if (i == 0) {
        return 100;
    }
    return 100 + (64 + 100) * i;
}

uint8_t pillar_img_single[64*64];

int createAPillar(struct pillar *currentPillar, int index_data, int is_up) {
    for(int i = 0; i < 64*64; i++) pillar_img_single[i] = 10;
    for (int i = 0; i < currentPillar->block_number; i++) {
        setLargeSpriteControl(index_data, 64, 64, currentPillar->blocks[i].x, currentPillar->blocks[i].y, 1);
        #if 1
        if(is_up)
        {
            setLargeSpriteDataImage(index_data, i == 0 ? pillar_img_head_up : pillar_img_body_up);
        }else
        {
            setLargeSpriteDataImage(index_data, i == currentPillar->block_number-  1 ? pillar_img_head_down : pillar_img_body_down);
        }
        #else
            setLargeSpriteDataImage(index_data, pillar_img_single);
        #endif
        
        currentPillar->blocks[i].controlIndex = index_data;
        index_data++;
    }
    return index_data;
}


int movePillar(struct pillar *currentPillar, int offset) {
    for (int i = 0; i < currentPillar->block_number; i++) {
        currentPillar->blocks[i].x = currentPillar->blocks[i].x - offset;
        currentPillar->x = currentPillar->blocks[i].x;
        if (currentPillar->blocks[i].x < -64) {
            currentPillar->blocks[i].x = 512;
        }
        setLargeSpriteControl(currentPillar->blocks[i].controlIndex, 64, 64, currentPillar->blocks[i].x,
                              currentPillar->blocks[i].y, 1);
    }
    return 0;
}


void createABottom(int x, int height, struct pillar *bottomPillar) {
    bottomPillar->x = x;
    bottomPillar->y = 288 - height;
    bottomPillar->height = height;
    // blocks of bottom
    int remaining_height = height;
    int idx_blocks = 0;
    while (remaining_height > 0) {
        struct pillarBlock tempBlock;
        tempBlock.height = 64;
        tempBlock.x = bottomPillar->x;
        tempBlock.y = idx_blocks * tempBlock.height + 288 - bottomPillar->height;
        bottomPillar->blocks[idx_blocks] = tempBlock;

        remaining_height = remaining_height - tempBlock.height;
        idx_blocks++;
    }
    bottomPillar->block_number = idx_blocks;
}

void createBottoms(struct pillar *pillars) {
    int heights[3] = {99, 130, 110};
    for (int i = 0; i < 3; i++) {
        int x = countXPosition(i, 64);
        int height = heights[i];
        struct pillar tempBottom;
        createABottom(x, height, &tempBottom);
        pillars[i] = tempBottom;
    }
}

void createATop(int x, int height, struct pillar *topPillar) {
    topPillar->height = height;
    topPillar->x = x;
    topPillar->y = 0;

    int data_height = 64;
    int remaining = height % data_height;

    int numbers = height / data_height;

    int idx_blocks = 0;
    if (remaining != 0) {
        topPillar->block_number = (numbers + 1);
        // generate a block
        struct pillarBlock remaining_block;
        remaining_block.height = remaining;
        remaining_block.x = topPillar->x;
        remaining_block.y = remaining - data_height;

        topPillar->blocks[idx_blocks] = remaining_block;
        idx_blocks++;
    } else {
        topPillar->block_number = numbers;
    }

    for (int i = 0; i < numbers; i++) {
        struct pillarBlock tempBlock;
        tempBlock.height = data_height;
        tempBlock.x = topPillar->x;
        tempBlock.y = remaining + 64 * i;
        topPillar->blocks[idx_blocks] = tempBlock;
        idx_blocks++;
    }

}

void createTops(struct pillar *pillars) {
    int heights[3] = {104, 64, 110};
    for (int i = 0; i < 3; i++) {
        int x = countXPosition(i, 64);
        int height = heights[i];
        struct pillar tempTop;
        createATop(x, height, &tempTop);
        pillars[i + 3] = tempTop;
    }
}


void movePillarThread(void *param) {
    // an array contains 3-pairs of pillars
    createBottoms(globalPillars);
    createTops(globalPillars);
    // the index of the large sprites
    int index_data = 5;

    /**
     * step 1: generate pillars
     */
    for (int i = 0; i < 6; i++) {
        index_data = createAPillar(&globalPillars[i], index_data, i < 3);
    }

    /**
     * step 2: move pillars in a dead-loop
     */
    int movement_offset = 2;
    while (running_flag) {
        for (int j = 0; j < 6; j++) {
            movePillar(&globalPillars[j], movement_offset);
        }
        threadYield();
    }
}

int calculateCollision(int bird_x, int bird_y, int pillarIndex) {
    struct pillar currentPillar = globalPillars[pillarIndex];
   // linePrintf(15, "current pillar x=%d,y=%d,height=%d                 ",currentPillar.x,currentPillar.y,currentPillar.height);

    int bird_x2 = bird_x+22;
    int bird_y2 = bird_y+22;

    // if the bird's position has beyond the screen
    if( bird_x< 0 || bird_x2 >512 || bird_y <0 || bird_y2>288){
        //linePrintf(12, "bird_collision has occurred: %d,%d                 ", bird_x, bird_y);
        return 1;
    }else{
       // linePrintf(12, "                                                              ");
    }

    int pillar_x2 = currentPillar.x+64;
    int pillar_y2 = currentPillar.y+currentPillar.height;
//    linePrintf(12, "B: x1=%d,x2=%d,y1=%d,y2=%d;P:x1=%d,x2=%d,y1=%d,y2=%d                       ",
//               bird_x,bird_x2,bird_y,bird_y2,
//        currentPillar.x,pillar_x2,currentPillar.y,pillar_y2);
    // 横相交 并且竖相交
    if(bird_x2 >= currentPillar.x && bird_x <= pillar_x2 && bird_y2 >= currentPillar.y && bird_y <= pillar_y2){
        return 1;
    }

    return 0;
}

void showGamerOverSprite(){
    setLargeSpriteControl(62, 64, 64, 192, 112, 1);
    setLargeSpriteControl(63, 64, 64, 256, 112, 1);

    setLargeSpriteDataImage(62,gameover_img_01);
    setLargeSpriteDataImage(63,gameover_img_02);
}
