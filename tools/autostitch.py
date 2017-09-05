# Texture Stitcher for x801
# Usage: python3 tools/autostitch.py assets/textures/terrain/blocks asset-src/textures/terrain/gimpfiles/blocknames.tti  assets/textures/terrain/blocks.tti asset-temp/textures/terrain/gimpfiles

import argparse
import fparser
import pathlib
import re
import readtable
from PIL import Image

parser = argparse.ArgumentParser(description='Stitch textures for Experiment801.')
parser.add_argument('destinationImage', metavar='di', type=str, nargs=1,
    help='the destination path for the image')
parser.add_argument('sourceTable', metavar='sd', type=str, nargs=1,
    help='the source path for the table')
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
st = args.sourceTable[0]
# name -> id
nametrans = lambda x: x
if st != "*":
  nametable = readtable.read(st)
  nametrans = lambda x: nametable[x]
table = {}

def save():
  image.save(args.destinationImage[0] + "." + str(pageno) + ".png")

for fn in pathlib.Path(args.images[0]).glob("*.png"):
  # Add file entry
  shortname = fn.name
  shortname = shortname[0:shortname.rfind('.')]
  if not shortname in nametable:
    fparser.error("Name not found: " + shortname)
  myid = nametrans(shortname)
  table[myid] = cumul + capat * pageno
  # Try to open image
  newImage = Image.open(str(fn))
  if newImage.height > tsize:
    fparser.error("Image is too tall: %d > %d", newImage.height, tsize)
  # Write image
  nSlots = (newImage.height + tsize - 1) // tsize;
  progress = 0
  while progress < nSlots:
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
    x = cumul % tdim
    y = cumul // tdim
    pasteAmt = min(nSlots, tdim - x)
    region = newImage.crop(
      (x * tsize, 0, (x + pasteAmt) * tsize, tsize)
    )
    image.paste(newImage, (x * tsize, y * tsize))
    cumul += pasteAmt
    progress += pasteAmt

save()

fh = open(args.destinationTable[0], "w")

for (name, index) in table.items():
  fh.write(str(name) + " " + str(index) + "\n")

fh.close()