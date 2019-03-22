#!/bin/bash

find . -printf '%p\n' -name "*.txt" -exec sed -n 212,1p {} \; > best_user_stats.txt
