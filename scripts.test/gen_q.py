#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import re
import collections
import random
import datetime

Q = 10000
path = "queries/madrid/"
seconds_day = 24*60*60
days_cycle = 31

def getRandomTime():
	return random.randint(0,seconds_day*(days_cycle-1))



def gen_q(v, fw, q=0):
	time = getRandomTime()
	fw.write("%i %s %s %s\n%i\n" % (q, v, str(time), str(time+seconds_day), 0))
	#fw.write("%i %s %s %s\n%i\n" % (q, v, str(time), str(time+random.randint(0,10)), 0))

def main(argv):
	# starts with x, ends with y
	fw = open(path + "10k-starts-with-x.txt", "w+")
	for s in random.sample(sys.stdin.readlines(), Q):
		nodes = filter(None, re.split(',|:|\n', s))
		gen_q("%s" % nodes[1], fw, 0)
	fw.close()

	sys.exit(0)

	# starts with x, ends with y
	fw = open(path + "10k-starts-with-x-ends-with-y.txt", "w+")
	for s in random.sample(sys.stdin.readlines(), Q):
		nodes = filter(None, re.split(',|:|\n', s))
		gen_q("%s:%s %s:%s" % (nodes[0], nodes[1], nodes[-3], nodes[-2]), fw, 15)
	fw.close()

def main2(argv):
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
