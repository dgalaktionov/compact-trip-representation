#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import re
import collections

def main(argv):
	counter = collections.Counter()
	#sys.stdin.readline()
	data = sys.stdin.readlines()

	#print sum([len(filter(None, re.split(',|\$|\n', line))) for line in data])/float(len(data))

	for line in data[1:]:
		counter.update(map(int, [x[2] for x in [y.split(":") for y in filter(None, re.split('\$|\n|,', line))]]))

	for k,v in iter(sorted(counter.iteritems())):
		print "%s %s" % (k,v)

if __name__ == "__main__":
	main(sys.argv)
