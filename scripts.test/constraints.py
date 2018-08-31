from time import clock
from operator import itemgetter
from collections import namedtuple
from collections import Counter
from copy import deepcopy
import re
import random
import math
import sys

k = 2
first_constraint = [[] for _ in range(k)]
constraints = [first_constraint]
split = 0

N = int(2**20)

def bounds(constraint):
	return len([1 for c in constraint if any(math.copysign(i, c[0]) != i for i in c)])

for i in range(1,N):
	split = int(math.log2(i))%k
	left = constraints[i-1][:]
	left[split] = left[split][:]
	left[split].append(-i)
	right = left[:]
	right[split] = right[split][:]
	right[split][-1] = i

	if bounds(left) < bounds(right):
		left,right = right,left
	
	#print(i*2, left, bounds(left))
	#print(i*2+1, right, bounds(right))
	constraints.append(left)
	constraints.append(right)

boundless = [i+1 for i,c in enumerate(constraints) if bounds(c) < 2]
#print(boundless)
print(len(boundless))

powahs = Counter([int(math.log2(i+1)) for i,c in enumerate(constraints) if bounds(c) == 1])
print(list(zip([v-4 for v in sorted(powahs.values())], [2**(i//2)-12 if i % 2 == 0 else int(1.5*2**(i//2))-12 for i in range(7,30)])))
