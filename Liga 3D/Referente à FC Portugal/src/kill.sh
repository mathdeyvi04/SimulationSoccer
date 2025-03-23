#!/bin/bash
# Encerrar todas as instâncias de Jogadores de forma forçada.

# *pkill* é um comando que permite matar processos com base
# em seus nomes ou outros atributos. Ele é semelhante ao
# kill, mas em vez de exigir o PID (ID do processo), você
# pode especificar o nome do processo ou parte dele.

# *-9* é um sinal enviado ao processo. No Unix,
# os sinais são usados para se comunicar com processos.
# O sinal 9 corresponde ao SIGKILL, que força o término
# imediato do processo. O SIGKILL não pode ser ignorado
# ou interceptado pelo processo, então ele é usado quando
# você precisa encerrar um processo de forma abrupta.

# O -e faz com que o pkill exiba informações sobre os
# processos que foram encerrados. Ele mostra o PID e o
# nome do processo que foi morto.

# se você rodar pkill -f "python script.py", ele matará
# processos cuja linha de comando contenha python
# script.py, e não apenas processos chamados python.

pkill -9 -e -f "python3 ./Run_"
