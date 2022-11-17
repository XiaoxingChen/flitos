#include <stdint.h>
#include <string.h>
#include "video_api.h"
#include "hook_functions.h"
#include "nanoprintf.h"

uint32_t writeTargetMem(uint32_t handle, uint32_t src_addr, uint32_t len);
uint32_t writeTarget(uint32_t handle, uint32_t value);
// extern FuncWriteTargetMem writeTargetMem;
// extern FuncWriteTarget writeTarget;

void setLargeSpriteControl(uint32_t idx, uint32_t h, uint32_t w, uint32_t x, uint16_t y, uint32_t palette)
{
    uint32_t larget_sprite_handle_offset = 0xFF114;
    uint32_t handle = larget_sprite_handle_offset + idx * 4;
    h = (h - 33) & 0x1F;
    w = (w - 33) & 0x1F;
    y = (y + 64) & 0x1FF;
    x = (x + 64) & 0x3FF;
    palette = palette & 0x3;
    uint32_t value = (h << 26) + (w << 21) + (y << 12) + (x << 2) + palette;
    writeTarget(handle, value);
}

void setBackgroundControl(uint32_t idx, uint32_t x, uint32_t y, uint32_t z, uint32_t palette)
{
    uint32_t background_handle_offset = 0xFF100;
    uint32_t handle = background_handle_offset + idx * 4;
    y = (y + 288) & 0x3FF;
    x = (x + 512) & 0x3FF;
    z = (z + 0) & 0x7;
    palette = palette & 0x3;
    uint32_t value = (z << 22) + (y << 12) + (x << 2) + palette;
    writeTarget(handle, value);
}
extern uint8_t bird_background_img[288*512];
int writeIndexedTarget(uint32_t offset, uint32_t mem_size, uint32_t idx, uint8_t * addr, uint32_t mem_len)
{
    uint32_t handle = offset + idx * mem_size;
    if(mem_len > mem_size) return -1;
    writeTargetMem(handle, (uint32_t)addr, mem_len);
    // writeTargetMem((uint32_t)0x00, (uint32_t)bird_background_img, 512*10);
    return 0;
}

void wrapCall(uint32_t handle, uint32_t addr, uint32_t mem_len)
{
    writeTargetMem(handle, addr, mem_len);
}
int initBackgroundPalette(uint32_t idx, uint8_t * addr, uint32_t mem_len)
{
    return writeIndexedTarget(0xFC000, 0x400, idx, addr, mem_len);
}

int initSpritePalette(uint32_t idx, uint8_t * addr, uint32_t mem_len)
{
    return writeIndexedTarget(0xFD000, 0x400, idx, addr, mem_len);
}

int initTransparentSpritePalette(uint32_t idx)
{
    uint32_t data_len = 0x400;
    uint8_t data[0x400];
    memset(data, 0, data_len);

    return writeIndexedTarget(0xFD000, data_len, idx, data, data_len);
}

int setBackgroundDataImage(uint32_t idx, uint8_t * addr)
{
    return writeIndexedTarget(0x000, 0x24000, idx, addr, 288*512);
}

int setLargeSpriteDataImage(uint32_t idx, uint8_t * addr)
{
    return writeIndexedTarget(0xB4000, 0x1000, idx, addr, 64*64);
}

void setDisplayMode(uint32_t mode)
{
    writeTarget(0xFF414, mode & 0x1);
}

int linePrintf(uint32_t line_idx, const char *format, ...)
{
#if 1
    uint32_t line_width = 0x40;
    char vsnprintf_buff[0x40];
    va_list args;
    int n = line_width;
    va_start(args, format);
	n = npf_vsnprintf(vsnprintf_buff, line_width, format, args);
	va_end(args);

    if(n > line_width) n = line_width;
    return writeIndexedTarget(0xFE800, line_width, line_idx, vsnprintf_buff, n);

#endif
}