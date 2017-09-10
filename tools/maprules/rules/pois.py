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

def export_pOIS(sec, table, dtable):
  output = io.BytesIO()
  poisWithNPCs = sec["Poin"]
  emptyPOIs = sec["Poi"]
  writeInt(output, len(poisWithNPCs) + len(emptyPOIs), 2)
  for poi in poisWithNPCs:
    [x, y, z, tn, to, tit, nam] = poi
    writeInt(output, x, 2)
    writeInt(output, y, 2)
    writeInt(output, z, 1)
    writeInt(output, 1, 1)
    writeString16(output, tn)
    writeInt(output, to, 1)
    writeString16(output, tit)
    writeString16(output, nam)
  for poi in emptyPOIs:
    [x, y, z] = poi
    writeInt(output, x, 2)
    writeInt(output, y, 2)
    writeInt(output, z, 1)
    writeInt(output, 0, 1)
  return output.getvalue()
