from F_Importacoes import *

COEF_ARRASTO = 0.2


class Objeto:
    """
    Descrição:
        Classe responsável por representar o alicerce
        de cada coisa que estará apresentada na tela.
    """

    def __init__(
            self,
    ):

        # Características Cinemáticas
        self.pos = pg.Vector2(0, 0)
        self.vel = pg.Vector2(0, 0)
        self.acel = pg.Vector2(0, 0)

    def estar_preso_ao_campo(
            self
    ) -> None:
        """
        Descrição:
            Função responsável por realizar as conversões
            necessárias quando algo tentar sair do campo.

        Parâmetros:
            Nenhum.

        Retorno:
            Objeto sofre reflexão.
        """

        if self.pos.x <= DIMENSOES_MINIMAS[0][0] or self.pos.x >= DIMENSOES_MINIMAS[0][1]:  # Bordas Laterais.
            self.vel.x *= -1
            return None

        if self.pos.y <= DIMENSOES_MINIMAS[1][0] or self.pos.y >= DIMENSOES_MINIMAS[1][1]:
            self.vel.y *= -1

    def movimentar(self, interv: float):
        """
        Descrição:
            Função responsável pelo movimento cinemático de cada
            corpo.

        Parâmetros:
            Autoexplicativos.

        Retorno:
            Movimento cinemático do objeto.
        """

        self.vel += self.acel * interv

        self.pos += self.vel * interv + 0.5 * self.acel * interv * interv

    def arrastar(
            self
    ):
        """
        Descrição:
            Função responsável por impedir que tanto a bola quanto os jogadores
            sem movam indefinidamente.
        """

        self.acel = - COEF_ARRASTO * self.vel
