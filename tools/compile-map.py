# Map Compiler for X801
# Usage: python3 tools/compile-map.py asset-src/maps/map.0.0.msf assets/maps/map.0.0.map

import argparse
import array
import fparser
import io
import pprint

def writeInt(f, n, b):
  f.write(n.to_bytes(b, byteorder='little'))

xmap = b"XMap"
version = bytes.fromhex("00 00 00 00 01 00 00 00")

parser = argparse.ArgumentParser(description='Compile a map for Experiment801.')
parser.add_argument('source', metavar='source', type=str, nargs=1,
    help='the source path')
parser.add_argument('destination', metavar='destination', type=str, nargs=1,
    help='the destination path')
pp = pprint.PrettyPrinter(indent=2)

args = parser.parse_args()
source = open(args.source[0], "r")
dest = open(args.destination[0], "wb")

header, sections, dsCount = fparser.parse(source)

pp.pprint(header)
pp.pprint(sections)

dest.write(xmap)
dest.write(version)
writeInt(dest, header['WorldID'][0][0], 2)
writeInt(dest, header['AreaID'][0][0], 2)
writeInt(dest, dsCount, 4)

for name, seclist in sections.items():
  for sec in seclist:
    pass
