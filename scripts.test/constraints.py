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

EXP = 10
N = int(2**EXP)

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
print(14*2**(EXP//2) - 4*EXP - 13 if EXP % 2 == 0 else 20*2**(EXP//2) - 4*EXP - 13)


powahs = Counter([int(math.log2(i+1)) for i,c in enumerate(constraints) if bounds(c) < 2])
print(list(zip([v for v in sorted(powahs.values())], [4*2**(i//2)-4 if i % 2 == 0 else int(6*2**(i//2))-4 for i in range(30)])))

