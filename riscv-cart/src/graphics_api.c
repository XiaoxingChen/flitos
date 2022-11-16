#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "/usr/include/png.h"
#include "video_api.h"


#define NUM_SPRITE_AVAILABLE 64

#define NUM_IMAGE_AVAILABLE 16
#define LENGTH_IMAGE_NAME 128

#define LENGTH_PALETTE 256
#define NUM_PALETTE_AVAILABLE 4

// save image number in sprite array
int sprite[NUM_SPRITE_AVAILABLE] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

uint32_t palettes[NUM_PALETTE_AVAILABLE][LENGTH_PALETTE];

// Images related
uint8_t imageData[NUM_IMAGE_AVAILABLE][64 * 64];
int imagePaletteRange[NUM_IMAGE_AVAILABLE][2];
int imageSpriteCount[NUM_IMAGE_AVAILABLE];
char imageNames[NUM_IMAGE_AVAILABLE][LENGTH_IMAGE_NAME];
bool imageUsed[NUM_IMAGE_AVAILABLE];


/*******************DECLARATION************************/
int find_image(char* imageName);
int process_image(char* imageName);
bool is_name_same(char* name1, char* name2);

/*********************PUBLIC***************************/

int sprite_open(char* texture_name) {


    int imgIndex = process_image(texture_name);
    if (imgIndex == -1) {
        return -1;
    }
    
    for (int i = 0; i < NUM_SPRITE_AVAILABLE; i++) {
        if (sprite[i] == -1) {
            sprite[i] = imgIndex;
            setLargeSpriteDataImage(i, imageData[imgIndex]);
            return i;
        }
    }
    return -1;
}

//int sprite_close(int fd) {
//    spritUsed[fd] = false;
//    setLargeSpriteDataImage(fd, -1);
//}

//int sprite_set_scale(int fd, float x, float y) {
//
//}

/*********************PRIVATE***************************/

void process_png(char *filename) {
    FILE *fp = fopen(filename, "rb");
}

int process_image(char* imageName) {
    int imageIndex = find_image(imageName);
    if (imageIndex != -1) {
        return imageIndex;
    }



    //for (int i = 0; i < image)
}

int add_image() {
    return -1;
}

int remove_image() {
    return -1;
}

int find_image(char* imageName) {
    for (int i = 0; i < NUM_IMAGE_AVAILABLE; i++) {
        if (imageUsed[i] &&
            is_name_same(imageName, imageNames[i])) {
            return i;
        }
    }
    return -1;
}

bool is_name_same(char* name1, char* name2) {
    for (int i = 0; i < LENGTH_IMAGE_NAME; i++) {
        if (name1[i] != name2[i]) {
            return false;
        }
    }
    return true;
}