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
	line_times = {}
	current_line = None

	for line in sys.stdin:
		day_match = re.match("DAY (\d+)", line)
		line_match = re.match("([^:]+):\s*$", line)
		time_match = re.match("^([0-9:]+)-", line)

		if day_match:
			pass
		elif line_match:
			current_line = line_match.group(1)
			counter.update([current_line])
		elif time_match:
			times = [parse_time(s.split("-")[0]) for s in line.split(",")]
			times = [t-times[0] for t in times[1:]]

			if current_line in line_times:
				if len(line_times[current_line]) != len(times):
					if len(line_times[current_line]) < len(times):
						line_times[current_line] = times
						counter[current_line] = 1
					else:
						continue

				line_times[current_line] = [a+b for a,b in zip(line_times[current_line], times)]
				counter.update([current_line])
			else:
				line_times[current_line] = times
				counter[current_line] = 1

	for line,times in line_times.items():
		print("%s: %s" % (line, ",".join([str(int(t/counter[line])) for t in times])))


if __name__ == "__main__":
	main(sys.argv)
