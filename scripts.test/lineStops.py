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

def change_line(line_stops, line_indices, current_line, new_stops):
	line_stops[current_line] = new_stops
	line_indices[current_line] = {}

	for i,stop in enumerate(new_stops):
		if stop not in line_indices[current_line]:
			line_indices[current_line][stop] = i

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

				if all(i is None for i in current_indices):
					if len(stops) > len(line_stops[current_line]):
						change_line(line_stops, line_indices, current_line, stops)

				elif None in current_indices:
					j = 0
					k = 0
					last_j = -1
					deltas = [0]*(len(line_stops[current_line])+1)
					for i,stop in enumerate(stops):
						j = current_indices[i]

						if j is None:
							k += 1
							current_indices[i] = last_j + k
						else:
							if j < last_j and current_indices.index(j) == i:
								break

							deltas[last_j+1] = k
							last_j = j
							current_indices[i] = last_j + k

					if j is not None and j < last_j:
						pizdec_count += 1
						if len(stops) > len(line_stops[current_line]):
							change_line(line_stops, line_indices, current_line, stops)
					else:
						d = 0
						new_indices = {}

						for i,stop in enumerate(line_stops[current_line]):
							d = max(d, deltas[i])

							if stop not in new_indices:
								new_indices[stop] = line_indices[current_line][stop] + d

						for i,stop in enumerate(stops):
							if stop not in new_indices:
								new_indices[stop] = current_indices[i]

						line_indices[current_line] = new_indices
						line_stops[current_line] = [t[1] for t in sorted([tuple(reversed(x)) for x in new_indices.items()])]

			else:
				change_line(line_stops, line_indices, current_line, stops)

	#print(pizdec_count)
	for line,stops in sorted(line_stops.items()):
		print("%s: %s" % (line, ",".join(stops)))


if __name__ == "__main__":
	main(sys.argv)
