# Takes an mdf and an mda an creates a mod.
# Usage: python3 tools/model-compiler.py
#   <something.mdf> <something.mda>
#   <something.tti>

import argparse
import fparser
import pathlib
import pprint
import re
from PIL import Image

def error(msg):
  raise InputException(msg)

version = bytes.fromhex("00 00 00 00 01 00 00 00")

def writeInt(f, n, b):
  f.write((n % (1 << 8 * b)).to_bytes(b, byteorder='little'))

def writeIntSigned(f, n, b):
  f.write((n).to_bytes(b, byteorder='little', signed=True))

parser = argparse.ArgumentParser(description='Stitch textures for Experiment801.')
parser.add_argument('sourceDirectory', metavar='sd', type=str, nargs=1,
    help='the directory containing the compiled mfs')
parser.add_argument('output', metavar='out', type=str, nargs=1,
    help='the path of the archive')
parser.add_argument('outputTab', metavar='outt', type=str, nargs=1,
    help='the path of the model name table')

args = parser.parse_args()

direct = pathlib.Path(args.sourceDirectory[0])

out = open(args.output[0], "wb")
out2 = open(args.outputTab[0], "w")

out.write(b'XMDF')
out.write(version)
writeInt(out, 0, 4)

count = 0

for fname in direct.glob("*.cmf"):
  name = str(fname)
  fh = open(name, "rb")
  contents = fh.read()
  fh.close()
  shortname = fname.name
  shortname = shortname[0:shortname.rfind('.')]
  out2.write(shortname + " " + str(count) + "\n")
  out.write(contents)
  count += 1

out.seek(12, 0)
writeInt(out, count, 4)
out.close()
out2.close()