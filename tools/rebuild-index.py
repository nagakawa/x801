# Rebuild asset indices
# Usage: python3 rebuild-index.py in the project root

import io
import json
import pathlib

index = {}

try:
  stream = open("assets/index.json")
  index = json.loads(stream.read())
except Exception:
  pass

# An index is a dictionary from filename to a structure:
# "filename" : { "timestamp": 9583, "version": 2 }, ...

for path in pathlib.Path("assets").glob("**/*"):
  if str(path) == "assets/index.json" or not path.is_file(): continue
  entry = { "timestamp": 0, "version": -1 }
  try: entry = index[str(path)]
  except: pass
  newtime = path.stat().st_mtime
  if newtime != entry["timestamp"]:
    newentry = { "timestamp": newtime, "version": entry["version"] + 1 }
    index[str(path)] = newentry

stream = open("assets/index.json", mode="w")
stream.write(json.dumps(index))
