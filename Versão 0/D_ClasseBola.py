from E_ClasseObjeto import *

TAMANHO_BOLA = (
    20,
    20
)


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
