#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import re
import math
import collections

def parse_time(time_str):
	time_val = [int(t) for t in time_str.split(":")]
	return time_val[2] + time_val[1]*60 + time_val[0]*3600

def encode_time(t):
	return "%02d:%02d:%02d" % (t/3600, (t/60) % 60, t % 60)

def main(argv):
	lines = {}
	current_line = None
	day = 0

	for line in sys.stdin:
		day_match = re.match("DAY (\d+)", line)
		line_match = re.match("([^:]+):\s*$", line)
		time_match = re.match("^([0-9:]+)-", line)

		if day_match:
			day = int(day_match.group(1))
		elif line_match:
			current_line = line_match.group(1)

			if current_line not in lines:
				lines[current_line] = []

		elif time_match:
			t = day*24*60*60 + parse_time(time_match.group(1))
			lines[current_line].append(t)
			#print("%s:%d" % (current_line, t))

	td = 0
	bits = 0

	for times in lines.values():
		n = len(times)
		times = sorted(times)
		counter = collections.Counter([t-t0 for t0,t in zip(times, times[1:])])
		td += len(counter)
		#print(counter)

		for v in counter.values():
			bits += v*math.log2(n/v)

	print(td, td/len(lines))
	print(bits/8)

if __name__ == "__main__":
	main(sys.argv)
