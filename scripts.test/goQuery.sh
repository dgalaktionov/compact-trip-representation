#../benchmark indexes/trayectorias ./queries/trayectorias/starts-with-x.txt 3 280 400
#../benchmark indexes/trayectorias ./queries/trayectorias/100-starts-with-x-ends-with-y.txt 3 280 400
#../benchmark indexes/trayectorias ./queries/trayectorias/100-starts-with-x.txt 3 280 400
#../benchmark indexes/trayectorias ./queries/trayectorias/100-ends-with-x.txt 3 280 400
#../benchmark indexes/trayectorias ./queries/trayectorias/100-x-in-the-middle.txt 3 280 400
#../benchmark indexes/trayectorias ./queries/trayectorias/100-starts-or-ends-with-x.txt 3 280 400
#../benchmark indexes/trayectorias ./queries/trayectorias/top-k.txt 3 280 400

#../benchmark indexes/generadas ./queries/trayectorias/top-k.txt 3 280 400
#../benchmark indexes/generadas ./queries/trayectorias/starts-with-x.txt 3 280 400
#../benchmark indexes/generadas ./queries/trayectorias/100-starts-with-x-ends-with-y.txt 3 280 400

#../benchmark indexes/nyc ./queries/trayectorias/top-k.txt 3 280 400

#../benchmark indexes/trayectorias_mini ./queries/trayectorias/starts-with-x-ends-with-y.txt 3 280 400

#../benchmark indexes/madrid ./queries/trayectorias/top-k.txt 3 280 400
#../benchmark indexes/madrid ./queries/trayectorias/top-k-starts.txt 3 280 400

#../benchmark indexes/madrid ./queries/madrid/10k-starts-with-x.txt 3 280 400
#../benchmark indexes/madrid ./queries/madrid/10k-ends-with-x.txt 3 280 400
#../benchmark indexes/madrid ./queries/madrid/10k-x-in-the-middle.txt 3 280 400
#../benchmark indexes/madrid ./queries/madrid/10k-starts-or-ends-with-x.txt 3 280 400
#../benchmark indexes/madrid ./queries/madrid/10k-starts-with-x-ends-with-y.txt 3 280 400
#../benchmark indexes/madrid ./queries/madrid/10k-starts-with-x-ends-with-y-strong.txt 3 280 400
#../benchmark indexes/madrid ./queries/madrid/10k-starts-with-x-ends-with-y-weak.txt 3 280 400
#../benchmark indexes/madrid ./queries/madrid/10k-uses-x.txt 3 280 400
#../benchmark indexes/madrid ./queries/madrid/10k-uses-t.txt 3 280 400
#../benchmark indexes/madrid ./queries/madrid/top-k.txt 3 280 400
#../benchmark indexes/madrid ./queries/madrid/fail.txt 3 280 400

# echo 'SPATIAL '$1' '$3''
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-starts-with-x.txt 1
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-ends-with-x.txt 1
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-x-in-the-middle.txt 1
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-starts-or-ends-with-x.txt 1
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-starts-with-x-ends-with-y-strong.txt 1
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-starts-with-x-ends-with-y-weak.txt 1
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-uses-x.txt 1
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-10.txt 1 1000
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-10-seq.txt 1 1000
# ../benchmark indexes/$1 indexes/$3 ./queries/madrid/top-10-starts.txt 1 100
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-100.txt 1 1000
# ../benchmark indexes/$1 indexes/$3 ./queries/madrid/top-100-seq.txt 1 1
# ../benchmark indexes/$1 indexes/$3 ./queries/madrid/top-100-starts.txt 1 100
#echo ''

# echo 'SPATIOTEMPORAL (psi='$2' times='$3')'
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-starts-with-x.txt
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-ends-with-x.txt
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-x-in-the-middle.txt
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-starts-or-ends-with-x.txt
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-starts-with-x-ends-with-y-strong.txt
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-starts-with-x-ends-with-y-weak.txt
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-uses-x.txt
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-uses-t.txt
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-10-times.txt 0 10000
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-100-times.txt 0 1000
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-10.txt 0 10000
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-10-seq.txt 0 1000
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-10-starts.txt 0 1000
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-10-starts-seq.txt 0 100
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-100.txt 0 10000
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-100-seq.txt 0 1000
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-100-starts.txt 0 100
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/top-100-starts-seq.txt 0 100
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/$1/10k-starts-t.txt

#../benchmark indexes/$1$2 ./queries/$1/fail.txt
#../benchmark indexes/$1$2 ./queries/$1/10k-starts-with-x.txt 0 10000
#../benchmark indexes/$1$2 ./queries/$1/10k-starts-with-x-ends-with-y-strong.txt 0 10000
#../benchmark indexes/$1$2 ./queries/$1/10k-uses-x.txt

# ../benchmark indexes/madrid_flat indexes/madrid_flat5_HTRG32 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark indexes/madrid_flat indexes/madrid_flat5_HTRRR32 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark indexes/madrid_flat indexes/madrid_flat5_HTRRR64 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark indexes/madrid_flat indexes/madrid_flat5_HTRRR128 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark6 indexes/madrid_flat indexes/madrid_flat30_HTRG32 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark6 indexes/madrid_flat indexes/madrid_flat30_HTRRR32 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark6 indexes/madrid_flat indexes/madrid_flat30_HTRRR64 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark6 indexes/madrid_flat indexes/madrid_flat30_HTRRR128 ./queries/madrid_t/10k-uses-t5.txt

# ../benchmark indexes/madrid512 indexes/madrid8_5_HTRG32 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark indexes/madrid512 indexes/madrid8_5_HTRRR32 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark indexes/madrid512 indexes/madrid8_5_HTRRR64 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark indexes/madrid512 indexes/madrid8_5_HTRRR128 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark6 indexes/madrid512 indexes/madrid8_30_HTRG32 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark6 indexes/madrid512 indexes/madrid8_30_HTRRR32 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark6 indexes/madrid512 indexes/madrid8_30_HTRRR64 ./queries/madrid_t/10k-uses-t5.txt
# ../benchmark6 indexes/madrid512 indexes/madrid8_30_HTRRR128 ./queries/madrid_t/10k-uses-t5.txt

# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/madrid_t/10k-uses-t60.txt
# ../benchmark indexes/$1$2 indexes/$1$3 ./queries/madrid_t/10k-uses-t600.txt

#../benchmark indexes/madrid_bus ./queries/madrid_bus/10k-starts-with-x.txt 3 280 400
#../benchmark indexes/madrid_bus ./queries/madrid_bus/10k-ends-with-x.txt 3 280 400
#../benchmark indexes/madrid_bus ./queries/madrid_bus/10k-x-in-the-middle.txt 3 280 400
#../benchmark indexes/madrid_bus ./queries/madrid_bus/10k-starts-or-ends-with-x.txt 3 280 400
#../benchmark indexes/madrid_bus ./queries/madrid_bus/10k-starts-with-x-ends-with-y.txt 3 280 400
#../benchmark indexes/madrid_bus ./queries/madrid_bus/10k-starts-with-x-ends-with-y-strong.txt 3 280 400
#../benchmark indexes/madrid_bus ./queries/madrid_bus/10k-starts-with-x-ends-with-y-weak.txt 3 280 400
#../benchmark indexes/madrid_bus ./queries/madrid_bus/10k-uses-x.txt 3 280 400
#../benchmark indexes/madrid_bus ./queries/madrid_bus/10k-uses-t.txt 3 280 400
#../benchmark indexes/madrid_bus ./queries/madrid_bus/top-k.txt 3 280 400

#../benchmark indexes/madrid_no ./queries/madrid/1k-starts-with-x.txt 3 280 400
#../benchmark indexes/madrid_no ./queries/madrid/1k-ends-with-x.txt 3 280 400
#../benchmark indexes/madrid_no ./queries/madrid/1k-x-in-the-middle.txt 3 280 400
#../benchmark indexes/madrid_no ./queries/madrid/1k-starts-or-ends-with-x.txt 3 280 400
#../benchmark indexes/madrid_no ./queries/madrid/1k-starts-with-x-ends-with-y.txt 3 280 400
#../benchmark indexes/madrid_no ./queries/madrid/1k-uses-x.txt 3 280 400

#../benchmark indexes/madrid_no ./queries/trayectorias/top-k.txt 3 280 400

#../benchmark indexes/london ./queries/london/mixed.txt 3 280 400
#../benchmark indexes/london ./queries/london/1k-uses-x.txt 3 280 400
#../benchmark indexes/london ./queries/london/1k-starts-with-x.txt 3 280 400
#../benchmark indexes/london ./queries/london/1k-ends-with-x.txt 3 280 400
#../benchmark indexes/london ./queries/london/1k-x-in-the-middle.txt 3 280 400
#../benchmark indexes/london ./queries/london/1k-starts-or-ends-with-x.txt 3 280 400
#../benchmark indexes/london ./queries/london/1k-starts-with-x-ends-with-y-strong.txt 3 280 400
#../benchmark indexes/london ./queries/london/1k-starts-with-x-ends-with-y-weak.txt 3 280 400
#../benchmark indexes/london ./queries/london/top-k.txt 3 280 400

../benchmark indexes/madrid_baseline indexes/madrid_baseline ./queries/madrid/10k-uses-x.txt 1
#../benchmark indexes/madrid_ex indexes/madrid_ex5_HTRG32 ./queries/madrid/10k-uses-x.txt 1
