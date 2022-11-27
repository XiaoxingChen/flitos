#include "video_api.h"
#include <string.h>
#include "thread_api.h"


extern uint8_t bird_color_palette[4 * 8];
extern uint8_t bird_background_color_palette[4 * 16];
extern uint8_t bird_img_0[64 * 64];
extern uint8_t bird_img_1[64 * 64];
extern uint8_t bird_img_2[64 * 64];
extern uint8_t bird_background_img[288 * 512];

// extern FuncWriteTargetMem writeTargetMem;

void initVideoSetting() {
    setDisplayMode(DISPLAY_MODE_GRAPHICS);
    initBackgroundPalette(0, bird_background_color_palette, 4 * 16);
    // initPaletteBirdBackground(BACKGROUND_PALETTE_0);
    initSpritePalette(1, bird_color_palette, 4 * 8);
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



int createAPillar(struct pillar *currentPillar,int index_data) {
    for (int i = 0; i < currentPillar->block_number; i++) {
        setLargeSpriteControl(index_data, 64, 64, currentPillar->blocks[i].x, currentPillar->blocks[i].y, 1);
        setLargeSpriteDataImage(index_data, bird_img_1);
        currentPillar->blocks[i].controlIndex=index_data;
        index_data++;
    }
    return index_data;
}


int movePillar(struct pillar *currentPillar, int offset) {
    for (int i = 0; i < currentPillar->block_number; i++) {
        currentPillar->blocks[i].x = currentPillar->blocks[i].x - offset;
        if(currentPillar->blocks[i].x < -64){
            currentPillar->blocks[i].x = 512;
        }
        setLargeSpriteControl(currentPillar->blocks[i].controlIndex, 64, 64, currentPillar->blocks[i].x, currentPillar->blocks[i].y, 1);
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
    linePrintf(6 + idx_blocks, "bottom blockNumber=%d", bottomPillar->block_number);
}

void createATop(int x, int height,struct pillar *topPillar) {
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



void movePillarThread(void *param) {
    struct pillar topPillar;
    struct pillar bottomPillar;
    // x position
    int x_position = 200;
    // bottom
    int bottom_height = 110;
    //gap
    int gap = 0;
    // top
    int top_height = 210 - bottom_height - gap;
    // the index of the large sprites
    int index_data = 5;

    /**
     * step 1: generate pillars
     */
    createABottom(x_position, bottom_height, &bottomPillar);
    createATop(x_position, top_height, &topPillar);

    index_data = createAPillar(&bottomPillar,index_data);
    index_data = createAPillar(&topPillar,index_data);
    linePrintf(14, "bottom 0 control id=%d,top 0 id=%d", bottomPillar.blocks[0].controlIndex,topPillar.blocks[0].controlIndex);

    /**
     * step 2: move pillars in a dead-loop
     */
    int movement_offset = 1;
    while (1) {
        if (topPillar.block_number != 0) {
            movePillar(&topPillar, movement_offset);
        }
        if (bottomPillar.block_number != 0) {
            movePillar(&bottomPillar, movement_offset);
        }
        threadYield();
    }
}

