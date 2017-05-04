import importlib
import pathlib
import maprules.rules

handlers = {}

here = pathlib.Path(__file__).with_name("rules")

for f in here.glob('*.py'):
  module = importlib.import_module("." + f.name[:-3], "maprules.rules")
  for name, func in module.__dict__.items():
    if not name.startswith("export_"): continue
    truncatedName = name[7:]
    handlers[truncatedName] = func