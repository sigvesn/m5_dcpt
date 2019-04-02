#!/bin/bash

for i in $(seq 3 15); do
    for j in $(seq 2 4); do
        let ts=$i*20
        let dw=$j*5
        echo $ts $dw

        find . -name "stats_${ts}_*_*_${dw}*.txt" -printf '%p\n' -exec sed -n 212,1p {} \; | sed 's/\.\/stats_\|user\|\.txt//g' | awk '{ if(length($1) > 5) printf "%s ", $1; else print $1 }' | sed 's/_/ /g' > best_user_stats${ts}_${dw}.txt

        sort -n -k 3 -o best_user_stats${ts}_${dw}.txt best_user_stats${ts}_${dw}.txt
    done
done
