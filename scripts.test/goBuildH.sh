#!/bin/sh

#zcat ./texts/porto.txt.gz | ../BUILDALLwcsa stdin ./indexes/porto_baseline "sPsi=512; nsHuff=16;psiSF=1"
#cat ./texts/madrid_ex.txt | ../BUILDALLwcsa stdin ./indexes/madrid_baseline "sPsi=512; nsHuff=16;psiSF=1"
cat ./texts/madrid_lines.zst | zstd -d | ../BUILDALLwcsa stdin ./indexes/madrid_lines "sPsi=512; nsHuff=16;psiSF=1; bTimes=RR128"

