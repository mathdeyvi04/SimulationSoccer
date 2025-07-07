from Agent.BaseAgent import BaseAgent
from cpp.a_estrela import a_estrela
from scripts.commons.Script import Script
import numpy as np
from time import time

"""
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:::: Chamada da função: a_estrela.find_optimal_path(param_vec) :::::::::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Descrição
---------
Executa o algoritmo A* para encontrar o caminho ótimo do ponto de partida até um alvo, 
evitando obstáculos e respeitando as limitações do campo. O caminho pode ser influenciado 
por forças repulsivas e restrições de borda.

Parâmetros
----------
param_vec : numpy.ndarray (dtype=np.float32)
    Vetor de parâmetros com a seguinte estrutura:

    Índices fixos:
    - param_vec[0] : float
        Coordenada X inicial do robô.
    - param_vec[1] : float
        Coordenada Y inicial do robô.
    - param_vec[2] : float
        Permite sair do campo? (1 = sim, 0 = não).
    - param_vec[3] : float
        Caminho vai até o gol adversário? (1 = sim, 0 = usa alvo fornecido).
    - param_vec[4] : float
        Coordenada X do alvo (usado apenas se param_vec[3] == 0).
    - param_vec[5] : float
        Coordenada Y do alvo (usado apenas se param_vec[3] == 0).
    - param_vec[6] : float
        Tempo máximo de execução (em microssegundos).

    Obstáculos (opcional):
    - Cada obstáculo ocupa 5 posições consecutivas:
        [x, y, raio_duro, raio_suave, força_repulsiva]
    - Até n obstáculos podem ser adicionados a partir do índice 7 em diante.

Retorno
-------
path_ret : numpy.ndarray (dtype=np.float32)
    Um vetor unidimensional com as seguintes informações:

    - path_ret[:-2] : list[float]
        Sequência de coordenadas [x1, y1, x2, y2, ..., xn, yn] representando o caminho.
        Até 1024 posições (2048 floats).
        O caminho pode não terminar no alvo por motivos como:
            - Caminho ultrapassa o número máximo de pontos.
            - Alvo inacessível ou tempo esgotado.

    - path_ret[-2] : int
        Status da busca:
        0 - Sucesso (caminho até o alvo encontrado).
        1 - Timeout (tempo esgotado antes de alcançar o alvo).
        2 - Impossível (todas opções testadas, alvo inalcançável).
        3 - Sem obstáculos (caminho direto do início ao alvo).

    - path_ret[-1] : float
        Custo total do caminho A* (soma dos custos de cada passo + forças repulsivas).

Notas Adicionais
----------------
Mapa do Campo:
    - Área total: 32m x 22m (campo de jogo + 1m de borda).
    - Precisão: 10cm por célula.
    - Gol, postes e rede são obstáculos fixos e sempre intransponíveis.
    - Caminho pode sair do campo apenas se permitido (param_vec[2] = 1).
    - Apenas obstáculos dinâmicos são fornecidos via argumentos.

Força Repulsiva:
    - Define custo adicional ao caminhar perto de obstáculos (custo adicional somado ao custo base de movimentação).
    - Custo padrão de andar 10cm = 1, diagonal = √2.
    - Se a força f = 1, o custo extra é 1.
    - Forças de múltiplos obstáculos não se somam: é usada a maior (max(f1, f2)).
    - Caminhar em posição inacessível inicial é permitido, mas o custo é alto (100) para evitar abuso.

Exemplo de Caminhos com Força Repulsiva:
    Exemplo simples:
        Map 1   Map 2     Map 3
        ..x..   ..o..     ..o..
        ..1..   ..o..     .o1..
        ..o..   ..o..     ..o..

        Caminho 2: custo = 1 + (força=1) + 1 = 3
        Caminho 3: custo = √2 + √2 = 2.83  ✅ ótimo

    Exemplo com várias forças:
         Map 1     Map 2     Map 3     Map 4
        ...x...   ..oo...   ...o...   ...o...
        ..123..   .o123..   ..o23..   ..1o3..
        ...o...   ..oo...   ...o...   ...o...

        Caminho 2: evita todas forças, custo ≈ 4.83
        Caminho 3: atravessa força 1, custo ≈ 3.83 ✅ ótimo
        Caminho 4: atravessa força 2, custo = 4.0

Obstáculos:
    - **Raio duro**: zona inacessível absoluta.
    - **Raio suave**: zona acessível com custo adicional proporcional à distância.
    - Se `raio_suave <= raio_duro`, força repulsiva é ignorada.

    Exemplos:
    - `obstacle(0, 0, 1, 3, 5)`:
        - Até 1m: inacessível.
        - De 1m a 3m: força repulsiva decresce de 5 a 0.
    - `obstacle(-2.1, 3, 0, 0, 0)`:
        - Posição inacessível pontual.
    - `obstacle(-2.16, 3, 0, 0, 8)`:
        - Obstáculo localizado na célula mais próxima (resolução 10cm).
        - Força repulsiva ignorada (raios iguais).
"""

class PathFinding:
    def __init__(self, script: Script) -> None:
        self.player = None
        self.script = script
        a_estrela.find_optimal_path(np.zeros(6, np.float32))  # Initialize (not needed, but the first run takes a bit more time)

    def draw_grid(self):
        d = self.player.world.draw
        MAX_RAW_COST = 0.6  # dribble cushion

        for x in np.arange(-16, 16.01, 0.1):
            for y in np.arange(-11, 11.01, 0.1):
                s_in, cost_in = a_estrela.find_optimal_path(np.array([x, y, 0, 0, x, y, 5000], np.float32))[-2:]  # do not allow out of bounds
                s_out, cost_out = a_estrela.find_optimal_path(np.array([x, y, 1, 0, x, y, 5000], np.float32))[-2:]  # allow out of bounds
                # print(path_cost_in, path_cost_out)
                if s_out != 3:
                    d.point((x, y), 5, d.Color.red, "grid", False)
                elif s_in != 3:
                    d.point((x, y), 4, d.Color.blue_pale, "grid", False)
                elif 0 < cost_in < MAX_RAW_COST + 1e-6:
                    d.point((x, y), 4, d.Color.get(255, (1 - cost_in / MAX_RAW_COST) * 255, 0), "grid", False)
                elif cost_in > MAX_RAW_COST:
                    d.point((x, y), 4, d.Color.black, "grid", False)
                # else:
                #    d.point((x,y), 4, d.Color.white, "grid", False)
        d.flush("grid")

    def sync(self):
        r = self.player.world.robot
        self.player.behavior.head.execute()
        self.player.scom.commit_and_send(r.get_command())
        self.player.scom.receive()

    def draw_path_and_obstacles(self, obst, path_ret_pb, path_ret_bp):
        w = self.player.world

        # draw obstacles
        for i in range(0, len(obst[0]), 5):
            w.draw.circle(obst[0][i:i + 2], obst[0][i + 2], 2, w.draw.Color.red, "obstacles", False)
            w.draw.circle(obst[0][i:i + 2], obst[0][i + 3], 2, w.draw.Color.orange, "obstacles", False)

        # draw path
        path_pb = path_ret_pb[:-2]  # create view without status
        path_status_pb = path_ret_pb[-2]  # extract status
        path_cost_pb = path_ret_pb[-1]  # extract A* cost
        path_bp = path_ret_bp[:-2]  # create view without status
        path_status_bp = path_ret_bp[-2]  # extract status
        path_cost_bp = path_ret_bp[-1]  # extract A* cost

        c_pb = {0: w.draw.Color.green_lime, 1: w.draw.Color.yellow, 2: w.draw.Color.red, 3: w.draw.Color.blue_light}[path_status_pb]
        c_bp = {0: w.draw.Color.green_pale, 1: w.draw.Color.yellow_light, 2: w.draw.Color.red_salmon, 3: w.draw.Color.blue_pale}[path_status_bp]

        for i in range(2, len(path_pb) - 2, 2):
            w.draw.line(path_pb[i - 2:i], path_pb[i:i + 2], 5, c_pb, "path_player_ball", False)

        if len(path_pb) >= 4:
            w.draw.arrow(path_pb[-4:-2], path_pb[-2:], 0.4, 5, c_pb, "path_player_ball", False)

        for i in range(2, len(path_bp) - 2, 2):
            w.draw.line(path_bp[i - 2:i], path_bp[i:i + 2], 5, c_bp, "path_ball_player", False)

        if len(path_bp) >= 4:
            w.draw.arrow(path_bp[-4:-2], path_bp[-2:], 0.4, 5, c_bp, "path_ball_player", False)

        w.draw.flush("obstacles")
        w.draw.flush("path_player_ball")
        w.draw.flush("path_ball_player")

    def move_obstacles(self, obst):

        for i in range(len(obst[0]) // 5):
            obst[0][i * 5] += obst[1][i, 0]
            obst[0][i * 5 + 1] += obst[1][i, 1]
            if not -16 < obst[0][i * 5] < 16:
                obst[1][i, 0] *= -1
            if not -11 < obst[0][i * 5 + 1] < 11:
                obst[1][i, 1] *= -1

    def execute(self):

        a = self.script.args
        self.player = BaseAgent(a.i, a.p, a.m, a.u, a.r, a.t)  # Args: Server IP, Agent Port, Monitor Port, Uniform No., Robot Type, Team Name
        w = self.player.world
        r = self.player.world.robot
        timeout = 5000

        go_to_goal = 0
        obst_no = 50
        obst = [[0, 0, 0.5, 1, 1] * obst_no, np.random.uniform(-0.01, 0.01, (obst_no, 2))]  # obst[x,y,h,s,f] + random velocity

        print("\nMove player/ball around using RoboViz!")
        print("Press ctrl+c to return.")
        print("\nPathfinding timeout set to", timeout, "us.")
        print("Pathfinding execution time:")

        self.draw_grid()

        while True:
            ball = w.ball_abs_pos[:2]
            rpos = r.loc_head_position[:2]

            self.move_obstacles(obst)

            param_vec_pb = np.array([*rpos, 1, go_to_goal, *ball, timeout, *obst[0]], np.float32)  # allow out of bounds (player->ball)
            param_vec_bp = np.array([*ball, 0, go_to_goal, *rpos, timeout, *obst[0]], np.float32)  # don't allow (ball->player)
            t1 = time()
            path_ret_pb = a_estrela.find_optimal_path(param_vec_pb)
            t2 = time()
            path_ret_bp = a_estrela.find_optimal_path(param_vec_bp)
            t3 = time()

            print(end=f"\rplayer->ball {int((t2 - t1) * 1000000):5}us (len:{len(path_ret_pb[:-2]) // 2:4})      ball->player {int((t3 - t2) * 1000000):5}us  (len:{len(path_ret_bp[:-2]) // 2:4}) ")

            self.draw_path_and_obstacles(obst, path_ret_pb, path_ret_bp)
            self.sync()

























