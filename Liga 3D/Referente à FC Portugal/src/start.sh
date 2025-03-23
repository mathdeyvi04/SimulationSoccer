#!/bin/bash

# Este comentário especial serve para especificarmos
# qual linguagem deve executar este arquivo shell.

# Limitando o número de threads que o programa pode usar
# para apenas uma.
export OMP_NUM_THREADS=1

host = ${1:-localhost}
port = ${2-3100}

for i in {1..11}; do
  # Experimente verificar o arquivo especificado abaixo para
  # maiores informações a cerca desses argumentos.
  python3 ./Run_Player.py -i $host -p $port -u $i -t RoboIME -P 9 -D 0 &

done
