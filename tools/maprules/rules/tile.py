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

def get(_, x, y):
  return _[y][x]

def put(_, x, y, t):
  _[y][x] = t % (1 << 32)

def fill(_, x1, y1, x2, y2, t):
  for x in range(x1, x2 + 1):
    for y in range(y1, y2 + 1):
      put(_, x, y, t)

otherModules = ['math', 'random']

safeBuiltins = copymodule(bi)
safeBuiltins.__import__ = fakeImport

yourfuncs = {
  "get": get,
  "put": put,
  "fill": fill,
  "__builtins__": safeBuiltins,
}

for module in otherModules:
  yourfuncs.update(importlib.import_module(module).__dict__)

def export_TILE(sec):
  output = io.BytesIO()
  layers = sec['Layer']
  layerCount = len(layers)
  writeInt(output, layerCount, 2)
  for layer in layers:
    width = layer['Width'][0][0]
    height = layer['Height'][0][0]
    startx = layer['StartX'][0][0]
    starty = layer['StartY'][0][0]
    writeInt(output, width, 2)
    writeInt(output, height, 2)
    writeInt(output, startx, 2)
    writeInt(output, starty, 2)
    tilesec = layer['Tiles'][0]
    tiles = np.zeros((height, width), np.int32)
    codess = tilesec["Code"]
    for codes in codess:
      for code in codes:
        exec(code, {**yourfuncs, "_": tiles})
    output.write(tiles.data)
  return output.getvalue()
