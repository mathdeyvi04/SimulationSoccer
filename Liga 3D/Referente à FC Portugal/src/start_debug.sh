#!/bin/bash
export OMP_NUM_THREADS=1

host=${1:-localhost}
port=${2:-3100}

for i in {1..11}; do
  # Observe como os argumentos são diferentes!
  python3 ./Run_Player.py -i $host -p $port -u $i -t Robo-debug -P 0 -D 1 &
done
