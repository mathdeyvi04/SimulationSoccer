from D_ClasseBola import *

TAMANHO_JOGADOR = 10
VEL_DE_IMPULSO_JOGADOR = 30

DISTANCIA_MINIMA_PARA_CAPTURA = 20


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
        self._jogador_esta_com_a_bola = False
        self._acumulando_forca_de_chute = 0

    # Métodos de Revisão ou Técnicos

    # Métodos de Estado
    def _estar_perto_de_um_ponto(
            self,
            ponto: Bola
    ) -> bool:
        """
        Descrição:
            Função responsável por permitir saber se o jogador está perto.
        """

        return (
                ponto.pos - self.pos
        ).magnitude() <= DISTANCIA_MINIMA_PARA_CAPTURA

    def fazer_existir(
            self,
            bola: Bola
    ):
        """
        Descrição:
            Função responsável por executar todos os métodos
            inerentes à existência de cada jogador.
        """

        self.estar_preso_ao_campo()
        self.arrastar()
        self.movimentar(
            INTERV_DE_TEMPO
        )

        if self._jogador_esta_com_a_bola:
            bola.vel = self.vel.copy()

        if self._acumulando_forca_de_chute:
            self._acumulando_forca_de_chute += 1

    # Métodos de Ações Ativa
    def segurar_bola(
            self,
            bola
    ):
        """
        Descrição:
            Função responsável por permitir que o jogador
            leve a bola consigo.
        """

        if self._estar_perto_de_um_ponto(
                bola
        ):
            self._jogador_esta_com_a_bola = True

    # Métodos de Ações Passivas
    def soltar_bola(
            self
    ):
        """
        Descrição:
            Autoexplicação.
        """

        if self._jogador_esta_com_a_bola:
            self._jogador_esta_com_a_bola = False
