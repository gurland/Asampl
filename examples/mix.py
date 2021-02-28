import PIL
from PIL import Image
import numpy as np

def asampl_apply(img):
    img2 = Image.open('data/sour.png')
    img2 = np.transpose(np.array(img2)[::, ::, :3], axes=[1,0,2])

    img[:img2.shape[0], :img2.shape[1], ::] += img2 // 2
    return img
    # return img / 2 + img2 / 2
