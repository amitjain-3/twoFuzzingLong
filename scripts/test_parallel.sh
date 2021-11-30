#!/bin/bash
rm temp.txt

for i in $(seq 10)
do
   ./multiprocessing.sh >> temp.txt
done

python filter.py > ../data/multiprocessing_out.csv

