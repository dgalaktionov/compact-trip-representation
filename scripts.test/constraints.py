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

N = 32

for i in range(1,N-1):
	split = int(math.log2(i))%k
	left = constraints[i-1][:]
	left[split] = left[split][:]
	left[split].append(-i)
	right = left[:]
	right[split] = right[split][:]
	right[split][-1] = i
	constraints.append(left)
	constraints.append(right)
	
	print(i*2, left)
	print(i*2+1, right)