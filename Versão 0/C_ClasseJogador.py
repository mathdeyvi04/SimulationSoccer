from D_ClasseBola import *

TAMANHO_JOGADOR = 10

VEL_DE_IMPULSO_JOGADOR = 30

COEF_DE_FORCA_DO_CHUTE = 0.5

MASSA_JOGADOR = 10

DISTANCIA_MINIMA_PARA_CAPTURA = TAMANHO_JOGADOR + TAMANHO_BOLA[0]


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

        self._estar_com_a_bola = False
        self.forca_de_chute = 0


    def apresentar_possiveis_pontos_de_captura(
            self,
            tela: pg.Surface
    ):
        """
        Descrição:
            Função responsável por apresentar uma forma de visualizar
            os pontos possiveis que o jogador pode capturar.
        """

        # Vamos desenhar uma circunferencia em torno dos pontos
        PASSO = 1
        for x_ in range(int(self.pos.x) - 100, int(self.pos.x) + 100, PASSO):
            for y_ in range(int(self.pos.y) - 100, int(self.pos.y) + 100, PASSO):
                # Aqui vai depender da métrica usada para medir a distância.
                if abs((
                    pg.Vector2(
                        x_, y_
                    ) - self.pos
                ).magnitude() - DISTANCIA_MINIMA_PARA_CAPTURA) <= 1:
                    pg.draw.circle(
                        tela,
                        "white",
                        (x_, y_),
                        1,
                        5
                    )



    def segurar_bola(
            self,
            bola: Bola,
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
                30
        ):
            # Caso esteja em uma distância mínima
            # Primeiro, bola acompanha
            bola.vel = self.vel
            self._estar_com_a_bola = True

    def soltar_bola(
            self,
            bola: Bola
    ):
        """
        Descrição:
            Função responsável pelo jogador soltar a bola.
        """

        if self._estar_com_a_bola:
            bola.vel = self.vel.copy()
            self._estar_com_a_bola = False

    def chutar_bola(
            self,
            bola: Bola
    ):
        """
        Descrição:
            Função responsável por chutar a bola e providenciar as
            respectivas consequências.
        """

        if (
                bola.pos - self.pos
        ).magnitude() <= (
                DISTANCIA_MINIMA_PARA_CAPTURA
        ):
            # Devemos chutar a bola conservando momento.
            bola.vel = COEF_DE_FORCA_DO_CHUTE * self.forca_de_chute * self.vel.normalize()

            print(self.vel.magnitude())
            self.vel = self.vel - (MASSA_BOLA / MASSA_JOGADOR) * bola.vel
            print(self.vel.magnitude())
        self.forca_de_chute = 0
