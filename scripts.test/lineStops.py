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
	line_indices = {}
	current_line = None
	pizdec_count = 0

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
				current_indices = [line_indices[current_line].get(stop, None) for stop in stops]
				#current_indices = list(filter(None, current_indices))

				if len(current_indices) == 0:
					if len(stops) > len(line_stops[current_line]):
						line_stops[current_line] = stops
						line_indices[current_line].clear()

						for i,stop in enumerate(stops):
							if stop not in line_indices[current_line]:
								line_indices[current_line][stop] = i
					continue

				if None in current_indices:
					print(current_indices)

				k = 0
				last_j = 0
				deltas = [0]*len(line_stops[current_line])
				for i,stop in enumerate(stops):
					j = current_indices[i]

					if j == None:
						line_indices[current_line][stop] = last_j + k
						k += 1
						deltas[last_j] = k
					else:
						deltas[last_j:j] = [k]*(j-last_j)
						last_j = j

					current_indices[i] = last_j + k

				deltas[last_j:] = [k]*(len(deltas)-last_j)

				if(current_indices[0] == current_indices[-1]):
					current_indices[0] = 0

				if deltas[-1] > 0:
					print(current_indices)
					print(deltas)

					if not all(x<=y for x,y in zip(current_indices, current_indices[1:])):
						print(line_stops[current_line])
						print(stops)
						pizdec_count += 1
				#line_indices[current_line].clear()
			else:
				line_stops[current_line] = stops
				line_indices[current_line] = {}

				for i,stop in enumerate(stops):
					if stop not in line_indices[current_line]:
						line_indices[current_line][stop] = i

	print(pizdec_count)
	sys.exit(0)
	for line,stops in line_stops.items():
		print("%s: %s" % (line, ",".join([s for s in stops])))


if __name__ == "__main__":
	main(sys.argv)
