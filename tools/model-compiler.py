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

def resolve2(vertex, vlookup):
  for i in range(len(vertex)):
    if i == 0:
      vertex[i] = vlookup[vertex[i]]
    elif type(vertex[i]) is str:
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
  ti = -1
  if texSym in texlookup:
    ti = texlookup[texSym]
  else:
    ti = len(texlookup)
    texlookup[texSym] = ti
  v1 = section['Vertex1'][0]
  v2 = section['Vertex2'][0]
  v3 = section['Vertex3'][0]
  resolve2(v1, vlookup)
  resolve2(v2, vlookup)
  resolve2(v3, vlookup)
  triangles += [
    (v1, v2, v3, ti, occlusionFlags, pt),
  ]

def parseQuad(section, triangles, vlookup, texlookup):
  occlusionFlags = getDirectionFlags(section['Occlusion'][0])
  pt = False
  try:
    pt = section['PartiallyTransparent'][0][0]
  except KeyError:
    pass
  except IndexError:
    pass
  texSym = section['Texture'][0][0]
  ti = -1
  if texSym in texlookup:
    ti = texlookup[texSym]
  else:
    ti = len(texlookup)
    texlookup[texSym] = ti
  v1 = section['Vertex1'][0]
  v2 = section['Vertex2'][0]
  v3 = section['Vertex3'][0]
  v4 = section['Vertex4'][0]
  resolve2(v1, vlookup)
  resolve2(v2, vlookup)
  resolve2(v3, vlookup)
  resolve2(v4, vlookup)
  triangles += [
    (v1, v2, v3, ti, occlusionFlags, pt),
    (v3, v4, v1, ti, occlusionFlags, pt),
  ]

parser = argparse.ArgumentParser(description='Stitch textures for Experiment801.')
parser.add_argument('sourceFunction', metavar='sf', type=str, nargs=1,
    help='the model function source')
parser.add_argument('output', metavar='out', type=str, nargs=1,
    help='the path of the model')
parser.add_argument('outputTab', metavar='outt', type=str, nargs=1,
    help='the path of the texture name table')

args = parser.parse_args()

mdf = fparser.parse(open(args.sourceFunction[0]))

out = open(args.output[0], "wb")

pp = pprint.PrettyPrinter(indent=1, compact=True)

hitboxType = hitboxTypes[mdf[0]['Hitbox'][0][0]]
opacityFlags = getDirectionFlags(mdf[0]['Opaque'][0])

vertices = []
verticesByName = {}
mdfVertices = mdf[1]['Vertices'][0]['Vertex']

for i in range(len(mdfVertices)):
  v = mdfVertices[i]
  name = v[0]
  vertex = v[1:]
  resolve(vertex)
  vertices += [vertex]
  verticesByName[name] = i

triangles = []
mdfFaces = mdf[1]['Faces'][0]

texlookup = {}

for q in mdfFaces['Quad']:
  parseQuad(q, triangles, verticesByName, texlookup)
for t in mdfFaces['Triangle']:
  parseTriangle(t, triangles, verticesByName, texlookup)

def writeInt(f, n, b):
  f.write((n % (1 << 8 * b)).to_bytes(b, byteorder='little'))

def writeIntSigned(f, n, b):
  f.write((n).to_bytes(b, byteorder='little', signed=True))

writeInt(out, hitboxType, 2)
writeInt(out, opacityFlags, 1)
writeInt(out, len(texlookup), 1)
writeInt(out, len(vertices), 2)
writeInt(out, len(triangles), 2)

for v in vertices:
  writeIntSigned(out, int(v[0] * 128), 1)
  writeIntSigned(out, int(v[1] * 128), 1)
  writeIntSigned(out, int(v[2] * 128), 1)

for t in triangles:
  for i in range(3):
    writeInt(out, t[i][0], 2)
  for i in range(3):
    writeInt(out, int(t[i][1] * 128), 1)
    writeInt(out, int(t[i][2] * 128), 1)
  writeInt(out, t[3], 1)
  writeInt(out, t[4] | (t[5] << 1), 1)

out.close()

out2 = open(args.outputTab[0], "w")

for name, index in texlookup.items():
  out2.write(name + " " + str(index) + "\n")

out2.close()