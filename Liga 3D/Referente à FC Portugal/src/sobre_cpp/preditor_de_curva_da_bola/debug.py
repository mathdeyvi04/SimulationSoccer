import preditor_de_curva_da_bola

# Caso entre aqui, basta apertar Q, de quit, para sair.
# help(preditor_de_curva_da_bola)

from time import perf_counter


def testando_get_ball_kinematic_predition() -> tuple[list, int]:
    print("=" * 35)
    print("\nTestando Função de Atributos Cinemáticos:")

    inicio = perf_counter()
    total_previsoes = preditor_de_curva_da_bola.get_ball_kinematic_prediction(
        [3, 4],  # Posição Inicial
        [-5, -1] # Velocidade Inicial
    )
    fim = perf_counter()

    index = 0
    index_lim_de_pos = int(len(total_previsoes) / 2.5)
    while index < index_lim_de_pos:
        print(f"index =  {index}")
        print(f"pos   = ({total_previsoes[index]:.3f}, {total_previsoes[index + 1]:.3f})")
        print(f"vel   = ({total_previsoes[index + index_lim_de_pos]:.3f}, {total_previsoes[index + 1 + index_lim_de_pos]:.3f})")
        print(f"|vel| =  {total_previsoes[(index // 2) + 2 * index_lim_de_pos]:.3f}\n")

        index += 2

    print(f"\nTempo de Cálculo: {fim - inicio:.5f}s")
    print("=" * 35)

    return total_previsoes, index_lim_de_pos

    """
    # Segue o gabarito do original do material virgem:
    ===================================

    Testando Função de Atributos Cinemáticos:
    index =  0
    pos   = (3.000, 4.000)
    vel   = (-5.000, -1.000)
    |vel| =  5.099

    index =  2
    pos   = (2.901, 3.980)
    vel   = (-4.895, -0.980)
    |vel| =  4.992

    ...

    index =  308
    pos   = (-1.618, 3.059)
    vel   = (-0.212, -0.044)
    |vel| =  0.217

    index =  310
    pos   = (-1.622, 3.058)
    vel   = (-0.208, -0.043)
    |vel| =  0.213

    ...

    index =  518
    pos   = (-1.803, 3.020)
    vel   = (-0.025, -0.005)
    |vel| =  0.026

    index =  520
    pos   = (-1.803, 3.020)
    vel   = (-0.025, -0.005)
    |vel| =  0.025

    Tempo de Cálculo: 0.056s
    ===================================
    """


previsao_de_atributos_cinematicos, index_lim_de_pos = testando_get_ball_kinematic_predition()


def testando_get_possible_intersection_with_ball(posicoes_bola: list[float]):
    print("=" * 35)

    # Exemplo: Bola se movendo em linha reta na direção do robô.
    pos_x = -1  # Pos x do robo
    pos_y =  1  # Pos y do robo
    max_speed = 0.7 * 0.02  # Max dist percorrida pelo robo em um intervalo de tempo

    inicio = perf_counter()
    ret_x, ret_y, ret_d = preditor_de_curva_da_bola.get_possible_intersection_with_ball(
        # Note que fornecemos um array único de valores, não há array dentro de arrays.
        [pos_x, pos_y],
        max_speed,
        posicoes_bola
    )
    fim = perf_counter()

    # Mostrar os resultados
    print("Testando Função de Ponto de interseção:")
    print("X:", ret_x)
    print("Y:", ret_y)
    print("Distância:", ret_d)
    print(f"\nTempo de Cálculo: {fim - inicio:.5f}s")
    print("=" * 35)

    """
    # Segue o gabarito do original do material virgem:
    # ===================================
    # Testando Função de Ponto de interseção:
    # X: -1.5433185
    # Y: 3.0741436
    # Distância: 2.1441238

    # Tempo de Cálculo: 0.00008s
    # ===================================
    """

testando_get_possible_intersection_with_ball(previsao_de_atributos_cinematicos[:index_lim_de_pos])