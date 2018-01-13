## TODO support direct quantities

import argparse
import array
import fparser
import io
import pathlib
import pprint
import readtable
import zlib
from xutils import *

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

targetsByName = {
  "enemyTarget": 0,
  "allyTarget": 1,
  "allEnemies": 2,
  "allAllies": 3,
  "self": 4,
}

# Various other properties managed by SpellIndex::Metadata's flags field.
def getFlags(attrs):
  flag = 0
  for attr in attrs:
    if flag == "singleEnemy": flag |= 1
    elif flag == "singleAlly": flag |= 2
  return flag

def parseQuantity(sec):
  tp = sec["Type"][0][0]
  if tp == "range":
    return (
      0,
      1, 0, sec["Min"][0][0],
      1, 0, sec["Max"][0][0],
    )

def parseStep(sec, quantitiesByLabel):
  tp = sec["Type"][0][0]
  if tp == "damage":
    return (
      0,
      1, 0, schoolsByName[sec["School"][0][0]],
      1, 1, quantitiesByLabel[sec["DamageAmt"][0][0]],
      1, 0, targetsByName[sec["Target"][0][0]],
    )
  elif tp == "healing":
    return (
      1,
      1, 1, quantitiesByLabel[sec["HealAmt"][0][0]],
      1, 0, targetsByName[sec["Target"][0][0]],
    )

parser = argparse.ArgumentParser(
    description='Compile spell behaviours for Experiment801.')
parser.add_argument('source', metavar='source', type=str,
    help='the source path, containing the .emps')
parser.add_argument('destination', metavar='destination', type=str,
    help='the destination path')

args = parser.parse_args()

source = args.source
destination = args.destination

sourceFiles = pathlib.Path(source).glob("*.emp")

quantities = []
steps = []
spells = []

pp = pprint.PrettyPrinter(indent=2, compact=True)

for sourceFile in sourceFiles:
  with open(sourceFile) as fh:
    contents = fparser.parse(fh)
    print("File " + str(sourceFile) + ":\n")
    pp.pprint(contents)
    print("\n")
    metasec = contents[1]["Meta"][0]
    meta = [
      sourceFile.stem,
      len(steps),
      0,
      metasec["MinSpeed"][0][0],
      metasec["MaxSpeed"][0][0],
      metasec["Accuracy"][0][0],
      schoolsByName[metasec["School"][0][0]],
      metasec["Cost"][0][0],
      getFlags(metasec["Flags"][0]),
    ]
    quantitiesByLabel = {}
    quantlist = contents[1]["Quantities"][0]["Quantity"]
    for quantsec in quantlist:
      label = quantsec["Label"][0][0]
      quantitiesByLabel[label] = len(quantities)
      quantities.append(parseQuantity(quantsec))
    steplist = contents[1]["Steps"][0]["Step"]
    for stepsec in steplist:
      steps.append(parseStep(stepsec, quantitiesByLabel))
    meta[2] = len(steps) - meta[1]
    spells.append(meta)

# pp.pprint(quantities)
# pp.pprint(steps)
# pp.pprint(spells)

def addrsByIndex(l):
  res = []
  k = 0
  for q in l:
    res.append(k)
    # 1 + sum of values in odd indices (count "0" as bigint)
    k += 1
    for i in range(1, len(q), 3):
      size = q[i]
      if size != 0: k += size
      else:
        # How big?
        size += 1 + byteLength(q[i + 2])
  res.append(k)
  return res

def writeQuantity(fh, q, lookup):
  for i in range(1, len(q), 3):
    size = q[i]
    n = q[i + 2]
    if q[i + 1] == 1:
      n = lookup[n]
    if size != 0: writeInt(fh, q[i + 2], 4 * size)
    else: writeBigint(fh, q[i + 2])

qaddrsByIndex = addrsByIndex(quantities)
saddrsByIndex = addrsByIndex(steps)

fh = open(destination, "wb")

writeInt(fh, len(spells), 4)
for spell in spells:
  writeString16(fh, spell[0])

writeInt(fh, len(spells), 4)
for spell in spells:
  (name, firstStep, nSteps,
    minSpeed, maxSpeed,
    accuracy, school, cost, flags) = spell
  writeInt(fh, saddrsByIndex[firstStep], 4)
  writeInt(fh, minSpeed, 2)
  writeInt(fh, maxSpeed, 2)
  writeInt(fh, accuracy, 2)
  writeInt(fh, school, 2)
  writeInt(fh, cost, 1)
  writeInt(fh, flags, 1)
  writeInt(fh, nSteps, 1)
  writeInt(fh, 0, 1)

writeInt(fh, qaddrsByIndex[-1], 4)
for quantity in quantities:
  writeQuantity(fh, quantity, qaddrsByIndex)

writeInt(fh, saddrsByIndex[-1], 4)
for step in steps:
  writeQuantity(fh, step, saddrsByIndex)

fh.close()