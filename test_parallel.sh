#!/bin/bash

for i in $(seq 100)
do
   ./all coverage >> single_out.csv
done
