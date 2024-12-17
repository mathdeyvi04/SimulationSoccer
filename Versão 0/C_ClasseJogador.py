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

        # Definimos características inerentes à cada jogador.
        self.numero_jogador = numero_jogador
        self.cor = "red" if numero_jogador % 2 == 0 else "blue"

        # Definimos o local inicial do jogador.
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

        # Atributos Interessantes

        # Atributos Privados
        self._PESO_PARA_DISTANCIA_EM_X = 1
        self._PESO_PARA_DISTANCIA_EM_Y = 1

    # Métodos de Revisão ou Técnicos

    # Métodos de Estado
    def estar_perto_de_um_ponto(
            self,
            ponto: pg.Vector2
    ) -> bool:
        """
        Descrição:
            Função responsável por permitir saber se o jogador está perto.

            Infelizmente, acredito que devido a não-quadratura da janela,
            não basta apenas colocar a distância. Deve haver uma espécie
            de peso relativo no eixo_y.
        """

        return self.pos != ponto

    # Métodos de Ações Ativa

    # Métodos de Ações Passivas
