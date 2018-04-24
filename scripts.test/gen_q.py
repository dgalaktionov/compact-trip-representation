#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import re
import collections
import random
import datetime

Q = 1000
path = "queries/madrid/"
minutes_sample = 5
hours_day = 24

class TDay():
	Low = 0
	High = 1
	Working = 0
	Friday = 1
	Saturday = 2
	Holiday = 3

	def __init__(self, Season, Type):
		self.season = Season
		self.type = Type

	def val(self):
		return self.season * 4 + self.type
		#return 0

	def __eq__(self, y):
		return self.val() == y.val()

	def next(self):
		return TDay(self.season, (self.type + 1) % 4)
		#return self

def getRandomDay():
		return TDay(random.randint(0,1), random.randint(0,3))
		#return TDay(0, 0)

class TTime():
	def __init__(self, day, hour, minute):
		self.day = day
		self.hour = hour
		self.minute = minute

	def add(self, minutes):
		m = self.minute + minutes
		h = self.hour + m/60
		d = self.day

		if (h >= hours_day):
			d = d.next()

		return TTime(d, h % hours_day, m % 60)

	def __add__(self, minutes):
		return self.add(minutes)

	def val(self):
		ms = minutes_sample
		return self.minute/ms + self.hour * (60/ms) + self.day.val() * hours_day*60/ms

	def __str__(self):
		#return self.strftime("%H:%M")
		return str(self.val())

def getRandomTime():
	return TTime(getRandomDay(), random.randint(0,hours_day-1), random.randint(0,59))



def gen_q(lines, v, fw, q=0):
	time = getRandomTime()
	fw.write("%i %s %s %s\n%i\n" % (q, v, str(time), str(time+random.randint(0,120)), 0))
	#fw.write("%i %s %s %s\n%i\n" % (q, v, str(time), str(time+random.randint(0,10)), 0))

# TODO make it fast!
def main(argv):
	freqs = dict()
	freqs_f = open(argv[1], "r")
	current_line = 1
	for line in freqs_f:
		f = line.split()
		#freqs[f[0]] = int(f[1])
		freqs[current_line] = int(f[1])
		current_line += 1
	freqs_f.close()

	text_f = open(argv[2], "r")
	lines = text_f.readlines()[1:]
	text_f.close()

	fw = None

	global Q
	N = min(len(freqs), Q)

	# starts with x
	fw = open(path + "10k-starts-with-x.txt", "w+")
	for s in random.sample(freqs, N):
		gen_q(lines, s, fw, 0)
	fw.close()

	# ends with x
	fw = open(path + "10k-ends-with-x.txt", "w+")
	for s in random.sample(freqs, N):
		gen_q(lines, s,fw, 1)
	fw.close()

	# x in the middle (bad)
	fw = open(path + "10k-x-in-the-middle.txt", "w+")
	for s in random.sample(freqs, N):
		gen_q(lines, s, fw, 2)
	fw.close()

	# starts or ends with x
	fw = open(path + "10k-starts-or-ends-with-x.txt", "w+")
	for s in random.sample(freqs, N):
		gen_q(lines, s, fw, 5)
	fw.close()

	# starts with x, ends with y (Strong)
	fw = open(path + "10k-starts-with-x-ends-with-y-strong.txt", "w+")
	for s in random.sample(lines, Q):
		nodes = filter(None, re.split(',|:|\n', s))
		gen_q(lines, "%s %s" % (nodes[0], nodes[-2]), fw, 8)
	fw.close()

	# starts with x, ends with y (Weak)
	fw = open(path + "10k-starts-with-x-ends-with-y-weak.txt", "w+")
	for s in random.sample(lines, Q):
		nodes = filter(None, re.split(',|:|\n', s))
		gen_q(lines, "%s %s" % (nodes[0], nodes[-2]), fw, 9)
	fw.close()

	#uses x
	fw = open(path + "10k-uses-x.txt", "w+")
	for s in random.sample(freqs, N):
		gen_q(lines, s, fw, 7)
	fw.close()

	#uses t
	fw = open(path + "10k-uses-t.txt", "w+")
	for s in xrange(N):
		gen_q(lines, "", fw, 10)
	fw.close()


if __name__ == "__main__":
	main(sys.argv)
