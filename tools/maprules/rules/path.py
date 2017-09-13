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

def writeString16(f, s):
  b = s.encode()
  writeInt(f, len(b), 2)
  f.write(b)

def export_pATH(sec, table, dtable):
  output = io.BytesIO()
  paths = sec["Path"]
  writeInt(output, len(paths), 2)
  for path in paths:
    writeInt(output, path["Z"][0][0], 1)
    nodes = path["Node"]
    mobs = path["Mob"]
    bosses = path["Boss"]
    writeInt(output, len(nodes), 1)
    writeInt(output, len(mobs), 1)
    writeInt(output, len(bosses), 1)
    for node in nodes:
      writeInt(output, node[0], 2)
      writeInt(output, node[1], 2)
    for mob in mobs:
      writeInt(output, mob[1], 1)
      writeString16(output, mob[0])
    for boss in bosses:
      writeString16(output, boss[0])
  return output.getvalue()
