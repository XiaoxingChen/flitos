#include "video_api.h"
#include <string.h>

extern uint8_t bird_color_palette[4*8];
extern uint8_t bird_background_color_palette[4*16];
extern uint8_t bird_img_0[64*64];
extern uint8_t bird_img_1[64*64];
extern uint8_t bird_img_2[64*64];
extern uint8_t bird_background_img[288*512];

// extern FuncWriteTargetMem writeTargetMem;

void initVideoSetting()
{
    setDisplayMode(DISPLAY_MODE_GRAPHICS);
    initBackgroundPalette(0, bird_background_color_palette, 4*16);
    // initPaletteBirdBackground(BACKGROUND_PALETTE_0);
    initSpritePalette(1, bird_color_palette, 4*8);
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