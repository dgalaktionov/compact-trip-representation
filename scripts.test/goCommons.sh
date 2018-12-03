#!/bin/bash

cat madrid_trips.zst | zstd -d | python3 lineStops.py > texts/lineStops.txt 
cat madrid_trips.zst | zstd -d | python3 tiempoMedio.py texts/lineStops.txt > texts/avgTimes.txt 
cat madrid_trips.zst | zstd -d | python3 tiempoInicial.py > texts/initialTimes.txt 
cat texts/lineStops.txt | python3 stopLines.py > texts/stopLines.txt 

