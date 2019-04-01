#!/bin/bash

src=stats
dst=best_user_stats

# henter user stats fra resulteter og sampler det i filer basert på entry table size og delta width
for i in $(seq 3 15); do
    for j in $(seq 2 4); do
        let ts=$i*20
        let dw=$j*5

        find $src/ -name "stats_${ts}_*_*_${dw}*.txt" -printf '%p\n' -exec sed -n 212,1p {} \; | sed 's/\.\/stats_\|user\|\.txt//g' | awk '{ if(length($1) > 5) printf "%s ", $1; else print $1 }' | sed 's/_/ /g' > $dst/best_user_stats${ts}_${dw}.txt

    done
done

# sletter tomme filer og sorterer de med innhold
cd $dst
find . -empty -type f -print0 | xargs -0 rm

for f in ./*; do
    sort -n -k 3 -o $f $f
done
