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

def export_XDAT(sec, table, dtable):
  output = io.BytesIO()
  worldname = sec["WorldName"][0][0]
  areaname = sec["AreaName"][0][0]
  bgcolour = sec["BackgroundColour"][0]
  writeString16(output, worldname)
  writeString16(output, areaname)
  for i in range(0, 3):
    val = min(1, max(0, bgcolour[i]))
    writeInt(output, int(255 * val), 1)
  return output.getvalue()
