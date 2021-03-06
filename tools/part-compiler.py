# Takes an emp and creates an entity part.
# Usage: python3 tools/model-compiler.py
#   <something.emp> <something.osa>

import argparse
from collections import *
import fparser
import pathlib
import pprint
from pyquaternion import Quaternion
from qhelper import *
import re
import simpleeval
import struct
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
  v[0] = componentIndicesByName[v[0]]
  vertex[4] += offset[0]
  vertex[5] += offset[1]

def parseTriangle(section, triangles):
  v1 = section['Vertex1'][0]
  v2 = section['Vertex2'][0]
  v3 = section['Vertex3'][0]
  offsetl = section['UVoffset']
  offset = offsetl[0] if offsetl else [0, 0]
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
  offsetl = section['UVoffset']
  offset = offsetl[0] if offsetl else [0, 0]
  resolve(v1, offset)
  resolve(v2, offset)
  resolve(v3, offset)
  resolve(v4, offset)
  insertQuad(triangles, v1, v2, v3, v4)

def cvertex(vertexXYZ, uvAxes, a, i, j, k):
  v = vertexXYZ[a][:]
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

anames = ['xp', 'xm', 'yp', 'ym', 'zp', 'zm']

def parseCube(section, triangles):
  vertexXYZ = []
  for i in range(8):
    v = section['Vertex' + str(i)][0]
    v[0] = componentIndicesByName[v[0]]
    vertexXYZ.append(v)
  offsetl = section['UVoffset']
  offset = offsetl[0] if offsetl else [0, 0]
  uvAxes = []
  for s in anames:
    axis = section['UV' + s][0]
    uvAxes += axis
  parseCubeHelper(triangles, vertexXYZ, offset, uvAxes)

def parseCubeRanged(section, triangles, componentIndicesByName):
  xr = section['XRange'][0]
  yr = section['YRange'][0]
  zr = section['ZRange'][0]
  componentName = section['Component'][0][0]
  componentIndex = componentIndicesByName[componentName]
  vertexXYZ = []
  for i in range(8):
    b0 = (i & 1) != 0
    b1 = (i & 2) != 0
    b2 = (i & 4) != 0
    v = [componentIndex, xr[b0], yr[b1], zr[b2]]
    vertexXYZ.append(v)
  offsetl = section['UVoffset']
  offset = offsetl[0] if offsetl else [0, 0]
  uvAxes = []
  for s in anames:
    axis = section['UV' + s][0]
    uvAxes.append(axis)
  parseCubeHelper(triangles, vertexXYZ, offset, uvAxes)

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

parser = argparse.ArgumentParser(description='Compile entity parts for x801.')
parser.add_argument('sourcePart', metavar='sf', type=str, nargs=1,
    help='the part source')
parser.add_argument('output', metavar='out', type=str, nargs=1,
    help='the path of the part')
parser.add_argument('table', metavar='tab', type=str, nargs=1,
    help='the path where the table between component names and IDs should be output')

args = parser.parse_args()

emp = fparser.parse(open(args.sourcePart[0]))

empComponents = emp[1]['Components'][0]['Component']
empFaces = emp[1]['Faces'][0]

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

for q in empFaces['Quad']:
  parseQuad(q, triangles)
for t in empFaces['Triangle']:
  parseTriangle(t, triangles)
for c in empFaces['CubeRanged']:
  parseCubeRanged(c, faces, componentIndicesByName)
for c in empFaces['Cube']:
  parseCube(c, faces)

divisor = empFaces['UVDivisor'][0][0]

hitboxSize = emp[0]['HitboxSize'][0]

out = open(args.output[0], "wb")

# Header
for i in range(3):
  writeFloat(out, hitboxSize[i])
writeInt(out, len(components), 4)
writeInt(out, len(faces), 4)
writeInt(out, len(controlAngles), 4)

# Components
for component in components:
  writeInt(out, component[1], 4)
  for i in range(4):
    writeFloat(out, component[4][i])
  for i in range(3):
    writeFloat(out, component[2][i])
  for i in range(3):
    writeFloat(out, component[3][i])

# Faces
for face in faces:
  for i in range(3):
    vertex = face[i]
    writeInt(out, vertex[0], 4)
    writeFloat(out, vertex[1])
    writeFloat(out, vertex[2])
    writeFloat(out, vertex[3])
    writeFloat(out, vertex[4] / divisor)
    writeFloat(out, vertex[5] / divisor)

# Components (again)
for component in components:
  writeString16(out, component[0])

# Control angles
for (name, comps) in controlAngles.items():
  writeString16(out, name)
  writeInt(out, len(comps), 2)
  for c in comps:
    writeInt(out, c, 4)

out.close()

tab = open(args.table[0], "w")

for (name, iden) in componentIndicesByName.items():
  if name != 'ground':
    tab.write(name + " " + str(iden) + "\n")

tab.close()