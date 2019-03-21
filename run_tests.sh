#!/bin/bash

mkdir -p results

idx=0
function add {
    table[$idx]=$1
    in_flight[$idx]=$2
    delta_buffer[$idx]=$3
    delta_width[$idx]=$4
    let idx++
}

add 98 32 19 12

for t in $(seq 3 8); do
    for i_f in $(seq 1 2); do
        for d_b in $(seq 3 6); do
            for d_w in $(seq 2 4); do

                add $(($t*20)) $(($i_f*32)) $(($d_b*4)) $(($d_w*5))

            done
        done
    done
done

for i in ${!table[*]}; do
    t=${table[$i]}; i_f=${in_flight[$i]}; d_b=${delta_buffer[$i]}; d_s=${delta_width[$i]}
    
    echo "running: $t, $i_f, $d_b, $d_s"

    sed -i "s/\(define ENTRY_TABLE_SIZE\).*/\1      $t/g"   src/buffer.hh
    sed -i "s/\(define IN_FLIGHT_BUFFER_SIZE\).*/\1 $i_f/g" src/buffer.hh
    sed -i "s/\(define DELTA_BUFFER_SIZE\).*/\1     $d_b/g" src/buffer.hh
    sed -i "s/\(define DELTA_WIDTH\).*/\1           $d_s/g" src/buffer.hh

    make
    mv stats.txt results/stats_${i}.txt

    echo "\nrun with:
    \ttable size:        $t
    \tin_flight size:    $i_f
    \tdelta buffer size: $d_b
    \tdelta width:       $d_s" >> results/stats_${t}_${i_f}_${d_b}_${d_w}.txt
done
