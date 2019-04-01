#!/bin/bash
# Generates a plot from the files created by aggregate_plotdata.sh using
# gnuplot and the graph_template.plg template file.

DIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd)"
# template="graph_template.plg"
template="hist_temp.plg"

# set terminal png nocrop enhanced font "/usr/share/fonts/dejavu/DejaVuSansMono.ttf,8" size 800,600
gnuplot << EOF
set terminal pngcairo enhanced font "~/.local/share/fonts/DejaVu\ Sans\ Mono\ Nerd\ Font\ Complete.ttf, 12" size 800,600
set output '/dev/null'
load '$DIR/$template'
set output '$1.png'
replot
EOF
