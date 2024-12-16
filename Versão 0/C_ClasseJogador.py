from D_ClasseBola import *

TAMANHO_JOGADOR = 10

VEL_DE_IMPULSO_JOGADOR = 30




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

    def segurar_bola(
            self,
            bola: Bola
    ):
        """
        Descrição:
            Função responsável por capacitar o jogador de segurar a bola.

        Parâmetros:
            Autoexplicativos.

        Retorno:
            Bola seguirá o jogador.
        """

        if (
            bola.pos - self.pos
        ).magnitude() <= (
                TAMANHO_JOGADOR + TAMANHO_BOLA[0]
        ):
            # Caso esteja em uma distância mínima

            # Primeiro, bola acompanha
            bola.vel = self.vel

