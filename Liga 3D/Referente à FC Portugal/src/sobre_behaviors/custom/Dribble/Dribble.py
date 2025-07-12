from sobre_behaviors.custom.Dribble.Env import Env
from math_ops.GeneralMath import GeneralMath
from math_ops.NeuralNetwork import run_mlp
import numpy as np
import pickle


class Dribble:

    def __init__(self, base_agent) -> None:
        """
        Descrição:
            Inicializa a classe de drible autônomo baseada em aprendizado por reforço (RL).
    
            Este construtor configura todas as dependências essenciais para o comportamento de drible, 
            incluindo referências ao mundo simulado, gerenciador de caminhos, módulo de comportamentos, 
            ambiente de observação, e o modelo de política treinado específico para o tipo de robô.

        Parâmetros:
            base_agent: Base_Agent
                Instância do agente base, usada para acessar módulos de comportamento, caminho, mundo e especificações do robô.
    
        Atributos Definidos:
            behavior: object
                Gerenciador de comportamentos do agente (usado para composições de ações complexas).
            path_manager: object
                Gerenciador de caminhos e trajetórias, útil para movimentações orientadas por objetivos.
            world: object
                Ambiente simulado, incluindo referência ao robô e sensores.
            description: str
                Descrição textual do comportamento ("RL dribble").
            auto_head: bool
                Ativa/desativa o controle automático da cabeça durante o drible (True por padrão).
            env: Env
                Instância do ambiente de observação, parametrizada conforme o tipo de robô para ajustar o step_width do passo.
            model: object
                Política de rede neural carregada via pickle, treinada especificamente para o tipo do robô em uso.
        """

        self.phase = None
        self.approach_orientation = None
        self.behavior = base_agent.behavior
        self.path_manager = base_agent.path_manager
        self.world = base_agent.world
        self.description = "Dribble Reinforcement Learning"
        self.auto_head = True

        # Define o step_width do ambiente conforme o tipo do robô
        self.env = Env(base_agent, 0.9 if self.world.robot.type == 3 else 1.2)

        # Carrega o modelo RL adequado ao tipo do robô (um arquivo para cada tipo)
        with open(GeneralMath.obter_diretorio_ativo([
                                             "/sobre_behaviors/custom/Dribble/dribble_R0.pkl",
                                             "/sobre_behaviors/custom/Dribble/dribble_R1.pkl",
                                             "/sobre_behaviors/custom/Dribble/dribble_R2.pkl",
                                             "/sobre_behaviors/custom/Dribble/dribble_R3.pkl",
                                             "/sobre_behaviors/custom/Dribble/dribble_R4.pkl"
                                         ][self.world.robot.type]), 'rb') as f:
            self.model = pickle.load(f)

    def define_approach_orientation(self) -> None:
        """
        Descrição:
            Determina e ajusta a orientação ideal de aproximação à bola, levando em conta a posição da bola no campo
            e as margens de segurança próximas às linhas laterais e de fundo.
    
            O método calcula uma faixa angular aceitável para a aproximação à bola, evitando que o robô aproxime-se
            de ângulos perigosos, especialmente quando a bola está próxima das bordas do campo. Caso a orientação atual
            de aproximação esteja fora da faixa permitida, o método ajusta `self.approach_orientation` para o valor
            mais próximo dentro da faixa segura.

            - As margens de segurança são usadas para evitar aproximações muito próximas às linhas do campo, reduzindo
              riscos de sair da área de jogo.
            - O método utiliza diferentes regras para quadrantes e regiões do campo, ajustando dinamicamente o intervalo
              de ângulos aceitáveis.
            - Se a orientação atual já estiver dentro do intervalo permitido, nenhuma alteração é feita.
            - Caso contrário, a orientação é forçada para o limite mais próximo da faixa segura.

        Parâmetros: 
            None
            
        Retorno:
            approach_orientation: float ou None
                Define a orientação de aproximação sugerida (em graus). Se None, a orientação atual é aceita.
        """
        w = self.world
        b = w.ball_abs_pos[:2]  # Posição da bola no campo (x, y)
        me = w.robot.loc_head_position[:2]  # Posição do robô no campo (x, y)

        self.approach_orientation = None  # Reset da orientação sugerida

        # Constantes do campo e margens de segurança
        MARGIN = 0.8  # Margem de segurança (m)
        M90 = 90 / MARGIN  # Auxiliar para cálculo de ângulos
        DEV = 25  # Desvio aplicado ao se aproximar das linhas
        MDEV = (90 + DEV) / MARGIN  # Auxiliar para desvio

        # Inicializa faixa angular completa (-180 a 180 graus)
        a1 = -180
        a2 = 180

        # Ajusta faixa angular de aproximação conforme a posição da bola no campo
        if b[1] < -10 + MARGIN:  # Perto da linha de fundo inferior
            if b[0] < -15 + MARGIN:  # Canto inferior esquerdo
                a1 = DEV - M90 * (b[1] + 10)
                a2 = 90 - DEV + M90 * (b[0] + 15)
            elif b[0] > 15 - MARGIN:  # Canto inferior direito
                a1 = 90 + DEV - M90 * (15 - b[0])
                a2 = 180 - DEV + M90 * (b[1] + 10)
            else:  # Próximo à linha de fundo, mas não nos cantos
                a1 = DEV - MDEV * (b[1] + 10)
                a2 = 180 - DEV + MDEV * (b[1] + 10)
        elif b[1] > 10 - MARGIN:  # Perto da linha de fundo superior
            if b[0] < -15 + MARGIN:  # Canto superior esquerdo
                a1 = -90 + DEV - M90 * (b[0] + 15)
                a2 = -DEV + M90 * (10 - b[1])
            elif b[0] > 15 - MARGIN:  # Canto superior direito
                a1 = 180 + DEV - M90 * (10 - b[1])
                a2 = 270 - DEV + M90 * (15 - b[0])
            else:  # Próximo à linha de fundo, mas não nos cantos
                a1 = -180 + DEV - MDEV * (10 - b[1])
                a2 = -DEV + MDEV * (10 - b[1])
        elif b[0] < -15 + MARGIN:  # Próximo à linha lateral esquerda
            a1 = -90 + DEV - MDEV * (b[0] + 15)
            a2 = 90 - DEV + MDEV * (b[0] + 15)
        elif b[0] > 15 - MARGIN and abs(b[1]) > 1.2:  # Próximo à linha lateral direita (mas não no centro)
            a1 = 90 + DEV - MDEV * (15 - b[0])
            a2 = 270 - DEV + MDEV * (15 - b[0])

        # Calcula direção atual de aproximação (ângulo entre robô e bola)
        cad = GeneralMath.angle_horizontal_from_vector2D(b - me)

        # Normaliza ângulos para a faixa [-180, 180]
        a1 = GeneralMath.normalize_deg(a1)
        a2 = GeneralMath.normalize_deg(a2)

        # Verifica se a direção atual está dentro do intervalo permitido
        if a1 < a2:
            if a1 <= cad <= a2:
                return  # Orientação atual já é aceitável
        else:
            if a1 <= cad or cad <= a2:
                return

        # Caso fora do intervalo, escolhe o limite mais próximo
        a1_diff = abs(GeneralMath.normalize_deg(a1 - cad))
        a2_diff = abs(GeneralMath.normalize_deg(a2 - cad))
        self.approach_orientation = a1 if a1_diff < a2_diff else a2  # Define orientação corrigida

    def execute(self, reset: bool, orientation: float | None, is_orientation_absolute: bool, speed: float = 1, stop=False):
        """
        Descrição:
            Executa o comportamento de drible autônomo, controlando transições entre caminhar até a bola,
            iniciar o drible e realizar a finalização do movimento. O método faz a seleção dinâmica das estratégias
            de aproximação, determina a orientação desejada, ajusta parâmetros de velocidade e utiliza uma política
            treinada por aprendizado de máquina para gerar comandos de ação apropriados em cada etapa.

            - O método alterna automaticamente entre caminhar até a bola, iniciar e finalizar o drible, conforme a posição da bola e o estado interno.
            - Utiliza política de rede neural para gerar as ações de drible, observando o estado do robô e do campo.
            - Inclui mecanismo de "wind-down" para finalizar o drible de forma suave antes de resetar o comportamento.
            - A orientação de aproximação é ajustada em situações de bola próxima às linhas de fundo ou laterais, para evitar saídas de campo.

        Parâmetros:
            reset: bool
                Reinicia o estado interno do comportamento, útil para início de novos episódios ou após interrupções.
            orientation: float ou None
                Orientação desejada do torso durante o drible, em graus. Se None, o robô dribla automaticamente em direção ao gol adversário.
            is_orientation_absolute: bool
                Indica se o valor de `orientation` é absoluto (referenciado ao campo) ou relativo ao torso do robô.
            speed: float, opcional
                Velocidade de drible (0 a 1), padrão é 1. O efeito não é linear.
            stop: bool, opcional
                Se True, interrompe o comportamento imediatamente se estiver caminhando ou finaliza o drible assim que possível.

        Retorno:
            bool
                Retorna True quando o comportamento de drible foi finalizado ou interrompido;
                False caso o drible ainda esteja em execução.
         """

        w = self.world
        r = self.world.robot
        me = r.loc_head_position[:2]
        b = w.ball_abs_pos[:2]
        b_rel = w.ball_rel_torso_cart_pos[:2]
        b_dist = np.linalg.norm(b - me)
        behavior = self.behavior
        reset_dribble = False
        # Considera a bola "perdida" se não foi vista recentemente ou está muito longe do robô
        lost_ball = (w.ball_last_seen <= w.time_local_ms - w.VISUALSTEP_MS) or np.linalg.norm(b_rel) > 0.4

        if reset:
            self.phase = 0
            # Se comportamento anterior era drible e a bola está muito próxima, já inicia na fase de drible
            if behavior.previous_behavior == "Push_RL" and 0 < b_rel[0] < 0.25 and abs(b_rel[1]) < 0.07:
                self.phase = 1
                reset_dribble = True

        if self.phase == 0:  # Caminhar até a bola
            reset_walk = reset and behavior.previous_behavior not in ["Walk", "Push_RL"]  # Só reseta o caminhar se não era o anterior

            # 1. Decide se precisa de uma orientação de abordagem especial (bola próxima das linhas)
            if reset or b_dist > 0.4:
                self.define_approach_orientation()

            # 2A. Se precisa de melhor orientação de abordagem (bola quase fora)
            if self.approach_orientation is not None:
                next_pos, next_ori, dist_to_final_target = self.path_manager.get_path_to_ball(
                    x_ori=self.approach_orientation, x_dev=-0.24, torso_ori=self.approach_orientation, safety_margin=0.4
                )

                # Pronto para iniciar o drible
                if 0.18 < b_rel[0] < 0.26 and abs(b_rel[1]) < 0.04 and w.ball_is_visible:
                    self.phase += 1
                    reset_dribble = True
                else:
                    dist = max(0.08, dist_to_final_target * 0.7)
                    behavior.execute_sub_behavior("Walk", reset_walk, next_pos, True, next_ori, True, dist)

            # 2B. Bola não visível, caminha para último alvo conhecido
            elif w.time_local_ms - w.ball_last_seen > 200:
                abs_ori = GeneralMath.angle_horizontal_from_vector2D(b - me)
                behavior.execute_sub_behavior("Walk", reset_walk, b, True, abs_ori, True, None)

            # 2C. Bola visível, caminha para posição relativa ao robô
            else:
                if 0.18 < b_rel[0] < 0.25 and abs(b_rel[1]) < 0.05 and w.ball_is_visible:
                    self.phase += 1
                    reset_dribble = True
                else:
                    rel_target = b_rel + (-0.23, 0)  # alvo relativo é um círculo ao redor da bola
                    rel_ori = GeneralMath.angle_horizontal_from_vector2D(b_rel)  # orientação em relação à bola
                    dist = max(0.08, np.linalg.norm(rel_target) * 0.7)
                    behavior.execute_sub_behavior("Walk", reset_walk, rel_target, False, rel_ori, False, dist)

            if stop:
                return True  # Interrompe imediatamente se solicitado

        # Fase de drible propriamente dita
        if self.phase == 1 and (stop or (b_dist > 0.5 and lost_ball)):
            self.phase += 1  # Entra em wind-down do drible
        elif self.phase == 1:
            # 1. Define parâmetros do drible
            self.env.dribble_speed = speed

            # 2. Determina orientação do drible
            if orientation is None:
                if b[0] < 0:  # Bola à esquerda, dribla para as laterais
                    dribble_target = (15, 5) if b[1] > 0 else (15, -5)
                else:
                    dribble_target = None  # Dribla para o gol
                self.env.dribble_rel_orientation = self.path_manager.get_dribble_path(optional_2d_target=dribble_target)[1]
            elif is_orientation_absolute:
                self.env.dribble_rel_orientation = GeneralMath.normalize_deg(orientation - r.imu_torso_orientation)
            else:
                self.env.dribble_rel_orientation = float(orientation)  # Orientação relativa

            # 3. Executa ação de drible via política RL
            obs = self.env.observe(reset_dribble)
            action = run_mlp(obs, self.model)
            self.env.execute(action)

        # Fase de wind-down para finalizar o drible suavemente
        if self.phase > 1:
            WIND_DOWN_STEPS = 60
            self.env.dribble_speed = 1 - self.phase / WIND_DOWN_STEPS
            self.env.dribble_rel_orientation = 0

            obs = self.env.observe(reset_dribble, virtual_ball=True)
            action = run_mlp(obs, self.model)
            self.env.execute(action)

            self.phase += 1
            if self.phase >= WIND_DOWN_STEPS - 5:
                self.phase = 0
                return True

        return False
    
    def is_ready(self):
        """ Returns True if this behavior is ready to start/continue under current game/robot conditions """
        return True
