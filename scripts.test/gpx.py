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

def haversine(lon1, lat1, lon2, lat2):
    """
    Calculate the great circle distance between two points 
    on the earth (specified in decimal degrees)
    """
    # convert decimal degrees to radians 
    lon1, lat1, lon2, lat2 = map(radians, [lon1, lat1, lon2, lat2])
    # haversine formula 
    dlon = lon2 - lon1 
    dlat = lat2 - lat1 
    a = sin(dlat/2)**2 + cos(lat1) * cos(lat2) * sin(dlon/2)**2
    c = 2 * asin(sqrt(a)) 
    m = 6367000 * c
    return m

def print_trip(seq, times):
	seq = [str(edge/2) for edge in seq]

	if times:
		seq = zip(seq,times)
		seq = [x[0] + ":" + str(x[1]) for x in seq]

	print ",".join(seq)

def main(argv):
	f = csv.DictReader(sys.stdin)
	headers = {'Content-Type': 'application/gpx+xml'}
	good = 0
	lowcon = 0
	fail = 0
	features = []

	for row in f:
		if row["MISSING_DATA"] != "False":
			fail += 1
			continue

		line = json.loads(row["POLYLINE"])

		hitler = gpx.GPX()
		gpx_track = gpx.GPXTrack()
		hitler.tracks.append(gpx_track)
		gpx_segment = gpx.GPXTrackSegment()
		gpx_track.segments.append(gpx_segment)

		for point in line:
			gpx_segment.points.append(gpx.GPXTrackPoint(point[1],point[0]))

		res = json.loads(requests.post("http://localhost:8989/match", data=hitler.to_xml(), \
			params={"vehicle":"car","type":"json","points_encoded":False,"instructions":False, \
			"traversal_keys":True,"locale":"pt","gps_accuracy":25}, headers=headers).text)

		#print res

		if "paths" not in res:
			fail += 1
			continue

		good += 1
		#print len(res["paths"][0]["points"]["coordinates"])
		#print len(line)
		#print len(res["paths"][0]["instructions"])
		#print len(res["traversal_keys"])

		if good % 10000 == 0:
			sys.stderr.write(str(good) + " trips read!\n")

		#break
		print_trip(res["traversal_keys"], [t*15 + int(row["TIMESTAMP"]) for t in res["times"]])

		'''
		for path in res["paths"]:
			good += 1
			features.append(g.Feature(geometry=path["points"], \
				properties={"name": good+fail}))
		'''

	sys.stderr.write(json.dumps({"good": good, "lowcon": lowcon, "fail": fail}))
	print ""
	#print g.FeatureCollection(features, properties={"good": good, "lowcon": lowcon, "fail": fail})


if __name__ == "__main__":
	main(sys.argv)
