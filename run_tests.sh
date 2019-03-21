#!/bin/bash

mkdir -p results

idx=0
function add {
    table[$idx]=$1
    in_flight[$idx]=$2
    delta_buffer[$idx]=$3
    delta_size[$idx]=$4
    let idx++
}

add 98 32 19 12
add 10 10 10 10

for i in ${!table[*]}; do
    t=${table[$i]}; i_f=${in_flight[$i]}; d_b=${delta_buffer[$i]}; d_s=${delta_size[$i]}
    
    echo "running: $t, $i_f, $d_b, $d_s"

    sed -i "s/\(define ENTRY_TABLE_SIZE\).*/\1      $t/g"   src/buffer.hh
    sed -i "s/\(define IN_FLIGHT_BUFFER_SIZE\).*/\1 $i_f/g" src/buffer.hh
    sed -i "s/\(define DELTA_BUFFER_SIZE\).*/\1     $d_b/g" src/buffer.hh
    sed -i "s/\(define DELTA_WIDTH\).*/\1           $d_s/g" src/buffer.hh

    make
    mv stats.txt results/stats_${i}.txt

    echo " run with:
    table size:        $t
    in_flight size:    $i_f
    delta buffer size: $d_b
    delta size:        $d_s" >> results/stats_${i}.txt
done
