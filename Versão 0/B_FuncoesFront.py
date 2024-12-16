from C_ClasseJogador import *


def carregando_visual(

) -> tuple[pg.Surface, pg.Surface]:
    """
    Descrição:
        Função responsável pela geração e carregamento de todas
        as interfaces da aplicação.

    Parâmetros:
        Nenhum.

    Retorno:
        Tupla das superfícies.
    """
    janela = pg.display.set_mode(
        TAMANHO_TELA
    )
    pg.display.set_caption(
        "Simulando Partida"
    )

    campo = pg.transform.scale(
        pg.image.load(
            "campo.png"
        ),
        TAMANHO_TELA
    )

    return janela, campo



