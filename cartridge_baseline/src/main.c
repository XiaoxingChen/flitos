#include <stdint.h>
#include <string.h>
#include "video_api.h"
#include "thread_api.h"

void initVideoSetting()
{
    setDisplayMode(DISPLAY_MODE_GRAPHICS);
    uint8_t background_color_palette[4*0x100];
    for(int i = 0; i < 0x100; i++)
    {
        background_color_palette[4 * i + 0] = 0xff - i;
        background_color_palette[4 * i + 1] = 0;
        background_color_palette[4 * i + 2] = i;
        background_color_palette[4 * i + 3] = 0xff;
    }
    initBackgroundPalette(0, background_color_palette, 4*0x100);
    initSpritePalette(1, background_color_palette, 4*0x100);
    initTransparentSpritePalette(0);

    setBackgroundControl(0, 0, 0, 0, 0);
    setLargeSpriteControl(0, 64, 64, 30, 30, 1);
    uint8_t background_img[DISPLAY_WIDTH_PX * DISPLAY_HEIGHT_PX];
    for(int i = 0; i < DISPLAY_HEIGHT_PX; i++)
    {
        for(int j = 0; j < DISPLAY_WIDTH_PX; j++)
        {
            int cnt = i * DISPLAY_WIDTH_PX + j;
            background_img[cnt] = cnt & 0xff;
        }
    }
    setBackgroundDataImage(0, background_img);

    setLargeSpriteDataImage(0, background_img + DISPLAY_WIDTH_PX * 100);
}

void threadSpriteMotion(void* param)
{
    uint32_t x_pos = 100;
    while(1)
    {
        x_pos += 1;
        if(x_pos > 450) x_pos = 50;
        setLargeSpriteControl(0, 64, 64, x_pos, 30, 1);
        threadYield();
    }
}

int main(int argc, char const *argv[])
{
    initVideoSetting();
    threadCreate(threadSpriteMotion, NULL);
    while(1) threadYield();
    return 0;
}