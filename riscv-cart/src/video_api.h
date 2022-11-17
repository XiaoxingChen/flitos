#if !defined(_VIDEO_API_H_)
#define _VIDEO_API_H_

#include <stdint.h>

void setLargeSpriteControl(uint32_t idx, uint32_t h, uint32_t w, uint32_t x, uint16_t y, uint32_t palette);
void setBackgroundControl(uint32_t idx, uint32_t x, uint32_t y, uint32_t z, uint32_t palette);
int initBackgroundPalette(uint32_t idx, uint8_t * addr, uint32_t mem_len);
int initSpritePalette(uint32_t idx, uint8_t * addr, uint32_t mem_len);
int initTransparentSpritePalette(uint32_t idx);
int setBackgroundDataImage(uint32_t idx, uint8_t * addr);
int setLargeSpriteDataImage(uint32_t idx, uint8_t * addr);
#define DISPLAY_MODE_TEXT 0
#define DISPLAY_MODE_GRAPHICS 1
void setDisplayMode(uint32_t mode);

// internal interfaces, export for debug only
#if 1
void wrapCall(uint32_t handle, uint32_t addr, uint32_t mem_len);
#endif

#endif // _VIDEO_API_H_
