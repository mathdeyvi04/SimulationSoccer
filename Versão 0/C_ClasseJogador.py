from D_ClasseBola import *

TAMANHO_JOGADOR = 10

VEL_DE_IMPULSO_JOGADOR = 10


class Jogador(Objeto):
    """
    Descrição:
        Classe responsável por representar nossos jogadores e suas
        respectivas ações.
    """

    def __init__(
            self,
            numero_jogador: int
    ):
        super().__init__()

        self.numero_jogador = numero_jogador
        self.cor = "red" if numero_jogador % 2 == 0 else "blue"

        self.pos = pg.Vector2(
            rd.randint(
                DIMENSOES_MINIMAS[0][0],
                DIMENSOES_MINIMAS[0][1]
            ),
            rd.randint(
                DIMENSOES_MINIMAS[1][0],
                DIMENSOES_MINIMAS[1][1]
            )
        )
