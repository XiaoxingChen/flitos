#include "video_api.h"
#include <string.h>


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

void createAPairOfPipes(struct pipe currentPipe) {
    int bottom_x = currentPipe.x;
    int bottom_y = currentPipe.y;
    setLargeSpriteControl(5, 64, 64, bottom_x, bottom_y, 1);
    setLargeSpriteDataImage(5, bird_img_1);
    struct pipe topPipe;
    topPipe.x = bottom_x;
    topPipe.height = 210 - currentPipe.height;
    topPipe.y = 0;
    setLargeSpriteControl(6, 64, 64, topPipe.x, topPipe.y, 1);
    setLargeSpriteDataImage(6, bird_img_0);
}
volatile int index_data = 4;
void createAPipe(struct pipe currentPipe) {
    for (int i = 0; i < currentPipe.block_number; i++) {
        setLargeSpriteControl(index_data, 64, 64, currentPipe.blocks[i].x, currentPipe.blocks[i].y, 1);
        setLargeSpriteDataImage(index_data, bird_img_1);
        index_data++;
    }
}


struct pipe createABottom(int x, int height){
    struct pipe bottom;
    bottom.x = x;
    bottom.y = 288 - height;
    bottom.height = height;
    // blocks of bottom
    int remaining_height = height;
    int idx_blocks = 0;
    while (remaining_height > 0) {
        struct pipeBlock tempBlock;
        tempBlock.height = 64;
        tempBlock.x = bottom.x;
        tempBlock.y = idx_blocks * tempBlock.height + 288 - bottom.height;

        bottom.blocks[idx_blocks] = tempBlock;

        remaining_height = remaining_height - tempBlock.height;
        idx_blocks++;
    }
    bottom.block_number = idx_blocks;
    return bottom;
}

struct pipe createATop(int x, int height){
    struct pipe top;
    top.height=height;
    top.x=x;
    top.y=0;

    int data_height =64;
    int remaining = height%data_height;

    int numbers = height/data_height;

    int idx_blocks =0;
    if(remaining!=0){
        top.block_number = (numbers+1);
        // generate a block
        struct pipeBlock remaining_block;
        remaining_block.height = remaining;
        remaining_block.x = top.x;
        remaining_block.y = remaining-data_height;
        top.blocks[idx_blocks]=remaining_block;
        linePrintf(20+idx_blocks, "top x=%d,y=%d",remaining_block.x,remaining_block.y );

        idx_blocks++;
    }else{
        top.block_number = numbers;
    }

    for(int i=0;i<numbers;i++){
        struct pipeBlock tempBlock;
        tempBlock.height = data_height;
        tempBlock.x = top.x;
        tempBlock.y = remaining+64*i;
        top.blocks[idx_blocks]=tempBlock;
        linePrintf(20+idx_blocks, "top x=%d,y=%d",tempBlock.x,tempBlock.y );
        idx_blocks++;
    }
    return top;
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
    struct pipe bottom= createABottom(x_position,bottom_height);


    // build the top pillar
    struct pipe top = createATop(x_position,top_height);

    createAPipe(bottom);
    createAPipe(top);

}

