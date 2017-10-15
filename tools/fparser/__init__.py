from collections import *
import re

class InputException(Exception):
  def __init__(self, message):
    self.message = message

def error(msg):
  raise InputException(msg)

def unescape(line):
  out = ""
  for i in range(len(line)):
    c = line[i]
    if c != '\\':
      out += c
    else:
      d = line[i + 1]
      if d == 'n': out += '\n'
      elif d == 't': out += '\t'
      elif d == '\\': out += '\\'
      else: error("Unrecognised escape sequence {} in {}".format("d", "line"))
      i += 1
  return out

def degree(header):
  if header == '@': return 1
  else: return len(header) + 1

def follow(tree, path):
  current = tree
  for name, index in path:
    current = current[name][index]
  return current

commentRE = re.compile(r"\s*#")
fieldRE = re.compile(r"\s*(\w+):(.*)")
sectionRE = re.compile(r"\s*(@|=+)(\w+)(?:\s.*)?")
valueID = re.compile(r'-?\d+(?:\.\d+)?|"((?:[^\\"]|\\[\\nt])*)"|to\s+(\w+)')

def parse(source):
  header = defaultdict(list)
  # e. g. {"TILE": [{"Layers": [[1]], "Layer": [{"Width": [[80]]}]}]}
  sections = defaultdict(list)
  currentSection = header
  sectionPath = []
  # tuples of (name, list to insert into, index of arg)
  mlStringNames = []
  mlCurrentString = ""
  dsCount = 0
  for line in source:
    line = line.rstrip('\n')
    if mlStringNames: # we are in the middle of a multi-line string
      if line == mlStringNames[0][0]: # end of string
        li = mlStringNames[0][1]
        i = mlStringNames[0][2]
        li[i] = mlCurrentString
        mlCurrentString = ""
        mlStringNames = mlStringNames[1:]
      else:
        mlCurrentString += line + '\n'
      continue
    line = line.strip()
    if re.match(commentRE, line): continue
    matchField = re.fullmatch(fieldRE, line)
    matchSection = re.fullmatch(sectionRE, line)
    if matchField:
      name = matchField.group(1)
      body = matchField.group(2)
      currentSection[name].append([])
      matches = re.finditer(valueID, body)
      i = 0
      arglist = currentSection[name][-1]
      for match in matches:
        mstr = match.group(0)
        if mstr.startswith("to"):
          mlStringNames.append((match.group(2), currentSection[name][-1], i))
          arglist.append("")
        elif mstr.startswith('"'):
          arglist.append(unescape(match.group(1)))
        else:
          try:
            arglist.append(int(mstr))
          except ValueError:
            arglist.append(float(mstr))
        i += 1
    elif matchSection:
      deg = degree(matchSection.group(1))
      name = matchSection.group(2)
      if deg > len(sectionPath) + 1:
        error("Cannot jump from degree {} to {}".format(len(sectionPath), deg))
      sectionPath = sectionPath[0 : deg - 1]
      section = follow(sections, sectionPath)
      currentSubsections = section[name]
      sectionPath.append((name, len(currentSubsections)))
      currentSubsections.append(defaultdict(list))
      currentSection = currentSubsections[-1]
      if deg == 1: dsCount += 1
    else: error(line + " is ill-formed")
  return (header, sections, dsCount)