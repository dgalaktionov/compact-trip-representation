#!/usr/bin/python3
# -*- coding: utf-8 -*-

from rtree import index
import re
import random
import math

stops = []

with open("coords.txt", "r") as f:
	for line in f:
		stops.append([int(x) for x in re.split("[:,]", line)])

p = index.Property()
p.leaf_capacity = 4
p.index_capacity = 4
p.near_minimum_overlap_factor = 4
p.fill_factor = 0.99
p.tight_mbr = True
p.variant = index.RT_Star

idx = index.Index([(s[0], (s[1], s[2], s[1], s[2]), None) for s in stops], properties=p)
[min_x,min_y,max_x,max_y] = idx.get_bounds()
stops_dict = dict(zip(idx.intersection((min_x,min_y,max_x,max_y)), range(1,len(stops)+1)))
print((min_x,min_y,max_x,max_y))

def random_xy():
	return (random.randint(min_x,max_x), random.randint(min_y,max_y))

def print_result(res):
	print([(x.id, x.object) for x in res])

i = 0
N = 1000
queries = 0

while i < N:
	((x1,y1), (x2,y2)) = random_xy(), random_xy()
	nodes = list(idx.intersection((min(x1,x2),min(y1,y2),max(x1,x2),max(y1,y2)), objects="internal"))
	
	if len(nodes) > 0:
		ids = sorted([stops_dict[s] for x in nodes for s in x.object])
		consecutive = [j for j,s in enumerate(ids[1:]) if s-ids[j] > 1]

		if len(consecutive) > 0:
			consecutive[0] += 1

		consecutive.append(len(ids))
		consecutive.insert(0,0)
		islands = [k-j for j,k in zip(consecutive,consecutive[1:])]
		#print(islands)
		queries += len(islands)
		i += 1

print(queries/N)
print(math.log2(len(stops)))