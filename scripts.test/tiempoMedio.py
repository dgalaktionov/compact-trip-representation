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
	if len(argv) < 2:
		print("Usage: tiempoMedio.py <lineStops.txt>")
		sys.exit(1)

	line_stops = {}
	line_times = {}
	line_freqs = {}
	line_vars = {}
	current_line = None
	for_later = []

	with open(argv[1]) as lineStops:
		for line in lineStops:
			line_match = re.match("([^:]+):\s*([^\s]+)$", line)

			if line_match:
				current_line = line_match.group(1)
				stops = line_match.group(2).split(",")
				line_stops[current_line] = stops
				line_times[current_line] = [0] * len(stops)
				line_freqs[current_line] = [0] * len(stops)
				line_vars[current_line] = [0] * len(stops)

	current_line = None

	for line in sys.stdin:
		line = line.strip()
		day_match = re.match("DAY (\d+)", line)
		line_match = re.match("([^:]+):\s*$", line)
		time_match = re.match("^([0-9:]+)-", line)

		if day_match:
			pass
		elif line_match:
			current_line = line_match.group(1)
		elif time_match:
			t0 = parse_time(time_match.group(0)[:-1])
			stops = [(parse_time(t[0])-t0, t[1]) for t in [s.split("-") for s in line.split(",")]]
			last_i = 0

			for s in stops:
				try:
					i = line_stops[current_line].index(s[1], last_i)

					line_times[current_line][i] += s[0]
					line_vars[current_line][i] += s[0]*s[0]
					line_freqs[current_line][i] += 1
					last_i = i+1
				except ValueError:
					pass

	for line,times in sorted(line_times.items()):
		line_freqs[line] = [max(f,1) for f in line_freqs[line]]
		stops = sorted(list([(t/f,s) for t,f,s in zip(times, line_freqs[line], enumerate(line_stops[line]))]))
		t0 = stops[0][0]
		stops = [(t-t0,s) for t,(_,s) in stops]

		print("%s: %s" % (line, ",".join([str(int(t)) for t,_ in stops])))

		last_t = 0

		for s in stops[1:]:
			if s[0] <= last_t:
				#print("wtf " + str(s) + " > " + str(last_t))
				pass

			last_t = s[0]


if __name__ == "__main__":
	main(sys.argv)
