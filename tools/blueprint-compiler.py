# Takes an emc and creates an entity blueprint.
# Usage: python3 tools/model-compiler.py
#   <something.emc> <something.ebp>

import argparse
from collections import *
import fparser
import pathlib
import pprint
from pyquaternion import Quaternion
from qhelper import *
import re
import readtable
import simpleeval
import struct
from PIL import Image

def error(msg):
  raise InputException(msg)

def writeInt(f, n, b):
  f.write((n % (1 << 8 * b)).to_bytes(b, byteorder='little'))

def writeIntSigned(f, n, b):
  f.write((n).to_bytes(b, byteorder='little', signed=True))

def writeFloat(f, x):
  f.write(struct.pack('>f', x))

def writeString16(f, s):
  b = s.encode()
  writeInt(f, len(b), 2)
  f.write(b)

parser = argparse.ArgumentParser(description='Compile entity blueprints for x801.')
parser.add_argument('sourceBP', metavar='sb', type=str, nargs=1,
    help='the blueprint source')
parser.add_argument('tableDirectory', metavar='tabs', type=str, nargs=1,
    help='the path of the directory containing the part tabs')
parser.add_argument('output', metavar='out', type=str, nargs=1,
    help='the path of the compiled blueprint')

args = parser.parse_args()

tabDir = args.tableDirectory[0]
tableCache = {}
def getCompID(part, comp):
  if not (part in tableCache):
    tableCache[part] = readtable.read(tabDir + "/" + part + ".tab")
  return tableCache[part][comp]

emc = fparser.parse(open(args.sourceBP[0]))

emcParts = emc[1]["Parts"][0]["Part"]
parentsByName = {}
parts = []

for i in range(0, len(emcParts)):
  part = emcParts[i]
  partName = part["PartName"][0][0]
  partID = partName
  if "PartID" in part:
    partID = part["PartID"][0][0]
  parentsByName[partID] = i

for part in emcParts:
  partName = part["PartName"][0][0]
  partID = partName
  if "PartID" in part:
    partID = part["PartID"][0][0]
  offsetCoordinates = part["OffsetCoordinates"][0]
  offsetAngle = quaternionFromCommands(part["OffsetAngle"][0])
  texture = part["Texture"][0][0]
  parentID = part["Parent"][0][0]
  parent = 0xFFFFFFFF if parentID == "" else parentsByName[parentID]
  componentName = "ground"
  try:
    componentName = part["Component"][0][0]
  except:
    pass
  component = 0xFFFFFFFF if componentName == "ground" \
    else getCompID(parentID, componentName)
  parts.append((
    partName,
    partID,
    texture,
    parent,
    component,
    offsetAngle,
    offsetCoordinates
  ))

out = open(args.output[0], "wb")

# Header
writeInt(out, len(parts), 4)

# Parts
for part in parts:
  writeString16(out, part[0])
  writeString16(out, part[1])
  writeString16(out, part[2])
  writeInt(out, part[3], 4)
  writeInt(out, part[4], 4)
  for i in range(4):
    writeFloat(out, part[5][i])
  for i in range(3):
    writeFloat(out, part[6][i])
