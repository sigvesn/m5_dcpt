export M5_CPU2000=$(pwd)/data/cpu2000

gdb --args build/ALPHA_SE/m5.debug --remote-gdb-port=0 \
-re --outdir=output/ammp-user \
/opt/prefetcher/m5/configs/example/se.py \
--checkpoint-dir=/opt/prefetcher/lib/cp --checkpoint-restore=1000000000 \
--at-instruction --caches --l2cache --standard-switch \
--warmup-insts=10000000 --max-inst=10000000 --l2size=1MB \
--bench=ammp --prefetcher=on_access=true:policy=proxy \
