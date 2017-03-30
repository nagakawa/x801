import re

# reads a tti

class InputException(Exception):
  def __init__(self, message):
    self.message = message

def error(msg):
  raise InputException(msg)

tableRE = re.compile(r"(\w+) (\d+)")

def readfh(fh):
  table = {}
  for line in fh:
    line = line.strip()
    if not line: continue
    match = re.fullmatch(tableRE, line)
    if not match:
      error("malformatted line: " + line)
    name = match.group(1)
    index = int(match.group(2))
    table[name] = index
  return table

def read(fname):
  fh = open(fname)
  thing = readfh(fh)
  fh.close()
  return thing