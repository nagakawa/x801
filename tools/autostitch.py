# Texture Stitcher for x801
# Usage: python3 tools/autostitch.py assets/textures/terrain/blocks.png assets/textures/terrain/blocks.tti asset-temp/textures/terrain/gimpfiles

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
cumul = 0
capat = 8
image = Image.new(
  "RGBA",
  (tsize, capat * tsize),
  (0, 0, 0, 0)
)
table = {}

for fn in pathlib.Path(args.images[0]).glob("*.png"):
  if cumul >= capat:
    capat <<= 1
    image = image.crop((0, 0, tsize, capat * tsize))
  newImage = Image.open(str(fn))
  image.paste(newImage, (0, cumul * tsize))
  shortname = fn.name
  shortname = shortname[0:shortname.rfind('.')]
  table[shortname] = cumul
  cumul += 1

image.save(args.destinationImage[0])

fh = open(args.destinationTable[0], "w")

for (name, index) in table.items():
  fh.write(name + " " + str(index) + "\n")

fh.close()