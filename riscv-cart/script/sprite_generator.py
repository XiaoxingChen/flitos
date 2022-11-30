import os
from pickletools import uint8
import cv2
import numpy as np

script_folder = os.path.abspath(os.path.dirname(__file__))

def colorMap(im, color_idx):
    ret = np.ndarray(im.shape[:2], dtype=np.uint8)
    for i in range(im.shape[0]):
        for j in range(im.shape[1]):
            ret[i,j] = color_idx[tuple(im[i,j])]
    return ret

def generateCArray(img, name):
    ret = 'uint8_t ' + name + '[' + str(img.shape[0] * img.shape[1]) + '] = {\n'

    idx_per_line = 30
    cnt = 0
    for idx in img.ravel():
        ret += (str(idx) + ',')
        cnt += 1
        if cnt > idx_per_line:
            # print(cnt)
            cnt = 0
            ret += '\n'
    ret += '};\n'
    return ret 

def doubleImage(img):
    img = cv2.resize(img, (img.shape[1]*2,img.shape[0]*2), interpolation=cv2.INTER_NEAREST)
    return img

def fitToLargeSprite(img):
    img_out = np.zeros([64,64], dtype=np.uint8)
    img_out[:img.shape[0], :img.shape[1]] = img
    return img_out

def generateFullImageData():
    raw_img = cv2.imread(script_folder + '/../img/flappy_bird.png', cv2.IMREAD_UNCHANGED)
    
    output_file = os.path.join(script_folder, '..', 'src', 'img_data_gen.c')
    color_set = set()
    raw_img[197,396,:] = [255,255,255]
    raw_img[198,394,:] = [255,255,255]
    for i in range(raw_img.shape[0]):
        for j in range(raw_img.shape[1]):
            color_set.add(tuple(raw_img[i,j]))

    palette = list(color_set)
    # pick transparent color to first
    palette.remove((255,255,255))
    palette.insert(0, (255,255,255))

    color_idx = {}
    for i, c in enumerate(palette):
        color_idx[c] = int(i)

    output_palette = np.array(palette)
    # attach alpha channel
    output_palette = np.hstack([output_palette, 255*np.ones([len(color_set), 1], dtype=np.uint8)])
    output_palette[0,:] *= 0

    background_img = colorMap(raw_img[70:70+144, 3:3+128, :], color_idx)
    background_img = np.hstack([background_img, np.flip(background_img, 1)])
    background_img = cv2.resize(background_img, (512,288), interpolation=cv2.INTER_NEAREST)
    print("backround shape: ", background_img.shape)

    bird_img_data_01 = colorMap(raw_img[187:187+12, 381:381+17, :], color_idx)
    bird_img_data_01 = fitToLargeSprite(doubleImage(bird_img_data_01))
    print("bird_img_data_01 shape: ", bird_img_data_01.shape)

    


    bird_img_data_02 = colorMap(raw_img[213:213+12, 381:381+17, :], color_idx)
    bird_img_data_02 = fitToLargeSprite(doubleImage(bird_img_data_02))

    bird_img_data_03 = colorMap(raw_img[239:239+12, 381:381+17, :], color_idx)
    bird_img_data_03 = fitToLargeSprite(doubleImage(bird_img_data_03))

    pillar_up_head = colorMap(raw_img[3:3+32, 180:180+26, :], color_idx)
    pillar_up_head = fitToLargeSprite(doubleImage(pillar_up_head))

    pillar_up_body = colorMap(raw_img[20:20+32, 180:180+26, :], color_idx)
    pillar_up_body = fitToLargeSprite(doubleImage(pillar_up_body))

    pillar_down_head = colorMap(raw_img[138:138+32, 152:152+26, :], color_idx)
    pillar_down_head = fitToLargeSprite(doubleImage(pillar_down_head))

    pillar_down_body = colorMap(raw_img[100:100+32, 152:152+26, :], color_idx)
    pillar_down_body = fitToLargeSprite(doubleImage(pillar_down_body))

    gameover_01 = colorMap(raw_img[173:173+21, 152:152+64, :], color_idx)
    gameover_01 = fitToLargeSprite(gameover_01)

    gameover_02 = colorMap(raw_img[173:173+21, 152+64:248, :], color_idx)
    gameover_02 = fitToLargeSprite(gameover_02)

    with open(output_file, 'w') as f:
        f.write('#include <stdint.h>\n\n')
        f.write(generateCArray(output_palette, 'bird_color_palette') )
        f.write(generateCArray(background_img, 'bird_background_img') )
        f.write(generateCArray(bird_img_data_01, 'bird_img_0') )
        f.write(generateCArray(bird_img_data_02, 'bird_img_1') )
        f.write(generateCArray(bird_img_data_03, 'bird_img_2') )

        f.write(generateCArray(pillar_up_head, 'pillar_img_head_up') )
        f.write(generateCArray(pillar_up_body, 'pillar_img_body_up') )
        f.write(generateCArray(pillar_down_head, 'pillar_img_head_down') )
        f.write(generateCArray(pillar_down_body, 'pillar_img_body_down') )

        f.write(generateCArray(gameover_01, 'gameover_img_01') )
        f.write(generateCArray(gameover_02, 'gameover_img_02') )
        
    
    # print(len(palette))
    # print((palette))
    # print((255,255,255) in palette)


def generateBird():
    im_3frames = cv2.imread(script_folder + '/../docs/img/flappy.png', cv2.IMREAD_UNCHANGED)
    output_folder = os.path.join(script_folder, '..', 'bin')
    color_set = set()
    for i in range(im_3frames.shape[0]):
        for j in range(im_3frames.shape[1]):
            color_set.add(tuple(im_3frames[i,j]))
    palette = list(color_set)
    palette.remove((0,0,0,0))
    palette.insert(0,(0,0,0,0))
    color_idx = {}
    for i, c in enumerate(palette):
        color_idx[c] = int(i)

    # print(im_3frames.shape)
    # im1 = colorMap(im_3frames[:,:34,:], color_idx)
    # im2 = colorMap(im_3frames[:,34:68,:], color_idx)
    # im3 = colorMap(im_3frames[:,68:,:], color_idx)
    idx_imgs = []
    for i in range(3):
        img_tmp = colorMap(im_3frames[:,i*34:(i+1)*34,:], color_idx)
        img_full = np.zeros([64, 64], dtype=np.uint8)
        img_full[:img_tmp.shape[0], :img_tmp.shape[1]] = img_tmp
        idx_imgs.append(img_full)
    print(repr(idx_imgs[2].ravel()))


    
    


if __name__ == "__main__":
    np.set_printoptions(linewidth=100, threshold=np.inf)
    generateFullImageData()