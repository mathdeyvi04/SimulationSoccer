from behaviors.custom.Step.Step_Generator import Step_Generator
from math_ops.GeneralMath import GeneralMath
import math
import numpy as np


class Env:
    """
    Descrição:
        Focada em como o robô lidará com o ambiente.

    Métodos Disponíveis:
        - observe
        - execute_ik
        - execute
    """
    
    def __init__(
            self,
            base_agent
    ) -> None:
        """
        Descrição:
            Inicializa o ambiente de locomoção e controle do agente, configurando parâmetros de espaço de estados,
            cinemática inversa e geração de passos para o robô.

            - O ambiente é responsável por encapsular a lógica de geração de passos, espaço de observação e parâmetros de locomoção.
            - Os parâmetros e atributos permitem fácil ajuste dos padrões de caminhada e integração com módulos de RL/controle.


        Parâmetros:
            base_agent: Base_Agent
                Instância do agente base, que fornece referências ao mundo (ambiente do robô) e à cinemática inversa.

        Atributos Definidos:
            world: World
                Referência ao ambiente/mundo do agente, herdada do agente base.
            ik: Inv_Kinematics
                Instância de cinemática inversa, usada para controle preciso das juntas dos membros.
            obs: np.array
                Vetor do espaço de observação/estado do ambiente (tamanho 63, tipo float32), inicializado em zeros.
            STEP_DUR: int
                Duração padrão de um passo (em unidades de tempo discretas).
            STEP_Z_SPAN: float
                Amplitude vertical padrão do passo (altura máxima do pé durante o passo).
            STEP_Z_MAX: float
                Altura máxima permitida para o pé durante a trajetória do passo.
            leg_length: float
                Comprimento total da perna (coxa + canela), baseado nas especificações do robô NAO.
            step_generator: Step_Generator
                Instância do gerador de trajetórias de passos, configurado conforme especificações do robô.
            DEFAULT_ARMS: np.array
                Array com as posições padrão dos braços durante a caminhada.
            walk_rel_orientation: qualquer
                Variável para armazenar orientação relativa do alvo de caminhada (inicialmente None).
            walk_rel_target: qualquer
                Variável para armazenar alvo relativo de caminhada (inicialmente None).
            walk_distance: qualquer
                Distância a ser percorrida na caminhada (inicialmente None).
        """

        self.values_r = None
        self.values_l = None
        self.internal_rel_orientation = None
        self.internal_target = None
        self.step_counter = None
        self.act = None
        self.world = base_agent.world  # Referência ao ambiente/mundo do robô
        self.ik = base_agent.inv_kinematics  # Kinemática inversa do robô

        # Espaço de estado (observação): inicializa com zeros
        self.obs = np.zeros(63, np.float32)

        # Parâmetros padrão para o comportamento de passo
        self.STEP_DUR = 8  # Duração de um passo (em ciclos ou frames)
        self.STEP_Z_SPAN = 0.02  # Amplitude vertical padrão do passo
        self.STEP_Z_MAX = 0.70  # Altura máxima permitida do pé durante o passo

        # Obtenção de parâmetros do robô para cálculo do comprimento da perna e ajuste de passada
        nao_specs = self.ik.NAO_SPECS
        self.leg_length = nao_specs[1] + nao_specs[3]  # Comprimento da perna: coxa + canela
        feet_y_dev = nao_specs[0] * 1.12  # Distância lateral do pé para passo mais largo
        sample_time = self.world.robot.STEPTIME  # Tempo de amostragem do passo
        max_ankle_z = nao_specs[5]  # Altura máxima permitida do tornozelo

        # Inicializa o gerador de passos com parâmetros ajustados
        self.step_generator = Step_Generator(feet_y_dev, sample_time, max_ankle_z)
        self.DEFAULT_ARMS = np.array([-90, -90, 8, 8, 90, 90, 70, 70], np.float32)  # Posição padrão dos braços

        # Variáveis de controle de caminhada (inicialmente None, usadas durante o planejamento de caminhada)
        self.walk_rel_orientation = None
        self.walk_rel_target = None
        self.walk_distance = None

    def observe(self, init: bool = False) -> np.ndarray:
        """
        Descrição:
            Atualiza e retorna o vetor de observação do ambiente, refletindo o estado atual do robô.
            Inclui informações sensoriais, pose, velocidades das juntas, progresso da caminhada,
            estado das pernas, e alvos internos suavizados para controle de locomoção.

            - As observações incluem sensores inerciais, posições relativas, forças nos pés, posições e velocidades das juntas,
              progresso e estado do passo, além dos alvos internos suavizados para locomoção.
            - Vários valores são normalizados para facilitar o uso em algoritmos de aprendizado de máquina/controle.
            - O método pode ser chamado com init=True para reinicializar variáveis de memória ao início de um novo episódio.

        Parâmetros:
            init: bool, opcional
                Se True, reinicializa variáveis internas e ajusta observações para o início do episódio.

        Retorno:
            obs: np.ndarray
                Vetor de observação (tamanho 63, tipo float32),
                contendo as variáveis normalizadas do estado atual do robô.
         """

        r = self.world.robot

        if init:
            # Reinicializa variáveis internas ao início do episódio
            self.act = np.zeros(16, np.float32)  # Memória para ações passadas
            self.step_counter = 0  # Zera contador de passos

        # Preenche observações com informações normalizadas:
        self.obs[0] = min(self.step_counter, 15 * 8) / 100  # Contador de passos normalizado
        self.obs[1] = r.loc_head_z * 3  # Posição Z do torso
        self.obs[2] = r.loc_head_z_vel / 2  # Velocidade Z do torso
        self.obs[3] = r.imu_torso_roll / 15  # Ângulo de roll do torso
        self.obs[4] = r.imu_torso_pitch / 15  # Ângulo de pitch do torso
        self.obs[5:8] = r.gyro / 100  # Giroscópio normalizado
        self.obs[8:11] = r.acc / 10  # Acelerômetro normalizado

        # Forças e pontos de contato nos pés (origem relativa + vetor de força)
        self.obs[11:17] = r.frp.get('lf', np.zeros(6)) * (10, 10, 10, 0.01, 0.01, 0.01)  # Pé esquerdo
        self.obs[17:23] = r.frp.get('rf', np.zeros(6)) * (10, 10, 10, 0.01, 0.01, 0.01)  # Pé direito
        # Se o pé não está tocando o chão, vetor é zero

        # Cinemática direta: posição dos tornozelos relativa ao quadril
        rel_lankle = self.ik.get_body_part_pos_relative_to_hip("lankle")
        rel_rankle = self.ik.get_body_part_pos_relative_to_hip("rankle")
        # Rotação dos pés relativa ao torso
        lf = r.head_to_body_part_transform("torso", r.body_parts['lfoot'].transform)
        rf = r.head_to_body_part_transform("torso", r.body_parts['rfoot'].transform)
        lf_rot_rel_torso = np.array([lf.get_roll_deg(), lf.get_pitch_deg(), lf.get_yaw_deg()])
        rf_rot_rel_torso = np.array([rf.get_roll_deg(), rf.get_pitch_deg(), rf.get_yaw_deg()])

        # Observações de pose (posição e rotação dos tornozelos/pés e braços)
        self.obs[23:26] = rel_lankle * (8, 8, 5)
        self.obs[26:29] = rel_rankle * (8, 8, 5)
        self.obs[29:32] = lf_rot_rel_torso / 20
        self.obs[32:35] = rf_rot_rel_torso / 20
        self.obs[35:39] = r.joints_position[14:18] / 100  # Posições dos braços (pitch + roll)

        # Velocidade das juntas (última ação)
        self.obs[39:55] = r.joints_target_last_speed[2:18]

        '''
        Observações esperadas durante caminhada:
        Time step        R  0   1   2   3   4   5   6   7   0
        Progress         1  0 .14 .28 .43 .57 .71 .86   1   0
        Left leg active  T  F   F   F   F   F   F   F   F   T
        '''

        if init:
            # Parâmetros de caminhada no início do episódio (irrelevantes após reset)
            self.obs[55] = 1  # Progresso do passo
            self.obs[56] = 1  # Perna esquerda ativa
            self.obs[57] = 0  # Perna direita ativa
        else:
            # Atualiza progresso do passo e qual perna está ativa
            self.obs[55] = self.step_generator.external_progress
            self.obs[56] = float(self.step_generator.state_is_left_active)
            self.obs[57] = float(not self.step_generator.state_is_left_active)

        '''
        Cria alvo interno suavizado (movimento mais contínuo e seguro)
        '''
        MAX_LINEAR_DIST = 0.5  # Distância máxima linear (m)
        MAX_LINEAR_DIFF = 0.014  # Variação máxima linear por etapa (m)
        MAX_ROTATION_DIFF = 1.6  # Variação máxima angular por etapa (graus)
        MAX_ROTATION_DIST = 45  # Distância máxima de rotação (graus)

        if init:
            # Alvos internos resetados ao início do episódio
            self.internal_rel_orientation = 0
            self.internal_target = np.zeros(2)

        previous_internal_target = np.copy(self.internal_target)

        # --------------------------- Cálculo do alvo linear interno suavizado
        rel_raw_target_size = np.linalg.norm(self.walk_rel_target)

        if rel_raw_target_size == 0:
            rel_target = self.walk_rel_target
        else:
            rel_target = self.walk_rel_target / rel_raw_target_size * min(self.walk_distance, MAX_LINEAR_DIST)

        internal_diff = rel_target - self.internal_target
        internal_diff_size = np.linalg.norm(internal_diff)

        if internal_diff_size > MAX_LINEAR_DIFF:
            # Suaviza a mudança de alvo: limita a variação máxima por etapa
            self.internal_target += internal_diff * (MAX_LINEAR_DIFF / internal_diff_size)
        else:
            self.internal_target[:] = rel_target

        # Cálculo do alvo de rotação interno suavizado
        internal_ori_diff = np.clip(
            GeneralMath.normalize_deg(self.walk_rel_orientation - self.internal_rel_orientation),
            -MAX_ROTATION_DIFF, MAX_ROTATION_DIFF
        )
        self.internal_rel_orientation = np.clip(
            GeneralMath.normalize_deg(self.internal_rel_orientation + internal_ori_diff),
            -MAX_ROTATION_DIST, MAX_ROTATION_DIST
        )

        #  Observações de alvo interno e sua velocidade
        internal_target_vel = self.internal_target - previous_internal_target

        self.obs[58] = self.internal_target[0] / MAX_LINEAR_DIST
        self.obs[59] = self.internal_target[1] / MAX_LINEAR_DIST
        self.obs[60] = self.internal_rel_orientation / MAX_ROTATION_DIST
        self.obs[61] = internal_target_vel[0] / MAX_LINEAR_DIFF
        self.obs[62] = internal_target_vel[0] / MAX_LINEAR_DIFF  # Bug no eixo Y (mantido para compatibilidade com modelos treinados)

        return self.obs

    def execute_ik(self, l_pos: tuple, l_rot: tuple, r_pos: tuple, r_rot: tuple) -> None:
        """
        Descrição:
            Executa a cinemática inversa (IK) para ambas as pernas do robô,
            atualizando diretamente as posições-alvo das juntas.

        Parâmetros:
            l_pos: tuple ou array-like
                Posição alvo do tornozelo esquerdo no espaço cartesiano (x, y, z).
            l_rot: tuple ou array-like
                Rotação alvo do pé esquerdo (roll, pitch, yaw).
            r_pos: tuple ou array-like
                Posição alvo do tornozelo direito no espaço cartesiano (x, y, z).
            r_rot: tuple ou array-like
                Rotação alvo do pé direito (roll, pitch, yaw).
        
        Retorno:
            - Chama a função de IK para cada perna, obtendo os índices das juntas e valores-alvo.
            - Atualiza diretamente as posições-alvo das juntas das pernas, sem harmonização para maior precisão/simplicidade.
        """
        r = self.world.robot

        # Aplica IK para a perna esquerda e define os alvos das juntas
        indices, self.values_l, error_codes = self.ik.leg(l_pos, l_rot, True, dynamic_pose=False)
        r.set_joints_target_position_direct(indices, self.values_l, harmonize=False)

        # Aplica IK para a perna direita e define os alvos das juntas
        indices, self.values_r, error_codes = self.ik.leg(r_pos, r_rot, False, dynamic_pose=False)
        r.set_joints_target_position_direct(indices, self.values_r, harmonize=False)

    def execute(self, action: np.ndarray) -> None:
        """
        Descrição:
            Executa um ciclo de controle de locomoção do robô, processando a ação recebida, atualizando estados internos,
            e aplicando os comandos de posição para pernas e braços.

        Parâmetros:
            action: array-like
                Vetor de 16 elementos contendo os comandos normalizados para as seguintes articulações:
                    [0-2]   - posição do tornozelo esquerdo (x, y, z)
                    [3-5]   - posição do tornozelo direito (x, y, z)
                    [6-8]   - rotação do pé esquerdo (roll, pitch, yaw)
                    [9-11]  - rotação do pé direito (roll, pitch, yaw)
                    [12-13] - pitch dos braços esquerdo/direito
                    [14-15] - roll dos braços esquerdo/direito

        Retorno:
            - Escala e suaviza o vetor de ação utilizando média móvel exponencial para garantir movimentos suaves.
            - Gera posições-alvo dos tornozelos usando o step_generator, ajustando a altura e o avanço de cada perna.
            - Calcula as rotações dos pés, limitando yaw/pitch para evitar movimentos não naturais ou colisões.
            - Calcula a pose dos braços, incluindo balanço sincronizado com a passada.
            - Chama execute_ik para aplicar IK nas pernas e atualiza diretamente as juntas dos braços.
            - Incrementa o contador de passos ao final.
        """
        r = self.world.robot

        # Calcula o multiplicador de ação baseado na distância do alvo interno, suavizando movimentos quando distante
        internal_dist = np.linalg.norm(self.internal_target)
        action_mult = 1 if internal_dist > 0.2 else (0.7 / 0.2) * internal_dist + 0.3

        # Suaviza comandos usando média móvel exponencial
        self.act = 0.8 * self.act + 0.2 * action * action_mult * 0.7

        # Extrai as posições alvo das pernas da máquina de passos
        lfy, lfz, rfy, rfz = self.step_generator.get_target_positions(
            self.step_counter == 0, self.STEP_DUR, self.STEP_Z_SPAN, self.leg_length * self.STEP_Z_MAX
        )

        # Aplica offsets de ação às posições dos tornozelos
        a = self.act
        l_ankle_pos = (a[0] * 0.02, max(0.01, a[1] * 0.02 + lfy), a[2] * 0.01 + lfz)  # Limita y para evitar colisão
        r_ankle_pos = (a[3] * 0.02, min(a[4] * 0.02 + rfy, -0.01), a[5] * 0.01 + rfz)  # Limita y para evitar colisão

        # Calcula rotações dos pés a partir da ação
        l_foot_rot = a[6:9] * (3, 3, 5)
        r_foot_rot = a[9:12] * (3, 3, 5)

        # Limita yaw/pitch dos pés para evitar movimentos não naturais
        l_foot_rot[2] = max(0, l_foot_rot[2] + 7)
        r_foot_rot[2] = min(0, r_foot_rot[2] - 7)

        # Define pose padrão dos braços e aplica balanço sincronizado com a passada
        arms = np.copy(self.DEFAULT_ARMS)
        arm_swing = math.sin(self.step_generator.state_current_ts / self.STEP_DUR * math.pi) * 6
        inv = 1 if self.step_generator.state_is_left_active else -1
        arms[0:4] += a[12:16] * 4 + (-arm_swing * inv, arm_swing * inv, 0, 0)

        # Aplica a cinemática inversa para as pernas
        self.execute_ik(l_ankle_pos, l_foot_rot, r_ankle_pos, r_foot_rot)
        # Atualiza diretamente as juntas dos braços
        r.set_joints_target_position_direct(slice(14, 22), arms, harmonize=False)

        # Incrementa o contador de passos
        self.step_counter += 1
