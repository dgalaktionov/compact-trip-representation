#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import re
import collections
import random
from sets import Set
import transitfeed
import pickle
import datetime
import mmap

mm = None

def traj_key(i):
	global mm

	mm.seek(i)
	line = mm.readline()

	if line == "":
		return [999999]

	return map(int, re.split(",|:", line.strip())[::2])

def main(argv):
	global mm

	lines = [0]
	f_in = open(argv[1], "r")
	mm = mmap.mmap(f_in.fileno(), 0, prot=mmap.PROT_READ)

	for line in iter(mm.readline, ""):
		lines.append(mm.tell())

	lines.sort(key=traj_key)
	#f_out = open(argv[2], "w")
	f_out = sys.stdout

	for i in lines:
		mm.seek(i)
		f_out.write(mm.readline())
		#f_out.write(",".join(re.split(",|:", mm.readline().strip())))
		#f_out.write("\n")

	mm.close()
	f_in.close()
	f_out.close()

if __name__ == "__main__":
	main(sys.argv)