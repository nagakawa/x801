# Mobinfo Compiler for X801
# Usage: python3 tools/mobinfo-compiler.py asset-src/mobs/lostSoul.emp assets/mobs/lostSoul.mob

import argparse
import array
import fparser
import io
from xutils import *

parser = argparse.ArgumentParser(description='Compile a mobinfo file for Experiment801.')
parser.add_argument('source', metavar='source', type=str, nargs=1,
    help='the source path')
parser.add_argument('destination', metavar='destination', type=str, nargs=1,
    help='the destination path')

args = parser.parse_args()
source = open(args.source[0], "r")
dest = open(args.destination[0], "wb")

headers, sections, dsCount = fparser.parse(source)

typesByName = {
  "none": 0,
  "player": 1,
  "npc": 2,
  "common1": 3,
  "common2": 4,
  "elite": 5,
  "boss": 6
}
schoolsByName = {
  "fire": 0,
  "ice": 1,
  "lightning": 2,
  "water": 3,
  "earth": 4,
  "wind": 5,
  "light": 6,
  "darkness": 7
}

# Write misc data

miscSec = sections["Misc"][0]

writeInt(dest, miscSec["Rank"][0][0], 2)
writeInt(dest, typesByName[miscSec["Type"][0][0]], 2)
writeString16(dest, miscSec["Internal"][0][0])
writeString16(dest, miscSec["Name"][0][0])
writeString16(dest, miscSec["Title"][0][0])
writeString16(dest, miscSec["Texture"][0][0])

# Write mob stats

statSec = sections["Stats"][0]

writeInt(dest, statSec["Level"][0][0], 2)
writeInt(dest, schoolsByName[statSec["School"][0][0]], 2)
writeInt(dest, statSec["PowerPip"][0][0], 2)
writeInt(dest, len(statSec["SEntry"]), 2)
writeBigint(dest, statSec["Health"][0][0])

for entry in statSec["SEntry"]:
  writeInt(dest, schoolsByName[entry["School"][0][0]], 2)
  def getStat(name):
    try: return entry[name][0][0]
    except: return 0
  writeInt(dest, getStat("Accuracy"), 2)
  writeInt(dest, getStat("Pierce"), 2)
  writeBigint(dest, getStat("Damage"))
  writeBigint(dest, getStat("Defence"))