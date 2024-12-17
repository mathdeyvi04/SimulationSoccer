import pygame as pg
from math import sqrt
import random as rd

TAMANHO_TELA = (
    800, 500
)
DIMENSOES_MINIMAS = (
    (
        int(0.053 * TAMANHO_TELA[0] + 0.7),
        int(0.94 * TAMANHO_TELA[0] + 0.2)
    ),
    (
        int(0.085 * TAMANHO_TELA[1] + 0.8),
        int(0.926 * TAMANHO_TELA[1] + 0.8)
    ),
)

# 1000 milisegundos = 1s
# 29 -> 30 quadros por segundo.
MEDIDOR_DE_FREQUENCIA_DE_FRAMES = 29

INTERV_DE_TEMPO = 0.03

