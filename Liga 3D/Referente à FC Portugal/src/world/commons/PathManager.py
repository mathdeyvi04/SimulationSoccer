from sobre_cpp.a_estrela import a_estrela  # Finalmente usamos a nossa função!.
from math_ops.GeneralMath import GeneralMath
from world.World import World
import math
import numpy as np


class PathManager:
    """
    A classe PathManager é responsável pelo gerenciamento e pelo cálculo de caminhos para um robô
    se mover pelo campo, evitando obstáculos, considerando diferentes estratégias de locomoção.
    Ele controla tanto o modo de desvio quanto o status do caminho encontrado, aumentando ou
    diminuindo margens de segurança conforme o contexto.

    O path é gerada a fim de mover o robô de um ponto de saída até um ponto de chegada de forma
    suave, evitando outros robôs, a bola ou quaisquer outros obstáculos presentes no campo de jogo.

    A classe também define diferentes configurações que influenciam o path planning, como o modo
    (cauteloso, agressivo ou drible) e a situação final do caminho (concluído, atingiu o timeout,
    é impossível ou é um caminho direto).

    Métodos Disponíveis:
        - get_obstacles
        - get_path_to_ball
        - get_path_to_target
        - get_dribble_path
        -

    Variáveis de Classe:
        - MODE_CAUTIOUS (int): modo cauteloso; margens de segurança maiores.
        - MODE_DRIBBLE (int): modo de drible; margens de segurança ainda maiores.
        - MODE_AGGRESSIVE (int): modo agressivo; margens de segurança reduzidas para se mover mais facilmente.
        - STATUS_SUCCESS (int): o algoritmo chegou ao destino com sucesso.
        - STATUS_TIMEOUT (int): o algoritmo excedeu o tempo enquanto ainda procurava um caminho.
        - STATUS_IMPOSSIBLE (int): é impossível encontrar um caminho para o destino.
        - STATUS_DIRECT (int): o caminho é uma linha reta, ou seja, o robô consegue ir diretamente até o destino.
        - HOT_START_DIST_WALK (float): a distância usada como ponto de saída na hora de planejar o caminho enquanto se mover caminhando.
        - HOT_START_DIST_DRIBBLE (float): a distância usada como ponto de saída na hora de planejar o caminho enquanto se mover driblando a bola.
    """
    ########################
    # Modos de path planning
    MODE_CAUTIOUS = 0  # modo cauteloso (margens de segurança maior)
    MODE_DRIBBLE = 1  # modo de drible (margens de segurança ainda maior)
    MODE_AGGRESSIVE = 2  # modo agressivo (margens de segurança menor)

    ###################################
    # Códigos de saída do path planning
    STATUS_SUCCESS = 0  # o algoritmo chegou com sucesso ao destino
    STATUS_TIMEOUT = 1  # o algoritmo excedeu o tempo enquanto ainda procurava um caminho
    STATUS_IMPOSSIBLE = 2  # é impossível encontrar um caminho para o destino
    STATUS_DIRECT = 3  # o caminho é uma linha reta até o destino (não houve a presença de quaisquer obstáculos)

    #####################################################################
    # Distância usada para o ponto de saída na hora de planejar o caminho
    HOT_START_DIST_WALK = 0.05  # saída a uma distância de 0.05 m, enquanto se mover caminhando
    HOT_START_DIST_DRIBBLE = 0.10  # saída a uma distância de 0.10 m, enquanto se mover driblando

    def __init__(self, world: World) -> None:
        """
        Descrição:
            Inicializa o gerenciador de caminhos com uma referência para o mundo atual.

        Parâmetros:
            world (World):
                instância que representa o estado atual do mundo,
                incluindo a posição de robôs, a bola e quaisquer
                outros obstáculos presentes.
        """
        self.world = world

        # Opções de desenho para debug (ativadas pelo método 'draw_options')
        self._draw_obstacles = False  # exibe ou não os obstáculos presentes no caminho
        self._draw_path = False  # exibe ou não o caminho gerado pelo path planning
        self._use_team_channel = False  # controla se serão utilizadas mensagens do time na hora de desenhar ou compartilhar o caminho

        # Variáveis internas utilizadas para dar um ponto de saída para o path planning
        # a fim de deixá-lo mais estável nas atualizações.
        self.last_direction_rad = None  # última direção (em radianos) usada como referência
        self.last_update = 0  # timestamp da última atualização do path planning
        self.last_start_dist = None  # última distância usada como ponto de saída na hora de gerar o caminho

    def draw_options(self, enable_obstacles: bool, enable_path: bool, use_team_drawing_channel: bool = False) -> None:
        """
        Descrição:
            Ativa ou desativa o desenho de caminhos e obstáculos, assim como controla o canal de desenho utilizado.

            Se `self.world.draw.enable` for False, essas configurações serão ignoradas.

        Parâmetros:
            enable_obstacles (bool):
                se True, serão desenhados os obstáculos relevantes para o path planning.
            enable_path (bool):
                se True, será desenhado o caminho computado pelo path planning.
            use_team_drawing_channel (bool, opcional):
                se True, o desenho será realizado pelo canal da equipe;
                caso False, será utilizado um canal específico para cada jogador. Quando o canal é compartilhado,
                um desenho com o mesmo nome sobrescreve o anterior, enquanto nos canais específicos ele é acrescentado.
        """
        self._draw_obstacles = enable_obstacles
        self._draw_path = enable_path
        self._use_team_channel = use_team_drawing_channel

    def get_obstacles(
            self,
            include_teammates: bool,
            include_opponents: bool,
            include_play_mode_restrictions: bool,
            max_distance: float = 4,
            max_age: float = 500,
            ball_safety_margin: float = 0,
            goalpost_safety_margin: float = 0,
            mode: int = MODE_CAUTIOUS,
            priority_unums: list = None
    ) -> list:
        """
        Descrição:
            Retorna uma lista de obstáculos para serem considerados pelo path planning.

            Os obstáculos representam posições que o robô deve evitar ou respeitar enquanto se locomove pelo campo.

        Parâmetros:
            include_teammates (bool) :
                se True, serão incluídos como obstáculos os companheiros de equipe nas proximidades.
            include_opponents (bool) :
                se True, serão incluídos como obstáculos os oponentes que estão nas proximidades.
            include_play_mode_restrictions (bool) :
                se True, serão incluídas restrições especiais dependendo do modo de jogo vigente.
            max_distance (float) :
                a distância máxima (em metros) para considerar um jogador como obstáculo. Padrão = 4.
            max_age (float) :
                o tempo máximo (em milissegundos) desde a última atualização para considerar um jogador. Padrão = 500.
            ball_safety_margin (float) :
                raio de segurança ao redor da bola, aumentando a área de desvio. Padrão = 0.
            goalpost_safety_margin (float) :
                raio de desvio nas traves do gol adversário. Padrão = 0.
            mode (int) :
                atitude do robô quanto às margens de segurança (cauteloso, agressivo ou drible). Padrão = MODE_CAUTIOUS.
            priority_unums (list) :
                lista de números de uniforme dos companheiros que serão evitados com maior ênfase. Padrão = [].

        Retorna:
            obstacles (list) :
                uma lista de tuplas, onde cada tupla representa um obstáculo:
                (x, y, raio_hard, raio_soft, força_repulsiva).
        """

        if priority_unums is None:
            priority_unums = []

        w = self.world

        ball_2d = w.ball_abs_pos[:2]
        obstacles = []

        # 'comparator' is a variable local to the lambda, which captures the current value of (w.time_local_ms - max_age)
        check_age = lambda last_update, comparator=w.time_local_ms - max_age: last_update > 0 and last_update >= comparator

        # Get recently seen close teammates
        if include_teammates:
            soft_radius = 1.1 if mode == PathManager.MODE_DRIBBLE else 0.6  # soft radius: repulsive force is max at center and fades

            def get_hard_radius(t):
                if t.unum in priority_unums:
                    return 1.0  # extra distance for priority roles
                else:
                    return t.state_ground_area[1] + 0.2

            # Get close teammates (center, hard radius, soft radius, force)
            obstacles.extend((*t.state_ground_area[0],
                              get_hard_radius(t),
                              1.5 if t.unum in priority_unums else soft_radius,
                              1.0)  # repulsive force
                             for t in w.teammates if not t.is_self and check_age(t.state_last_update) and t.state_horizontal_dist < max_distance)

        # Get recently seen close opponents
        if include_opponents:

            # É impressionante relembrar como usamos esses dados dentro do /sobre_cpp/a_estrela
            # soft radius: repulsive force is max at center and fades
            if mode == PathManager.MODE_AGGRESSIVE:
                soft_radius = 0.6
                hard_radius = lambda o: 0.2
            elif mode == PathManager.MODE_DRIBBLE:
                soft_radius = 2.3
                hard_radius = lambda o: o.state_ground_area[1] + 0.9
            else:
                soft_radius = 1.0
                hard_radius = lambda o: o.state_ground_area[1] + 0.2

            # Get close opponents (center, hard radius, soft radius, force)
            obstacles.extend((*o.state_ground_area[0],
                              hard_radius(o),
                              soft_radius,
                              1.5 if o.unum == 1 else 1.0)  # repulsive force (extra for their GK)
                             for o in w.opponents if o.state_last_update > 0 and w.time_local_ms - o.state_last_update <= max_age and o.state_horizontal_dist < max_distance)

        # ---------------------------------------------- Get play mode restrictions
        if include_play_mode_restrictions:
            if w.play_mode == World.M_THEIR_GOAL_KICK:
                obstacles.extend((15, i, 2.1, 0, 0) for i in range(-2, 3))  # 5 circular obstacles to cover their goal area
            elif w.play_mode == World.M_THEIR_PASS:
                obstacles.append((*ball_2d, 1.2, 0, 0))
            elif w.play_mode in [World.M_THEIR_KICK_IN, World.M_THEIR_CORNER_KICK, World.M_THEIR_FREE_KICK, World.M_THEIR_DIR_FREE_KICK, World.M_THEIR_OFFSIDE]:
                obstacles.append((*ball_2d, 2.5, 0, 0))

        # ---------------------------------------------- Get ball
        if ball_safety_margin > 0:

            # increase ball safety margin in certain game scenarios
            if (w.play_mode_group != w.MG_OTHER) or abs(ball_2d[1]) > 9.5 or abs(ball_2d[0]) > 14.5:
                ball_safety_margin += 0.12

            obstacles.append((*ball_2d, 0, ball_safety_margin, 8))

        # ---------------------------------------------- Get goal posts
        if goalpost_safety_margin > 0:
            obstacles.append((14.75, 1.10, goalpost_safety_margin, 0, 0))
            obstacles.append((14.75, -1.10, goalpost_safety_margin, 0, 0))

        # ---------------------------------------------- Draw obstacles
        if self._draw_obstacles:
            d = w.team_draw if self._use_team_channel else w.draw
            if d.enabled:
                for o in obstacles:
                    if o[3] > 0:
                        d.circle(o[:2], o[3], o[4] / 2, d.Color.orange, "path_obstacles", False)
                    if o[2] > 0:
                        d.circle(o[:2], o[2], 1, d.Color.red, "path_obstacles", False)
                d.flush("path_obstacles")

        return obstacles

    def _get_hot_start(self, start_distance: float) -> np.ndarray:
        """
        Descrição:
            Retorna um ponto de partida "hot start" para o path planning,
            considerando o caminho calculado anteriormente.

            Isso evita instabilidades na nova trajetória, aumentando a continuidade do caminho.
            Se as condições para o "hot start" não forem atendidas, é utilizado um "cold start"
            a partir da posição atual do robô.

        Parâmetros:
            start_distance (float) : a distância na qual o "hot start" deve ser iniciado a frente do robô.

        Retorna:
            numpy.ndarray:
                um array 2D (x, y) representando o ponto de partida para o path planning.

            Condições para o hot start:
            - o path foi atualizado exatamente 20ms atrás;
            - o start_distance é o mesma que o último utilizado;
            - é possível extrapolar na mesma direção do caminho vigente.

            Se quaisquer uma das condições não for atendida, o ponto de partida será a posição atual do robô.
        """

        if self.last_update > 0 and (self.world.time_local_ms - self.last_update) == 20 and self.last_start_dist == start_distance:
            return self.world.robot.loc_head_position[:2] + GeneralMath.unit_vector_by_angle(self.last_direction_rad, is_rad=True) * start_distance
        else:
            # return cold start if start_distance was different or the position was not updated in the last step
            return self.world.robot.loc_head_position[:2]

    def _update_hot_start(self, next_dir_rad: float, start_distance: float) -> None:
        """
        Descrição:
            Atualiza as condições para o "hot start" na próxima execução do path planning.

            Isso evita instabilidades na nova trajetória, aumentando a continuidade do caminho.

        Parâmetros:
            next_dir_rad (float) : nova direção, em radianos, para o "hot start".
            start_distance (float) : nova distância na qual o "hot start" deve ser iniciado.

        Retorno:
            - Atualiza o atributo `last_direction_rad`.
            - Registra o timestamp atual em `last_update`.
            - Armazena a nova distância de início em `last_start_dist`.
        """
        self.last_direction_rad = next_dir_rad
        self.last_update = self.world.time_local_ms
        self.last_start_dist = start_distance

    @staticmethod
    def _extract_target_from_path(path: list | np.ndarray, path_len: int, ret_segments: float) -> list | np.ndarray:
        """
        Descrição
            Extrai um ponto-alvo específico do caminho, dada a quantidade de segmentos desejados.

            Se o caminho possuir pelo menos o número de segmentos especificado, serão retornadas
            as últimas dois posições que representam o ponto-alvo ou uma interpolação
            entre dois segmentos. Caso o caminho seja menor que o número de segmentos, serão
            retornados as duas últimas posições do caminho.

        Parâmetros:
            path (list ou ndarray) :
                sequência de pontos que representam o caminho.
            path_len (int) :
                número de segmentos presentes no caminho.
            ret_segments (float) :
                posição desejada junto ao caminho, podendo ser fracionária.
                Um número inteiro significa exatamente um ponto de segmento,
                enquanto um número fracionário faz uma interpolação linear
                entre dois segmentos adjacentes.

        Retorna:
            ndarray:
                um array 2D (2, N) com as posições (x, y) do ponto extraído do caminho.
        """
        ret_seg_ceil = math.ceil(ret_segments)

        if path_len >= ret_seg_ceil:
            i = ret_seg_ceil * 2  # path index of ceil point (x)
            if ret_seg_ceil == ret_segments:
                return path[i:i + 2]
            else:
                floor_w = ret_seg_ceil - ret_segments
                return path[i - 2:i] * floor_w + path[i:i + 2] * (1 - floor_w)
        else:
            return path[-2:]  # path end

    def get_path_to_ball(
            self,
            x_ori=None,
            x_dev=-0.2,
            y_dev=0,
            torso_ori=None,
            torso_ori_thrsh=1,
            priority_unums=None,
            is_aggressive=True,
            safety_margin=0.25,
            timeout=3000
    ) -> tuple[np.ndarray, float, float]:
        """
        Descrição:
            Obtém o próximo ponto-alvo no caminho até a bola (próxima posição absoluta + orientação).

            Se o robô é um jogador ativo e está próximo da bola, é interessante que ele vá de forma agressiva.
            Se ele estiver distante, é melhor que ele vá à posição de seu papel (role) para se preparar para uma eventual interação com a bola.

        Parâmetros:
            x_ori : float
                (Esta variável específica a posição-alvo relativa à bola, em um referencial customizado).
                Orientação absoluta do eixo x do referencial customizado.
                Se None, a orientação é dada pelo vetor (robô -> bola).
            x_dev : float
                (Esta variável específica o desvio da posição-alvo, em relação à bola, neste referencial customizado).
                Desvio na posição no eixo x do referencial customizado.
            y_dev : float
                (Esta variável específica o desvio da posição-alvo, em relação à bola, neste referencial customizado).
                Desvio na posição no eixo y do referencial customizado.
            torso_ori : float
                Orientação absoluta do torso que o robô deve adotar (veja `torso_ori_thrsh`).
                Se None, o robô se orientará automaticamente para o ponto-alvo final.
            torso_ori_thrsh : float
                Apenas se a distância até o ponto-alvo for menor que `torso_ori_thrsh` (em metros) o robô será orientado para `torso_ori`.
                Do contrário, ele permanecerá orientado para o ponto-alvo.
            priority_unums : list
                Lista de números que identificam os companheiros de equipe que serão evitados, já que eles estão envolvidos em outras atividades.
            is_aggressive : bool
                Se True, serão usados margens de segurança menor para os adversários.
            safety_margin : float
                Raio de repulsão ao redor da bola para evitar uma colisão.
            timeout : float
                Tempo máximo de execução (em microssegundos).

        Retorna:
            next_pos : ndarray
                A próxima posição absoluta no caminho até a bola.
            next_ori : float
                A próxima orientação absoluta.
            distance : float
                A menor das seguintes distâncias:
                (distância até o ponto-alvo final) e (distância até a bola)

        Exemplo:

        -----------------------------------------------------
        x_ori        |  x_dev  |  y_dev  |  torso_ori  |  OBS
        ------------+---------+---------+---------+----------
        None =>      |    -    |   !0    |      -      |  Não é recomendado. Não haverá convergence.
        (orient. of: |    0    |    0    |     None    |  Busca frontal da bola, aproximação lenta*
        robot->ball) |    0    |    0    |    valor    |  Busca frontal da bola, com orientação forçada*
                     |   >0    |    0    |      -      |  Não é recomendado. Não haverá convergence.
                     |   <0    |    0    |     None    |  Busca frontal da bola até que a distância == x_dev
                     |   <0    |    0    |    valor    |  Busca da bola, orientando o robô até que a distância == x_dev
        ------------+---------+---------+---------+---------
        valor        |    -    |    -    |     None    |  Busca de um ponto específico
                     |    -    |    -    |    valor    |  Busca de um ponto específico, com orientação forçada
        ----------------------------------------------------------------------------------------------
        * Isso depende da função que faz essa mesma chamado (espera-se uma aproximação lenta junto ao ponto).
        `torso_ori` será utilizado apenas se a distância até o ponto final for menor que `torso_ori_thrsh`.
        """

        if priority_unums is None:
            priority_unums = []

        w = self.world
        r = w.robot
        dev = np.array([x_dev, y_dev])  # desvio
        dev_len = np.linalg.norm(dev)  # comprimento do desvio
        dev_mult = 1

        # usar a predição da bola se estamos a mais de 0.5 m e em PlayOn
        if np.linalg.norm(w.ball_abs_pos[:2] - r.loc_head_position[:2]) > 0.5 and w.play_mode_group == w.MG_OTHER:
            # ponto de interseção, enquanto se move a 0.4 m/s
            ball_2d = w.get_intersection_point_with_ball(0.4)[0]
        else:
            ball_2d = w.ball_abs_pos[:2]

        # orientação do referencial personalizado
        vec_me_ball = ball_2d - r.loc_head_position[:2]
        if x_ori is None:
            x_ori = GeneralMath.angle_horizontal_from_vector2D(vec_me_ball)

        distance_boost = 0  # reforço para a distância até o alvo
        if torso_ori is not None and dev_len > 0:
            approach_ori_diff = abs(GeneralMath.normalize_deg(r.imu_torso_orientation - torso_ori))
            if approach_ori_diff > 15:  # aumentar velocidade de caminhada se o robô estiver distante da orientação de aproximação
                distance_boost = 0.15
            if approach_ori_diff > 30:  # aumentando a distância do alvo até a bola se o robô estiver distante da orientação de aproximação
                dev_mult = 1.3
            if approach_ori_diff > 45:  # aumentando a margem de segurança ao redor da bola se o robô estiver distante da orientação de aproximação
                safety_margin = max(0.32, safety_margin)

        ####################
        # obter o alvo
        ####################

        front_unit_vec = GeneralMath.unit_vector_by_angle(x_ori)
        left_unit_vec = np.array([-front_unit_vec[1], front_unit_vec[0]])

        rel_target = front_unit_vec * dev[0] + left_unit_vec * dev[1]
        target = ball_2d + rel_target * dev_mult
        target_vec = target - r.loc_head_position[:2]
        target_dist = np.linalg.norm(target_vec)

        if self._draw_path:
            d = self.world.team_draw if self._use_team_channel else self.world.draw
            d.point(target, 4, d.Color.red, "path_target")  # não será desenhado se o desenho estiver desativado

        ####################
        # obter obstáculos
        ####################

        # Ignorar a bola se estamos do mesmo lado que o alvo (com uma pequena margem)
        if dev_len > 0 and np.dot(vec_me_ball, rel_target) < -0.10:
            safety_margin = 0

        obstacles = self.get_obstacles(
            include_teammates=True, include_opponents=True, include_play_mode_restrictions=True,
            ball_safety_margin=safety_margin,
            mode=PathManager.MODE_AGGRESSIVE if is_aggressive else PathManager.MODE_CAUTIOUS,
            priority_unums=priority_unums
        )

        # Adicionar um obstáculo no lado oposto ao do alvo
        if dev_len > 0 and safety_margin > 0:
            center = ball_2d - GeneralMath.normalize_vec(rel_target) * safety_margin
            obstacles.append((*center, 0, safety_margin * 0.9, 5))
            if self._draw_obstacles:
                d = w.team_draw if self._use_team_channel else w.draw
                if d.enabled:
                    d.circle(center, safety_margin * 0.8, 2.5, d.Color.orange, "path_obstacles_1")

        ####################
        # obter o caminho
        ####################

        # veja a explicação para o contexto na seção de atualização do hot start
        start_pos = self._get_hot_start(PathManager.HOT_START_DIST_WALK) if target_dist > 0.4 else self.world.robot.loc_head_position[:2]

        path, path_len, path_status, path_cost = self.get_path(start_pos, True, obstacles, target, timeout)
        path_end = path[-2:]  # última posição permitida pelo A*

        ####################
        # obter distâncias relevantes
        ####################

        if w.ball_last_seen > w.time_local_ms - w.VISUALSTEP_MS:  # a bola é visível
            raw_ball_dist = np.linalg.norm(w.ball_rel_torso_cart_pos[:2])  # - distância entre o centro do torso e o centro da bola
        else:  # caso tenha sido perdida de visão, usar as coordenadas absolutas
            raw_ball_dist = np.linalg.norm(vec_me_ball)  # - distância entre o centro da cabeça e a bola

        avoid_touching_ball = (w.play_mode_group != w.MG_OTHER)
        distance_to_final_target = np.linalg.norm(path_end - r.loc_head_position[:2])  # distância até o destino final
        distance_to_ball = max(0.07 if avoid_touching_ball else 0.14, raw_ball_dist - 0.13)
        caution_dist = min(distance_to_ball, distance_to_final_target)

        ####################
        #  obter a próxima posição do alvo
        ####################

        next_pos = self._extract_target_from_path(path, path_len, ret_segments=1 if caution_dist < 1 else 2)

        ####################
        #  obter a próxima orientação do alvo
        ####################

        # usar a orientação dada se ela existe; se não, a orientação do alvo se ele estiver distante; ou a atual
        if torso_ori is not None:
            if caution_dist > torso_ori_thrsh:
                next_ori = GeneralMath.angle_horizontal_from_vector2D(target_vec)
            else:
                mid_ori = GeneralMath.normalize_deg(GeneralMath.angle_horizontal_from_vector2D(vec_me_ball) - GeneralMath.angle_horizontal_from_vector2D(-dev) - x_ori + torso_ori)
                mid_ori_diff = abs(GeneralMath.normalize_deg(mid_ori - r.imu_torso_orientation))
                final_ori_diff = abs(GeneralMath.normalize_deg(torso_ori - r.imu_torso_orientation))
                next_ori = mid_ori if mid_ori_diff + 10 < final_ori_diff else torso_ori
        elif target_dist > 0.1:
            next_ori = GeneralMath.angle_horizontal_from_vector2D(target_vec)
        else:
            next_ori = r.imu_torso_orientation

        ####################
        #  atualizar o hot start para a próxima execução
        ####################

        ''' Definindo a distância do hot start:
        - se o path_len é zero, não existe um hot start, pois já estamos no destino (dist=0)
        - se o destino é próximo, o hot start não é aplicado (veja acima)
        - se a nova posição é muito próxima (devido a um obstáculo), o hot start é a nova posição (dist < PathManager.HOT_START_DIST_WALK)
        - caso contrário, o hot start é uma distância constante (dist = PathManager.HOT_START_DIST_WALK)
        '''
        if path_len != 0:
            next_pos_vec = next_pos - self.world.robot.loc_head_position[:2]
            next_pos_dist = np.linalg.norm(next_pos_vec)
            self._update_hot_start(GeneralMath.angle_horizontal_from_vector2D(next_pos_vec, is_rad=True), min(PathManager.HOT_START_DIST_WALK, next_pos_dist))

        return next_pos, next_ori, min(distance_to_ball, distance_to_final_target + distance_boost)

    def get_path_to_target(
            self,
            target,
            ret_segments=1.0,
            torso_ori=None,
            priority_unums: list = None,
            is_aggressive=True,
            timeout=3000
    ) -> tuple[np.ndarray, float, float]:
        """
        Descrição:
            Calcula o próximo ponto de interesse junto ao caminho até o alvo (próxima posição e orientação).

        Parâmetros:
            target: ndarray
                Posição (x, y) do ponto-alvo.
            ret_segments : float
                Distância máxima a ser percorrida junto ao caminho para determinar o próximo ponto.
                A distância é limitada pelo menor entre ret_segments e o comprimento total do caminho.
                Cada segmento de caminho mede 0.1 m ou 0.1*sqrt(2) m (caso seja na diagonal).
                Se ret_segments == 0, é retornada a posição atual.
            torso_ori : float, opcional
                Orientação final desejada para o torso. Se None, é usada a mesma orientação que o vetor (robô -> ponto).
            priority_unums : list, opcional
                Lista de teammates que serão evitados, sendo eles de maior prioridade.
            is_aggressive : bool, opcional
                Se True, serão utilizadas margens de segurança menor para os oponentes.
            timeout : float, opcional
                Tempo máximo de execução (em microsegundos) para o path planning.

        Retorno:
            next_pos : ndarray
                A posição (x, y) do próximo ponto junto ao caminho.
            next_ori : float
                A orientação para o robô se mover em relação a essa posição.
            distance_to_final_target : float
                Distância restante até o ponto final do caminho.
        """

        if priority_unums is None:
            priority_unums = []

        w = self.world

        ##################
        # obter o ponto-alvo
        ##################

        target_vec = target - w.robot.loc_head_position[:2]
        target_dist = np.linalg.norm(target_vec)

        ##################
        # obter os obstáculos
        ##################

        obstacles = self.get_obstacles(include_teammates=True, include_opponents=True, include_play_mode_restrictions=True,
                                       mode=PathManager.MODE_AGGRESSIVE if is_aggressive else PathManager.MODE_CAUTIOUS,
                                       priority_unums=priority_unums)

        ##################
        # obter o caminho
        ##################

        # Veja a explicação na seção de atualização do hot start
        start_pos = self._get_hot_start(PathManager.HOT_START_DIST_WALK) if target_dist > 0.4 else self.world.robot.loc_head_position[:2]

        path, path_len, path_status, path_cost = self.get_path(start_pos, True, obstacles, target, timeout)
        path_end = path[-2:]  # última posição permitida pelo A*

        #########################################
        # ima posição junto ao caminho
        ##################

        next_pos = self._extract_target_from_path(path, path_len, ret_segments)

        ##################
        # obter a próxima orientação do robô
        ##################

        # usar a orientação dada se ela existe; se não, usar a do ponto se ele estiver distante; ou a atual
        if torso_ori is not None:
            next_ori = torso_ori
        elif target_dist > 0.1:
            next_ori = GeneralMath.angle_horizontal_from_vector2D(target_vec)
        else:
            next_ori = w.robot.imu_torso_orientation

        ##################
        # atualizar o hot start para a próxima execução
        ##################

        ''' Definição da distância do hot start:
        - se o path_len é zero, não é preciso aplicar o hot start, pois já estamos no destino
        - se o destino é próximo, o hot start não é atualizado (veja acima)
        - se o próximo ponto é muito próximo (devido a um obstáculo), o hot start passa a ser o próximo ponto
        - caso contrário, o hot start é uma distância constante
        '''
        if path_len != 0:
            next_pos_vec = next_pos - self.world.robot.loc_head_position[:2]
            next_pos_dist = np.linalg.norm(next_pos_vec)
            self._update_hot_start(GeneralMath.angle_horizontal_from_vector2D(next_pos_vec, is_rad=True), min(PathManager.HOT_START_DIST_WALK, next_pos_dist))

        distance_to_final_target = np.linalg.norm(path_end - w.robot.loc_head_position[:2])

        return next_pos, next_ori, distance_to_final_target

    def get_dribble_path(
            self,
            ret_segments=None,
            optional_2d_target=None,
            goalpost_safety_margin=0.4,
            timeout=3000
    ) -> tuple[np.ndarray, float]:
        """
        Descrição:
            Calcula o próximo ponto do caminho para o robô enquanto ele dribla a bola, junto com a nova orientação relativa.
            O caminho é otimizado para o drible, considerando a posição da bola, dos adversários e o gol adversário.

        Parâmetros:
            ret_segments : float, opcional
                Distância máxima do próximo ponto, expressa em segmentos do caminho (cada um de 0.1m ou 0.1*sqrt(2)m se na diagonal).
                Se ret_segments == 0, é retornada a posição atual.
                Se for None, é automaticamente calculado conforme a velocidade do robô.

            optional_2d_target : ndarray ou None
                Ponto 2D opcional que deve ser considerado o gol ou o destino do drible.
                Se for None, o gol adversário é utilizado automaticamente pelo A*.

            goalpost_safety_margin : float
                Raio de repulsão para os postes do gol adversário.
                Se for 0, é utilizado o raio mínimo de seguridadade.

            timeout : float
                Tempo máximo de execução em microsegundos para o A* encontrar um caminho.

        Retorna:
            next_pos : ndarray
                Posição 2D do próximo ponto na trajetória.

            next_rel_ori : float
                Orientação relativa, em graus, que o robô deve adotar junto com o próximo ponto de drible.
        """
        r = self.world.robot
        ball_2d = self.world.ball_abs_pos[:2]

        # ------------------------------------------- get obstacles

        obstacles = self.get_obstacles(include_teammates=True, include_opponents=True, include_play_mode_restrictions=False,
                                       max_distance=5, max_age=1000, goalpost_safety_margin=goalpost_safety_margin, mode=PathManager.MODE_DRIBBLE)

        # ------------------------------------------- get path

        start_pos = self._get_hot_start(PathManager.HOT_START_DIST_DRIBBLE)

        path, path_len, path_status, path_cost = self.get_path(start_pos, False, obstacles, optional_2d_target, timeout)

        # ------------------------------------------- get next target position & orientation

        if ret_segments is None:
            ret_segments = 2.0

        next_pos = self._extract_target_from_path(path, path_len, ret_segments)
        next_rel_ori = GeneralMath.normalize_deg(GeneralMath.angle_horizontal_from_vector2D(next_pos - ball_2d) - r.imu_torso_orientation)

        # ------------------------------------------ update hot start for next run

        if path_len != 0:
            self._update_hot_start(GeneralMath.deg_to_rad(r.imu_torso_orientation), PathManager.HOT_START_DIST_DRIBBLE)

        # ------------------------------------------ draw
        if self._draw_path and path_status != PathManager.STATUS_DIRECT:
            d = self.world.team_draw if self._use_team_channel else self.world.draw
            d.point(next_pos, 2, d.Color.pink, "path_next_pos", False)  # will not draw if drawing object is internally disabled
            d.line(ball_2d, next_pos, 2, d.Color.pink, "path_next_pos")  # will not draw if drawing object is internally disabled

        return next_pos, next_rel_ori

    def get_push_path(
            self,
            ret_segments=1.5,
            optional_2d_target=None,
            avoid_opponents=False,
            timeout=3000
    ) -> np.ndarray:
        """
        Descrição:
            Calcula o próximo ponto na trajetória para um push crítico da bola.
            O caminho é otimizado para dar estabilidade na execução do push, evitando
            obstáculos e aumentando o controle junto à bola.

        Parâmetros:
            ret_segments : float, opcional
                Distância máxima do próximo ponto na trajetória, expressa em segmentos de caminho
                (cada um de 0.1m ou 0.1*sqrt(2)m se na diagonal).
                Se ret_segments == 0, é retornada a posição atual.
                Se maior que o comprimento do caminho, é limitada automaticamente.

            optional_2d_target : ndarray ou None
                Ponto 2D opcional que deve ser considerado como o gol ou o destino do push.
                Se for None, o gol adversário é utilizado automaticamente pelo A*.

            avoid_opponents : bool, opcional
                Se True, serão considerados os oponentes como obstáculos.
                Se False, serão ignorados na hora de preparar o caminho.

            timeout : float
                Tempo máximo de execução em microsegundos para o A* encontrar um caminho.

        Retorna:
            next_pos : ndarray
                Posição 2D do próximo ponto na trajetória de push.
        """

        ball_2d = self.world.ball_abs_pos[:2]

        # ------------------------------------------- get obstacles

        obstacles = self.get_obstacles(include_teammates=False, include_opponents=avoid_opponents, include_play_mode_restrictions=False)

        # ------------------------------------------- get path

        path, path_len, path_status, path_cost = self.get_path(ball_2d, False, obstacles, optional_2d_target, timeout)

        # ------------------------------------------- get next target position & orientation

        next_pos = self._extract_target_from_path(path, path_len, ret_segments)

        return next_pos

    def get_path(
            self,
            start,
            allow_out_of_bounds,
            obstacles=None,
            optional_2d_target=None,
            timeout=3000
    ) -> tuple[np.ndarray, int, int, float]:
        """
        Descrição:
            Calcula um caminho a ser seguido pelo robô utilizando o algoritmo A*.

        Parâmetros:
            start : iterable de dois floats
                Ponto de partida (x, y) do robô na tabela.

            allow_out_of_bounds : bool
                Se False, o caminho permanecerá dentro dos límites do campo.
                Se True, é permitido que o caminho vá para fora das bordas.
                Geralmente deve ser False ao driblar a bola.

            obstacles : list of tuples
                Lista de obstáculos presentes no caminho.
                Cada obstáculo é uma tupla de 5 floats:
                    (x, y, raio_forte, raio_suave, forca_repulsiva).
                - (x, y): posição do obstáculo
                - raio_forte: raio onde o robô deve definitivamente desviar
                - raio_suave: raio onde o robô é influenciado para se desviar
                - forca_repulsiva: intensidade dessa influência

            optional_2d_target : iterable de dois floats ou None
                Ponto opcional que deve ser o destino.
                Se for None, o gol adversário é escolhido automaticamente pelo A*.

            timeout : float
                Tempo máximo de execução do A* em microsegundos.

        Retorna:
            path : ndarray
                Uma sequência de posições (x, y) que formam o caminho.

            path_len : int
                Quantidade de segmentos que formam o caminho (path_len == len(path)/2 - 1).

            path_status : int
                Código que indica o resultado do A*:
                    0 = caminho encontrado
                    1 = caminhos com saída pelas bordas
                    2 = caminhos bloqueados
                    3 = caminhos muito próximos de um obstáculo

            path_cost : float
                Custo total do caminho, utilizado pelo A* para avaliar a rota escolhida.
        """

        if obstacles is None:
            obstacles = []

        go_to_goal = int(optional_2d_target is None)

        if optional_2d_target is None:
            optional_2d_target = (0, 0)

        # Concatenará os obstaculos
        # Antes : obst = [(1, 2), (3, 4), (5, 6)]
        # Depois: obst = (1, 2, 3, 4, 5, 6)
        obstacles = sum(obstacles, tuple())
        assert len(obstacles) % 5 == 0, "Cada obstáculo deve ser caracterizado como 5 valores float, veja PathManager.py"

        # Path parameters: start, allow_out_of_bounds, go_to_goal, optional_target, timeout (us), obstacles
        params = np.array([*start, int(allow_out_of_bounds), go_to_goal, *optional_2d_target, timeout, *obstacles], np.float32)
        path_ret = a_estrela.find_optimal_path(params)
        path = path_ret[:-2]
        path_status = path_ret[-2]

        # ---------------------------------------------- Draw path segments
        if self._draw_path:
            d = self.world.team_draw if self._use_team_channel else self.world.draw
            if d.enabled:
                c = {0: d.Color.green_lawn, 1: d.Color.yellow, 2: d.Color.red, 3: d.Color.cyan}[path_status]
                for j in range(0, len(path) - 2, 2):
                    d.line((path[j], path[j + 1]), (path[j + 2], path[j + 3]), 1, c, "path_segments", False)
                d.flush("path_segments")

        return path, len(path) // 2 - 1, path_status, path_ret[-1]  # path, path_len (number of segments), path_status, path_cost (A* cost)
