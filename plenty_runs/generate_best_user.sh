#!/bin/bash

find . -name "stats*.txt" -printf '%p\n' -exec sed -n 212,1p {} \; | sed 's/\.\/stats_\|user\|\.txt//g' | awk '{ if(length($1) > 5) printf "%12s -> ", $1; else print $1 }' > best_user_stats.txt
sort -n -k 3 -o best_user_stats.txt best_user_stats.txt
