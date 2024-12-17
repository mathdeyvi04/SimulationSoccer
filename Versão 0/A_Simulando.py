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
            elif evento.type == pg.KEYDOWN:

                match evento.key:

                    case pg.K_ESCAPE:
                        print(f"\nÚltima posição do mouse registrada: {pg.mouse.get_pos()}")
                        FIM_DE_JOGO = True

                    # Direcionais
                    case pg.K_w:
                        lista_de_jogadores[
                            0
                        ].vel.y += - VEL_DE_IMPULSO_JOGADOR

                    case pg.K_s:
                        lista_de_jogadores[
                            0
                        ].vel.y += VEL_DE_IMPULSO_JOGADOR

                    case pg.K_a:
                        lista_de_jogadores[
                            0
                        ].vel.x += - VEL_DE_IMPULSO_JOGADOR

                    case pg.K_d:
                        lista_de_jogadores[
                            0
                        ].vel.x += VEL_DE_IMPULSO_JOGADOR

                    case pg.K_p:
                        lista_de_jogadores[
                            0
                        ].vel = pg.Vector2(
                            0,
                            0
                        )

                    case _:
                        # Default
                        pass

            # Verificação de Teclas Soltas
            elif evento.type == pg.KEYUP:

                match evento.key:

                    case _:
                        pass

            elif evento.type == pg.MOUSEBUTTONDOWN:
                # 3 É O Esquerdo

                match evento.button:
                    case 3:
                        lista_de_jogadores[
                            0
                        ].segurar_bola(
                            bola
                        )

                    case _:
                        pass

            elif evento.type == pg.MOUSEBUTTONUP:
                match evento.button:

                    case 3:
                        lista_de_jogadores[
                            0
                        ].soltar_bola()

                    case _:
                        pass

        # Aplicando Física na Bola
        bola.fazer_existir()

        # Atualizações na tela
        janela.blit(
            campo,
            (0, 0)
        )

        janela.blit(
            bola.imagem,
            bola.POS_IMAGEM
        )

        for jogador in lista_de_jogadores:
            jogador.fazer_existir(
                bola
            )

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
