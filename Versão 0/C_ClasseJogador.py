from D_ClasseBola import *

TAMANHO_JOGADOR = 10
VEL_DE_IMPULSO_JOGADOR = 30

DISTANCIA_MINIMA_PARA_CAPTURA = 20
QUANTO_DE_FORCA_GANHA_PARA_CHUTE = 3


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
        self.numero_jogador = numero_jogador // 2 if numero_jogador % 2 == 0 else (numero_jogador + 1) // 2
        self.cor = (
            252,
            114,
            114
        ) if numero_jogador % 2 == 0 else (
            97,
            97,
            250
        )

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
        self.texto_do_numero_do_jogador = pg.font.Font(
            None,
            25
        ).render(
            str(self.numero_jogador),
            True,
            (
                255,
                255,
                255
            )
        )

        # Atributos Privados
        self._jogador_esta_com_a_bola = False
        self._acumulando_forca_de_chute = 0

    # Métodos de Revisão ou Técnicos
    def visualizando_direcao_de_chute(
            self,
            janela: pg.Surface
    ):
        """
        Descrição:
            Função responsável por possibilitar forma de visualizar
            a direção da intenção de chute.
        """
        pos_mouse = pg.Vector2(
            pg.mouse.get_pos()
        )

        vetor_rel = (pos_mouse - self.pos).normalize()

        pg.draw.line(
            janela,
            "black",
            self.pos,
            self.pos + vetor_rel * 50,
            2
        )

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
            bola: Bola,
            janela: pg.Surface
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
            if self.vel.magnitude() != 0:
                bola.pos = self.pos + self.vel.normalize() * (
                        bola.pos - self.pos
                ).magnitude()

        if self._acumulando_forca_de_chute:
            self._acumulando_forca_de_chute += QUANTO_DE_FORCA_GANHA_PARA_CHUTE
            """
            self.visualizando_direcao_de_chute(
                janela
            )
            """

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

    def iniciar_preparacao_para_chute(self):
        """
        Descrição:
            Apesar de autoexplicativo, tenha noção de que ele não
            precisará estar perto da bola para chutá-la. Esta última,
            sim, precisa que esteja perto.
        """

        self._acumulando_forca_de_chute = 1

    def chutar_bola(
            self,
            bola: Bola
    ):
        """
        Descrição:
            Função responsável por possibilitar o chute.

            A velocidade final da bola depende unicamente do quanto
            de força o jogador acumulou no tempo.
            Haverá conservação de momento, algo que poderá fazer o jogador
            parar ou se mover opostamente.
        """

        if self._estar_perto_de_um_ponto(
                bola
        ):
            bola.vel = self._acumulando_forca_de_chute * (
                    pg.Vector2(
                        pg.mouse.get_pos()
                    ) - self.pos
            ).normalize()

            self.vel = self.vel - 0.1 * bola.vel

        # De qualquer forma, liberar acúmulo de força
        self._acumulando_forca_de_chute = 0

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
