import argparse
import builtins as bi
import array
import io
import numpy as np
import pathlib
from PIL import Image
from typing import Optional, Tuple

def writeInt(f, n, b):
  f.write((n % (1 << 8 * b)).to_bytes(b, byteorder='little'))

def writeIntSigned(f, n, b):
  f.write((n).to_bytes(b, byteorder='little', signed=True))

def writeString16(f, s):
  b = s.encode()
  writeInt(f, len(b), 2)
  f.write(b)

# Thanks http://blackpawn.com/texts/lightmaps/default.html for the algo

class Rectangle:
  def __init__(self, x1: int, y1: int, x2: int, y2: int):
    self.x1 = x1
    self.y1 = y1
    self.x2 = x2
    self.y2 = y2
  def can_fit(self, w: int, h: int) -> int:
    rw = self.x2 - self.x1
    rh = self.y2 - self.y1
    if rw < w or rh < h: return 0
    if rw == w and rh == h: return 1
    return 2

class Node:
  def __init__(self, rect):
    self.child0 = None
    self.child1 = None
    self.image = None
    self.image_name = None
    self.rect = rect
  def insert(self, image, name, margin=0):
    if self.child0 or self.child1: # Not a leaf
      new_node = self.child0.insert(image, name, margin)
      if new_node: return new_node
      return self.child1.insert(image, name, margin)
    else: # Leaf
      netwidth = image.width + margin
      netheight = image.height + margin
      if self.image: return None # Already occupied
      fit_score = self.rect.can_fit(netwidth, netheight)
      if fit_score == 0: return None
      if fit_score == 1:
        self.image = image
        self.image_name = name
        return self
      # Split into 2 nodes
      rect_width = self.rect.x2 - self.rect.x1
      rect_height = self.rect.y2 - self.rect.y1
      dw = rect_width - netwidth
      dh = rect_height - netheight
      if dw > dh:
        self.child0 = Node(Rectangle(
          self.rect.x1, self.rect.y1,
          self.rect.x1 + netwidth, self.rect.y2
        ))
        self.child1 = Node(Rectangle(
          self.rect.x1 + netwidth, self.rect.y1,
          self.rect.x2, self.rect.y2
        ))
      else:
        self.child0 = Node(Rectangle(
          self.rect.x1, self.rect.y1,
          self.rect.x2, self.rect.y1 + netheight
        ))
        self.child1 = Node(Rectangle(
          self.rect.x1, self.rect.y1 + netheight,
          self.rect.x2, self.rect.y2
        ))
      return self.child0.insert(image, name, margin)


class InputException(Exception):
  def __init__(self, message):
    self.message = message

def error(msg):
  raise InputException(msg)

parser = argparse.ArgumentParser(description='Stitch textures of any size for Experiment801.')
parser.add_argument('destinationImage', metavar='di', type=str,
    help='the prefix for the destination images. Images are named di.0.png, di.1.png, etc.')
parser.add_argument('destinationTable', metavar='dd', type=str,
    help='the destination path for the table')
parser.add_argument('images', metavar='images', type=str,
    help='the path with the appropriate images')
parser.add_argument('--size', metavar='size', type=int, default=4096,
    help='how big to make the atlas (default=4096)')
parser.add_argument('--margin', metavar='size', type=int, default=0,
    help='width of margins (default=0)')
parser.add_argument('--verbose', metavar='verbose',
    action='store_const', const=True, default=False,
    help='enables verbose output')
parser.add_argument('--binary', metavar='binary',
    action='store_const', const=True, default=False,
    help='use binary instead of text output')

args = parser.parse_args()

destinationPrefix = args.destinationImage
destinationTable = args.destinationTable
sourceImagesDir = args.images
asize = args.size
margin = args.margin
verbose = args.verbose
binary = args.binary

def new_image():
  return Image.new(
    "RGBA",
    (asize, asize),
    (0, 0, 0, 0)
  )

image = new_image()
pageno = 0
atlas = Node(Rectangle(0, 0, asize, asize))
fh = open(args.destinationTable, "wb" if binary else "w")

def save():
  image.save(destinationPrefix + "." + str(pageno) + ".png")

name_and_size = []

for fn in pathlib.Path(sourceImagesDir).glob("*.png"):
  newImage = Image.open(str(fn))
  name_and_size.append([fn, min(newImage.width, newImage.height), False])

name_and_size.sort(key=lambda s: s[1], reverse=True)
remaining = len(name_and_size)

while remaining > 0:
  # Insert everything that fits
  for entry in name_and_size:
    fn, size, used = entry
    if used: continue
    newImage = Image.open(str(fn))
    if newImage.width > asize or newImage.height > asize:
      error("Image is too big! %d x %d with asize = %d" %
        (newImage.width, newImage.height, asize))
    shortname = fn.name
    shortname = shortname[0:shortname.rfind('.')]
    location = atlas.insert(newImage, shortname, margin)
    if not location: continue
    if binary:
      writeString16(fh, shortname)
      writeInt(fh, pageno, 2)
      writeInt(fh, location.rect.x1, 2)
      writeInt(fh, location.rect.y1, 2)
      writeInt(fh, location.rect.x2, 2)
      writeInt(fh, location.rect.y2, 2)
    else:
      fh.write("%s %d %d %d %d %d\n" % (
        shortname, pageno,
        location.rect.x1, location.rect.y1,
        location.rect.x2, location.rect.y2
      ))
    image.paste(newImage, (location.rect.x1, location.rect.y1))
    if verbose: print("Pasted image " + shortname)
    remaining -= 1
    entry[2] = True
  if verbose: print("Page " + str(pageno) + " is full")
  save()
  image = new_image()
  pageno += 1
  atlas = Node(Rectangle(0, 0, asize, asize))

save()
fh.close()
