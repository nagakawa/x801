# Map Compiler for X801
# Usage: python3 tools/compile-map.py asset-src/maps/map.0.0.msf assets/maps/map.0.0.map

import argparse
import array
import fparser
import io
import maprules
import readtable
import zlib

def writeInt(f, n, b):
  f.write((n % (1 << 8 * b)).to_bytes(b, byteorder='little'))

xmap = b"XMap"
version = bytes.fromhex("00 00 00 00 01 00 00 00")

parser = argparse.ArgumentParser(description='Compile a map for Experiment801.')
parser.add_argument('source', metavar='source', type=str, nargs=1,
    help='the source path')
parser.add_argument('table', metavar='table', type=str, nargs=1,
    help='the path to the texture table')
parser.add_argument('destination', metavar='destination', type=str, nargs=1,
    help='the destination path')
parser.add_argument('--level', metavar='L', type=int, nargs='?',
    help='the level of compression (-1 is default; higher numbers indicate smaller size but longer compression times)')

args = parser.parse_args()
source = open(args.source[0], "r")
dest = open(args.destination[0], "wb")
table = readtable.read(args.table[0])
level = args.level if args.level else -1

header, sections, dsCount = fparser.parse(source)

dest.write(xmap)
dest.write(version)
writeInt(dest, header['WorldID'][0][0], 2)
writeInt(dest, header['AreaID'][0][0], 2)
writeInt(dest, dsCount, 4)

for name, seclist in sections.items():
  for sec in seclist:
    bname = name.encode()
    if len(bname) != 4:
      fparser.error("Section names must be 4 bytes long. You put" + name)
    dest.write(bname)
    buf = maprules.handlers[name](sec, table=table)
    usize = len(buf)
    cbuf = zlib.compress(buf, level)
    csize = len(cbuf)
    if csize + 4 < usize:
      writeInt(dest, csize | 0x80000000, 4)
      writeInt(dest, usize, 4)
      writeInt(dest, zlib.adler32(cbuf), 4)
      dest.write(cbuf)
    else:
      writeInt(dest, usize, 4)
      writeInt(dest, zlib.adler32(buf), 4)
      dest.write(buf)
