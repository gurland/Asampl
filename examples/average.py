import numpy as np

def asampl_color(image, channel):
    return image[:, :, int(channel)].mean()
