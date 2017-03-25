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

faceToOffset = {
  'up': 0,
  'down': 1,
  'north': 2,
  'south': 3,
  'east': 4,
  'west': 5,
}

hitboxTypes = {
  'none': 0,
  'full': 1,
  'top_half': 2,
  'bottom_half': 3,
}

def getDirectionFlags(ls):
  flags = 0
  for d in ls:
    if d in faceToOffset:
      flags |= (1 << faceToOffset[d])
  return flags

evaluator = simpleeval.SimpleEval(names=lambda x: parameters[x])

def resolve(vertex):
  for i in range(len(vertex)):
    if type(vertex[i]) is str:
      vertex[i] = evaluator.eval(vertex[i])

def parseTri(section, triangles, table, parameters):
  occlusionFlags = getDirectionFlags(section['Occlusion'][0])
  pt = False
  try:
    pt = section['PartiallyTransparent'][0][0]
  except KeyError:
    pass
  except IndexError:
    pass
  texSym = section['Texture'][0][0]
  texPath = parameters[texSym]
  texPos = table[texPath]
  v1 = section['Vertex1'][0]
  v2 = section['Vertex2'][0]
  v3 = section['Vertex3'][0]
  resolve(v1)
  resolve(v2)
  resolve(v3)
  v1[4] += texPos
  v2[4] += texPos
  v3[4] += texPos
  triangles += [
    (v1, v2, v3, occlusionFlags, pt),
  ]

def parseQuad(section, triangles, table, parameters):
  occlusionFlags = getDirectionFlags(section['Occlusion'][0])
  pt = False
  try:
    pt = section['PartiallyTransparent'][0][0]
  except KeyError:
    pass
  except IndexError:
    pass
  texSym = section['Texture'][0][0]
  texPath = parameters[texSym]
  texPos = table[texPath]
  v1 = section['Vertex1'][0]
  v2 = section['Vertex2'][0]
  v3 = section['Vertex3'][0]
  v4 = section['Vertex4'][0]
  resolve(v1)
  resolve(v2)
  resolve(v3)
  resolve(v4)
  v1[4] += texPos
  v2[4] += texPos
  v3[4] += texPos
  v4[4] += texPos
  triangles += [
    (v1, v2, v3, occlusionFlags, pt),
    (v3, v4, v1, occlusionFlags, pt),
  ]

parser = argparse.ArgumentParser(description='Stitch textures for Experiment801.')
parser.add_argument('sourceApplication', metavar='sa', type=str, nargs=1,
    help='the model application file')
parser.add_argument('sourceTable', metavar='sd', type=str, nargs=1,
    help='the texture location table')
parser.add_argument('sourceFunctions', metavar='sf', type=str, nargs=1,
    help='the path of the directory of source functions')
parser.add_argument('output', metavar='out', type=str, nargs=1,
    help='the path of the model')

args = parser.parse_args()

table = readtable.read(args.sourceTable[0])
mda = fparser.parse(open(args.sourceApplication[0]))
mdf = fparser.parse(open(args.sourceFunctions[0] + '/' + mda[0]['Function'][0][0] + '.mdf'))

out = open(args.output[0], "wb")

pp = pprint.PrettyPrinter(indent=1, compact=True)

parameters = {}
for key, value in mda[1]['Parameters'][0].items():
  v = value[0]
  if v[0] == 'sc':
    parameters[key] = v[1]
  else:
    parameters[key] = v[1:]

hitboxType = hitboxTypes[mdf[0]['Hitbox'][0][0]]
opacityFlags = getDirectionFlags(mdf[0]['Opaque'][0])

triangles = []
mdfFaces = mdf[1]['Faces'][0]

for q in mdfFaces['Quad']:
  parseQuad(q, triangles, table, parameters)
for t in mdfFaces['Triangle']:
  parseTriangle(t, triangles, table, parameters)

def writeInt(f, n, b):
  f.write((n % (1 << 8 * b)).to_bytes(b, byteorder='little'))

def writeIntSigned(f, n, b):
  f.write((n).to_bytes(b, byteorder='little', signed=True))

writeInt(out, hitboxType, 2)
writeInt(out, opacityFlags, 1)
writeInt(out, len(triangles), 2)

def writeVertex(out, v):
  writeIntSigned(out, int(v[0] * 128), 1)
  writeIntSigned(out, int(v[1] * 128), 1)
  writeIntSigned(out, int(v[2] * 128), 1)
  writeInt(out, int(v[3] * 128), 1)
  writeInt(out, int(v[4] * 128), 4)

for t in triangles:
  writeVertex(out, t[0])
  writeVertex(out, t[1])
  writeVertex(out, t[2])
  writeInt(out, t[3] | (t[4] << 6), 1)

out.close()