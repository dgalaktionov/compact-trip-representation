# echo ''
# echo ''
# ./goQuery.sh porto32 benchmark porto5_WMRG32
# ./goQuery.sh porto128 benchmark porto5_WMRG32
# ./goQuery.sh porto512 benchmark porto5_WMRG32
#
# ./goQuery.sh madrid32 benchmark madrid8_5_WMRG32
# ./goQuery.sh madrid128 benchmark madrid8_5_WMRG32
# ./goQuery.sh madrid512 benchmark madrid8_5_WMRG32

# echo ''
# echo 'Madrid 5'
# ./goQuery.sh madrid 32 8_5_HTRG32
# ./goQuery.sh madrid 32 8_5_HTRRR32
# ./goQuery.sh madrid 32 8_5_HTRRR64
# ./goQuery.sh madrid 32 8_5_HTRRR128

# ./goQuery.sh madrid 32 8_5_WMRG32
# ./goQuery.sh madrid 32 8_5_WMRRR32
# ./goQuery.sh madrid 32 8_5_WMRRR64
# ./goQuery.sh madrid 32 8_5_WMRRR128

# echo ''
# echo 'Porto 5'
# ./goQuery.sh porto 32 5_HTRG32
# ./goQuery.sh porto 32 5_HTRRR32
# ./goQuery.sh porto 32 5_HTRRR64
# ./goQuery.sh porto 32 5_HTRRR128

# ./goQuery.sh porto 32 5_WMRG32
# ./goQuery.sh porto 32 5_WMRRR32
# ./goQuery.sh porto 32 5_WMRRR64
# ./goQuery.sh porto 32 5_WMRRR128


# ./goQueryFari.sh madrid_flat benchmark madrid_flat5 10k-uses-t5
# ./goQueryFari.sh madrid_flat benchmark madrid_flat5 10k-uses-t60
# ./goQueryFari.sh madrid_flat benchmark madrid_flat5 10k-uses-t600
# ./goQueryFari.sh madrid_flat30 benchmark6 madrid_flat30 10k-uses-t5
# ./goQueryFari.sh madrid_flat30 benchmark6 madrid_flat30 10k-uses-t60
# ./goQueryFari.sh madrid_flat30 benchmark6 madrid_flat30 10k-uses-t600

# ./goQueryFari.sh madrid512 benchmark madrid8_5 10k-uses-t5
# ./goQueryFari.sh madrid512 benchmark madrid8_5 10k-uses-t60
# ./goQueryFari.sh madrid512 benchmark madrid8_5 10k-uses-t600
# ./goQueryFari.sh madrid512_30 benchmark6 madrid8_30 10k-uses-t5
# ./goQueryFari.sh madrid512_30 benchmark6 madrid8_30 10k-uses-t60
# ./goQueryFari.sh madrid512_30 benchmark6 madrid8_30 10k-uses-t600

# ./goQueryFari.sh madrid512 benchmark madrid8_5 top-10-times
# ./goQueryFari.sh madrid512_30 benchmark6 madrid8_30 top-10-times
# ./goQueryFari.sh madrid512 benchmark madrid8_5 top-100-times
# ./goQueryFari.sh madrid512_30 benchmark6 madrid8_30 top-100-times
#
# ./goQueryFari.sh porto512 benchmark porto5 top-10-times
# ./goQueryFari.sh porto512 benchmark6 porto30 top-10-times
# ./goQueryFari.sh porto512 benchmark porto5 top-100-times
# ./goQueryFari.sh porto512 benchmark6 porto30 top-100-times

# ./goQueryFari.sh madrid_ex benchmark madrid_ex5 10k-uses-t5
# ./goQueryFari.sh madrid_ex benchmark madrid_ex5 10k-uses-t60
# ./goQueryFari.sh madrid_ex benchmark madrid_ex5 10k-uses-t600
# ./goQueryFari.sh madrid_ex30 benchmark6 madrid_ex30 10k-uses-t5
# ./goQueryFari.sh madrid_ex30 benchmark6 madrid_ex30 10k-uses-t60
# ./goQueryFari.sh madrid_ex30 benchmark6 madrid_ex30 10k-uses-t600

./goQuery.sh madrid
echo ''
./goQuery.sh porto