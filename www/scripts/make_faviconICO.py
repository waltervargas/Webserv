#!/usr/bin/env python3

from PIL import Image

#create a blank image, standard size: 32x32 (I used pink)
img = Image.new("RGBA", (32, 32), (252, 216, 247, 255))
# Open your PNG or create a blank one

img.save("../favicon.ico", format="ICO")

