from E_ClasseObjeto import *

TAMANHO_BOLA = (
    20,
    20
)

VELOCIDADE_DE_EXPULSAO_DO_GOL = 100

class Bola(Objeto):
    """
    Descrição:
        Função responsável por representar a bola e sua física.
    """

    def __init__(self):
        super().__init__()  # Iniciando a classe pai.

        self.imagem = pg.transform.scale(
            pg.image.load(
                "bola.png"
            ),
            TAMANHO_BOLA
        )

        self.pos = pg.Vector2(
            # Essas correções mínimas tinham que vir para cá.
            TAMANHO_TELA[0] // 2 - 2,
            TAMANHO_TELA[1] // 2 + 3
        )
        # Pois a imagem fica deslocada.
        self.POS_IMAGEM = self.pos + pg.Vector2(
            - TAMANHO_BOLA[0] // 2,
            - TAMANHO_BOLA[1] // 2
        )

    # Métodos de Revisão ou Técnicos
    def desenhar_redondeza(self, janela):
        """
        Descrição:
            Função responsável por desenhar um círculo
            em uma região de distância fixa da bola.
        """

        DISTANCIA_TESTE = TAMANHO_BOLA[0] * 10
        for x_ in range(int(self.pos.x - DISTANCIA_TESTE), int(self.pos.x + DISTANCIA_TESTE), 1):
            for y_ in range(int(self.pos.y - DISTANCIA_TESTE), int(self.pos.y + DISTANCIA_TESTE), 1):
                if abs(
                        (
                                pg.Vector2(
                                    x_,
                                    y_
                                ) - self.pos
                        ).magnitude() - TAMANHO_BOLA[0]
                ) <= 0.5:
                    pg.draw.circle(
                        janela,
                        "white",
                        (x_, y_),
                        1,
                        2
                    )

    # Métodos de Ação Ativa
    def fazer_existir(self):
        """
        Descrição:
            Função responsável por executar todos os métodos
            inerentes à existência da bola.
        """
        self.estar_preso_ao_campo()
        self.arrastar()
        self.movimentar(
            INTERV_DE_TEMPO
        )

        self.POS_IMAGEM = self.pos + pg.Vector2(
            - TAMANHO_BOLA[0] // 2,
            - TAMANHO_BOLA[1] // 2
        )

    # Métodos de Ação Passiva
    def verificar_se_eh_gol(
            self,
    ) -> bool:
        """
        Descrição:
            Função responsável por verificar se houve gol.
            Caso sim, iniciará todas as medidas cabíveis.
        """

        MARGEM_PARA_QUE_HAJA_GOL = 0

        se_chegou_ao_limite_lateral_do_campo = self.pos.x <= (
               MARGEM_PARA_QUE_HAJA_GOL + DIMENSOES_MINIMAS[0][0]
        ) or self.pos.x >= (
               DIMENSOES_MINIMAS[0][1] - MARGEM_PARA_QUE_HAJA_GOL
        )

        se_esta_nos_limites_verticais_do_gol = LIMITES_VERTICAIS_DOS_GOLS[0] <= self.pos.y <= LIMITES_VERTICAIS_DOS_GOLS[1]

        return se_chegou_ao_limite_lateral_do_campo and se_esta_nos_limites_verticais_do_gol

    def houve_gol(
            self
    ) -> None:
        """
        Descrição:
            Função responsável por tomar
            as providências quando há gol de fato.
        """

        self.pos = pg.Vector2(
            self.pos.x + (
                5 if self.pos.x < TAMANHO_TELA[0] // 2 else -5
            ),
            sum(LIMITES_VERTICAIS_DOS_GOLS) // 2
        )

        self.vel = pg.Vector2(
            VELOCIDADE_DE_EXPULSAO_DO_GOL * (
                1 if self.pos.x < TAMANHO_TELA[0] // 2 else -1
            ),
            rd.randint(
                -VELOCIDADE_DE_EXPULSAO_DO_GOL,
                VELOCIDADE_DE_EXPULSAO_DO_GOL
            )
        )

        QUANTIDADE_DE_GOLS[
            # Alto nível aqui
            self.pos.x < TAMANHO_TELA[0]
        ] += 1





