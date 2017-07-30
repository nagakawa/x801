# Texture Stitcher for x801
# Usage: python3 tools/autostitch.py assets/textures/terrain/blocks assets/textures/terrain/blocks.tti asset-temp/textures/terrain/gimpfiles

import argparse
import pathlib
import re
from PIL import Image

def error(msg):
  raise InputException(msg)

parser = argparse.ArgumentParser(description='Stitch textures for Experiment801.')
parser.add_argument('destinationImage', metavar='di', type=str, nargs=1,
    help='the destination path for the image')
parser.add_argument('destinationTable', metavar='dd', type=str, nargs=1,
    help='the destination path for the table')
parser.add_argument('images', metavar='images', type=str, nargs=1,
    help='the path with the appropriate images')

args = parser.parse_args()

tsize = 32
# Reasonable requirement for max texture size
# according to http://feedback.wildfiregames.com/report/opengl/feature/GL_MAX_TEXTURE_SIZE
asize = 4096
tdim = asize // tsize
cumul = 0
pageno = 0
capat = tdim * tdim
image = Image.new(
  "RGBA",
  (asize, asize),
  (0, 0, 0, 0)
)
table = {}

def save():
  image.save(args.destinationImage[0] + "." + str(pageno) + ".png")

for fn in pathlib.Path(args.images[0]).glob("*.png"):
  if cumul >= capat:
    # No more room.
    # Save the current image and start a new page
    save()
    image = Image.new(
      "RGBA",
      (asize, asize),
      (0, 0, 0, 0)
    )
    pageno += 1
    cumul -= capat
  newImage = Image.open(str(fn))
  x = cumul % tdim
  y = cumul // tdim
  image.paste(newImage, (x * tsize, y * tsize))
  shortname = fn.name
  shortname = shortname[0:shortname.rfind('.')]
  table[shortname] = cumul + capat * pageno
  cumul += 1

save()

fh = open(args.destinationTable[0], "w")

for (name, index) in table.items():
  fh.write(name + " " + str(index) + "\n")

fh.close()