#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import re
from collections import Counter
import csv
import json
import operator
from math import radians, cos, sin, asin, sqrt
import requests
import geojson as g
import gpxpy.gpx as gpx

MINUTE = 60
HOUR = 60*MINUTE
DAY = 24*HOUR

INTERVAL = 5*MINUTE
CYCLE = DAY*600
MAX_TIME = CYCLE/INTERVAL

def print_trip(seq):
	print ",".join(seq)

def main(argv):
	min_time = None
	c = Counter()
	transl = {}
	i = 1

	if len(argv) > 2:
		min_time = int(argv[1])

		with open(argv[2]) as freqs:
			for line in freqs:
				transl[line.split()[0]] = i
				i+=1

	else:
		sys.exit(1)

	f = sys.stdin

	for line in f:
		try:
			print_trip([str(transl[x[0]]) + ":" + str((int(x[1]) - min_time)/INTERVAL % MAX_TIME) for x in [y.split(":") for y in line.split(",")]])
			c.update([(int(line.split(",")[0].split(":")[1]) - min_time)/INTERVAL % MAX_TIME])
		except IndexError:
			pass

	sys.stderr.write("Time vals: %s\n" % str(c))


if __name__ == "__main__":
	main(sys.argv)
