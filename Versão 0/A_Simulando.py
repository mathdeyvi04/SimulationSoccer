from B_FuncoesFront import *


def simular() -> None:
    """
    Descrição:
        Função responsável por executar a tela e puxar toda a aplicação.
    """
    pg.init()

    janela, campo = carregando_visual()

    FIM_DE_JOGO = False
    while not FIM_DE_JOGO:
        for evento in pg.event.get():
            if evento.type == pg.QUIT:
                FIM_DE_JOGO = True

            # Verificação de Teclas Pressionadas
            if evento.type == pg.KEYDOWN:
                if evento.key == pg.K_ESCAPE:
                    print(f"\nÚltima posição do mouse registrada: {pg.mouse.get_pos()}")
                    FIM_DE_JOGO = True
        janela.blit(
            campo,
            (0, 0)
        )

        pg.time.wait(
            # Milisegundos
            DT
        )
        pg.display.flip()

    pg.quit()


if __name__ == '__main__':
    simular()
