from collections import deque
from sobre_cpp.preditor_de_curva_da_bola import preditor_de_curva_da_bola
from sobre_cpp.ambientacao import ambientacao
from sobre_logs.Logger import Logger
from math import atan2, pi
from math_ops.Matriz4x4 import Matriz4x4
from world.commons.Draw import Draw
from world.commons.OtherRobot import OtherRobot
from world.Robot import Robot
import numpy as np


class World:
    """
    Descrição:
        A classe `World` define configurações, parâmetros e constantes utilizadas pelo robô para se
        localizar e tomar decisão nas diferentes fases de um jogo de robô-futebol. Inclui o
        passo de tempo, os diferentes tipos de jogadas, grupos de jogadas, posições das bandeirinhas
        de escanteio e postes do gol.

    Métodos Disponíveis:
        - log
        - get_ball_rel_vel
        - get_ball_abs_vel
        - get_predicted_ball_pos
        - get_intersection_point_with_ball
        - update
        - update_other_robot

    Variáveis de Ambiente:
        Configurações, parâmetros e constantes utilizadas pelo robô para se
        localizar e tomar decisão nas diferentes fases de um jogo de robô-futebol. Inclui o
        passo de tempo, os diferentes tipos de jogadas, grupos de jogadas, posições das bandeirinhas
        de escanteio e postes do gol:
        - STEPTIME: (float) Duração de um passo de simulação, em segundos.
        - STEPTIME_MS: (int) Duração de um passo de simulação, em milissegundos.
        - VISUALSTEP: (float) Duração de um passo de atualização visual, em segundos.
        - VISUALSTEP_MS: (int) Duração de um passo de atualização visual, em milissegundos.

        - M_OUR_KICKOFF: (int) Jogo iniciado pelo nosso time.
        - M_OUR_KICK_IN: (int) Lateral a favor do nosso time.
        - M_OUR_CORNER_KICK: (int) Escanteio a favor do nosso time.
        - M_OUR_GOAL_KICK: (int) Tiro de meta a favor do nosso time.
        - M_OUR_FREE_KICK: (int) Tiro livre a favor do nosso time.
        - M_OUR_PASS: (int) Passe iniciado pelo nosso time.
        - M_OUR_DIR_FREE_KICK: (int) Tiro livre indireto a favor do nosso time.
        - M_OUR_GOAL: (int) Gol realizado pelo nosso time.
        - M_OUR_OFFSIDE: (int) Impedimento do nosso time.

        - M_THEIR_KICKOFF: (int) Jogo iniciado pelo adversário.
        - M_THEIR_KICK_IN: (int) Lateral a favor do adversário.
        - M_THEIR_CORNER_KICK: (int) Escanteio a favor do adversário.
        - M_THEIR_GOAL_KICK: (int) Tiro de meta a favor do adversário.
        - M_THEIR_FREE_KICK: (int) Tiro livre a favor do adversário.
        - M_THEIR_PASS: (int) Passe iniciado pelo adversário.
        - M_THEIR_DIR_FREE_KICK: (int) Tiro livre indireto a favor do adversário.
        - M_THEIR_GOAL: (int) Gol realizado pelo adversário.
        - M_THEIR_OFFSIDE: (int) Impedimento do adversário.

        - M_BEFORE_KICKOFF: (int) Antes do início do jogo.
        - M_GAME_OVER: (int) Fim de jogo.
        - M_PLAY_ON: (int) Jogo em andamento.

        - MG_OUR_KICK: (int) Grupo que relaciona às jogadas a favor do nosso time.
        - MG_THEIR_KICK: (int) Grupo que relaciona às jogadas a favor adversário.
        - MG_ACTIVE_BEAM: (int) Grupo que relaciona o feixe ativo.
        - MG_PASSIVE_BEAM: (int) Grupo que relaciona o feixe passivo.
        - MG_OTHER: (int) Outros grupos que englobam diferentes jogadas.

        - FLAGS_CORNERS_POS: (tuple) Uma tupla de tuplas que especificam as posições das quatro bandeirinhas de escanteio. Formato: (x, y, z).
        - FLAGS_POSTS_POS: (tuple) Uma tupla de tuplas que especificam as posições das traves do gol. Formato: (x, y, z).
    """

    STEPTIME = 0.02  # Fixed step time
    STEPTIME_MS = 20  # Fixed step time in milliseconds
    VISUALSTEP = 0.04  # Fixed visual step time
    VISUALSTEP_MS = 40  # Fixed visual step time in milliseconds

    # play modes in our favor
    M_OUR_KICKOFF = 0
    M_OUR_KICK_IN = 1
    M_OUR_CORNER_KICK = 2
    M_OUR_GOAL_KICK = 3
    M_OUR_FREE_KICK = 4
    M_OUR_PASS = 5
    M_OUR_DIR_FREE_KICK = 6
    M_OUR_GOAL = 7
    M_OUR_OFFSIDE = 8

    # play modes in their favor
    M_THEIR_KICKOFF = 9
    M_THEIR_KICK_IN = 10
    M_THEIR_CORNER_KICK = 11
    M_THEIR_GOAL_KICK = 12
    M_THEIR_FREE_KICK = 13
    M_THEIR_PASS = 14
    M_THEIR_DIR_FREE_KICK = 15
    M_THEIR_GOAL = 16
    M_THEIR_OFFSIDE = 17

    # neutral play modes
    M_BEFORE_KICKOFF = 18
    M_GAME_OVER = 19
    M_PLAY_ON = 20

    # play mode groups
    MG_OUR_KICK = 0
    MG_THEIR_KICK = 1
    MG_ACTIVE_BEAM = 2
    MG_PASSIVE_BEAM = 3
    MG_OTHER = 4  # play on, game over

    FLAGS_CORNERS_POS = ((-15, -10, 0), (-15, +10, 0), (+15, -10, 0), (+15, +10, 0))
    FLAGS_POSTS_POS = ((-15, -1.05, 0.8), (-15, +1.05, 0.8), (+15, -1.05, 0.8), (+15, +1.05, 0.8))

    def __init__(
            self,
            robot_type: int,
            team_name: str,
            unum: int,
            apply_play_mode_correction: bool,
            enable_draw: bool,
            logger: Logger,
            host: str
    ) -> None:
        """
        Descrição:
            Inicializa o objeto que representa a compressão do mundo pelo robô, o estado do jogo e outras
            configurações utilizadas pelo agente na simulação de robô-futebol.

        Parâmetros:
            robot_type: (int) Tipo do robô.
            team_name: (str) Nome do nosso time.
            unum: (int) Número do robô na equipe.
            apply_play_mode_correction: (bool) Se deve aplicar a correção de posição de bola de acordo com o modo de jogo.
            enable_draw: (bool) Se deve habilitar o desenho para debug.
            logger: (Logger) Instância de logger para debug e logging de mensagens.
            host: (str) Endereço do host onde o robô se conectará.

        Retorno:
            None
        """

        self.team_name = team_name  # Nome do nosso time
        self.team_name_opponent = None  # (str) Nome do time adversário
        self.apply_play_mode_correction = apply_play_mode_correction  # Verdadeiro se deve ajustar a posição da bola conforme o modo de jogo

        self.step = 0  # Quantidade total de mensagens de simulação já recebidas (sempre em sincronia com self.time_local_ms)
        self.time_server = 0.0  # Tempo, em segundos, indicado pelo servidor (não é confiavelmente preciso; usar apenas para sincronização)
        self.time_local_ms = 0  # Tempo da simulação, em milissegundos (confiavelmente preciso; é incrementado a cada 20ms)
        self.time_game = 0.0  # Tempo de jogo, em segundos, indicado pelo servidor
        self.goals_scored = 0  # Gols marcados pelo nosso time
        self.goals_conceded = 0  # Gols sofridos pelo nosso time

        self.team_side_is_left = None  # (bool) Verdadeiro se o nosso time joga pelo lado esquerdo (definido pelo world parser)
        self.play_mode = None  # Modo de jogo, fornecido pelo servidor
        self.play_mode_group = None  # Alguns modos de jogo compartilham características, sendo agrupados

        self.flags_corners = None  # (dict) Bandeirinhas de escanteio, chave=(x, y, z), considerando que estamos do lado esquerdo
        self.flags_posts = None  # (dict) Traves do gol, chave=(x, y, z), considerando que estamos do lado esquerdo

        self.ball_rel_head_sph_pos = np.zeros(3)  # Posição da bola relativa à cabeça (coordenadas esféricas) (m, graus, graus)
        self.ball_rel_head_cart_pos = np.zeros(3)  # Posição da bola relativa à cabeça (coordenadas cartesianas) (m)
        self.ball_rel_torso_cart_pos = np.zeros(3)  # Posição da bola relativa ao torso (coordenadas cartesianas) (m)
        self.ball_rel_torso_cart_pos_history = deque(maxlen=20)  # Histórico das posições da bola em relação ao torso (máximo de 20 posições)
        self.ball_abs_pos = np.zeros(3)  # Posição absoluta da bola
        self.ball_abs_pos_history = deque(maxlen=20)  # Histórico das posições absolutas da bola
        self.ball_abs_pos_last_update = 0  # Momento (World.time_local_ms) da última atualização da posição da bola

        self.ball_abs_vel = np.zeros(3)  # Vetor velocidade da bola
        self.ball_abs_speed = 0  # Velocidade escalar da bola
        self.ball_is_visible = False  # Verdadeiro se a bola é visível pelo robô
        self.is_ball_abs_pos_from_vision = False  # Verdadeiro se a posição da bola é fruto da visão
        self.ball_last_seen = 0  # Momento (World.time_local_ms) em que a bola foi vista

        self.ball_cheat_abs_pos = np.zeros(3)  # Posição da bola fornecida pelo servidor como cheat
        self.ball_cheat_abs_vel = np.zeros(3)  # Velocidade da bola fornecida pelo servidor como cheat

        self.ball_2d_pred_pos = np.zeros((1, 2))  # Posições 2D previstas da bola
        self.ball_2d_pred_vel = np.zeros((1, 2))  # Velocidades 2D previstas da bola
        self.ball_2d_pred_spd = np.zeros(1)  # Velocidades 2D previstas da bola

        # *at intervals of 0.02 s until ball comes to a stop or gets out of bounds (according to prediction)
        self.lines = np.zeros((30, 6))  # Linhas visível pelo robô
        self.line_count = 0  # Quantidade de linhas visível pelo robô

        self.vision_last_update = 0  # Momento (World.time_local_ms) da última atualização da visão
        self.vision_is_up_to_date = False  # Verdadeiro se a visão foi atualizada na última mensagem

        self.teammates = [OtherRobot(i, True) for i in range(1, 12)]  # Lista de colegas de time, classificados pelo número
        self.opponents = [OtherRobot(i, False) for i in range(1, 12)]  # Lista de oponentes, classificados pelo número

        self.teammates[unum - 1].is_self = True  # Este é o robô propriamente dicho

        # Observe a porta.
        self.draw = Draw(enable_draw, unum, host, 32769)  # Objeto para desenhar pelo robô atual
        self.team_draw = Draw(enable_draw, 0, host, 32769)  # Objeto compartilhado de desenho com o time

        self.logger = logger
        self.robot = Robot(unum, robot_type)

    def log(self, msg: str) -> None:
        """
        Descrição:
            Registra uma mensagem junto ao logger, associando-a ao passo atual da simulação.

        Parâmetros:
            msg: (str) Mensagem a ser gravada junto ao logger.

        Retorno:
            None
        """

        self.logger.escrever(msg, True, self.step)

    def get_ball_rel_vel(self, history_steps: int):
        """
        Descrição:
            Calcula a velocidade da bola em relação ao torso (m/s).

        Parâmetros:
            history_steps: (int)Quantidade de posições anteriores a serem utilizadas para o cálculo.
                Deve estar no intervalo [1, 20].

        Retorno:
            numpy.ndarray
                Um vetor de 3 elementos representando a velocidade da bola em relação ao torso (m/s).

        """
        assert 1 <= history_steps <= 20, "O parâmetro 'history_steps' deve estar no intervalo [1, 20]"

        if len(self.ball_rel_torso_cart_pos_history) == 0:
            return np.zeros(3)

        h_step = min(history_steps, len(self.ball_rel_torso_cart_pos_history))
        t = h_step * World.VISUALSTEP

        return (self.ball_rel_torso_cart_pos - self.ball_rel_torso_cart_pos_history[h_step - 1]) / t

    def get_ball_abs_vel(self, history_steps: int):
        """
        Descrição:
            Calcula a velocidade absoluta da bola (m/s) com base nas posições anteriores.

        Parâmetros:
            history_steps: (int)Quantidade de posições anteriores a serem utilizadas para o cálculo.
                Deve estar no intervalo [1, 20].

        Retorno:
            numpy.ndarray
                Um vetor de 3 elementos representando a velocidade absoluta da bola (m/s).
        """
        assert 1 <= history_steps <= 20, "O parâmetro 'history_steps' deve estar no intervalo [1, 20]"

        if len(self.ball_abs_pos_history) == 0:
            return np.zeros(3)

        h_step = min(history_steps, len(self.ball_abs_pos_history))
        t = h_step * World.VISUALSTEP

        return (self.ball_abs_pos - self.ball_abs_pos_history[h_step - 1]) / t

    def get_predicted_ball_pos(self, max_speed) -> np.ndarray:
        """
        Descrição:
            Retorna a posição 2D prevista da bola, considerando que a mesma se moverá
            até que a velocidade seja menor ou igual a `max_speed`. Se essa posição
            exceder o horizonte de predição, é retornada a última posição prevista.

        Parâmetros:
            max_speed: (float)
                Velocidade máxima da bola na posição desejada (m/s).

        Retorno:
            numpy.ndarray
                Um array 2D (x, y) representando a posição prevista da bola.
        """

        b_sp = self.ball_2d_pred_spd
        index = len(b_sp) - max(1, np.searchsorted(b_sp[::-1], max_speed, side='right'))
        return self.ball_2d_pred_pos[index]

    def get_intersection_point_with_ball(self, player_speed) -> tuple[np.ndarray, float]:
        """
        Descrição:
            Calcula o ponto de interseção 2D com a bola em movimento,
            considerando a velocidade média do robô.

            Utilizamos o módulo preditor_de_curva_da_bola para safar.

        Parâmetros:
            player_speed: (float)
                Velocidade média na qual o robô se moverá para encontrar a bola (m/s).

        Retorno:
            intersection_point : numpy.ndarray
                Ponto de interseção 2D (x, y) onde o robô consegue encontrar a bola.

            intersection_distance : float
                Distância entre a posição atual do robô e o ponto de interseção (m).
        """
        # params = np.array([*self.robot.loc_head_position[:2], player_speed * 0.02, *self.ball_2d_pred_pos.flat], np.float32)
        pred_ret = preditor_de_curva_da_bola.get_possible_intersection_with_ball(self.robot.loc_head_position[:2], player_speed * 0.02, self.ball_2d_pred_pos.flat)
        return pred_ret[:2], pred_ret[2]

    def update(self):
        """
        Descrição:
            Realiza a atualização do estado interno do robô, da bola, dos companheiros de equipe
            e dos adversários.

            A função é chamado a cada passo de simulação para manter o modelo atualizado
            com o que aconteceu neste passo.

        Retorno:
        - Atualização do modelo cinemático do robô.
        - Cálculo da posição da bola, tanto a partir da visão quanto pelas regras do jogo.
        - Localização do robô no campo a partir de fontes de visão, toques dos pés e posições de marcos.
        - Aplicação de um decaimento nas velocidades dos outros robôs.
        - Atualização das posições, velocidade, distância horizontal e outros parâmetros de companheiros de equipe e adversários.
        - Cálculo da velocidade da bola, assim como da posição e velocidade previstas.
        - Atualização da unidade inercial (IMU) do robô, depois que a localização é atualizada.
        """

        i_am_the_robot = self.robot
        PM = self.play_mode
        W = World

        # reset variables
        i_am_the_robot.loc_is_up_to_date = False
        i_am_the_robot.loc_head_z_is_up_to_date = False

        # update play mode groups
        if PM in (W.M_PLAY_ON, W.M_GAME_OVER):  # most common group
            self.play_mode_group = W.MG_OTHER
        elif PM in (W.M_OUR_KICKOFF, W.M_OUR_KICK_IN, W.M_OUR_CORNER_KICK, W.M_OUR_GOAL_KICK,
                    W.M_OUR_OFFSIDE, W.M_OUR_PASS, W.M_OUR_DIR_FREE_KICK, W.M_OUR_FREE_KICK):
            self.play_mode_group = W.MG_OUR_KICK
        elif PM in (W.M_THEIR_KICK_IN, W.M_THEIR_CORNER_KICK, W.M_THEIR_GOAL_KICK, W.M_THEIR_OFFSIDE,
                    W.M_THEIR_PASS, W.M_THEIR_DIR_FREE_KICK, W.M_THEIR_FREE_KICK, W.M_THEIR_KICKOFF):
            self.play_mode_group = W.MG_THEIR_KICK
        elif PM in (W.M_BEFORE_KICKOFF, W.M_THEIR_GOAL):
            self.play_mode_group = W.MG_ACTIVE_BEAM
        elif PM in (W.M_OUR_GOAL,):
            self.play_mode_group = W.MG_PASSIVE_BEAM
        elif PM is not None:
            raise ValueError(f'Unexpected play mode ID: {PM}, verify the World.py')

        i_am_the_robot.update_pose()  # update forward kinematics

        if self.ball_is_visible:
            # Compute ball position, relative to torso
            self.ball_rel_torso_cart_pos = i_am_the_robot.head_to_body_part_transform("torso", self.ball_rel_head_cart_pos)

        if self.vision_is_up_to_date:  # update vision based localization

            # Prepare all variables for localization

            feet_contact = np.zeros(6)

            lf_contact = i_am_the_robot.frp.get('lf', None)
            rf_contact = i_am_the_robot.frp.get('rf', None)
            if lf_contact is not None:
                feet_contact[0:3] = Matriz4x4(i_am_the_robot.body_parts["lfoot"].transform).translate(lf_contact[0:3], True).obter_vetor_de_translacao()
            if rf_contact is not None:
                feet_contact[3:6] = Matriz4x4(i_am_the_robot.body_parts["rfoot"].transform).translate(rf_contact[0:3], True).obter_vetor_de_translacao()

            ball_pos = np.concatenate((self.ball_rel_head_cart_pos, self.ball_cheat_abs_pos))

            corners_list = [[key in self.flags_corners, 1.0, *key, *self.flags_corners.get(key, (0, 0, 0))] for key in World.FLAGS_CORNERS_POS]
            posts_list = [[key in self.flags_posts, 0.0, *key, *self.flags_posts.get(key, (0, 0, 0))] for key in World.FLAGS_POSTS_POS]
            all_landmarks = np.array(corners_list + posts_list, float)

            # Compute localization

            loc = ambientacao.localize_agent_pose(
                i_am_the_robot.feet_toes_are_touching['lf'],
                i_am_the_robot.feet_toes_are_touching['rf'],
                feet_contact,
                self.ball_is_visible,
                ball_pos,
                i_am_the_robot.cheat_abs_pos,
                all_landmarks,
                self.lines[0:self.line_count]
            )

            i_am_the_robot.update_localization(loc, self.time_local_ms)

            # Update self in teammates list (only the most useful parameters, add as needed)
            me = self.teammates[i_am_the_robot.unum - 1]
            me.state_last_update = i_am_the_robot.loc_last_update
            me.state_abs_pos = i_am_the_robot.loc_head_position
            me.state_fallen = i_am_the_robot.loc_head_z < 0.3  # uses same criterion as for other teammates - not as reliable as player.behavior.is_ready("Get_Up")
            me.state_orientation = i_am_the_robot.loc_torso_orientation
            me.state_ground_area = (i_am_the_robot.loc_head_position[:2], 0.2)  # relevant for localization demo

            # Save last ball position to history at every vision cycle (even if not up to date)
            self.ball_abs_pos_history.appendleft(self.ball_abs_pos)  # from vision or radio
            self.ball_rel_torso_cart_pos_history.appendleft(self.ball_rel_torso_cart_pos)

            '''
            Get ball position based on vision or play mode
            Sources:
            Corner kick position - rcssserver3d/plugin/soccer/soccerruleaspect/soccerruleaspect.cpp:1927 (May 2022)
            Goal   kick position - rcssserver3d/plugin/soccer/soccerruleaspect/soccerruleaspect.cpp:1900 (May 2022)
            '''
            ball = None
            if self.apply_play_mode_correction:
                if PM == W.M_OUR_CORNER_KICK:
                    ball = np.array([15, 5.483 if self.ball_abs_pos[1] > 0 else -5.483, 0.042], float)
                elif PM == W.M_THEIR_CORNER_KICK:
                    ball = np.array([-15, 5.483 if self.ball_abs_pos[1] > 0 else -5.483, 0.042], float)
                elif PM in [W.M_OUR_KICKOFF, W.M_THEIR_KICKOFF, W.M_OUR_GOAL, W.M_THEIR_GOAL]:
                    ball = np.array([0, 0, 0.042], float)
                elif PM == W.M_OUR_GOAL_KICK:
                    ball = np.array([-14, 0, 0.042], float)
                elif PM == W.M_THEIR_GOAL_KICK:
                    ball = np.array([14, 0, 0.042], float)

                # Discard hard-coded ball position if robot is near that position (in favor of its own vision)
                if ball is not None and np.linalg.norm(i_am_the_robot.loc_head_position[:2] - ball[:2]) < 1:
                    ball = None

            if ball is None and self.ball_is_visible and i_am_the_robot.loc_is_up_to_date:
                ball = i_am_the_robot.loc_head_to_field_transform(self.ball_rel_head_cart_pos)
                ball[2] = max(ball[2], 0.042)  # lowest z = ball radius
                if PM != W.M_BEFORE_KICKOFF:  # for compatibility with tests without active soccer rules
                    ball[:2] = np.clip(ball[:2], [-15, -10], [15, 10])  # force ball position to be inside field

            # Update internal ball position (also updated by Radio)
            if ball is not None:
                time_diff = (self.time_local_ms - self.ball_abs_pos_last_update) / 1000
                self.ball_abs_vel = (ball - self.ball_abs_pos) / time_diff
                self.ball_abs_speed = np.linalg.norm(self.ball_abs_vel)
                self.ball_abs_pos_last_update = self.time_local_ms
                self.ball_abs_pos = ball
                self.is_ball_abs_pos_from_vision = True

            # Velocity decay for teammates and opponents (it is later neutralized if the velocity is updated)
            for p in self.teammates:
                p.state_filtered_velocity *= p.vel_decay
            for p in self.opponents:
                p.state_filtered_velocity *= p.vel_decay

            # Update teammates and opponents
            if i_am_the_robot.loc_is_up_to_date:
                for p in self.teammates:
                    if not p.is_self:  # if teammate is not self
                        if p.is_visible:  # if teammate is visible, execute full update
                            self.update_other_robot(p)
                        elif p.state_abs_pos is not None:  # otherwise update its horizontal distance (assuming last known position)
                            p.state_horizontal_dist = np.linalg.norm(i_am_the_robot.loc_head_position[:2] - p.state_abs_pos[:2])

                for p in self.opponents:
                    if p.is_visible:  # if opponent is visible, execute full update
                        self.update_other_robot(p)
                    elif p.state_abs_pos is not None:  # otherwise update its horizontal distance (assuming last known position)
                        p.state_horizontal_dist = np.linalg.norm(i_am_the_robot.loc_head_position[:2] - p.state_abs_pos[:2])

        # Update prediction of ball position/velocity
        if self.play_mode_group != W.MG_OTHER:  # not 'play on' nor 'game over', so ball must be stationary
            self.ball_2d_pred_pos = self.ball_abs_pos[:2].copy().reshape(1, 2)
            self.ball_2d_pred_vel = np.zeros((1, 2))
            self.ball_2d_pred_spd = np.zeros(1)

        elif self.ball_abs_pos_last_update == self.time_local_ms:  # make new prediction for new ball position (from vision or radio)

            pred_ret = preditor_de_curva_da_bola.get_ball_kinematic_prediction(self.ball_abs_pos[:2], np.copy(self.get_ball_abs_vel(6)[:2]))
            sample_no = len(pred_ret) // 5 * 2
            self.ball_2d_pred_pos = pred_ret[:sample_no].reshape(-1, 2)
            self.ball_2d_pred_vel = pred_ret[sample_no:sample_no * 2].reshape(-1, 2)
            self.ball_2d_pred_spd = pred_ret[sample_no * 2:]

        elif len(self.ball_2d_pred_pos) > 1:  # otherwise, advance to next predicted step, if available
            self.ball_2d_pred_pos = self.ball_2d_pred_pos[1:]
            self.ball_2d_pred_vel = self.ball_2d_pred_vel[1:]
            self.ball_2d_pred_spd = self.ball_2d_pred_spd[1:]

        i_am_the_robot.update_imu(self.time_local_ms)  # update imu (must be executed after localization)

    def update_other_robot(self, other_robot: OtherRobot):
        """
        Descrição:
            Atualiza o estado de outro robô com base nas posições relativas das partes visíveis do corpo.

            Esta função recalcula a posição absoluta das partes do corpo do robô observado,
            determina seu estado (como se está caído), atualiza sua velocidade filtrada,
            calcula sua posição e distância horizontal relativas ao robô próprio,
            estima sua orientação a partir da posição dos braços ou pés,
            e calcula a área projetada do robô no chão.

            A orientação é calculada em graus, com a convenção de 0° apontando para o eixo Y positivo (topo do campo).

        Parâmetros:
            other_robot : OtherRobot
                Instância do robô a ser atualizada, contendo informações sobre partes do corpo, velocidade,
                posição e outros estados.

        Retorno:
            - Atualiza posições absolutas das partes do corpo usando a transformação de localização do robô próprio.
            - Calcula a média 2D das posições visíveis das partes do corpo para estimar a posição do robô se a cabeça não estiver visível.
            - Atualiza o estado "caído" baseado na altura da cabeça (se visível).
            - Calcula e filtra a velocidade do robô observado com base na posição da cabeça ou na média das partes visíveis.
            - Atualiza a posição absoluta do robô, preferencialmente pela cabeça.
            - Calcula a distância horizontal entre o robô próprio e o observado.
            - Estima a orientação do robô observado com base no vetor formado pelos braços inferiores ou pés.
            - Determina uma área circular projetada no chão representando a presença física do robô.
            - Atualiza o timestamp da última atualização do estado.
        """

        o = other_robot
        r = self.robot

        # update body parts absolute positions
        o.state_body_parts_abs_pos = o.body_parts_cart_rel_pos.copy()
        for bp, pos in o.body_parts_cart_rel_pos.items():
            # Using the IMU could be beneficial if we see other robots but can't self-locate
            o.state_body_parts_abs_pos[bp] = r.loc_head_to_field_transform(pos, False)

        # auxiliary variables
        bps_apos = o.state_body_parts_abs_pos  # read-only shortcut
        bps_2d_apos_list = [v[:2] for v in bps_apos.values()]  # list of body parts' 2D absolute positions
        avg_2d_pt = np.average(bps_2d_apos_list, axis=0)  # 2D avg pos of visible body parts
        head_is_visible = 'head' in bps_apos

        # evaluate robot's state (unchanged if head is not visible)
        if head_is_visible:
            o.state_fallen = bps_apos['head'][2] < 0.3

        # compute velocity if head is visible
        if o.state_abs_pos is not None:
            time_diff = (self.time_local_ms - o.state_last_update) / 1000
            if head_is_visible:
                # if last position is 2D, we assume that the z coordinate did not change, so that v.z=0
                old_p = o.state_abs_pos if len(o.state_abs_pos) == 3 else np.append(o.state_abs_pos, bps_apos['head'][2])
                velocity = (bps_apos['head'] - old_p) / time_diff
                decay = o.vel_decay  # neutralize decay in all axes
            else:  # if head is not visible, we only update the x & y components of the velocity
                velocity = np.append((avg_2d_pt - o.state_abs_pos[:2]) / time_diff, 0)
                decay = (o.vel_decay, o.vel_decay, 1)  # neutralize decay (except in the z-axis)
            # apply filter
            if np.linalg.norm(velocity - o.state_filtered_velocity) < 4:  # otherwise assume it was beamed
                o.state_filtered_velocity /= decay  # neutralize decay
                o.state_filtered_velocity += o.vel_filter * (velocity - o.state_filtered_velocity)

        # compute robot's position (preferably based on head)
        if head_is_visible:
            o.state_abs_pos = bps_apos['head']  # 3D head position, if head is visible
        else:
            o.state_abs_pos = avg_2d_pt  # 2D avg pos of visible body parts

        # compute robot's horizontal distance (head distance, or avg. distance of visible body parts)
        o.state_horizontal_dist = np.linalg.norm(r.loc_head_position[:2] - o.state_abs_pos[:2])

        # compute orientation based on pair of lower arms or feet, or average of both
        lr_vec = None
        if 'llowerarm' in bps_apos and 'rlowerarm' in bps_apos:
            lr_vec = bps_apos['rlowerarm'] - bps_apos['llowerarm']

        if 'lfoot' in bps_apos and 'rfoot' in bps_apos:
            if lr_vec is None:
                lr_vec = bps_apos['rfoot'] - bps_apos['lfoot']
            else:
                lr_vec = (lr_vec + (bps_apos['rfoot'] - bps_apos['lfoot'])) / 2

        if lr_vec is not None:
            o.state_orientation = atan2(lr_vec[1], lr_vec[0]) * 180 / pi + 90

        # compute projection of player area on ground (circle)
        if o.state_horizontal_dist < 4:  # we don't need precision if the robot is farther than 4m
            max_dist = np.max(np.linalg.norm(bps_2d_apos_list - avg_2d_pt, axis=1))
        else:
            max_dist = 0.2
        o.state_ground_area = (avg_2d_pt, max_dist)

        # update timestamp
        o.state_last_update = self.time_local_ms
