#!/bin/sh

cat ./texts/madrid_flat.txt | ../BUILDALLHTRG32 stdin ./indexes/madrid_flat "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_flat.times indexes/madrid_flat5_HTRG32.times
cat ./texts/madrid_flat.txt | ../BUILDALLHTRRR32 stdin ./indexes/madrid_flat "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_flat.times indexes/madrid_flat5_HTRRR32.times
cat ./texts/madrid_flat.txt | ../BUILDALLHTRRR64 stdin ./indexes/madrid_flat "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_flat.times indexes/madrid_flat5_HTRRR64.times
cat ./texts/madrid_flat.txt | ../BUILDALLHTRRR128 stdin ./indexes/madrid_flat "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_flat.times indexes/madrid_flat5_HTRRR128.times

cat ./texts/madrid_flat.txt | ../BUILDALLHTRG32_30 stdin ./indexes/madrid_flat "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_flat.times indexes/madrid_flat30_HTRG32.times
cat ./texts/madrid_flat.txt | ../BUILDALLHTRRR32_30 stdin ./indexes/madrid_flat "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_flat.times indexes/madrid_flat30_HTRRR32.times
cat ./texts/madrid_flat.txt | ../BUILDALLHTRRR64_30 stdin ./indexes/madrid_flat "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_flat.times indexes/madrid_flat30_HTRRR64.times
cat ./texts/madrid_flat.txt | ../BUILDALLHTRRR128_30 stdin ./indexes/madrid_flat "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_flat.times indexes/madrid_flat30_HTRRR128.times

cat ./texts/madrid_ex.txt | ../BUILDALLHTRG32 stdin ./indexes/madrid_ex "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_ex.times indexes/madrid_ex5_HTRG32.times
cat ./texts/madrid_ex.txt | ../BUILDALLHTRRR32 stdin ./indexes/madrid_ex "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_ex.times indexes/madrid_ex5_HTRRR32.times
cat ./texts/madrid_ex.txt | ../BUILDALLHTRRR64 stdin ./indexes/madrid_ex "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_ex.times indexes/madrid_ex5_HTRRR64.times
cat ./texts/madrid_ex.txt | ../BUILDALLHTRRR128 stdin ./indexes/madrid_ex "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_ex.times indexes/madrid_ex5_HTRRR128.times

cat ./texts/madrid_ex.txt | ../BUILDALLHTRG32_30 stdin ./indexes/madrid_ex "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_ex.times indexes/madrid_ex30_HTRG32.times
cat ./texts/madrid_ex.txt | ../BUILDALLHTRRR32_30 stdin ./indexes/madrid_ex "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_ex.times indexes/madrid_ex30_HTRRR32.times
cat ./texts/madrid_ex.txt | ../BUILDALLHTRRR64_30 stdin ./indexes/madrid_ex "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_ex.times indexes/madrid_ex30_HTRRR64.times
cat ./texts/madrid_ex.txt | ../BUILDALLHTRRR128_30 stdin ./indexes/madrid_ex "sPsi=512; nsHuff=16;psiSF=1"
mv indexes/madrid_ex.times indexes/madrid_ex30_HTRRR128.times
