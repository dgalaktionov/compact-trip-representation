#scp gtfs.py bichogrande:~/HDD3/spire/scripts.test/gtfs.py
#ssh bichogrande "cd HDD3/spire/scripts.test; time ./gtfs.py > texts/madrid.txt"

echo '2304 1152 1152' > texts/madrid_ex.txt
time ./gtfs.py >> texts/madrid_ex.txt
