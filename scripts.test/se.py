#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import re
import collections
import random
import struct

def main(argv):
	sep = 9999999

	f = open(argv[1], 'r')
	fo = open(argv[2], 'w')

	for line in f:
		for i in re.split(',|:|\n|\$', line)[::2]:
			if i:
				fo.write(struct.pack('i', int(i)))

		fo.write(struct.pack('i', sep))
		sep+=1

	f.close()
	fo.close()

if __name__ == "__main__":
	main(sys.argv)