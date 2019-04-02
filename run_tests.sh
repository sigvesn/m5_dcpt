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

for t in $(seq 1 1); do
    for i_f in $(seq 2 2); do
        for d_b in $(seq 4 24); do
            for d_w in $(seq 1 1); do

                add $(($t*88)) $(($i_f*32)) $(($d_b*1)) $(($d_w*14))

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
    filename=results/stats_${t}_${i_f}_${d_b}_${d_s}.txt
    mv stats.txt ${filename}

    echo -e "\nrun with:
    \ttable size:        $t
    \tin_flight size:    $i_f
    \tdelta buffer size: $d_b
    \tdelta width:       $d_s" >> ${filename}
done
