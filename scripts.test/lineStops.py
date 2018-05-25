#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import re
import collections

def parse_time(time_str):
	time_val = [int(t) for t in time_str.split(":")]
	return time_val[2] + time_val[1]*60 + time_val[0]*3600

def encode_time(t):
	return "%02d:%02d:%02d" % (t/3600, (t/60) % 60, t % 60)

def main(argv):
	counter = collections.Counter()
	line_stops = {}
	current_line = None

	for line in sys.stdin:
		day_match = re.match("DAY (\d+)", line)
		line_match = re.match("([^:]+):\s*$", line)
		time_match = re.match("^([0-9:]+)-", line)

		if day_match:
			pass
		elif line_match:
			current_line = line_match.group(1)
		elif time_match:
			stops = [s.split("-")[1] for s in line.strip().split(",")]

			if current_line in line_stops:
				if len(line_stops[current_line]) != len(stops):
					if len(line_stops[current_line]) < len(stops):
						line_stops[current_line] = stops
					else:
						continue
			else:
				line_stops[current_line] = stops

	for line,stops in line_stops.items():
		print("%s: %s" % (line, ",".join([s for s in stops])))


if __name__ == "__main__":
	main(sys.argv)
