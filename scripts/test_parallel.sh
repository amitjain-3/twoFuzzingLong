#!/bin/bash
rm data/temp.txt

for i in $(seq 80)
do
   #./scripts/multiprocessing.sh >> data/temp.txt
   ./all coverage >> data/4_core_out_multithread_2.csv

done

#python3 scripts/filter.py > data/2_core_out_multiprocess.csv
