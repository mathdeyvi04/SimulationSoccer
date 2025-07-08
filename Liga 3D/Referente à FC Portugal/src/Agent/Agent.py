from Agent.BaseAgent import BaseAgent
from math_ops.GeneralMath import GeneralMath
import math
import numpy as np


class Agent(BaseAgent):
    """
    Descrição:

    Métodos Disponíveis:
        - beam
        - move
        - kick
        - think_and_send
        - fat_proxy_kick
        - fat_proxy_move
    """
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
            Inicializa um agente especializado com formação e tipo de robô definidos
             conforme seu número de uniforme.

        Parâmetros:
            host: str
                Endereço do servidor (hostname ou IP) para comunicação.
            agent_port: int
                Porta de rede utilizada pelo agente para envio/recebimento de comandos.
            monitor_port: int
                Porta de monitoramento do servidor.
            unum: int
                Número de uniforme do agente; determina o tipo e posição inicial do robô.
            team_name: str
                Nome do time ao qual o agente pertence.
            enable_log: bool
                Habilita ou desabilita o registro de logs do agente.
            enable_draw: bool
                Habilita ou desabilita recursos gráficos e de anotação visual.
            wait_for_server: bool, opcional
                Define se o agente deve aguardar o servidor antes de iniciar (padrão: True).
            is_fat_proxy: bool, opcional
                Indica se o agente atua como fat proxy (padrão: false).
        """
        # Define o tipo de robô conforme o número do uniforme (padronizado para a formação da equipe)
        self.min_opponent_ball_dist = None
        self.min_teammate_ball_dist = None
        robot_type = (0, 1, 1, 1, 2, 3, 3, 3, 4, 4, 4)[unum - 1]

        # Inicializa o agente base com todos os parâmetros necessários, incluindo tipo de robô e formação
        # O parâmetro play_mode_correction é definido como True
        super().__init__(host, agent_port, monitor_port, unum, robot_type, team_name,
                         enable_log, enable_draw, True, wait_for_server, None)

        self.enable_draw = enable_draw
        self.state = 0  # 0-Normal, 1-Levantando, 2-Chutando
        self.kick_direction = 0  # Direção do chute
        self.kick_distance = 0  # Distância do chute
        self.fat_proxy_cmd = "" if is_fat_proxy else None  # Comando especial para fat proxy
        self.fat_proxy_walk = np.zeros(3)  # Parâmetros filtrados de caminhada para fat proxy

        # Posição inicial definida conforme número do agente para formar a linha tática inicial
        self.init_pos = (
            [-14, 0],
            [-9, -5], [-9, 0], [-9, 5],
            [-5, -5], [-5, 0], [-5, 5],
            [-1, -6], [-1, -2.5], [-1, 2.5], [-1, 6]
        )[unum - 1]

    def beam(
            self,
            avoid_center_circle=False
    ) -> None:
        """
        Descrição:
            Realiza o reposicionamento instantâneo ("beam") do agente para sua posição inicial predefinida,
            com opção de evitar o círculo central do campo.

        Parâmetros:
            avoid_center_circle: bool, opcional
                Se True, ajusta a posição inicial para evitar o centro do campo caso esteja muito próxima ao círculo central (padrão: false).

        Comentários:
            - Utilizado tipicamente em situações de reinício de jogo (kickoff, após gol, etc).
            - O agente é enviado para a posição inicial definida em self.init_pos, e orientado para o centro do campo.
            - Se o agente estiver muito próximo do círculo central e avoid_center_circle for True, reposiciona para fora do círculo.
            - Caso o agente já esteja na posição, executa comportamento de espera ("Zero_Bent_Knees_Auto_Head") ou, no modo fat proxy, envia comando especial de proxy.
            - Garante que o estado do agente seja resetado para o valor normal (self.state = 0).
        """

        r = self.world.robot
        pos = self.init_pos[:]  # Faz uma cópia da posição inicial para evitar modificar a original
        self.state = 0  # Reseta o estado do agente (normal)

        # Se solicitado, evita o círculo central reposicionando o agente para trás
        if avoid_center_circle and np.linalg.norm(self.init_pos) < 2.5:
            pos[0] = -2.3

        # Se o agente não está suficientemente próximo da posição-alvo, ou se está caído, envia comando de beam
        if np.linalg.norm(pos - r.loc_head_position[:2]) > 0.1 or self.behavior.is_ready("GetUp"):
            # Realiza o beam para a posição desejada, orientando o agente para o centro do campo (0, 0)
            self.scom.commit_beam(pos, GeneralMath.angle_horizontal_from_vector2D((-pos[0], -pos[1])))
        else:
            if self.fat_proxy_cmd is None:  # Comportamento normal (não proxy)
                self.behavior.execute("Zero_Bent_Knees_Auto_Head")
            else:  # Comportamento especial para fat proxy
                self.fat_proxy_cmd += "(proxy dash 0 0 0)"
                self.fat_proxy_walk = np.zeros(3)  # Reseta o comando de caminhada do fat proxy

    def move(
            self,
            target_2d=(0, 0),
            orientation=None,
            is_orientation_absolute=True,
            avoid_obstacles=True,
            priority_unums=None,
            is_aggressive=False,
            timeout=3000
    ):
        """
        Descrição:
            Move o robô até uma posição alvo no campo, podendo considerar obstáculos e diferentes estratégias de navegação.

        Parâmetros:
            target_2d: array_like, opcional
                Coordenada 2D do alvo em referência absoluta no campo. Padrão: (0,0)
            orientation: float, opcional
                Orientação desejada do torso em graus. Se None, o robô se orienta automaticamente para o alvo.
            is_orientation_absolute: bool, opcional
                Indica se a orientação fornecida é absoluta em relação ao campo (True) ou relativa ao torso do robô (False).
            avoid_obstacles: bool, opcional
                Se True, ativa o desvio de obstáculos com planejamento de caminho. Padrão: True.
            priority_unums: list, opcional
                Lista de números de uniforme de companheiros considerados prioritários a serem evitados no planejamento.
            is_aggressive: bool, opcional
                Se True, reduz as margens de segurança para adversários, permitindo caminhos mais arriscados. Padrão: False.
            timeout: float, opcional
                Tempo máximo (em microssegundos) permitido para o algoritmo de planejamento de caminho. Padrão: 3000.

        Comentários:
            - Se o agente está operando em modo fat proxy, delega o movimento para o método específico, ignorando obstáculos.
            - Quando o desvio de obstáculos está ativado, utiliza o planejador de caminho para determinar a melhor rota até o alvo.
            - Caso contrário, calcula a distância diretamente até o alvo sem considerar obstáculos.
            - Executa o comportamento de caminhada com parâmetros calculados para alcançar o objetivo de forma eficiente.
        """
        
        if priority_unums is None:
            priority_unums = []

        r = self.world.robot

        # Se estiver em modo fat proxy, utiliza função específica e ignora obstáculos
        if self.fat_proxy_cmd is not None:
            self.fat_proxy_move(target_2d, orientation, is_orientation_absolute)
            return

        if avoid_obstacles:
            # Planeja caminho até o alvo, evitando obstáculos e considerando prioridades/agressividade
            target_2d, _, distance_to_final_target = self.path_manager.get_path_to_target(
                target_2d,
                priority_unums=priority_unums,
                is_aggressive=is_aggressive,
                timeout=timeout
            )
        else:
            # Caminho direto até o alvo (sem desvio de obstáculos)
            distance_to_final_target = np.linalg.norm(target_2d - r.loc_head_position[:2])

        # Executa o comportamento de andar até o alvo, com os parâmetros apropriados
        self.behavior.execute("Walk", target_2d, True, orientation, is_orientation_absolute, distance_to_final_target)

    def kick(
            self,
            kick_direction=None,
            kick_distance=None,
            abort=False,
            enable_pass_command=False
    ):
        """
        Descrição:
            Caminha até a bola e executa o chute, podendo abortar a ação ou
             executar um passe conforme a situação.
             
            - Antes de chutar, verifica se há adversário próximo. Se sim e o comando de passe está habilitado, envia o comando de passe.
            - Atualiza direção e distância do chute apenas se novos valores forem fornecidos.
            - Executa o chute básico no modo normal, ou delega para comportamento específico no modo fat proxy.
       

        Parâmetros:
            kick_direction: float, opcional
                Direção do chute em graus, relativa ao campo. Se None, utiliza a direção previamente armazenada.
            kick_distance: float, opcional
                Distância do chute em metros. Se None, utiliza a distância previamente armazenada.
            abort: bool, opcional
                Se True, solicita o aborto da ação de chute. O método retorna True se o aborto foi bem-sucedido,
                ocorrendo imediatamente durante o alinhamento, mas pode ser adiado caso o chute já tenha iniciado.
            enable_pass_command: bool, opcional
                Se True, ativa o comando de passe automático quando há pelo menos um adversário próximo à bola.

        Retorno:
            finished: bool
                Retorna True se o comportamento de chute foi concluído ou abortado com sucesso.
         """

        # Se há adversário próximo e comando de passe está habilitado, envia comando de passe
        if self.min_opponent_ball_dist < 1.45 and enable_pass_command:
            self.scom.commit_pass_command()

        # Atualiza direção e distância do chute apenas se novos parâmetros forem fornecidos
        self.kick_direction = self.kick_direction if kick_direction is None else kick_direction
        self.kick_distance = self.kick_distance if kick_distance is None else kick_distance

        if self.fat_proxy_cmd is None:  # Comportamento normal
            # Executa o comportamento de chute básico (sem controle preciso de distância)
            return self.behavior.execute("BasicKick", self.kick_direction, abort)
        else:  # Comportamento fat proxy
            # Executa o chute via fat proxy
            return self.fat_proxy_kick()

    def think_and_send(self):
        """
        Descrição:
            Função principal de decisão e execução do agente.
            Analisa o estado do mundo e do robô, determina a ação apropriada para o ciclo atual e envia os comandos ao simulador.

            O método executa as seguintes etapas:

            1. Pré-processamento do estado atual:

               - Calcula posições relativas, distâncias e velocidades relevantes (bola, robô, adversários e companheiros).
               - Determina quem é o jogador ativo (mais próximo da bola).
            2. Tomada de decisão:

               - Escolhe a ação com base no modo de jogo (kickoff, corner, play on, etc.), na função do agente e na posse da bola.
               - Executa movimentação, chute, beam ou comportamento de goleiro.
            3. Comunicação:

               - Realiza broadcast de mensagens de rádio para a equipe.
            4. Envio de comandos:

               - Envia comandos de ação para o servidor de simulação.
            5. Anotações gráficas (debug):

               - Adiciona pontos, setas e mensagens para facilitar a depuração visual.

            - O agente alterna entre estados de movimentação, chute e recuperação dependendo de sua função e do contexto do jogo.
            - Utiliza distâncias quadráticas para otimizar comparações de proximidade.
            - Suporta modos especiais como fat proxy.
        """

        w = self.world
        r = self.world.robot
        my_head_pos_2d = r.loc_head_position[:2]
        my_ori = r.imu_torso_orientation
        ball_2d = w.ball_abs_pos[:2]
        ball_vec = ball_2d - my_head_pos_2d
        ball_dir = GeneralMath.angle_horizontal_from_vector2D(ball_vec)
        ball_dist = np.linalg.norm(ball_vec)
        ball_sq_dist = ball_dist * ball_dist  # Distância ao quadrado para comparações rápidas
        ball_speed = np.linalg.norm(w.get_ball_abs_vel(6)[:2])
        behavior = self.behavior
        goal_dir = GeneralMath.target_abs_angle(ball_2d, (15.05, 0))
        path_draw_options = self.path_manager.draw_options
        PM = w.play_mode
        PM_GROUP = w.play_mode_group

        # --------------------------------------- 1. Pré-processamento

        # Posição futura da bola quando sua velocidade for <= 0.5 m/s
        slow_ball_pos = w.get_predicted_ball_pos(0.5)

        # Distância quadrada entre cada companheiro e a bola (desconsidera agentes caídos ou desatualizados)
        teammates_ball_sq_dist = [
            np.sum((p.state_abs_pos[:2] - slow_ball_pos) ** 2)
            if p.state_last_update != 0 and (w.time_local_ms - p.state_last_update <= 360 or p.is_self) and not p.state_fallen
            else 1000
            for p in w.teammates
        ]

        # Distância quadrada entre cada adversário e a bola (desconsidera agentes caídos ou desatualizados)
        opponents_ball_sq_dist = [
            np.sum((p.state_abs_pos[:2] - slow_ball_pos) ** 2)
            if p.state_last_update != 0 and w.time_local_ms - p.state_last_update <= 360 and not p.state_fallen
            else 1000
            for p in w.opponents
        ]

        # Distância mínima (real) entre bola e o companheiro mais próximo e entre bola e o adversário mais próximo
        min_teammate_ball_sq_dist = min(teammates_ball_sq_dist)
        self.min_teammate_ball_dist = math.sqrt(min_teammate_ball_sq_dist)
        self.min_opponent_ball_dist = math.sqrt(min(opponents_ball_sq_dist))

        # Número do jogador ativo (mais próximo da bola)
        active_player_unum = teammates_ball_sq_dist.index(min_teammate_ball_sq_dist) + 1

        # --------------------------------------- 2. Tomada de decisão

        if PM == w.M_GAME_OVER:
            pass  # Sem ação após término do jogo
        elif PM_GROUP == w.MG_ACTIVE_BEAM:
            self.beam()  # Reposiciona para formação inicial
        elif PM_GROUP == w.MG_PASSIVE_BEAM:
            self.beam(True)  # Reposiciona evitando o círculo central
        elif self.state == 1 or (behavior.is_ready("GetUp") and self.fat_proxy_cmd is None):
            # Se caído, tenta levantar
            self.state = 0 if behavior.execute("GetUp") else 1
        elif PM == w.M_OUR_KICKOFF:
            if r.unum == 9:
                self.kick(120, 3)  # Chuta para frente no kickoff
            else:
                self.move(self.init_pos, orientation=ball_dir)  # Caminha para posição inicial
        elif PM == w.M_THEIR_KICKOFF:
            self.move(self.init_pos, orientation=ball_dir)  # Caminha para posição inicial
        elif active_player_unum != r.unum:
            # Se não sou o jogador ativo
            if r.unum == 1:
                self.move(self.init_pos, orientation=ball_dir)  # Goleiro permanece na base
            else:
                # Calcula posição tática baseada na posição da bola
                new_x = max(0.5, (ball_2d[0] + 15) / 15) * (self.init_pos[0] + 15) - 15
                if self.min_teammate_ball_dist < self.min_opponent_ball_dist:
                    new_x = min(new_x + 3.5, 13)  # Avança se o time tem posse
                self.move((new_x, self.init_pos[1]), orientation=ball_dir, priority_unums=[active_player_unum])
        else:
            # Sou o jogador ativo
            path_draw_options(enable_obstacles=True, enable_path=True, use_team_drawing_channel=True)
            enable_pass_command = (PM == w.M_PLAY_ON and ball_2d[0] < 6)

            if r.unum == 1 and PM_GROUP == w.MG_THEIR_KICK:
                self.move(self.init_pos, orientation=ball_dir)  # Goleiro espera durante chute adversário
            if PM == w.M_OUR_CORNER_KICK:
                self.kick(-np.sign(ball_2d[1]) * 95, 5.5)  # Chuta para o espaço à frente do gol adversário
            elif self.min_opponent_ball_dist + 0.5 < self.min_teammate_ball_dist:
                # Defender: adversário está bem mais próximo da bola
                if self.state == 2:
                    self.state = 0 if self.kick(abort=True) else 2
                else:
                    # Move-se para posição entre bola e gol
                    self.move(slow_ball_pos + GeneralMath.normalize_vec((-16, 0) - slow_ball_pos) * 0.2, is_aggressive=True)
            else:
                # Ataca: tenta chutar para o gol
                self.state = 0 if self.kick(goal_dir, 9, False, enable_pass_command) else 2

            path_draw_options(enable_obstacles=False, enable_path=False)

        # --------------------------------------- 3. Broadcast
        self.radio.broadcast()

        # --------------------------------------- 4. Envio de comandos ao servidor
        if self.fat_proxy_cmd is None:
            self.scom.commit_and_send(r.get_command())
        else:
            self.scom.commit_and_send(self.fat_proxy_cmd.encode())
            self.fat_proxy_cmd = ""

        # ---------------------- Anotações para depuração visual
        if self.enable_draw:
            d = w.draw
            if active_player_unum == r.unum:
                d.point(slow_ball_pos, 3, d.Color.pink, "status", False)  # Bola prevista
                d.point(w.ball_2d_pred_pos[-1], 5, d.Color.pink, "status", False)  # Última previsão
                d.annotation((*my_head_pos_2d, 0.6), "I've got it!", d.Color.yellow, "status")
            else:
                d.clear("status")

    # Fat proxy auxiliary methods

    def fat_proxy_kick(self) -> bool:
        """
        Descrição:
            Executa o comando de chute no modo 'fat proxy'.
            
            - Se o robô estiver suficientemente próximo da bola, envia o comando de chute via fat proxy, 
              com potência máxima, ângulo relativo ao torso e ângulo vertical fixo.
            - Caso contrário, move o robô até a posição ideal para chutar, ignorando obstáculos.
        
        Parâmetros:
            None.
            
        Retorno:
            finished: bool
                Retorna True se o chute foi realizado (robô próximo da bola), False caso contrário.
        """
        w = self.world
        r = self.world.robot
        ball_2d = w.ball_abs_pos[:2]
        my_head_pos_2d = r.loc_head_position[:2]

        # Verifica se o robô está próximo o suficiente da bola para chutar
        if np.linalg.norm(ball_2d - my_head_pos_2d) < 0.25:
            # fat proxy kick arguments: power [0,10]; relative horizontal angle [-180,180]; vertical angle [0,70]
            # O chute é direcionado com potência máxima (10), ângulo relativo ao torso e ângulo vertical fixo (20)
            self.fat_proxy_cmd += f"(proxy kick 10 {GeneralMath.normalize_deg(self.kick_direction - r.imu_torso_orientation):.2f} 20)"
            self.fat_proxy_walk = np.zeros(3)  # Reseta parâmetros de caminhada
            return True  # Chute realizado
        else:
            # Se não estiver próximo, move-se até a posição de chute, ignorando obstáculos
            self.fat_proxy_move(ball_2d - (-0.1, 0), None, True)
            return False  # Chute ainda não realizado

    def fat_proxy_move(self, target_2d, orientation, is_orientation_absolute):
        """
        Descrição:
            Executa movimentação no campo usando comandos do modo 'fat proxy'.
            
            - Calcula a distância e direção até o alvo.
            - Se estiver próximo do destino e com pequeno desvio angular, executa rotação fina para ajustar a orientação.
            - Se estiver distante, executa movimento de caminhada em direção ao alvo.
            - Os comandos são concatenados em self.fat_proxy_cmd no formato esperado pelo simulador proxy.
            - Limita o ângulo de rotação para evitar comandos extremos.
        

        Parâmetros:
            target_2d: array_like
                Coordenada 2D alvo para onde o robô deve se mover.
            orientation: float ou None
                Orientação desejada do torso ao final da movimentação. Se None, ignora ajuste de orientação.
            is_orientation_absolute: bool
                Define se a orientação passada é absoluta (True) ou relativa ao torso do robô (False).
        """

        r = self.world.robot

        # Calcula distância e direção relativa ao alvo
        target_dist = np.linalg.norm(target_2d - r.loc_head_position[:2])
        target_dir = GeneralMath.angle_rel_between_target_position(r.loc_head_pos_2d, r.imu_torso_orientation, target_2d)

        # Se ainda distante do alvo e bem orientado, caminha em linha reta (dash máximo)
        if target_dist > 0.1 and abs(target_dir) < 8:
            self.fat_proxy_cmd += f"(proxy dash {100} {0} {0})"
            return

        # Se próximo do alvo, faz ajuste fino de orientação, se necessário
        if target_dist < 0.1:
            if is_orientation_absolute:
                orientation = GeneralMath.normalize_deg(orientation - r.imu_torso_orientation)
            target_dir = np.clip(orientation, -60, 60)
            self.fat_proxy_cmd += f"(proxy dash {0} {0} {target_dir:.1f})"
        else:
            # Caminha em direção ao alvo, aplicando rotação moderada se necessário
            self.fat_proxy_cmd += f"(proxy dash {20} {0} {target_dir:.1f})"
