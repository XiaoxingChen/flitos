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


struct pipeBlock {
    /**
     * x and y represent the top-left point of the pipe
     */
    int x;
    int y;
    /**
     * height represents the height in unit of pixel
     */
    int height;

    int controlIndex;
};
struct pipe {
    /**
     * x and y represent the top-left point of the pipe
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
    struct pipeBlock blocks[3];
};

volatile struct pipe bottomPillar;
volatile struct pipe topPillar;

volatile int index_data = 4;
void createAPipe(struct pipe currentPipe) {
    for (int i = 0; i < currentPipe.block_number; i++) {
        struct pipeBlock tempBlock = currentPipe.blocks[i];
        setLargeSpriteControl(tempBlock.controlIndex, 64, 64, tempBlock.x, tempBlock.y, 1);
        setLargeSpriteDataImage(tempBlock.controlIndex, bird_img_1);
    }
}

volatile int iddddd=0;

void movePillar(struct pipe currentPillar,int offset){
    linePrintf(12+iddddd, "pillar=%d", currentPillar.block_number);
    iddddd++;

    for(int i=0;i<currentPillar.block_number;i++){
        struct pipeBlock tempBlock = currentPillar.blocks[i];
        linePrintf(12+iddddd, "control id=%d, new x=%d,y=%d", tempBlock.controlIndex,tempBlock.x,tempBlock.y);
        tempBlock.x=tempBlock.x-offset;
        setLargeSpriteControl(tempBlock.controlIndex, 64, 64, tempBlock.x, tempBlock.y, 1);
        iddddd++;
    }
}

struct pipe createABottom(int x, int height){
    bottomPillar.x = x;
    bottomPillar.y = 288 - height;
    bottomPillar.height = height;
    // blocks of bottom
    int remaining_height = height;
    int idx_blocks = 0;
    while (remaining_height > 0) {
        struct pipeBlock tempBlock;
        tempBlock.height = 64;
        tempBlock.x = bottomPillar.x;
        tempBlock.y = idx_blocks * tempBlock.height + 288 - bottomPillar.height;
        tempBlock.controlIndex=index_data++;
        bottomPillar.blocks[idx_blocks] = tempBlock;

        remaining_height = remaining_height - tempBlock.height;
        idx_blocks++;
    }
    bottomPillar.block_number = idx_blocks;
    linePrintf(6+idx_blocks, "bottom blockNumber=%d",bottomPillar.block_number );
    return bottomPillar;
}

struct pipe createATop(int x, int height){
    topPillar.height=height;
    topPillar.x=x;
    topPillar.y=0;

    int data_height =64;
    int remaining = height%data_height;

    int numbers = height/data_height;

    int idx_blocks =0;
    if(remaining!=0){
        topPillar.block_number = (numbers+1);
        // generate a block
        struct pipeBlock remaining_block;
        remaining_block.height = remaining;
        remaining_block.x = topPillar.x;
        remaining_block.y = remaining-data_height;
        remaining_block.controlIndex=index_data++;

        topPillar.blocks[idx_blocks]=remaining_block;
        idx_blocks++;
    }else{
        topPillar.block_number = numbers;
    }

    for(int i=0;i<numbers;i++){
        struct pipeBlock tempBlock;
        tempBlock.height = data_height;
        tempBlock.x = topPillar.x;
        tempBlock.y = remaining+64*i;
        tempBlock.controlIndex=index_data++;
        topPillar.blocks[idx_blocks]=tempBlock;
        idx_blocks++;
    }

    return topPillar;
}


void createAPairOfPipe() {
    // x position
    int x_position=200;
    // bottom
    int bottom_height = 110;
    //gap
    int gap = 0;
    // top
    int top_height = 210-bottom_height-gap;

    // build the bottom pillar
    createABottom(x_position,bottom_height);

    // build the top pillar
    createATop(x_position,top_height);

    //draw a pair of pillars
    createAPipe(bottomPillar);
    createAPipe(topPillar);

    linePrintf(30, "bottom 0 control id = %d",bottomPillar.blocks[0].controlIndex );

    // store the pair of pillars into the list of pillars.
}



void movePillarThread(void* param){
    int movement_offset=1;
    while(1){
        if(topPillar.block_number!=0){
            movePillar(topPillar,movement_offset);
        }
        if(bottomPillar.block_number!=0){
            movePillar(bottomPillar,movement_offset);
        }
        threadYield();
    }
}

