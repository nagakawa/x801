# Takes an emp and creates an entity part.
# Usage: python3 tools/model-compiler.py
#   <something.emp> <something.osa>

import argparse
from collections import *
import fparser
import pathlib
import pprint
from pyquaternion import Quaternion
import re
import simpleeval
from PIL import Image

def error(msg):
  raise InputException(msg)

def getDirectionFlags(ls):
  flags = 0
  for d in ls:
    if d in faceToOffset:
      flags |= (1 << faceToOffset[d])
  return flags

evaluator = simpleeval.SimpleEval()

def resolve(vertex, offset):
  vertex[3] += offset[0]
  vertex[4] += offset[1]

def parseTri(section, triangles):
  v1 = section['Vertex1'][0]
  v2 = section['Vertex2'][0]
  v3 = section['Vertex3'][0]
  offset = section['UVoffset'][0]
  resolve(v1, offset)
  resolve(v2, offset)
  resolve(v3, offset)
  triangles += [
    (v1, v2, v3),
  ]

def insertQuad(triangles, v1, v2, v3, v4):
  triangles += [
    (v1, v2, v3,),
    (v3, v4, v1,),
  ]

def parseQuad(section, triangles):
  v1 = section['Vertex1'][0]
  v2 = section['Vertex2'][0]
  v3 = section['Vertex3'][0]
  v4 = section['Vertex4'][0]
  offset = section['UVoffset'][0]
  resolve(v1, offset)
  resolve(v2, offset)
  resolve(v3, offset)
  resolve(v4, offset)
  insertQuad(triangles, v1, v2, v3, v4)

def cvertex(vertexXYZ, uvAxes, a, i, j, k):
  v = vertexXYZ[a][0:-1]
  v.append(uvAxes[i][2 * j])
  v.append(uvAxes[i][1 + 2 * k])
  return v

def insertCubeFace(triangles, vertexXYZ, uvAxes, a, b, c, d, i):
  v1 = cvertex(vertexXYZ, uvAxes, a, i, 0, 0)
  v2 = cvertex(vertexXYZ, uvAxes, b, i, 1, 0)
  v3 = cvertex(vertexXYZ, uvAxes, c, i, 1, 1)
  v4 = cvertex(vertexXYZ, uvAxes, d, i, 0, 1)
  insertQuad(triangles, v1, v2, v3, v4)

# vertexXYZ is in the order 01234567 (obviously)
# bit 0 is set for x+, reset for x-
# bit 1 is set for y+, reset for y-
# bit 2 is set for z+, reset for z-
# uvAxes is in the order x+ x- y+ y- z+ z-
def parseCubeHelper(triangles, vertexXYZ, offset, uvAxes):
  for uv in uvAxes:
    uv[0] += offset[0]
    uv[1] += offset[1]
    uv[2] += offset[0]
    uv[3] += offset[1]
  # top face 4576
  insertCubeFace(triangles, vertexXYZ, uvAxes, 4, 5, 7, 6, 4)
  # bottom face 0132
  insertCubeFace(triangles, vertexXYZ, uvAxes, 0, 1, 3, 2, 5)
  # east face 1375
  insertCubeFace(triangles, vertexXYZ, uvAxes, 1, 3, 7, 5, 0)
  # west face 0264
  insertCubeFace(triangles, vertexXYZ, uvAxes, 0, 2, 6, 4, 1)
  # north face 2376
  insertCubeFace(triangles, vertexXYZ, uvAxes, 2, 3, 7, 6, 2)
  # south face 0154
  insertCubeFace(triangles, vertexXYZ, uvAxes, 0, 1, 5, 4, 3)

def parseCube(section, triangles):
  pass

def quaternionFromCommands(comm):
  i = 0
  n = len(comm)
  q = Quaternion()
  while i < n:
    label = comm[i]
    i += 1
    # if we have "identity", then we need not do anything
    if label == 'identity': continue
    flag = -1
    # axis rotations: this is degrees ccw, looking from
    # origin toward the positive end of the appropriate axis
    if label == 'aroundXAxis': flag = 0
    if label == 'aroundYAxis': flag = 1
    if label == 'aroundZAxis': flag = 2
    if flag == -1:
      fparser.error("Unknown label " + str(label) + "! Check docs for details.")
    # These commands take one argument; gobble that up
    if i >= n: # no more entries
      fparser.error("Reached end of argument list!")
    angle = 0
    try:
      angle = float(comm[i])
    except Exception:
      fparser.error(comm[i] + " is not a number")
    i += 1
    axis = [
      [1, 0, 0],
      [0, 1, 0],
      [0, 0, 1]
    ][flag]
    # the argument is the rotation CCW when our line of sight is parallel
    # to the positive axis, so CW when it's pointing toward negative
    rotation = Quaternion(axis=axis, degrees=-angle)
    # NOTE: the order matters!
    q = (rotation * q).normalised
  return q

parser = argparse.ArgumentParser(description='Compile entity parts for x801.')
parser.add_argument('sourcePart', metavar='sf', type=str, nargs=1,
    help='the part source')
parser.add_argument('output', metavar='out', type=str, nargs=1,
    help='the path of the part')

args = parser.parse_args()

emp = fparser.parse(open(args.sourcePart[0]))

out = open(args.output[0], "wb")

pp = pprint.PrettyPrinter(indent=1, compact=True)

empComponents = emp[1]['Components'][0]['Component']
#pp.pprint(empComponents)
empFaces = emp[1]['Faces']
pp.pprint(empFaces)

components = []
componentIndicesByName = {}
controlAngles = defaultdict(list)
faces = []

componentIndicesByName['ground'] = 0xFFFFFFFF

# Fill in componentIndicesByName
for i in range(len(empComponents)):
  comp = empComponents[i]
  name = comp['Name'][0][0]
  componentIndicesByName[name] = i

pp.pprint(componentIndicesByName)

# Fill in components
for i in range(len(empComponents)):
  comp = empComponents[i]
  name = comp['Name'][0][0]
  parent = componentIndicesByName[comp['Parent'][0][0]]
  offsetCoordinates = comp['OffsetCoordinates'][0]
  axisScale = comp['AxisScale'][0]
  offsetAngleCommands = comp['OffsetAngle'][0]
  offsetAngle = quaternionFromCommands(offsetAngleCommands)
  # ... What do we do with these?
  # Let's put them in a tuple and push that into a list.
  components.append((name, parent, offsetCoordinates, axisScale, offsetAngle))
  controls = comp['Control']
  for control1 in controls:
    for control in control1:
      controlAngles[control].append(i)

pp.pprint(components)
pp.pprint(controlAngles)

for c in mdfFaces['CubeRanged']:
  pass

exit()

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