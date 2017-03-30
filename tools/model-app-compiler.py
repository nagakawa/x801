# Takes an mdf and an mda an creates a mod.
# Usage: python3 tools/model-compiler.py
#   <something.mdf> <something.mda>
#   <something.tti>

import argparse
import fparser
import pathlib
import pprint
import re
import readtable
import simpleeval
from PIL import Image

def error(msg):
  raise InputException(msg)

def writeInt(f, n, b):
  f.write((n % (1 << 8 * b)).to_bytes(b, byteorder='little'))

def writeIntSigned(f, n, b):
  f.write((n).to_bytes(b, byteorder='little', signed=True))

parser = argparse.ArgumentParser(description='Compile model applications for x801.')
parser.add_argument('sourceApplication', metavar='sa', type=str, nargs=1,
    help='the application source')
parser.add_argument('tableFunction', metavar='tf', type=str, nargs=1,
    help='the .tab file containing function names')
parser.add_argument('tableBlocks', metavar='tb', type=str, nargs=1,
    help='the directory of individual block moel .tab files')
parser.add_argument('tableTextures', metavar='tb', type=str, nargs=1,
    help='the .tti file with the texture names')
parser.add_argument('output', metavar='out', type=str, nargs=1,
    help='the path of the model application file')

args = parser.parse_args()

mdf = fparser.parse(open(args.sourceApplication[0]))

functionTable = readtable.read(args.tableFunction[0])
textureTable = readtable.read(args.tableTextures[0])

out = open(args.output[0], "wb")

pp = pprint.PrettyPrinter(indent=1, compact=True)

modelName = mdf[0]['Function'][0][0]
locationTable = readtable.read(args.tableBlocks[0] + '/' + modelName + '.tab')

parameters = mdf[1]['Textures'][0]

modelNumber = functionTable[modelName]
textureCount = len(locationTable)

writeInt(out, modelNumber, 4)
writeInt(out, textureCount, 1)

arr = [0] * textureCount
for loc, tex in parameters.items():
  location = locationTable[loc]
  texture = textureTable[tex[0][0]]
  arr[location] = texture

for a in arr:
  writeInt(out, a, 4)

out.close()
