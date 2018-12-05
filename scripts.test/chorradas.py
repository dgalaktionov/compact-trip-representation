#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import re

def main(argv):
	total = 0
	queries = {}
	trips = {}

	with open(argv[1]) as qf:
		for line in qf.readlines()[::2]:
			q = list(filter(None, re.split(' |,|:|\n', line)))
			key = (int(q[2]),int(q[4]))

			if key not in queries:
				queries[key] = [(q[1],q[3])]
			else:
				queries[key].append((q[1],q[3]))

	for line in sys.stdin.readlines():
		nodes = list(filter(None, re.split(',|:|\n', line)))

		try:
			#total += len(queries[(int(nodes[1]),int(nodes[-2]))])
			#total += sum(1 for q in queries[(int(nodes[1]),int(nodes[-2]))] if q[0] == nodes[0])
			#total += sum(1 for q in queries[(int(nodes[1]),int(nodes[-2]))] if q[1] == nodes[-3])
			total += sum(1 for q in queries[(int(nodes[1]),int(nodes[-2]))] if q[0] == nodes[0] and q[1] == nodes[-3])
		except KeyError:
			pass

	print(total)


if __name__ == "__main__":
	main(sys.argv)
