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
        Tupla das superfícies, a janela e o campo onde poremos tudo.
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


def carregando_textos(

):
    """
    Descrição:
        Função responsável por disponibilizar
        diversas ferramentas ligadas a textos.

        Intimamente relacionado ao arquivo G_Variaveis_De_Texto.py

    Parâmetros:
        Nenhum.

    Retorno:
        Dicionário que conterá diversas coisas estranhas.
    """

    return {
        FONTE_PARA_GOL: pg.font.Font(
            FONTE_ESCOLHIDA,
            50
        )
    }


def animacao_de_gol(
        janela: pg.Surface
) -> None:
    """
    Descrição:
        Função responsável por apresentar
        a animação de gol.
    """
    TEMPO_TOTAL_DE_DURACAO = 2
    PULSACAO = 500  # Lembre-se que isto é milisegundos.
    TAMANHO_DA_FONTE_PARA_GOL = 50

    t = 0
    while t < TEMPO_TOTAL_DE_DURACAO:
        superficie = pg.font.Font(
            None,
            TAMANHO_DA_FONTE_PARA_GOL
        ).render(
            "G O L!",
            True,
            (
                0,
                0,
                0
            )
        )

        local = superficie.get_rect(
            center=(
                sum(DIMENSOES_MINIMAS[0]) // 2,
                30
            )
        )

        janela.blit(
            superficie,
            local
        )
        pg.display.flip()

        pg.time.wait(
            # Milisegundos
            PULSACAO
        )
        t += PULSACAO * pow(10, -3)
