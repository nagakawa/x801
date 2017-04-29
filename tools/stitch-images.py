# Texture Stitcher for x801
# Usage: python3 tools/stitch-iamges.py asset-src/textures/terrain/terrain-0.ttd assets/textures/terrain/terrain-0.png asset-temp/textures/terrain/gimpfiles

import argparse
import re
from PIL import Image

def error(msg):
  raise InputException(msg)

parser = argparse.ArgumentParser(description='Stitch textures for Experiment801.')
parser.add_argument('source', metavar='source', type=str, nargs=1,
    help='the source path')
parser.add_argument('destination', metavar='destination', type=str, nargs=1,
    help='the destination path')
parser.add_argument('images', metavar='images', type=str, nargs=1,
    help='the path with the appropriate images')

args = parser.parse_args()

tsize = 0
dim = (0, 0)
image = None

commentRE = re.compile(r"\s*#")
tsizeRE = re.compile(r'tsize (\d+)')
dimRE = re.compile(r'dim (\d+) (\d+)')
tlRE = re.compile(r'(\S+) @ (\d+) (\d+)')

for line in open(args.source[0]):
  line = line.strip()
  if re.match(commentRE, line): continue
  tsmatch = re.fullmatch(tsizeRE, line)
  dimmatch = re.fullmatch(dimRE, line)
  if tsmatch:
    tsize = int(tsmatch.group(1))
  elif dimmatch:
    dim = (int(dimmatch.group(1)), int(dimmatch.group(2)))
  else:
    if tsize == 0 or dim[0] == 0 or dim[1] == 0:
      error("tsize and dim must be set first")
    tlMatch = re.fullmatch(tlRE, line)
    if not tlMatch:
      error("Invalid line: " + tlMatch)
    tname = tlMatch.group(1)
    tx = int(tlMatch.group(2))
    ty = int(tlMatch.group(3))
    if not image:
      image = Image.new(
        "RGBA",
        (dim[0] * tsize, dim[1] * tsize),
        (0, 0, 0, 0)
      )
    newImage = Image.open(args.images[0] + "/" + tname + ".png")
    newImage = newImage.resize((tsize, tsize))
    image.paste(newImage, (tx * tsize, ty * tsize))

image.save(args.destination[0])