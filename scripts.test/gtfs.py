#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import re
import collections
import random
from sets import Set
import transitfeed
import pickle
import datetime
import csv

class Line():

	def __init__(self, id):
		self.id = id
		self.stops = []

	def add_stop(self, stop):
		if self.stops:
			if stop in self.stops:
				return

			self.stops[-1].connect(stop)
		self.stops.append(stop)
		stop.lines.add(self.id)

class Stop():

	def __init__(self, id, name = ""):
		self.id = id
		self.lines = set()
		self.connections = set()
		self.freq = 0
		self.name = name

	def connect(self, stop):
		stop.connections.add(self.id)
		self.connections.add(stop.id)

	def get_complimentary_id(self):
		if self.id.endswith("S"):
			return self.id[:-1] + "N"
		elif self.id.endswith("N"):
			return self.id[:-1] + "S"
		else:
			return None

class Network():

	def __init__(self):
		self.lines = {}
		self.stops = {}
		self.maxFreq = 0
		self.stops_by_name = {}

	def assign_freqs(self):
		H = get_harmonic(len(self.stops))
		self.maxFreq = 1/H
		stops = self.stops.values()
		random.shuffle(stops)

		for k, stop in enumerate(stops):
			stop.freq = get_zipf(H, k+1)

	def add_stop(self, stop):
		if stop.id not in self.stops:
			self.stops[stop.id] = stop
			self.stops_by_name[stop.name] = stop
		else:
			stop = self.stops[stop.id]

		return stop

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

	def __eq__(self, y):
		return self.val() == y.val()

	def next(self):
		return TDay(self.season, (self.type + 1) % 4)

def getRandomDay():
		return TDay(random.randint(0,1), random.randint(0,3))

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

def getRandomTime2():
	r = random.random()
	if r >= 0.80: # unclassified movement
		val = int(random.normalvariate(192,50))
	elif r >= 0.75: # lunch rush hour
		val = int(random.normalvariate(170,6))
	elif r >= 0.30: # return rush hour
		val = int(random.normalvariate(228,24))
	else: # morning rush hour
		val = int(random.normalvariate(96,12))

	return TTime(getRandomDay(),0,0) + val*minutes_sample

def getRandomTime():
	r = random.random()
	if r >= 0.98: # unclassified movement
		val = int(random.normalvariate(192,50))
	elif r >= 0.90: # lunch rush hour
		val = int(random.normalvariate(170,6))
	elif r >= 0.40: # return rush hour
		val = int(random.normalvariate(228,24))
	else: # morning rush hour
		val = int(random.normalvariate(96,12))

	return TTime(getRandomDay(),0,0) + val*minutes_sample


def parse_madrid(file_in, network = None):
	if not network:
		network = Network()

	with open(file_in) as file:
		for line in file:
			s = line.split()

			if len(s) > 1:
				(line_id, stop_id) = (s[0], " ".join(s[1:]))

				if line_id not in network.lines:
					network.lines[line_id] = Line(line_id)

				line = network.lines[line_id]
				stop = network.add_stop(Stop(stop_id))
				line.add_stop(stop)

def parse_gtfs(file_in, file_out, file_freqs = None, network = Network()):
	loader = transitfeed.Loader(file_in, problems = transitfeed.problems.ProblemReporter())
	sched = loader.Load()

	for trip in sched.GetTripList():

		route_id = trip.route_id + "d" + trip.direction_id
		if route_id not in network.lines:
			network.lines[route_id] = Line(route_id)

		line = network.lines[route_id]

		for stop_time in trip.GetStopTimes():
			stop = network.add_stop(Stop(stop_time.stop_id))
			line.add_stop(stop)

	if file_freqs:
		file = open(file_freqs)
		for line in file:
			try:
				data = line.split()
				network.stops[data[0]].freq = int(data[1])
				network.maxFreq = max(network.maxFreq, int(data[1]))
			except KeyError:
				pass
		file.close()

	file = open(file_out, "w")
	pickle.dump(network, file)
	file.close()

	return network

def load_gtfs(file_in):
	file = open(file_in)
	n = pickle.load(file)
	file.close()

	return n

def roulette_select(stops, maxFreq):
	if maxFreq == 0:
		return random.choice(stops)

	while True:
		stop = random.choice(stops)

		if random.random() < float(stop.freq)/maxFreq:
			return stop

def choice_next(trajectory, connections):
	next = random.choice(connections)

	while next in trajectory:
		connections.remove(next)
		next = random.choice(connections)

	return next

def get_harmonic(N, s=1):
	return sum([1/float(k**s) for k in range(1, N+1)])

def get_zipf(H, k, s=1):
	return 1/float((k**s) * H)

def load_subway(prefix, network):
	LINE_PREF = "SUB_LINE_"
	STOP_PREF = "SUB_STOP_"
	subway_to_rail = {}

	with open(prefix + "-subway.txt") as csvfile:
		csvreader = csv.DictReader(csvfile)

		for row in csvreader:
			if bool(int(row["rail"])) and network.stops_by_name.has_key(row["name"]):
				subway_to_rail[row["id"]] = network.stops_by_name[row["name"]].id
			else:
				subway_to_rail[row["id"]] = STOP_PREF + row["id"]

	with open(prefix + "-definitions.txt") as routes:
		routesReader = csv.DictReader(routes)

		for route in routesReader:
			try:
				line = network.lines[LINE_PREF + route["line"]]
			except KeyError:
				line = Line(LINE_PREF + route["line"])
				network.lines[line.id] = line

			st1 = network.add_stop(Stop(subway_to_rail[route["station1"]]))
			st2 = network.add_stop(Stop(subway_to_rail[route["station2"]]))
			st1.connect(st2)
			st1.lines.add(line.id)
			st2.lines.add(line.id)

def main(argv):
	#n_traj = 10000000
	n_traj = 100
	change_probs = [0.50, 0.90, 0.95, 0.98, 1.0]
	#change_probs = [0.80, 0.90, 0.95, 0.98, 1.0]
	changes = collections.Counter()
	lengths = collections.Counter()

	network = parse_gtfs("madrid_emt.zip", "madrid_bus.dat")
	network = parse_gtfs("madrid_bus.zip", "madrid_bus.dat", network=network)
	network = load_gtfs("madrid_bus.dat")
	#load_subway("london", network)

	#parse_gtfs("Madrid.zip", "madrid.dat")
	#network = load_gtfs("madrid.dat")

	#network = Network()
	#parse_madrid("Madrid_1.rdf", network)
	#parse_madrid("Madrid_2.rdf", network)

	#print len(network.lines.keys())
	#network.assign_freqs()
	stops = network.stops.values()
	stops_dict = {key: value+1 for value, key in enumerate(sorted(network.stops.keys()))}
	unused_stops = set(network.stops)
	i = 0

	while i < n_traj:
		origin = roulette_select(stops, network.maxFreq)
		next = network.stops[choice_next([], list(origin.connections))]
		#trajectory = [origin.id, next.id]
		trajectory = [origin.id]
		current_line = random.choice(list(origin.lines.intersection(next.lines)))
		cur_changes = 0
		current_time = getRandomTime()
		#times = [current_time, current_time + 5]
		times = [current_time]
		current_day = current_time.day
		current_time = current_time + 5
		prob_next = 0.0
		#prob_next = 0.02
		lines = [current_line]

		if current_time.day > current_day:
			continue

		try:
			while random.random() > prob_next and len(trajectory) <= 30:
				prob_next += 0.01
				connections = list(next.connections)
				next = network.stops[choice_next(trajectory, connections)]

				while current_line not in next.lines:
					if cur_changes >= 4:
							connections.remove(next.id)

					if random.random() < change_probs[cur_changes]:
						next = network.stops[choice_next(trajectory, connections)]
					else:
						current_line = random.choice(list(next.lines))
						lines.append(current_line)
						trajectory.append(next.id)
						times.append(current_time)
						cur_changes += 1
						break

				current_time = current_time + 5

				if current_time.day > current_day:
					continue
		except IndexError:
			pass
		
		if (trajectory[-1] != next.id):
			trajectory.append(next.id)
			times.append(current_time)
			lines.append(current_line)

		unused_stops.difference_update(trajectory)
		trajectory[:] = map(lambda (l,s,t): "%s:%s:%s" % (l,str(stops_dict[s]),str(t)), zip(lines, trajectory, times))
		loops = min(int(random.expovariate(0.5)) + 1, n_traj-i)

		for _ in xrange(loops):
			changes.update([cur_changes])
			lengths.update([len(trajectory)])
			sys.stdout.write(",".join(trajectory))
			sys.stdout.write("\n")
			i += 1

	if unused_stops:
		sys.stderr.write("\nWARNING: " + str(len(unused_stops)) + " Unused stops\n")

	sys.stderr.write("\nLENGTHS: " + str(lengths) + "\n")

	sys.stderr.write("\nCHANGES: " + str(changes) + "\n")

	for k,v in changes.iteritems():
		sys.stderr.write(str(k) + " " + str(float(v)/n_traj) + "\n")

if __name__ == "__main__":
	main(sys.argv)
