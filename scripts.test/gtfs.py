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
from pyqtree import Index
import utm

class Stop():

	def __init__(self, id, name = "", lat = 0, lng = 0):
		self.id = id
		self.routes = set()
		self.connections = set()
		# self.freq = 0
		# self.name = name
		(self.x, self.y) = (lat, lng)

	def connect(self, stop):
		stop.connections.add(self.id)
		self.connections.add(stop.id)
	
	def __str__(self):
		return str(self.id)

class Trip():

	def __init__(self, id, route, days):
		self.id = id
		self.route = route
		self.start_time = 0
		self.end_time = 0
		self.days = days
		self.stops = []

	def add_stop(self, start_time, end_time, stop):
		self.start_time = min(self.start_time, end_time)
		self.end_time = max(self.end_time, start_time)

		if self.stops:
			self.stops[-1][2].connect(stop)
		self.stops.append((start_time,end_time,stop))
		stop.routes.add(self.route)

class Network():

	def __init__(self):
		self.trips = {}
		self.tripsByStop = [{}] * 7
		self.stops = {}
		self.maxFreq = 0
		self.stops_by_name = {}
		self.spindex = Index(bbox=(331792, 4317252, 586592, 4540683))

	def assign_freqs(self):
		H = get_harmonic(len(self.stops))
		self.maxFreq = 1/H
		stops = self.stops.values()
		random.shuffle(stops)

		for k, stop in enumerate(stops):
			stop.freq = get_zipf(H, k+1)
	
	def find_near_stops(self, stop, d):
		return [s for s in \
			self.spindex.intersect((stop.x-d, stop.y-d, stop.x+d, stop.y+d)) \
			if (s.x-stop.x)**2 + (s.y-stop.y)**2 < d*d]

	def add_stop(self, stop):
		if stop.id not in self.stops:
			self.stops[stop.id] = stop
			# self.stops_by_name[stop.name] = stop

			(stop.x, stop.y, _, _) = utm.from_latlon(stop.x, stop.y)
			for s in self.find_near_stops(stop, 100):
				s.connect(stop)
			
			self.spindex.insert(stop, (stop.x, stop.y, stop.x, stop.y))
		else:
			stop = self.stops[stop.id]

		return stop
	
	def add_trip(self, trip):
		self.trips[trip.id] = trip
		return trip

	def compute_trips_by_stop(self):
		self.tripsByStop = [{}] * 7

		for trip in self.trips.values():
			for day in [d[0] for d in enumerate(trip.days) if d[1]]:
				for _,end_time,stop in trip.stops:
					if stop.id in self.tripsByStop[day]:
						self.tripsByStop[day][stop.id].append((end_time, trip.id))
					else:
						self.tripsByStop[day][stop.id] = [(end_time, trip.id)]

		for day in self.tripsByStop:
			for trips in day.values():
				trips.sort()


minutes_sample = 5
hours_day = 24
days_cycle = 31

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
		# return self.season * days_cycle + self.type
		return self.type

	def __eq__(self, y):
		return self.val() == y.val()

	def next(self):
		return TDay(self.season, (self.type + 1) % days_cycle)

def getRandomDay():
		#return TDay(random.randint(0,1), random.randint(0,3))
		return TDay(0, random.randint(0,days_cycle-1))

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

def parse_time(time_str):
	time_val = [int(t) for t in time_str.split(":")]
	return TTime(TDay(0,time_val[0]/24), time_val[0] % 24, time_val[1])

def parse_gtfs(file_in, file_out, file_freqs = None, network = Network()):
	loader = transitfeed.Loader(file_in, problems = transitfeed.problems.ProblemReporter())
	sched = loader.Load()

	for t in sched.GetTripList():

		route_id = t.route_id + "d" + t.direction_id
		days = sched.GetServicePeriod(t.service_id).day_of_week
		trip = Trip(t.trip_id, route_id, days)

		for stop_time in t.GetStopTimes():
			s = sched.GetStop(stop_time.stop_id)
			stop = network.add_stop(Stop(s.stop_id, lat=s.stop_lat, lng=s.stop_lon))
			trip.add_stop(parse_time(stop_time.arrival_time).val(), parse_time(stop_time.departure_time).val(), stop)

		network.add_trip(trip)

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

	if file_out:
		network.tripsByStop = None
		network.spindex = None
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
	#n_traj = 10000
	n_traj = 10000000
	#change_probs = [0.50, 0.90, 0.95, 0.98, 1.0]
	change_probs = [0.98, 0.98, 0.99, 1.0]
	changes = collections.Counter()
	lengths = collections.Counter()

	# network = parse_gtfs("madrid_emt.zip", None)
	# network = parse_gtfs("madrid_bus.zip", "madrid_bus.dat", network=network)
	network = load_gtfs("madrid_bus.dat")
	network.compute_trips_by_stop()
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
	max_waiting_time = 30
	err = 0

	while i < n_traj:
		current_day = getRandomDay()
		origin = network.stops[random.choice(network.tripsByStop[current_day.val() % 7].keys())]
		(t, trip_id) = random.choice(network.tripsByStop[current_day.val() % 7][origin.id])
		current_trip = network.trips[trip_id]
		next_stops = [s for s in reversed(current_trip.stops) if s[1] > t]
		
		if len(next_stops) == 0:
			continue
		
		trajectory = [origin.id]
		current_line = current_trip.route
		cur_changes = 0
		current_time = TTime(current_day, 0, 0) + t * minutes_sample
		times = [current_time]
		prob_next = 0.0
		lines = [current_line]
		(t,_,next) = next_stops.pop()

		try:
			while random.random() > prob_next:
				prob_next += 0.01
				should_change = len(next_stops) == 0 or random.random() > change_probs[cur_changes]

				if should_change:
					if cur_changes+1 == len(change_probs):
						break

					prev = next
					prev_t = t
					connections = list(next.connections)
					current_time = TTime(current_day, 0, 0) + t * minutes_sample

					while len(connections) > 0:
						next = network.stops[choice_next(trajectory, connections)]
						possible_trips = [(ttrip[0], network.trips[ttrip[1]]) for ttrip in \
							network.tripsByStop[current_time.day.val() % 7][next.id] \
							if t <= ttrip[0] <= t + max_waiting_time/minutes_sample \
							and network.trips[ttrip[1]].route != current_line\
							and len([s for s in network.trips[ttrip[1]].stops if s[1] > ttrip[0]]) > 1]

						if len(possible_trips) > 0:
							break
						else:
							connections.remove(next.id)

					if len(connections) == 0:
						next = prev
						t = prev_t
						prob_next = 1
					else:
						(t, current_trip) = random.choice(possible_trips)
						next_stops = [s for s in reversed(current_trip.stops) if s[1] > t]
						current_line = current_trip.route
						current_time = TTime(current_day, 0, 0) + t * minutes_sample
						cur_changes += 1
						lines.append(current_line)
						trajectory.append(next.id)
						times.append(current_time)
				else:
					(t,_,next) = next_stops.pop()
			
			if len(next_stops) > 0:
				(t,_,next) = next_stops.pop()

		except IndexError:
			err += 1
		
		if (trajectory[-1] != next.id):
			current_time = TTime(current_day, 0, 0) + t * minutes_sample
			trajectory.append(next.id)
			times.append(current_time)
			lines.append(current_line)

		unused_stops.difference_update(trajectory)
		trajectory[:] = map(lambda (l,s,t): "%s:%s:%s" % (l,str(stops_dict[s]),str(t)), zip(lines, trajectory, times))
		loops = min(int(random.expovariate(0.5)) + 1, n_traj-i)
		# loops = 1

		for _ in xrange(loops):
			changes.update([cur_changes])
			lengths.update([len(trajectory)])
			sys.stdout.write(",".join(trajectory))
			sys.stdout.write("\n")
			i += 1

	if unused_stops:
		sys.stderr.write("\nWARNING: " + str(len(unused_stops)) + " Unused stops\n")

	sys.stderr.write("\nERRORS: " + str(err) + "\n")

	sys.stderr.write("\nLENGTHS: " + str(lengths) + "\n")

	sys.stderr.write("\nCHANGES: " + str(changes) + "\n")

	for k,v in changes.iteritems():
		sys.stderr.write(str(k) + " " + str(float(v)/n_traj) + "\n")

if __name__ == "__main__":
	main(sys.argv)
