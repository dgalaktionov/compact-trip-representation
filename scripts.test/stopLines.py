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
	stop_lines = {}
	current_line = None

	for line in sys.stdin:
		line_match = re.match("([^:]+):\s*([^\s]+)$", line)

		if line_match:
			current_line = line_match.group(1)
			stops = line_match.group(2)
			
			for s in map(int, stops.split(",")):
				if s in stop_lines:
					stop_lines[s].append(current_line)
				else:
					stop_lines[s] = [current_line]

	for stop,lines in sorted(stop_lines.items()):
		print("%s: %s" % (stop, ",".join(sorted(set(lines)))))


if __name__ == "__main__":
	main(sys.argv)
