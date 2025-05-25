import preditor_de_curva_da_bola

# Caso entre aqui, basta apertar Q, de quit, para sair.
# help(preditor_de_curva_da_bola)

from time import perf_counter
from pprint import pprint

print("=" * 35)

# Exemplo: Bola se movendo em linha reta na direção do robô.
pos_x = 0  # Pos x do robo
pos_y = 0  # Pos y do robo
max_speed = 0.1  # Max dist percorrida pelo robo em um intervalo de tempo
posicoes_bola = [
    2, 0,    # t = 0
    1.1, 0,  # t = 1
    0.2, 0,  # t = 2
]

# Chamada da função
inicio = perf_counter()
ret_x, ret_y, ret_d = preditor_de_curva_da_bola.get_possible_intersection_with_ball(
    # Note que fornecemos um array único de valores, não há array dentro de arrays.
    [pos_x, pos_y, max_speed] + posicoes_bola + [len(posicoes_bola)] 
)
fim = perf_counter()

# Mostrar os resultados
print("Testando Função de Ponto de interseção:")
print("X:", ret_x)
print("Y:", ret_y)
print("Distância:", ret_d)
print(f"\nTempo de Cálculo: {fim - inicio:.3f}s")
print("=" * 35)


print("Testando Função de Atributos Cinemáticos:")

inicio = perf_counter()
total_previsoes = preditor_de_curva_da_bola.get_ball_kinematic_prediction(
    [
        0,  # Pos x inicial 
        0,  # Pos y inicial
        1,  # Vel x inicial
        0   # Vel y inicial
    ]
)
fim = perf_counter()

pprint(total_previsoes)
print(f"\nTempo de Cálculo: {fim - inicio:.3f}s")
print("=" * 35)
