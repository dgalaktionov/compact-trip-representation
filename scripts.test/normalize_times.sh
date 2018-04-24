#!/bin/sh

cat $1 | ./normalize_times.py $(cat $1 | ./find_min_time.py) $2
#cat $1 | ./normalize_times.py 0 $2
