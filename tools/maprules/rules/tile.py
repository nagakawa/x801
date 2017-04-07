import builtins as bi
import array
import importlib
import io
import numpy as np

def writeInt(f, n, b):
  f.write((n % (1 << 8 * b)).to_bytes(b, byteorder='little'))

# Thanks http://stackoverflow.com/a/11173076/3130218
def copymodule(old):
  new = type(old)(old.__name__, old.__doc__)
  new.__dict__.update(old.__dict__)
  return new

def fakeImport(*args, **kwargs):
  print("Sucks")

class Chunk:
  def __init__(self, x, y, z):
    self.x = x
    self.y = y
    self.z = z
    self.array = np.zeros(4096, np.int32)
  def get(self, x, y, z):
    index = (z << 8) | (x << 4) | y
    return array[index]
  def put(self, x, y, z, b):
    index = (z << 8) | (x << 4) | y
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
      chunk = self.chunks[(x >> 4, y >> 4, z >> 4)]
      return chunk.get(x & 15, y & 15, z & 15)
    except KeyError:
      return 0
  def putn(self, x, y, z, b):
    try:
      chunk = self.chunks[(x >> 4, y >> 4, z >> 4)]
      chunk.put(x & 15, y & 15, z & 15, b)
    except KeyError:
      chunk = Chunk(x >> 4, y >> 4, z >> 4)
      chunk.put(x & 15, y & 15, z & 15, b)
      self.chunks[x >> 4, y >> 4, z >> 4] = chunk
  def put(self, x, y, z, b):
    if b == "air": self.putn(x, y, z, 0)
    else: self.putn(x, y, z, self.table[b])
  def write(self, output):
    chunks = self.chunks.values()
    writeInt(output, len(chunks), 2)
    for c in chunks:
      c.write(output)
  def fill(self, x1, y1, z1, x2, y2, z2, t):
    n = self.table[t] if t != "air" else 0
    for x in range(x1, x2 + 1):
      for y in range(y1, y2 + 1):
        for z in range(z1, z2 + 1):
          self.putn(x, y, z, n)

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

def export_TIL3(sec, table):
  output = io.BytesIO()
  tiles = TileSec(table)
  codess = sec["Code"]
  for codes in codess:
    for code in codes:
      exec(code, {**yourfuncs, "_": tiles})
  tiles.write(output)
  return output.getvalue()
