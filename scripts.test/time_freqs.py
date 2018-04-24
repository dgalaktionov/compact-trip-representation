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

def main(argv):
	f = csv.DictReader(sys.stdin)
	headers = {'Content-Type': 'application/gpx+xml'}
	good = 0
	lowcon = 0
	fail = 0
	features = []
	c = Counter()

	for row in f:
		if row["MISSING_DATA"] != "False":
			fail += 1
			continue
		
		c.update([(int(row["TIMESTAMP"])/300) % 288])

	print c


if __name__ == "__main__":
	main(sys.argv)
