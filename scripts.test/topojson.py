#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import re
import collections
import json

def get_linestring(arc, times=None):
	linestring = {
		"type": "LineString",
		"arcs": [arc]
	}

	if times:
		linestring["properties"] = {"times": times}

	return json.dumps(linestring, separators=(',',':'))

def without_compression():
	first_line = True
	trips = 0

	sys.stdout.write("\"arcs\":[")

	for line in sys.stdin.readlines():
		if not first_line:
			sys.stdout.write(",\n")
		else:
			first_line = False

		sys.stdout.write(json.dumps(map(list, zip(*(iter(map(int, filter(None, re.split('\$|\n|,|:', line)))),) * 2)), separators=(',',':')))
		trips += 1

	sys.stdout.write("],\"objects\":{\"trips\":{\"type\":\"GeometryCollection\",\"geometries\":[")

	first_line = True
	for arc in xrange(trips):
		if not first_line:
			sys.stdout.write(",")
		else:
			first_line = False

		sys.stdout.write(get_linestring(arc))

	sys.stdout.write("]}}")

def with_compression(file_name):
	prev_line = []
	arc = -1
	first_line = True

	f = open(file_name, "r")
	sys.stdout.write("\"objects\":{\"trips\":{\"type\":\"GeometryCollection\",\"geometries\":[")

	for line in f.readlines():
		trip = map(int, filter(None, re.split('\$|\n|,|:', line)))
		if prev_line != trip[::2]:
			prev_line = trip[::2]
			arc += 1

		if not first_line:
			sys.stdout.write(",")
		else:
			first_line = False

		sys.stdout.write(get_linestring(arc, trip[1::2]))

	sys.stdout.write("]}},\"arcs\":[")

	f.seek(0)
	prev_line = []
	first_line = True
	for line in f.readlines():
		trip = map(int, filter(None, re.split('\$|\n|,|:', line)))
		if prev_line != trip[::2]:
			prev_line = trip[::2]

			if not first_line:
				sys.stdout.write(",")
			else:
				first_line = False

			sys.stdout.write(json.dumps([[stop] for stop in prev_line], separators=(',',':')))


	sys.stdout.write("]")
	f.close()

def main(argv):
	sys.stdout.write("{\"type\":\"Topology\",")

	if len(argv) > 1:
		with_compression(argv[1])
	else:
		without_compression()

	sys.stdout.write("}")

if __name__ == "__main__":
	main(sys.argv)