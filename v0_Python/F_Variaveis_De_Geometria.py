from G_Variaveis_De_Texto import *

TAMANHO_TELA = (
    800, 500
)
DIMENSOES_MINIMAS = (
    (
        # Sobre X
        int(0.053 * TAMANHO_TELA[0] + 0.7),  # Mais Esquerdo
        int(0.94 * TAMANHO_TELA[0] + 0.2)  # Mais Direito
    ),
    (
        # Sobre Y
        int(0.085 * TAMANHO_TELA[1] + 0.8),  # Mais 'De Alto'
        int(0.926 * TAMANHO_TELA[1] + 0.8)  # Mais 'De Baixo'
    ),
)

LIMITES_VERTICAIS_DOS_GOLS = (
    int(0.42 * TAMANHO_TELA[1] + 7.33),  # Mais Acima
    int(0.59 * TAMANHO_TELA[1] - 3)  # Mais Abaixo
)
