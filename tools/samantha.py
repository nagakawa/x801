import argparse
import builtins as bi
import array
import importlib
import io
import numpy as np
from PIL import Image
from typing import Optional, Tuple

# Thanks http://blackpawn.com/texts/lightmaps/default.html for the algo

class Rectangle:
  def __init__(self, x1: int, y1: int, x2: int, y2: int):
    self.x1 = x1
    self.y1 = y1
    self.x2 = x2
    self.y2 = y2
  def can_fit(w: int, h: int) -> int:
    rw = x2 - x1
    rh = y2 - y1
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
  def insert(self, image, name):
    if self.child0 or self.child1: # Not a leaf
      new_node = self.child0.insert(image, name)
      if new_node: return new_node
      return self.child1.insert(image, name)
    else: # Leaf
      if self.image: return None # Already occupied
      fit_score = self.rect.can_fit(image.width, image.height)
      if fit_score == 0: return None
      if fit_score == 1:
        self.image = image
        return self
      # Split into 2 nodes
      rect_width = self.rect.x2 - self.rect.x1
      rect_height = self.rect.y2 - self.rect.y1
      dw = rect_width - image.width
      dh = rect_height - image.height
      if dw > dh:
        self.child0 = Node(Rectangle(
          self.rect.x1, self.rect.y1,
          self.rect.x1 + image.width, self.rect.y2
        ))
        self.child1 = Node(Rectangle(
          self.rect.x1 + image.width, self.rect.y1,
          self.rect.x2, self.rect.y2
        ))
      else:
        self.child0 = Node(Rectangle(
          self.rect.x1, self.rect.y1,
          self.rect.x1, self.rect.y1 + image.height
        ))
        self.child1 = Node(Rectangle(
          self.rect.x1, self.rect.y1 + image.height,
          self.rect.x2, self.rect.y2
        ))
      return self.child0.insert(image, name)


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
    help='how big to make the atlas')

args = parser.parse_args()

destinationPrefix = args.destinationImage
destinationTable = args.destinationTable
sourceImagesDir = args.images
asize = args.size

def new_image():
  return Image.new(
    "RGBA",
    (asize, asize),
    (0, 0, 0, 0)
  )

image = new_image()
pageno = 0
atlas = Node(Rectangle(0, 0, asize, asize))
fh = open(args.destinationTable[0], "w")

def save():
  image.save(destinationPrefix + "." + str(pageno) + ".png")

for fn in pathlib.Path(sourceImagesDir).glob("*.png"):
  newImage = Image.open(str(fn))
  if newImage.width > asize or newImage.height > asize:
    error("Image is too big! %d x %d with asize = %d" %
      (newImage.width, newImage.height, asize))
  shortname = fn.name
  shortname = shortname[0:shortname.rfind('.')]
  while true:
    location = atlas.insert(newImage, shortname)
    if location: break
    save()
    image = new_image()
    pageno += 1
    atlas = Node(Rectangle(0, 0, asize, asize))
  fh.write("%s %d %d %d %d %d\n" % (
    shortname, pageno,
    location.rect.x1, location.rect.y1,
    location.rect.x2, location.rect.y2
  ))

save()
fh.close()

exit()

for (name, index) in table.items():
  fh.write(name + " " + str(index) + "\n")
