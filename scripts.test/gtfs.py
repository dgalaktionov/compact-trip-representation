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

seconds_day = 24*60*60
days_cycle = 31

def parse_time(time_str):
	time_val = [int(t) for t in time_str.split(":")]
	return time_val[2] + time_val[1]*60 + time_val[0]*3600

def encode_time(t):
	return "%02d:%02d:%02d" % (t/3600, (t/60) % 60, t % 60)

class Stop():

	def __init__(self, id, name = "", lat = 0, lng = 0):
		self.id = id
		# self.routes = set()
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

	def __init__(self, id, route, direction, days):
		self.id = id
		self.route = route
		self.direction = direction
		self.start_time = 2*seconds_day
		self.end_time = 0
		self.days = days
		self.stops = []

	def add_stop(self, start_time, end_time, stop):
		self.start_time = min(self.start_time, end_time)
		self.end_time = max(self.end_time, start_time)
		self.stops.append((start_time,end_time,stop.id))

	def get_line(self):
		return self.route + "d" + str(self.direction)

class Network():

	def __init__(self):
		self.trips = {}
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

		for (i, (_,_,stop)) in enumerate(trip.stops[1:]):
			self.stops[stop].connect(self.stops[trip.stops[i][2]])

		return trip

	def compute_trips_by_stop(self):
		tripsByStop = [{}] * 7

		for trip in self.trips.values():
			for day in [d[0] for d in enumerate(trip.days) if d[1]]:
				for _,end_time,stop in trip.stops:
					if stop in tripsByStop[day]:
						tripsByStop[day][stop].append((end_time, trip.id))
					else:
						tripsByStop[day][stop] = [(end_time, trip.id)]

		for day in tripsByStop:
			for trips in day.values():
				trips.sort()

		return tripsByStop

	def calculate_trips_by_day(self, stop_dict, cycle):
		tripsByDay = [{}] * cycle
		tripCounter = {}

		for day in xrange(cycle):
			# print "DAY " + str(day)

			for trip in sorted([t for t in self.trips.values() if t.days[day%7]], \
				key=lambda t: (t.route, t.direction, t.start_time, t.end_time)):

				line = trip.get_line()
				c = 1

				if line in tripCounter:
					c = tripCounter[line]
					tripCounter[line] = c+1
				else:
					tripCounter[line] = 1
				
				tripsByDay[day][(line, trip.start_time)] = c

				# print trip.get_line() + ": "
				# print ",".join(["%s-%s" % (encode_time(s[0]), stop_dict[s[2]]) for s in trip.stops])

		return tripsByDay

class TDay():
	def __init__(self, day):
		self.day = day

	def val(self):
		return self.day

	def __eq__(self, y):
		return self.val() == y.val()

	def next(self):
		return TDay((self.day + 1) % days_cycle)

def getRandomDay():
	return TDay(random.randint(0,days_cycle-1))

class TTime():
	def __init__(self, day, second):
		self.day = day
		self.second = second

	def add(self, seconds):
		s = self.second + seconds
		d = self.day

		if (s >= seconds_day):
			d = d.next()

		return TTime(d, s % seconds_day)

	def __add__(self, seconds):
		return self.add(seconds)

	def val(self):
		return self.day * seconds_day + self.second

	def __str__(self):
		#return self.strftime("%H:%M")
		return str(self.val())

def parse_gtfs(file_in, file_out, file_freqs = None, network = Network()):
	loader = transitfeed.Loader(file_in, problems = transitfeed.problems.ProblemReporter())
	sched = loader.Load()

	for t in sched.GetTripList():

		days = sched.GetServicePeriod(t.service_id).day_of_week
		trip = Trip(t.trip_id, t.route_id, t.direction_id, days)

		for stop_time in t.GetStopTimes():
			s = sched.GetStop(stop_time.stop_id)
			stop = network.add_stop(Stop(s.stop_id, lat=s.stop_lat, lng=s.stop_lon))
			trip.add_stop(parse_time(stop_time.arrival_time), parse_time(stop_time.departure_time), stop)

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
	#n_traj = 100000
	n_traj = 10000000
	#change_probs = [0.50, 0.90, 0.95, 0.98, 1.0]
	change_probs = [0.98, 0.98, 0.99, 1.0]
	changes = collections.Counter()
	lengths = collections.Counter()

	#network = parse_gtfs("madrid_emt.zip", None)
	#network = parse_gtfs("madrid_bus.zip", "madrid_bus.dat", network=network)
	network = load_gtfs("madrid_bus.dat")
	tripsByStop = network.compute_trips_by_stop()
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
	tripsByDay = network.calculate_trips_by_day(stops_dict, days_cycle)
	unused_stops = set(network.stops)
	i = 0
	max_waiting_time = 30*60
	err = 0

	while i < n_traj:
		current_day = getRandomDay()
		origin = network.stops[random.choice(tripsByStop[current_day.val() % 7].keys())]
		(t, trip_id) = random.choice(tripsByStop[current_day.val() % 7][origin.id])
		current_trip = network.trips[trip_id]
		next_stops = [s for s in reversed(current_trip.stops) if s[1] > t and s[2] != origin]
		
		if len(next_stops) == 0:
			continue
		
		trajectory = [origin.id]
		current_line = current_trip.get_line()
		cur_changes = 0
		current_time = TTime(current_day, 0) + t
		times = [tripsByDay[current_time.day.val()][(current_line, current_trip.start_time)]]
		prob_next = 0.0
		lines = [current_line]
		(t,_,next) = next_stops.pop()
		complete_trajectory = [origin.id, next]

		try:
			while random.random() > prob_next:
				prob_next += 0.01
				should_change = len(next_stops) == 0 or random.random() > change_probs[cur_changes]

				if should_change:
					if cur_changes+1 == len(change_probs):
						break

					prev = next
					prev_t = t
					connections = [s for s in network.stops[next].connections if s not in complete_trajectory]
					current_time = TTime(current_day, 0) + t

					while len(connections) > 0:
						try:
							next = random.choice(connections)
							possible_trips = [(ttrip[0], network.trips[ttrip[1]]) for ttrip in \
								tripsByStop[current_time.day.val() % 7][next] \
								if t <= ttrip[0] <= t + max_waiting_time \
								and network.trips[ttrip[1]].route != current_trip.route\
								and len([s for s in network.trips[ttrip[1]].stops if s[1] > ttrip[0]]) > 1]

							if len(possible_trips) > 0:
								break
							else:
								connections.remove(next)
						except IndexError:
							connections.remove(next)

					if len(connections) == 0:
						next = prev
						t = prev_t
						prob_next = 1
					else:
						(t, current_trip) = random.choice(possible_trips)
						next_stops = [s for s in reversed(current_trip.stops) if s[1] > t]

						lines.append(current_line)
						trajectory.append(prev)
						times.append(times[-1])

						current_line = current_trip.get_line()
						current_time = TTime(current_day, 0) + t
						cur_changes += 1

						lines.append(current_line)
						trajectory.append(next)
						times.append(tripsByDay[current_time.day.val()][(current_line, current_trip.start_time)])
						complete_trajectory.append(next)
				else:
					(t,_,next) = next_stops.pop()
					complete_trajectory.append(next)
			
			if len(next_stops) > 0:
				(t,_,next) = next_stops.pop()
				complete_trajectory.append(next)

		except IndexError:
			err += 1
			continue
		
		if (trajectory[-1] != next):
			trajectory.append(next)
			times.append(tripsByDay[current_time.day.val()][(current_line, current_trip.start_time)])
			lines.append(current_line)

		if len(trajectory) % 2 == 1:
			# Just discard this piece of shit
			err += 1
			continue

		unused_stops.difference_update(trajectory)
		trajectory[:] = map(lambda (l,s,t): "%s:%s:%s" % (l,str(stops_dict[s]),str(t)), zip(lines, trajectory, times))
		# loops = min(int(random.expovariate(0.5)) + 1, n_traj-i)
		loops = 1

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
