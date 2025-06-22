from behaviors.custom.Step.Step_Generator import Step_Generator
from math_ops.GeneralMath import GeneralMath


class BasicKick:

    def __init__(self, base_agent) -> None:
        """
        Descrição:
            Inicializa o comportamento de caminhar até a bola e realizar um chute básico.
            Define os principais componentes, parâmetros de alinhamento, limites da área de chute e ajustes
            específicos para o tipo de robô, preparando o ambiente para a execução do comportamento.

            - Os limites da área de chute e os ajustes de direção são definidos conforme o tipo do robô, permitindo especialização do comportamento.
            - Parâmetros como "bias_dir" corrigem desvios típicos do chute para diferentes modelos.
            - As variáveis centrais (centro da área de chute) são calculadas para facilitar alinhamentos posteriores.

        Parâmetros:
            base_agent: Base_Agent
                Instância do agente base, usada para acessar gerenciadores de comportamento, caminho e o mundo simulado.

        Retorno:
            None
        """
        self.reset_time = None
        self.phase = None
        self.behavior = base_agent.behavior
        self.path_manager = base_agent.path_manager
        self.world = base_agent.world
        self.description = "Andar até bola e chutá-la."
        self.auto_head = True

        # Ajustes dependentes do tipo do robô para compensar particularidades mecânicas
        r_type = self.world.robot.type
        self.bias_dir = [22, 29, 26, 29, 22][r_type]
        self.ball_x_limits = ((0.19, 0.215), (0.2, 0.22), (0.19, 0.22), (0.2, 0.215), (0.2, 0.215))[r_type]
        self.ball_y_limits = ((-0.115, -0.1), (-0.125, -0.095), (-0.12, -0.1), (-0.13, -0.105), (-0.09, -0.06))[r_type]
        self.ball_x_center = (self.ball_x_limits[0] + self.ball_x_limits[1]) / 2
        self.ball_y_center = (self.ball_y_limits[0] + self.ball_y_limits[1]) / 2

    def execute(self, reset, direction, abort=False) -> bool:
        """
        Descrição:
            Executa o comportamento de caminhar até a bola e realizar o chute, controlando as transições entre
            alinhamento, aproximação e execução do chute. O método também permite abortar o comportamento de
            forma segura, especialmente durante a fase de alinhamento.

            - Aplica um viés à direção do chute para compensar desvios mecânicos, conforme o tipo do robô.
            - O comportamento só avança para o chute se a bola estiver visível, posicionada corretamente e o robô estiver alinhado.
            - Utiliza um gerenciador de caminhos para planejar a aproximação e alinhamento ao alvo antes de chutar.
            - O parâmetro "abort" permite a interrupção segura do comportamento, respeitando o estado atual.

        Parâmetros
            reset: bool
                Reinicia o estado interno do comportamento, útil para novos episódios ou transições de tarefa.
            direction: float
                Direção desejada do chute, relativa ao campo, em graus.
            abort: bool, opcional
                Se True, aborta o comportamento. O aborto é imediato na fase de alinhamento, mas é aguardado
                até a conclusão se o chute já estiver em andamento.

        Retorno:
            bool
                Retorna True se o chute foi executado ou abortado com sucesso;
                retorna False se o comportamento ainda está em execução ou alinhamento.
       """

        w = self.world
        r = self.world.robot
        b = w.ball_rel_torso_cart_pos
        t = w.time_local_ms
        gait: Step_Generator = self.behavior.get_custom_behavior_object("Walk").env.step_generator

        if reset:
            self.phase = 0
            self.reset_time = t

        if self.phase == 0:
            # Adiciona viés de direção para corrigir imperfeições de hardware
            biased_dir = GeneralMath.normalize_deg(direction + self.bias_dir)
            # Diferença angular entre orientação desejada e atual
            ang_diff = abs(GeneralMath.normalize_deg(biased_dir - r.loc_torso_orientation))

            # Calcula posição e orientação-alvo para aproximação
            next_pos, next_ori, dist_to_final_target = self.path_manager.get_path_to_ball(
                x_ori=biased_dir, x_dev=-self.ball_x_center, y_dev=-self.ball_y_center, torso_ori=biased_dir)

            # Verifica se todas as condições para o chute estão atendidas
            if (w.ball_last_seen > t - w.VISUALSTEP_MS and ang_diff < 5
                    and self.ball_x_limits[0] < b[0] < self.ball_x_limits[1]
                    and self.ball_y_limits[0] < b[1] < self.ball_y_limits[1]
                    and t - w.ball_abs_pos_last_update < 100
                    and dist_to_final_target < 0.03
                    and not gait.state_is_left_active and gait.state_current_ts == 2
                    and t - self.reset_time > 500):
                # Avança para a fase de chute
                self.phase += 1
                # Executa o chute (comando pode ser customizado no Kick_Motion)
                return self.behavior.execute_sub_behavior("Kick_Motion", True)
            else:
                # Continua alinhando e aproximando
                dist = max(0.07, dist_to_final_target)
                reset_walk = reset and self.behavior.previous_behavior != "Walk"
                self.behavior.execute_sub_behavior("Walk", reset_walk, next_pos, True, next_ori, True, dist)
                return abort  # Permite aborto imediato durante o alinhamento

        else:
            # Fase de execução do chute (não pode ser abortada imediatamente)
            return self.behavior.execute_sub_behavior("Kick_Motion", False)

    def is_ready(self) -> any:  # You can add more arguments 
        """ Returns True if this behavior is ready to start/continue under current game/robot conditions """
        return True
    