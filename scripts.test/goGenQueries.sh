#scp gen_q.py count.py bichogrande:~/HDD3/spire/scripts.test/
#ssh bichogrande "cd HDD3/spire/scripts.test; cat texts/madrid.txt | ./count.py > freqs.txt; ./gen_q.py freqs.txt texts/madrid.txt"
#scp -r bichogrande:~/HDD3/spire/scripts.test/queries/madrid queries

#cat texts/madrid.txt | ./count.py > freqs_madrid.txt; ./gen_q.py freqs_madrid.txt texts/madrid.txt
cat texts/madrid_ex.txt | ./count.py > freqs_madrid.txt; ./gen_q.py freqs_madrid.txt texts/madrid_ex.txt
