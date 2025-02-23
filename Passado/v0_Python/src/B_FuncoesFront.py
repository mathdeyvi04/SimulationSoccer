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
    TEMPO_TOTAL_DE_DURACAO = 1
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


def dispondo_tempo_de_partida(
        tempo_de_partida: int,
        janela: pg.Surface
) -> int:
    """
    Descrição:
        Função responsável por prover a menção de quanto tempo já se passou.
    
    Parâmetros:
        Autoexplicativos.
        
    Retorno:
        Texto na tela da partida indicando quanto tempo
        já se passou.
    """
    horario = pg.font.Font(
        None,
        25
    ).render(
        f"{int(tempo_de_partida) // 60}min:{int(tempo_de_partida) % 60}sec",
        True,
        (
            255,
            255,
            255
        )
    )
    janela.blit(
        horario,
        horario.get_rect(
            topright=(
                DIMENSOES_MINIMAS[0][1],
                22
            )
        )
    )
    return tempo_de_partida + MEDIDOR_DE_FREQUENCIA_DE_FRAMES * pow(10, -3)


def dispondo_quantidade_de_gols(
        janela
):
    """
    Descrição:
        Função responsável por apresentar
        a quantidade de gols de cada time.

    Parâmetros:
        Autoexplicativos.

    Retorno:
        Texto da quantidade de gols.
    """
    quantidade_de_gols = pg.font.Font(
        None,
        35
    ).render(
        f"{QUANTIDADE_DE_GOLS[0]}      {QUANTIDADE_DE_GOLS[1]}",
        True,
        (
            0,
            0,
            0
        )
    )
    janela.blit(
        quantidade_de_gols,
        quantidade_de_gols.get_rect(
            center=(
                sum(DIMENSOES_MINIMAS[0]) // 2,
                60
            )
        )
    )


