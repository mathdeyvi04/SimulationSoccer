from Agent.BaseAgent import BaseAgent
from math_ops.GeneralMath import GeneralMath
import numpy as np
from random import choice


class Agent(BaseAgent):

    def __init__(
            self,
            host: str,
            agent_port: int,
            monitor_port: int,
            unum: int,
            team_name: str,
            enable_log,
            enable_draw,
            wait_for_server=True,
            is_fat_proxy=False
    ) -> None:
        """
        Descrição:
            Inicializa uma instância do agente especializado (por exemplo, um goleiro ou kicker) no ambiente de simulação.

        Parâmetros:
            host: str
                Endereço do servidor (hostname ou IP).
            agent_port: int
                Porta de comunicação do agente.
            monitor_port: int
                Porta de monitoramento.
            unum: int
                Número de identificação do robô (define também sua função: 1 para goleiro, outros para kicker).
            team_name: str
                Nome do time do agente.
            enable_log: bool
                Habilita ou desabilita logging detalhado.
            enable_draw: bool
                Habilita ou desabilita recursos gráficos.
            wait_for_server: bool, opcional
                Aguarda disponibilidade do servidor antes de iniciar (padrão: True).
            is_fat_proxy: bool, opcional
                Flag adicional para configuração do agente (não utilizado diretamente aqui).

        Comentários:
            - Define o tipo do robô com base no número (goleiro ou kicker).
            - Inicializa o agente base e registra subsistemas relevantes.
            - Define atributos de estado para controle de ações (chute, mergulho, etc).
        """
        # Define o tipo de robô: goleiro (0) se unum==1, kicker (4) caso contrário
        robot_type = 0 if unum == 1 else 4

        # Inicializa o agente base com os parâmetros fornecidos
        # Args: host, agent_port, monitor_port, unum, robot_type, team_name, enable_log, enable_draw, play_mode_correction, wait_for_server, hear_callback
        super().__init__(host, agent_port, monitor_port, unum, robot_type, team_name, enable_log, enable_draw, False, wait_for_server, None)

        self.enable_draw = enable_draw
        self.state = 0  # 0-Normal, 1-Levantar, 2-Mergulho Esquerda, 3-Mergulho Direita, 4-Espera

        self.kick_dir = 0  # Direção do chute
        self.reset_kick = True  # Quando True, uma nova direção de chute é gerada aleatoriamente

    def think_and_send(self):
        """
        Descrição:
            Define o ciclo de decisão e ação do agente para cada iteração do ambiente de simulação.
            Responsável por:
            - Analisar o estado atual do mundo e do robô
            - Escolher e executar comportamentos (andar, chutar, mergulhar, levantar, etc)
            - Atualizar o estado interno do agente
            - Enviar comandos ao servidor e transmitir mensagens

        Comentários:
            - Separa a lógica por situações de jogo (pré-kickoff, gol, mergulho, chute, etc)
            - Utiliza estados internos para controlar ações encadeadas (ex: sequência de mergulho e espera)
            - O goleiro (unum==1) segue estratégia defensiva/mergulho, o kicker busca executar o chute
            - Inclui anotações gráficas para depuração quando habilitado
        """
        w = self.world
        r = self.world.robot
        my_head_pos_2d = r.loc_head_position[:2]
        my_ori = r.imu_torso_orientation
        ball_2d = w.ball_abs_pos[:2]
        ball_vec = ball_2d - my_head_pos_2d
        ball_dir = GeneralMath.angle_horizontal_from_vector2D(ball_vec)
        ball_dist = np.linalg.norm(ball_vec)
        ball_speed = np.linalg.norm(w.get_ball_abs_vel(6)[:2])
        behavior = self.behavior
        PM = w.play_mode

        # 1. Decisão de ação conforme estado do jogo e do agente
        if PM in [w.M_BEFORE_KICKOFF, w.M_THEIR_GOAL, w.M_OUR_GOAL]:  # Beam para posição inicial e espera
            self.state = 0
            self.reset_kick = True
            pos = (-14, 0) if r.unum == 1 else (4.9, 0)
            if np.linalg.norm(pos - r.loc_head_position[:2]) > 0.1 or behavior.is_ready("GetUp"):
                self.scom.commit_beam(pos, 0)  # Teleporta para a posição inicial
            else:
                behavior.execute("Zero_Bent_Knees")  # Aguarda imóvel
        elif self.state == 2:  # Mergulho esquerda
            self.state = 4 if behavior.execute("Dive_Left") else 2  # Espera após mergulho
        elif self.state == 3:  # Mergulho direita
            self.state = 4 if behavior.execute("Dive_Right") else 3  # Espera após mergulho
        elif self.state == 4:  # Espera (após mergulho ou durante chute adversário)
            pass
        elif self.state == 1 or behavior.is_ready("GetUp"):  # Caído ou levantando
            self.state = 0 if behavior.execute("GetUp") else 1  # Volta ao normal se terminou de levantar
        elif PM == w.M_OUR_KICKOFF and r.unum == 1 or PM == w.M_THEIR_KICKOFF and r.unum != 1:
            self.state = 4  # Espera até o próximo beam
        elif r.unum == 1:  # Goleiro
            y_coordinate = np.clip(ball_2d[1], -1.1, 1.1)
            behavior.execute("Walk", (-14, y_coordinate), True, 0, True, None)  # Move na linha do gol
            if ball_2d[0] < -10:
                self.state = 2 if ball_2d[1] > 0 else 3  # Mergulha para defender
        else:  # Kicker
            if PM == w.M_OUR_KICKOFF and ball_2d[0] > 5:  # Chuta se bola está visível
                if self.reset_kick:
                    self.kick_dir = choice([-7.5, 7.5])
                    self.reset_kick = False
                behavior.execute("BasicKick", self.kick_dir)
            else:
                behavior.execute("Zero_Bent_Knees")  # Aguarda imóvel

        # 2. Broadcast das mensagens de rádio
        self.radio.broadcast()

        # 3. Envio do comando ao servidor
        self.scom.commit_and_send(r.get_command())

        # 4. Anotações gráficas para depuração
        if self.enable_draw:
            d = w.draw
            if r.unum == 1:
                d.annotation((*my_head_pos_2d, 0.8), "Goalkeeper", d.Color.yellow, "status")
            else:
                d.annotation((*my_head_pos_2d, 0.8), "Kicker", d.Color.yellow, "status")
                if PM == w.M_OUR_KICKOFF:  # Desenha seta indicando direção do chute
                    d.arrow(ball_2d, ball_2d + 5 * GeneralMath.unit_vector_by_angle(self.kick_dir), 0.4, 3, d.Color.cyan_light, "Target")
