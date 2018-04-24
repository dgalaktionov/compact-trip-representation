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


def print_trip(seq):
	print ",".join(seq)

def main(argv):
	min_time = 99999999999
	f = sys.stdin

	for line in f:
		try:
			#/print "x", line.split(",")[0]
			time = int(line.split(",")[0].split(":")[1])
			min_time = min(time, min_time)
		except IndexError:
			pass

	print min_time


if __name__ == "__main__":
	main(sys.argv)
