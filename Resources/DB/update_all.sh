#!/usr/bin/env bash

set -euo pipefail
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo downloading last allmods.zip
echo ...
rm -f allmods.zip
curl https://ftp.modland.com/allmods.zip --output allmods.zip
echo unzipping
echo ...
unzip allmods.zip
echo converting
echo ...
./conv_modland.sh allmods.txt
echo HVSC
echo ...
cd HVSC
./conv_stil.sh
./conv_hvscSL.sh
cd ..
echo ASMA
echo ...
cd ASMA
./get_asma_latest.sh
./gen_new_asma.sh Asma
cp trunk/Extras/Docs/STIL.txt .
./conv_stil.sh
./conv_asma.sh Asma.txt

cd ..
echo Create DB
./create_db.sh
rm -f comp*.txt
rm -f allmods.txt
rm -f allmods.zip
rm -f ASMA/asma*.txt
rm -f HVSC/hvsc*.txt
