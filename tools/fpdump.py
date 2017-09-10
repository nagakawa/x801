import argparse
import fparser
import pprint

parser = argparse.ArgumentParser(description='Dump x801 fparser files.')
parser.add_argument('source', metavar='s', type=str,
    help='the input file')
args = parser.parse_args()
doc = fparser.parse(open(args.source))
pp = pprint.PrettyPrinter(indent=1, compact=True)

pp.pprint(doc)