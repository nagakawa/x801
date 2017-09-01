import builtins as bi
import array
import importlib
import io
import numpy as np

class InputException(Exception):
  def __init__(self, message):
    self.message = message

def error(msg):
  raise InputException(msg)

def writeInt(f, n, b):
  f.write((n % (1 << 8 * b)).to_bytes(b, byteorder='little'))

# Thanks http://stackoverflow.com/a/11173076/3130218
def copymodule(old):
  new = type(old)(old.__name__, old.__doc__)
  new.__dict__.update(old.__dict__)
  return new

def fakeImport(*args, **kwargs):
  print("Sucks")

UP = 0
DOWN = 1
NORTH = 2
SOUTH = 3
EAST = 4
WEST = 5

oriNorthRawToCanonical = [
  # UP or DOWN
  [ NORTH, SOUTH, EAST, WEST ],
  # NORTH or SOUTH
  [ UP, DOWN, EAST, WEST ],
  # EAST or WEST
  [ UP, DOWN, NORTH, SOUTH ],
]

oriNorthCanonicalToRaw = [
  [ -1, -1, 0, 1, 2, 3 ],
  [ 0, 1, -1, -1, 2, 3 ],
  [ 0, 1, 2, 3, -1, -1 ],
]

oriNorthRawToEast = [
  # UP or DOWN
  [ EAST, WEST, SOUTH, NORTH ],
  # NORTH or SOUTH
  [ WEST, EAST, UP, DOWN ],
  # EAST or WEST
  [ NORTH, SOUTH, DOWN, UP ],
]

oriTable = None
oriTableInverse = None

def generateTable():
  oriTable = [[0] * 6 for i in range(0, 48)]
  oriTableInverse = [[0] * 6 for i in range(0, 48)]
  for i in range(0, 48):
    up = i >> 3
    northRaw = (i >> 1) & 3
    north = oriNorthRawToCanonical[up >> 1][northRaw]
    isFlipped = (i & 1) != 0
    east = oriNorthRawToEast[up >> 1][northRaw] ^ (up & 1) ^ isFlipped
    oriTable[i][UP] = up
    oriTable[i][DOWN] = up ^ 1
    oriTable[i][NORTH] = north
    oriTable[i][SOUTH] = north ^ 1
    oriTable[i][EAST] = east
    oriTable[i][WEST] = east ^ 1
    oriTableInverse[i][up] = UP
    oriTableInverse[i][up ^ 1] = DOWN
    oriTableInverse[i][north] = NORTH
    oriTableInverse[i][north ^ 1] = SOUTH
    oriTableInverse[i][east] = EAST
    oriTableInverse[i][east ^ 1] = WEST

def dirNameToNumber(name):
  if isinstance(name, int): return name
  return {
    "U": 0,
    "D": 1,
    "N": 2,
    "S": 3,
    "E": 4,
    "W": 5
  }[name]

def oriNameToNumber(name):
  if isinstance(name, int): return name
  d1 = dirNameToNumber(name[0])
  d2 = dirNameToNumber(name[1])
  d3 = dirNameToNumber(name[2])
  d2raw = oriNorthCanonicalToRaw[d1 >> 1][d2]
  if (d2raw == -1):
    error(str(name[1]) + " incompatible with " + str(name[0]))
  d3x = oriNorthRawToEast[d1 >> 1][d2] ^ (d1 & 1)
  if d3 != d3x and d3 != (d3x ^ 1):
    error(str(name[2]) + " incompatible with " +
      str(name[0]) + " and " + str(name[1]))
  isFlipped = d3 == (d3x ^ 1)
  return (d1 << 3) | (d2 << 1) | d3

# Block format:
# bit 31: solid?
# bit 16 - 30: decoration
# bit 0 - 15: base
def applyMethod(meth, new, old):
  return (old & ~meth) | (new & meth)

METHOD_ALL = 0xFFFFFFFF
METHOD_BASE = 0x0000FFFF
METHOD_DEC = 0x7FFF0000
METHOD_SOLID = 0x80000000

class Chunk:
  def __init__(self, x, y, z):
    self.x = x
    self.y = y
    self.z = z
    self.array = np.zeros(4096, np.int32)
  def get(self, x, y):
    index = (x << 4) | y
    return self.array[index]
  def put(self, x, y, b):
    index = (x << 4) | y
    self.array[index] = b
  def write(self, output):
    writeInt(output, self.x, 2)
    writeInt(output, self.y, 2)
    writeInt(output, self.z, 2)
    writeInt(output, 0, 2)
    output.write(self.array.data)

class TileSec:
  def __init__(self, table):
    self.chunks = {}
    self.table = table
  def createChunk(self, x, y, z):
    self.chunks[(x, y, z)] = Chunk(x, y, z)
  def getn(self, x, y, z):
    try:
      chunk = self.chunks[(x >> 4, y >> 4, z)]
      return chunk.get(x & 15, y & 15)
    except KeyError:
      return 0
  def put(self, x, y, z, b, method=METHOD_ALL):
    try:
      chunk = self.chunks[(x >> 4, y >> 4, z)]
      old = chunk.get(x & 15, y & 15)
      chunk.put(x & 15, y & 15, applyMethod(method, b, old))
    except KeyError:
      chunk = Chunk(x >> 4, y >> 4, z)
      old = chunk.get(x & 15, y & 15)
      chunk.put(x & 15, y & 15, applyMethod(method, b, old))
      self.chunks[(x >> 4, y >> 4, z)] = chunk
  def write(self, output):
    chunks = self.chunks.values()
    writeInt(output, len(chunks), 2)
    for c in chunks:
      c.write(output)
  def fill(self, x1, y1, z1, x2, y2, z2, b, method=METHOD_ALL):
    for x in range(x1, x2 + 1):
      for y in range(y1, y2 + 1):
        for z in range(z1, z2 + 1):
          self.put(x, y, z, b, method)
  def getid(self, name):
    if name == "air": return 0
    return self.table[name] + 1


otherModules = ['math', 'random']

safeBuiltins = copymodule(bi)
safeBuiltins.__import__ = fakeImport

yourfuncs = {
  "TileSec": TileSec,
  "Chunk": Chunk,
  "__builtins__": safeBuiltins,
}

for module in otherModules:
  yourfuncs.update(importlib.import_module(module).__dict__)

def export_TIL2(sec, table):
  output = io.BytesIO()
  tiles = TileSec(table)
  codess = sec["Code"]
  for codes in codess:
    for code in codes:
      exec(code, {**yourfuncs, "_": tiles})
  tiles.write(output)
  return output.getvalue()
