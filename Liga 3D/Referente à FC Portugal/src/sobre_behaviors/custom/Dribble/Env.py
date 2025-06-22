from sobre_behaviors.custom.Step.Step_Generator import Step_Generator
from math_ops.GeneralMath import GeneralMath
import math
import numpy as np


class Env:
    """
    Descrição:
        Classe de ambiente auxiliar para comportamentos de locomoção de robôs humanoides.

        A classe Env encapsula informações e parâmetros necessários para gerar passos e executar comportamentos
        de caminhada e manipulação do robô. Ela centraliza o espaço de observação, configurações de passo,
        especificações de cinemática e controle de membros, facilitando a reutilização em diferentes módulos de controle.
    """

    def __init__(
            self,
            base_agent,
            step_width
    ) -> None:
        """
        Descrição:
            Inicializa o ambiente de locomoção, configurando parâmetros de observação, passo e cinemática.

        Parâmetros:
            base_agent: BaseAgent
                Instância do agente base, usada para acessar o mundo simulado e o módulo de cinemática inversa (IK).

            step_width: float
                Fator multiplicativo para a largura do passo em relação ao padrão do robô.

        Atributos Definidos
            world: object
                Referência ao ambiente simulado do agente.
            ik: object
                Referência ao módulo de cinemática inversa.
            obs: np.ndarray
                Vetor de observação do ambiente, usado como entrada para modelos de controle.
            STEP_DUR: int
                Duração padrão de cada passo (ciclos de controle).
            STEP_Z_SPAN: float
                Amplitude vertical padrão do movimento do pé durante o passo.
            STEP_Z_MAX: float
                Fator máximo de altura do pé em relação ao comprimento da perna.
            leg_length: float
                Comprimento total da perna do robô (coxa + canela).
            step_generator: Step_Generator
                Instância responsável por gerar as trajetórias dos passos.
            DEFAULT_ARMS: np.ndarray
                Vetor com valores padrão para as juntas dos braços.
            dribble_rel_orientation: float ou None
                Orientação relativa para o comportamento de drible (em graus).
            dribble_speed: float
                Velocidade padrão para o comportamento de drible.
        """
        self.values_r = None
        self.values_l = None
        self.gym_last_internal_abs_ori = None
        self.internal_target_vel = None
        self.internal_rel_orientation = None
        self.act = None
        self.step_counter = None
        self.world = base_agent.world
        self.ik = base_agent.inv_kinematics

        # Espaço de observação (exemplo: posições articulares, sensores, etc.)
        self.obs = np.zeros(76, np.float32)

        # Parâmetros padrões do comportamento de passo
        self.STEP_DUR = 8  # Duração padrão do passo (em ciclos)
        self.STEP_Z_SPAN = 0.02  # Amplitude vertical padrão dos passos
        self.STEP_Z_MAX = 0.70  # Fator máximo de altura do pé (relativo à perna)

        # Parâmetros de cinemática inversa e passo
        r = self.world.robot
        nao_specs = self.ik.NAO_SPECS
        self.leg_length = nao_specs[1] + nao_specs[3]  # Comprimento total da perna

        # Calcula a largura do passo de acordo com o fator step_width
        feet_y_dev = nao_specs[0] * step_width  # Largura lateral dos pés
        sample_time = r.STEPTIME  # Tempo de amostragem do passo
        max_ankle_z = nao_specs[5]  # Altura máxima do tornozelo durante o passo

        # Inicializa o gerador de passos
        self.step_generator = Step_Generator(feet_y_dev, sample_time, max_ankle_z)

        # Valores padrão para as juntas dos braços (posição de repouso ou neutra)
        self.DEFAULT_ARMS = np.array([-90, -90, 8, 8, 90, 90, 70, 70], np.float32)

        # Parâmetros auxiliares para comportamentos de drible
        self.dribble_rel_orientation = None  # Orientação de drible em relação ao torso
        self.dribble_speed = 1  # Velocidade padrão de drible

    def observe(self, init=False, virtual_ball=False) -> np.ndarray:
        """
        Descrição:
            Atualiza e retorna o vetor de observação do ambiente, reunindo informações sensoriais,
            estados internos do robô e dados da bola (caso aplicável) para uso em modelos de controle
            ou aprendizado por reforço.

            - O vetor de observação é normalizado de forma ingênua para manter as entradas em faixas adequadas para redes neurais.
            - Inclui dados do torso, pernas, braços, sensores inerciais, forças nos pés, progresso do passo e estado do controle de caminhada.
            - Inclui também a posição e velocidade relativas da bola, tanto real quanto virtual.
            - Parâmetros internos de rotação e direção são suavizados para facilitar a aprendizagem.
        Parâmetros:
            init: bool, opcional
                Se True, reseta variáveis internas e define observações iniciais (útil no início de um episódio).
            virtual_ball : bool, opcional
                Se True, simula a presença da bola entre os pés do robô, sobrescrevendo parte da observação.

        Retorno:
            obs: np.ndarray
                Vetor de observação atualizado, contendo estados do robô, do passo, dos membros, sensores inerciais,
                informações sobre os pés, e dados da bola.
        """

        w = self.world
        r = self.world.robot

        if init:  # Reset de variáveis internas de controle e memória
            self.step_counter = 0
            self.act = np.zeros(16, np.float32)  # Variável de memória para ações anteriores

        # ---------------- Observações do torso e sensores inerciais --------------------
        self.obs[0] = min(self.step_counter, 12 * 8) / 100  # Contador de passos (limitado)
        self.obs[1] = r.loc_head_z * 3  # Altura do torso (normalizada)
        self.obs[2] = r.loc_head_z_vel / 2  # Velocidade vertical do torso
        self.obs[3] = r.imu_torso_roll / 15  # Inclinação lateral do torso (roll)
        self.obs[4] = r.imu_torso_pitch / 15  # Inclinação frontal do torso (pitch)
        self.obs[5:8] = r.gyro / 100  # Giroscópio (normalizado)
        self.obs[8:11] = r.acc / 10  # Acelerômetro (normalizado)

        # ---------------- Forças e posições nos pés --------------------
        # Esquerdo: posição relativa e vetor de força (p, f)
        self.obs[11:17] = r.frp.get('lf', np.zeros(6)) * (10, 10, 10, 0.01, 0.01, 0.01)
        # Direito: posição relativa e vetor de força (p, f)
        self.obs[17:23] = r.frp.get('rf', np.zeros(6)) * (10, 10, 10, 0.01, 0.01, 0.01)
        # Se o pé não estiver no chão, todos os valores são zero

        # ---------------- Juntas: posição e velocidade -----------------
        self.obs[23:43] = r.joints_position[2:22] / 100  # Posições das juntas (exceto cabeça e pés)
        self.obs[43:63] = r.joints_speed[2:22] / 6.1395  # Velocidades das juntas (escala baseada no máximo possível)

        '''
        Observações esperadas para estados de caminhar:
        Time step        R  0   1   2   3   4   5   6   7   0
        Progress         1  0 .14 .28 .43 .57 .71 .86   1   0
        Left leg active  T  F   F   F   F   F   F   F   F   T
        '''

        # ---------------- Estado do passo -----------------------------
        if init:
            self.obs[63] = 1  # Progresso do passo
            self.obs[64] = 1  # Perna esquerda ativa
            self.obs[65] = 0  # Perna direita ativa
            self.obs[66] = 0
        else:
            self.obs[63] = self.step_generator.external_progress  # Progresso do passo (0..1)
            self.obs[64] = float(self.step_generator.state_is_left_active)  # 1 se a perna esquerda está ativa
            self.obs[65] = float(not self.step_generator.state_is_left_active)  # 1 se a perna direita está ativa
            self.obs[66] = math.sin(self.step_generator.state_current_ts /
                                    self.step_generator.ts_per_step * math.pi)  # Seno do progresso do passo

        # ---------------- Observação da bola --------------------------
        ball_rel_hip_center = self.ik.torso_to_hip_transform(w.ball_rel_torso_cart_pos)
        ball_dist_hip_center = np.linalg.norm(ball_rel_hip_center)

        if init:
            self.obs[67:70] = (0, 0, 0)  # Velocidade inicial da bola é zero
        elif w.ball_is_visible:
            self.obs[67:70] = (ball_rel_hip_center - self.obs[70:73]) * 10  # Velocidade da bola relativa ao centro dos tornozelos

        self.obs[70:73] = ball_rel_hip_center  # Posição da bola relativa ao quadril
        self.obs[73] = ball_dist_hip_center * 2  # Distância da bola (normalizada)

        if virtual_ball:
            # Simula a bola entre os pés do robô (útil para testes sem bola real)
            self.obs[67:74] = (0, 0, 0, 0.05, 0, -0.175, 0.36)

        # ---------------- Parâmetros internos de rotação e alvo ------------------
        MAX_ROTATION_DIFF = 20  # Máxima diferença de rotação por passo (graus)
        MAX_ROTATION_DIST = 80  # Máxima distância de rotação permitida (graus)

        if init:
            self.internal_rel_orientation = 0
            self.internal_target_vel = 0
            self.gym_last_internal_abs_ori = r.imu_torso_orientation  # Para fins de recompensa em RL

        # ---------------- Atualização do alvo interno de rotação -----------------
        if w.vision_is_up_to_date:
            previous_internal_rel_orientation = np.copy(self.internal_rel_orientation)

            # Calcula diferença entre orientação desejada e atual (limitada para suavizar)
            internal_ori_diff = np.clip(
                GeneralMath.normalize_deg(self.dribble_rel_orientation - self.internal_rel_orientation),
                -MAX_ROTATION_DIFF, MAX_ROTATION_DIFF
            )
            self.internal_rel_orientation = np.clip(
                GeneralMath.normalize_deg(self.internal_rel_orientation + internal_ori_diff),
                -MAX_ROTATION_DIST, MAX_ROTATION_DIST
            )

            # Observação: velocidade de ajuste do alvo interno de rotação
            self.internal_target_vel = self.internal_rel_orientation - previous_internal_rel_orientation

            # Observação: orientação absoluta em relação ao torso (para avaliação/recompensa)
            self.gym_last_internal_abs_ori = self.internal_rel_orientation + r.imu_torso_orientation

        # ---------------- Observações finais: orientação e velocidade alvo -------
        self.obs[74] = self.internal_rel_orientation / MAX_ROTATION_DIST
        self.obs[75] = self.internal_target_vel / MAX_ROTATION_DIFF

        return self.obs

    def execute_ik(self, l_pos, l_rot, r_pos, r_rot) -> None:
        """
        Descrição:
            Executa a cinemática inversa (IK) para ambas as pernas do robô e envia os comandos diretamente às juntas correspondentes.

            - O método calcula os ângulos das juntas das pernas usando IK e define diretamente os alvos das juntas.
            - O parâmetro `harmonize=False` é usado para garantir resposta imediata, já que os alvos podem variar rapidamente.
            - Erros de IK (ex: limites de junta) não são tratados explicitamente aqui, mas podem ser monitorados através de `error_codes`.

        Parâmetros:
            l_pos : tuple
                Posição desejada do tornozelo da perna esquerda (x, y, z).
            l_rot : array-like
                Orientação desejada do pé esquerdo (roll, pitch, yaw) ou outra representação esperada pelo IK.
            r_pos : tuple
                Posição desejada do tornozelo da perna direita (x, y, z).
            r_rot : array-like
                Orientação desejada do pé direito (roll, pitch, yaw) ou outra representação esperada pelo IK.
        """
        r = self.world.robot
        # Aplica IK na perna esquerda e envia comandos às juntas
        indices, self.values_l, error_codes = self.ik.leg(l_pos, l_rot, True, dynamic_pose=False)
        r.set_joints_target_position_direct(indices, self.values_l, harmonize=False)

        # Aplica IK na perna direita e envia comandos às juntas
        indices, self.values_r, error_codes = self.ik.leg(r_pos, r_rot, False, dynamic_pose=False)
        r.set_joints_target_position_direct(indices, self.values_r, harmonize=False)

    def execute(self, action):
        """
        Descrição:
            Processa um vetor de ação, converte em comandos de movimento para pernas e braços, e os executa no robô.
            
             - Utiliza média móvel exponencial para suavizar as ações e evitar movimentos abruptos.
            - Extrai os alvos dos tornozelos e pés a partir do vetor de ação, aplicando normalização e viés conforme necessário.
            - Limita as rotações de yaw/pitch dos pés para garantir segurança e postura adequada.
            - Os braços recebem incrementos baseados na ação, partindo de uma pose padrão.
            - O método executa a cinemática inversa para as pernas e define as posições alvo dos braços.
            - Incrementa o contador de passos a cada chamada.
        
        Parâmetros:
            action: array-like
                Vetor de ações, normalmente saída de uma política de aprendizado por reforço, contendo:
                - 0,1,2: posição do tornozelo esquerdo
                - 3,4,5: posição do tornozelo direito
                - 6,7,8: rotação do pé esquerdo (roll, pitch, yaw)
                - 9,10,11: rotação do pé direito (roll, pitch, yaw)
                - 12,13: pitch dos braços esquerdo/direito
                - 14,15: roll dos braços esquerdo/direito
       """

        r = self.world.robot

        # Comentário: vetor de ações segue convenção específica para membros do robô

        # Média móvel exponencial para suavizar as ações
        self.act = 0.85 * self.act + 0.15 * action * 0.7 * 0.95 * self.dribble_speed

        # Executa o comportamento Step para obter as posições alvo padrão dos tornozelos (serão sobrescritas)
        lfy, lfz, rfy, rfz = self.step_generator.get_target_positions(
            self.step_counter == 0, self.STEP_DUR, self.STEP_Z_SPAN, self.leg_length * self.STEP_Z_MAX
        )

        # Cálculo das posições e rotações dos tornozelos com normalização e viés
        a = self.act
        l_ankle_pos = (a[0] * 0.025 - 0.01, a[1] * 0.01 + lfy, a[2] * 0.01 + lfz)
        r_ankle_pos = (a[3] * 0.025 - 0.01, a[4] * 0.01 + rfy, a[5] * 0.01 + rfz)
        l_foot_rot = a[6:9] * (2, 2, 3)
        r_foot_rot = a[9:12] * (2, 2, 3)

        # Limita o yaw dos pés: esquerdo só positivo, direito só negativo, aplicando viés correspondente
        l_foot_rot[2] = max(0.0, l_foot_rot[2] + 18.3)
        r_foot_rot[2] = min(0.0, r_foot_rot[2] - 18.3)

        # Atualiza a pose dos braços partindo de uma pose padrão
        arms = np.copy(self.DEFAULT_ARMS)
        arms[0:4] += a[12:16] * 4

        # Executa IK para as pernas e define alvos dos braços
        self.execute_ik(l_ankle_pos, l_foot_rot, r_ankle_pos, r_foot_rot)
        r.set_joints_target_position_direct(slice(14, 22), arms, harmonize=False)

        # Incrementa o contador de passos
        self.step_counter += 1






