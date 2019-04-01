#!/bin/bash

# henter speedup data fra resultatfiles $file og formaterer det til fila
# $result sa. det kan plottes i et histogram

file=./stats/stats_100_64_12_15.txt
bench=./benchmark.tmp
result=./benchmark.txt

sed -n -re "/(TEST:|sequential_on_access|rpt|tagged|user).*/p" $file > $bench
for _ in $(seq 1 5); do sed -i '$d' $bench; done
sed -i "s/sequential_on_access/sequential/g" $bench
sed -i -re "s/(sequential|rpt|tagged|user)\s*([0-9]\.[0-9]+)\s*([0-9]\.[0-9]+).*/\1 \3/g" $bench

printf "%-15s %-4s %-4s %-4s %-4s\n\n" "Program" "rpt" "seq" "tag" "user" > $result
for i in $(seq 0 11); do
    let s=i*5+1
    let e=s+4

    tst=$(sed -n "${s},${e}p" $bench | sed -n '/TEST:/p' | awk '{print $2}' | sed 's/\_/-/g')
    rpt=$(sed -n "${s},${e}p" $bench | sed -n '/rpt/p' | awk '{print $2}')
    tag=$(sed -n "${s},${e}p" $bench | sed -n '/tagged/p' | awk '{print $2}')
    seq=$(sed -n "${s},${e}p" $bench | sed -n '/sequential/p' | awk '{print $2}')
    usr=$(sed -n "${s},${e}p" $bench | sed -n '/user/p' | awk '{print $2}')

    printf "%-15s %4s %4s %4s %4s\n" $tst $rpt $seq $tag $usr >> $result

done

rm $bench
