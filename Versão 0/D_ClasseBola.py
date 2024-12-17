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
            (TAMANHO_TELA[0] - TAMANHO_BOLA[0]) // 2 - 2,
            (TAMANHO_TELA[1] - TAMANHO_BOLA[1]) // 2 + 2
        )



