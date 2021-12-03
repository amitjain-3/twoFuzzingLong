#!/bin/bash

declare -a pids

for i in $(seq 2)
do
    ./all coverage &
    pids[${i}]=$!
    taskset -cp $(($i-1)) ${pids[${i}]} > /dev/null
done

# wait for all pids
for pid in ${pids[*]}; do
    wait $pid
done