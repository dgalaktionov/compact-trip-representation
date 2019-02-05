mkdir indexes
#zcat ./texts/cnlong.txt.gz | ../BUILDALLwcsa stdin ./indexes/cnlong "sPsi=16; nsHuff=16;psiSF=4"
#zcat ./texts/trayectorias_int.txt.gz | ../BUILDALLwcsa stdin ./indexes/trayectorias "sPsi=64; nsHuff=16;psiSF=4"
#cat ./texts/trayectorias_mini.txt | ../BUILDALLwcsa stdin ./indexes/trayectorias_mini "sPsi=64; nsHuff=16;psiSF=4"
#zcat ./texts/trayectorias_int.txt.gz | head -c 426 | ../BUILDALLwcsa stdin ./indexes/trayectorias "sPsi=64; nsHuff=16;psiSF=4"
#cat ./texts/order_test.txt | ../BUILDALLwcsa stdin ./indexes/trayectorias "sPsi=64; nsHuff=16;psiSF=4"
#zcat ./texts/trayectorias_int_ordenadas.txt.gz | ../BUILDALLwcsa stdin ./indexes/trayectorias "sPsi=64; nsHuff=16;psiSF=4"
#cat ./trayectorias.txt | ../BUILDALLwcsa stdin ./indexes/generadas "sPsi=64; nsHuff=16;psiSF=4"
#cat ./nyc.txt | ../BUILDALLwcsa stdin ./indexes/nyc "sPsi=64; nsHuff=16;psiSF=4"
#cat ./trayectorias.txt | ../BUILDALLwcsa stdin ./indexes/madrid "sPsi=64; nsHuff=16;psiSF=4"
#cat ./madrid.txt | ../BUILDALLwcsa stdin ./indexes/madrid "sPsi=64; nsHuff=16;psiSF=4"

#cat ./texts/london.txt | ../BUILDALLwcsa stdin ./indexes/london "sPsi=64; nsHuff=16;psiSF=4"
#cat ./texts/test.txt | ../BUILDALLwcsa stdin ./indexes/test "sPsi=64; nsHuff=16;psiSF=1"
#cat ./texts/madrid.txt | ../BUILDALLwcsa stdin ./indexes/madrid "sPsi=512; nsHuff=16;psiSF=1"
#cat ./texts/madrid_bus.txt | ../BUILDALLwcsa stdin ./indexes/madrid_bus "sPsi=64; nsHuff=16;psiSF=4"

#cat ./texts/madrid.txt | ../BUILDALLwcsa stdin ./indexes/madrid32 "sPsi=32; nsHuff=16;psiSF=1"
#cat ./texts/madrid.txt | ../BUILDALLwcsa stdin ./indexes/madrid128 "sPsi=128; nsHuff=16;psiSF=1"
#cat ./texts/madrid.txt | ../BUILDALLwcsa stdin ./indexes/madrid512 "sPsi=512; nsHuff=16;psiSF=1"

#zcat ./texts/porto.txt.gz | ../BUILDALLwcsa stdin ./indexes/porto32 "sPsi=32; nsHuff=16;psiSF=1"
#zcat ./texts/porto.txt.gz | ../BUILDALLwcsa stdin ./indexes/porto128 "sPsi=128; nsHuff=16;psiSF=1"
#zcat ./texts/porto.txt.gz | ../BUILDALLwcsa stdin ./indexes/porto512 "sPsi=512; nsHuff=16;psiSF=1"

# cat ./texts/madrid.txt | ../BUILDALLwcsa stdin ./indexes/madrid512 "sPsi=512; nsHuff=16;psiSF=1"
#zcat ./texts/porto.txt.gz | ../BUILDALLwcsa stdin ./indexes/porto512 "sPsi=512; nsHuff=16;psiSF=1"
#cat ./texts/porto_big.txt | ../BUILDALLwcsa stdin ./indexes/porto512 "sPsi=512; nsHuff=16;psiSF=1"
#cat ./texts/madrid_flat.txt | ../BUILDALLwcsa stdin ./indexes/madrid_flat "sPsi=512; nsHuff=16;psiSF=1"

# ssh bichogrande "cd HDD3/spire/scripts.test; cat ./texts/madrid.txt | ../BUILDALLwcsa stdin ./indexes/madrid_16 \"sPsi=16; nsHuff=16;psiSF=4\"; \\
# 	cat ./texts/madrid.txt | ../BUILDALLwcsa stdin ./indexes/madrid_64 \"sPsi=64; nsHuff=16;psiSF=4\"; \\
# 	cat ./texts/madrid.txt | ../BUILDALLwcsa stdin ./indexes/madrid_256 \"sPsi=256; nsHuff=16;psiSF=4\""

cat ./texts/madrid_lines.zst | zstd -d | ../BUILDALLwcsa stdin ./indexes/madrid_lines32 "sPsi=32; nsHuff=16;psiSF=1; bTimes=RG32;bLines=RG32"
mv indexes/madrid_lines32.l indexes/madrid_lines_RG32.l
mv indexes/madrid_lines32.times indexes/madrid_lines_RG32.times
cat ./texts/madrid_lines.zst | zstd -d | ../BUILDALLwcsa stdin ./indexes/madrid_lines128 "sPsi=128; nsHuff=16;psiSF=1; bTimes=RG32;bLines=RRR128"
mv indexes/madrid_lines128.l indexes/madrid_lines_RRR128_1.l
mv indexes/madrid_lines128.times indexes/madrid_lines_RRR128_1.times
cat ./texts/madrid_lines.zst | zstd -d | ../BUILDALLwcsa stdin ./indexes/madrid_lines512 "sPsi=512; nsHuff=16;psiSF=1; bTimes=RRR128;bLines=RRR128"
mv indexes/madrid_lines512.l indexes/madrid_lines_RRR128_2.l
mv indexes/madrid_lines512.times indexes/madrid_lines_RRR128_2.times
