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

def generateBattleCity():
    im_3frames = cv2.imread(script_folder + '/../docs/img/sprite5551.png', cv2.IMREAD_UNCHANGED)
    # im_3frames = cv2.imread('/home/chenxx/Downloads/sprite.bmp', cv2.IMREAD_UNCHANGED)
    # print(im_3frames)
    output_folder = os.path.join(script_folder, '..', 'bin')
    color_set = set()
    for i in range(im_3frames.shape[0]):
        for j in range(im_3frames.shape[1]):
            color_set.add(tuple(im_3frames[i,j]))
    palette = list(color_set)
    print(len(palette))

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

    # print(palette)

def generateBackground():
    im_raw = cv2.imread(script_folder + '/../docs/img/background.png', cv2.IMREAD_UNCHANGED)
    print(im_raw.shape)
    color_set = set()
    for i in range(im_raw.shape[0]):
        for j in range(im_raw.shape[1]):
            color_set.add(tuple(im_raw[i,j]))
    palette = list(color_set)
    # palette.insert(0,(0,0,0,0))

    bg_width = 512
    bg_height = 288

    color_idx = {}
    for i, c in enumerate(palette):
        color_idx[c] = int(i)

    h_offset = 200
    img_full = colorMap(im_raw[h_offset:h_offset+bg_height, 0:bg_width,:], color_idx)
    # print(repr(img_full.ravel()))
    # print(len(palette))
    # print(palette)

    
    


if __name__ == "__main__":
    np.set_printoptions(linewidth=100, threshold=np.inf)
    # generateBird()
    # generateBackground()
    generateBattleCity()