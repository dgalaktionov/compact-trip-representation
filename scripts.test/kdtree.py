from time import clock
from operator import itemgetter
from collections import namedtuple
from collections import Counter
from copy import deepcopy
import re
import random
import math
import sys
 

def part1by1(n):
        n&= 0x0000ffff
        n = (n | (n << 8)) & 0x00FF00FF
        n = (n | (n << 4)) & 0x0F0F0F0F
        n = (n | (n << 2)) & 0x33333333
        n = (n | (n << 1)) & 0x55555555
        return n


def unpart1by1(n):
        n&= 0x55555555
        n = (n ^ (n >> 1)) & 0x33333333
        n = (n ^ (n >> 2)) & 0x0f0f0f0f
        n = (n ^ (n >> 4)) & 0x00ff00ff
        n = (n ^ (n >> 8)) & 0x0000ffff
        return n


def interleave2(x, y):
        return part1by1(x) | (part1by1(y) << 1)


def deinterleave2(n):
        return unpart1by1(n), unpart1by1(n >> 1)

 
def sqd(p1, p2):
    return sum((c1 - c2) ** 2 for c1, c2 in zip(p1, p2))
 
 
class KdNode(object):
    __slots__ = ("id", "dom_elt", "split", "left", "right")
 
    def __init__(self, id, dom_elt, split, left, right):
        self.id = id
        self.dom_elt = dom_elt
        self.split = split
        self.left = left
        self.right = right
 
 
class Orthotope(object):
    __slots__ = ("min", "max")
 
    def __init__(self, mi, ma):
        #self.min = [min(mi[k],ma[k]) for k in range(len(mi))]
        #self.max = [max(mi[k],ma[k]) for k in range(len(ma))]
        self.min = list(mi)
        self.max = list(ma)

    def __contains__(self, other):
        if isinstance(other, Orthotope):
            return all([o >= s for s,o in zip(self.min, other.min)]) \
                and all ([o <= s for s,o in zip(self.max, other.max)])
        else:
            return all([mi <= o <= ma for mi,o,ma in zip(self.min, other, self.max)])
    
    def is_out(self, other):
        if isinstance(other, Orthotope):
            return any([o < s for s,o in zip(self.min, other.max)]) \
                or any([o > s for s,o in zip(self.max, other.min)])
        else:
            return any([o < mi or o > ma for mi,o,ma in zip(self.min, other, self.max)])
 
 
class KdTree(object):
    __slots__ = ("n", "bounds")
 
    def __init__(self, pts, bounds=None):
        def nk2(split, exset):
            if not exset:
                return None
            exset.sort(key=lambda x: x[1][split])
            m = len(exset) // 2
            d = exset[m]
            del exset[m]
            while m < len(exset) and exset[m][1][split] == d[1][split]:
                m += 1
 
            s2 = (split + 1) % len(d[1])  # cycle coordinates
            return KdNode(d[0], d[1], split, nk2(s2, exset[:m]),
                                    nk2(s2, exset[m:]))
        self.n = nk2(0, pts)
        self.bounds = bounds

        if self.bounds is None:
            self.bounds = Orthotope(tuple([min([p[1][k] for p in pts]) for k in range(len(self.n.dom_elt))]), 
            tuple([max([p[1][k] for p in pts]) for k in range(len(self.n.dom_elt))]))

T3 = namedtuple("T3", "nearest dist_sqd nodes_visited")


def find_nearest(k, t, p):
    def nn(kd, target, hr, max_dist_sqd):
        if kd is None:
            return T3([0.0] * k, float("inf"), 0)
 
        nodes_visited = 1
        s = kd.split
        pivot = kd.dom_elt
        left_hr = deepcopy(hr)
        right_hr = deepcopy(hr)
        left_hr.max[s] = pivot[s]
        right_hr.min[s] = pivot[s]
 
        if target[s] <= pivot[s]:
            nearer_kd, nearer_hr = kd.left, left_hr
            further_kd, further_hr = kd.right, right_hr
        else:
            nearer_kd, nearer_hr = kd.right, right_hr
            further_kd, further_hr = kd.left, left_hr
 
        n1 = nn(nearer_kd, target, nearer_hr, max_dist_sqd)
        nearest = n1.nearest
        dist_sqd = n1.dist_sqd
        nodes_visited += n1.nodes_visited
 
        if dist_sqd < max_dist_sqd:
            max_dist_sqd = dist_sqd
        d = (pivot[s] - target[s]) ** 2
        if d > max_dist_sqd:
            return T3(nearest, dist_sqd, nodes_visited)
        d = sqd(pivot, target)
        if d < dist_sqd:
            nearest = pivot
            dist_sqd = d
            max_dist_sqd = dist_sqd
 
        n2 = nn(further_kd, target, further_hr, max_dist_sqd)
        nodes_visited += n2.nodes_visited
        if n2.dist_sqd < dist_sqd:
            nearest = n2.nearest
            dist_sqd = n2.dist_sqd
 
        return T3(nearest, dist_sqd, nodes_visited)
 
    return nn(t.n, p, t.bounds, float("inf"))


def inorder(n):
    def _inorder(l,n):
        if n is not None:
            _inorder(l,n.left)
            l.append(n.id)
            _inorder(l,n.right)
        return l
    return _inorder([],n)

def range_search(t, r):
    def _range(l, n, bound):
        if n is None:
            return l
        
        if bound.is_out(r):
            return l

        if bound in r:
            l += inorder(n)
        else:
            k = n.split
            pivot = n.dom_elt
            prev_bound = bound.max[k]
            bound.max[k] = pivot[k]

            _range(l, n.left, bound)
            bound.max[k] = prev_bound

            if pivot in r:
                l.append(n.id)

            prev_bound = bound.min[k]
            bound.min[k] = min(pivot[k]+1, bound.max[k])
            _range(l, n.right, bound)
            bound.min[k] = prev_bound

        return l
    return _range([], t.n, deepcopy(t.bounds))

def show_nearest(k, heading, kd, p):
    print(heading + ":")
    print("Point:           ", p)
    n = find_nearest(k, kd, p)
    print("Nearest neighbor:", n.nearest)
    print("Distance:        ", math.sqrt(n.dist_sqd))
    print("Nodes visited:   ", n.nodes_visited, "\n")
 
 
def random_point(k):
    return tuple([random.random() for _ in range(k)])
 
 
def random_points(k, n):
    return [(i,random_point(k)) for i in range(n)]

if __name__ == "__main2__":
    random.seed(1)
    kd1 = KdTree(list(enumerate([(2, 3), (5, 4), (9, 6), (4, 7), (8, 1), (7, 2)])),
                  Orthotope((0, 0), (10, 10)))
    print(inorder(kd1.n))
    show_nearest(2, "Wikipedia example data", kd1, (9, 2))
    print(range_search(kd1, Orthotope((5, 0), (10, 10))))
 
    N = 400000
    t0 = clock()
    kd2 = KdTree(random_points(3, N), Orthotope((0, 0, 0), (1, 1, 1)))
    t1 = clock()
    text = lambda *parts: "".join(map(str, parts))
    show_nearest(0, text("k-d tree with ", N,
                         " random 3D points (generation time: ",
                         t1-t0, "s)"),
                 kd2, random_point(3))


stops = []

def generate_mean_stops(k,n):
    EXP = 3

    def _generate_from(s, split):
        coords = list(s[1])
        diff = 2**(EXP-int(math.log2(s[0])))

        return [(s[0]*2, tuple([math.copysign(abs(c)-diff,c) if i != split else c+diff for i,c in enumerate(coords)])), \
            (s[0]*2+1, tuple([math.copysign(abs(c)-diff,c) if i != split else c-diff for i,c in enumerate(coords)]))]

    i = 1
    split = 0
    stops = [(1,tuple([-2**EXP if i != split else 0 for i in range(k)]))]

    while i < n:
        split = (int(math.log2(i)))%k
        stops = stops + _generate_from(stops[i-1], split)
        i+=1
    
    #return [(i,(8-c[0],c[1]-16)) for i,c in stops]
    return stops



with open("coords.txt", "r") as f:
    for line in f:
        stops.append([int(x) for x in re.split("[:,]", line)])

#stops = [(i, (i % 100, int(i/100)*2 + i%2)) for i in range(100**2)]
#stops = generate_mean_stops(2,64)
#print(stops)

#idx = KdTree(stops[:])
idx = KdTree([(s[0],(s[1],s[2])) for s in stops])
stops_dict = dict(zip(inorder(idx.n), range(1,len(stops)+2)))
#print(inorder(idx.n))
#stops_dict = dict(zip([s[0] for s in sorted(stops, key=lambda s: interleave2(s[1],s[2]))], range(1,len(stops)+2)))
#stops_dict = dict(zip(range(len(stops)+1), range(1, len(stops)+2)))

[[min_x,min_y], [max_x,max_y]] = [idx.bounds.min, idx.bounds.max]
print((min_x,min_y,max_x,max_y))

def random_xy():
    return (random.randint(min_x,max_x), random.randint(min_y,max_y))

def print_result(res):
    print([(x.id, x.object) for x in res])

i = 0
N = 10000
queries = 0
total = 0
max_islands = 0
max_q = None

while i < N:
    ((x1,y1), (x2,y2)) = random_xy(), random_xy()
    nodes = range_search(idx, Orthotope((min(x1,x2),min(y1,y2)), (max(x1,x2),max(y1,y2))))
    #nodes = range_search(idx, Orthotope(max_q[0], max_q[1]))
    #nodes = range_search(idx, Orthotope((-2,-6), (2,6)))
    #print(sorted(nodes))
    
    if len(nodes) > 0:
        ids = sorted([stops_dict[s] for s in nodes])
        #print(ids)
        total += math.sqrt(len(ids))
        consecutive = [j for j,s in enumerate(ids[1:]) if s-ids[j] > 1]

        if len(consecutive) > 0:
            consecutive[0] += 1

        consecutive.append(len(ids))
        consecutive.insert(0,0)
        islands = [k-j for j,k in zip(consecutive,consecutive[1:])]
        #print(islands)
        queries += len(islands)
        
        if len(islands) > max_islands:
            max_q = (x1,y1, x2,y2)
            max_islands = len(islands)

        i += 1

print(max_q)
print(max_islands)
print(max_islands/math.sqrt(len(stops)))
print(math.sqrt(len(stops)))
print(queries/N)
print(total/N)
print(queries/total)