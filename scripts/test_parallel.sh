#!/bin/bash
rm temp.txt

for i in $(seq 10)
do
   ./scripts/multiprocessing.sh >> data/temp.txt
done

python scripts/filter.py > data/multiprocessing_out.csv
