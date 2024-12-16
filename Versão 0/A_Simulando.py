from B_FuncoesFront import *

QUANTIDADE_DE_JOGADORES = 1


def simular() -> None:
    """
    Descrição:
        Função responsável por executar a tela e puxar toda a aplicação.
    """
    pg.init()

    janela, campo = carregando_visual()

    # Vamos criar nossos objetos e iniciar os trabalhos.
    bola = Bola()
    lista_de_jogadores = []
    for indice_do_jogador in range(
            0,
            QUANTIDADE_DE_JOGADORES
    ):
        lista_de_jogadores.append(
            Jogador(
                indice_do_jogador
            )
        )

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

                # Vamos controlar o primeiro jogador.
                if evento.key == pg.K_w:
                    lista_de_jogadores[
                        0
                    ].vel.y += - VEL_DE_IMPULSO_JOGADOR

                if evento.key == pg.K_s:
                    lista_de_jogadores[0].vel.y += VEL_DE_IMPULSO_JOGADOR

                if evento.key == pg.K_a:
                    lista_de_jogadores[0].vel.x += - VEL_DE_IMPULSO_JOGADOR

                if evento.key == pg.K_d:
                    lista_de_jogadores[0].vel.x += VEL_DE_IMPULSO_JOGADOR

                if evento.key == pg.K_k:
                    lista_de_jogadores[0].segurar_bola(bola)

        # Aplicando Física na Bola
        bola.estar_preso_ao_campo()
        bola.arrastar()
        bola.movimentar(INTERV_DE_TEMPO)

        # Atualizações na tela
        janela.blit(
            campo,
            (0, 0)
        )

        janela.blit(
            bola.imagem,
            bola.pos
        )

        for jogador in lista_de_jogadores:
            jogador.estar_preso_ao_campo()
            jogador.arrastar()
            jogador.movimentar(INTERV_DE_TEMPO)

            pg.draw.circle(
                janela,
                jogador.cor,
                jogador.pos,
                TAMANHO_JOGADOR
            )

        pg.time.wait(
            # Milisegundos
            MEDIDOR_DE_FREQUENCIA_DE_FRAMES
        )
        pg.display.flip()

    pg.quit()


if __name__ == '__main__':
    simular()
