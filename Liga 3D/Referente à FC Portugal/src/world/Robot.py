# Sobre a estrutura de dados deque:
# - Permite adicionar (append) ou remover (pop) elementos em ambas as extremidades com complexidade O(1).
# - É ideal para implementações de filas (queue) e pilhas (stack), além de outros algoritmos que exigem operações rápidas nas duas pontas.
# - Possui métodos como append(), appendleft(), pop(), popleft(), extend(), extendleft(), entre outros.
# - Pode ter um tamanho máximo definido, funcionando como uma fila circular (descartando elementos antigos quando cheia).
from collections import deque

from math import atan, pi, sqrt, tan
from math_ops.GeneralMath import GeneralMath
from math_ops.Matriz3x3 import Matriz3x3
from math_ops.Matriz4x4 import Matriz4x4
from world.commons.BodyPart import BodyPart
from world.commons.JointInfo import JointInfo
# Módulo especializado para analisar, criar e manipular arquivos .xml
import xml.etree.ElementTree as xmlp

import numpy as np


class Robot:
    """
    Descrição:
        Responsável por representar o robô e providenciar funcionalidades
        básicas para seu movimento e resposta.

        Existem diversas funcionalidades inerentes à movimentação, como _initialize_kinematics,
        à posição, como update_pose, e tantas outras. Apesar de utilizarmos fielmente
        dados recolhidos de funções construídas em /sobre_cpp/, não as importamos aqui.

        Nada aqui é trivial, recomendo que observe bem as respectivas documentações.


    Métodos Disponivéis:
        - get_head_abs_vel
        - _initialize_kinematics
        - update_localization
        - head_to_body_part_transform
        - get_body_part_to_field_transform
        - get_body_part_abs_position
        - get_joint_to_field_transform
        - get_joint_abs_position
        - update_pose
        - update_imu
        - set_joints_target_position_direct
        - get_command

    Variáveis de Ambiente
        - STEPTIME
        - VISUALSTEP
        - SQ_STEPTIME
        - GRAVITY
        - IMU_DECAY
        - MAP_PERCEPTOR_TO_INDEX
        - FIX_PERCEPTOR_SET
        - FIX_INDICES_LIST
        - BEAM_HEIGHTS
    """

    STEPTIME = 0.02  # Duração fixa de um passo na simulação
    VISUALSTEP = 0.04  # Duração fixa de um passo para a atualização visual
    SQ_STEPTIME = STEPTIME * STEPTIME  # Quadrado do STEPTIME
    GRAVITY = np.array([0, 0, -9.81])  # Aceleração da gravidade (m/s²)
    IMU_DECAY = 0.996  # Fator de decaimento da velocidade na IMU (unidade adimensional)

    # Constantes para forçar a simetria nas juntas/efetuadores

    # Mapeia o nome do perceptor para um índice na tabela de juntas
    MAP_PERCEPTOR_TO_INDEX = {"hj1": 0, "hj2": 1, "llj1": 2, "rlj1": 3,
                              "llj2": 4, "rlj2": 5, "llj3": 6, "rlj3": 7,
                              "llj4": 8, "rlj4": 9, "llj5": 10, "rlj5": 11,
                              "llj6": 12, "rlj6": 13, "laj1": 14, "raj1": 15,
                              "laj2": 16, "raj2": 17, "laj3": 18, "raj3": 19,
                              "laj4": 20, "raj4": 21, "llj7": 22, "rlj7": 23}

    # Conjunto de perceptors que precisam de tratamento especial na simetria
    FIX_PERCEPTOR_SET = {'rlj2', 'rlj6', 'raj2', 'laj3', 'laj4'}

    # Lista de índices que serão corrigidos para a simetria
    FIX_INDICES_LIST = [5, 13, 17, 18, 20]

    # Alturas recomendadas para a teleporte não-oficial (próxima do solo)
    # Na ordem dos tipos de robô.
    BEAM_HEIGHTS = [0.4, 0.43, 0.4, 0.46, 0.4]

    def __init__(self, unum: int, robot_type: int) -> None:
        """
        Descrição:
            A função faz o parsing do modelo a partir de um XML, populando a tabela de body_parts,
            joints, etc.

        Parâmetros::
            unum (int): Número identificador único do robô na simulação.
            robot_type (int): Tipo do robô (determina o modelo 3D, a altura, o número de juntas, etc.).

        Todos os Atributos Definidos Aqui:
            - type (int): Tipo do robô.
            - beam_height (float): Altura da viga de referência usada como apoio.
            - no_of_joints (int): Quantidade de juntas que o robô possui.
            - FIX_EFFECTOR_MASK (np.array): Máscara usada para forçar a simetria nas juntas.
            - body_parts (dict): Dicionário que relaciona o nome das peças do robô às instâncias de Body_Part.
            - unum (int): Número identificador único do robô na simulação.
            - gyro (np.array): Velocidade angular nas 3 direções (deg/s).
            - acc (np.array): Aceleração nas 3 direções (m/s²).
            - frp (dict): Leituras de pressão nas superfícies de apoio ("lf","rf","lf1","rf1").
            - feet_toes_last_touch (dict): Momento em que o pé ou o dedo tocou o chão.
            - feet_toes_are_touching (dict): Se o pé ou o dedo estão tocando o chão.
            - fwd_kinematics_list (list): Lista de partes do robô na sequência cinemática.
            - rel_cart_CoM_position (np.array): Posição do centro de massa em relação à cabeça.

            - joints_position (np.array): Posição angular das juntas (deg).
            - joints_speed (np.array): Velocidade angular das juntas (rad/s).
            - joints_target_speed (np.array): Velocidade angular desejada pelas juntas (rad/s).
            - joints_target_last_speed (np.array): Velocidade angular exigida na última execução (rad/s).
            - joints_info (list): Uma lista de instâncias de Joint_Info, que contêm metadados das juntas.
            - joints_transform (list): Matrises de transformação 4x4 de cada junta.
-
            - loc_head_to_field_transform (Matriz4x4): Transformação da cabeça para o campo.
            - loc_field_to_head_transform (Matriz4x4): Transformação inversa (campo para a cabeça).
            - loc_rotation_head_to_field (Matriz3x3): Rotação da cabeça para o campo.
            - loc_rotation_field_to_head (Matriz3x3): Rotação inversa (campo para a cabeça).
            - loc_head_position (np.array): Posição da cabeça no espaço 3D (m).
            - loc_head_position_history (deque): Histórico das últimas posições da cabeça.
            - loc_head_velocity (np.array): Velocidade da cabeça (m/s).
            - loc_head_orientation (float): Orientação da cabeça (deg).
            - loc_is_up_to_date (bool): Se os dados de posição estão atualizados.
            - loc_last_update (int): Momento (ms) da última atualização de posição.
            - loc_head_position_last_update (int): Momento (ms) da última atualização da posição da cabeça.
            - radio_fallen_state (bool): Se o robô caiu, a informação é dada pelo rádio.
            - radio_last_update (int): Momento (ms) da última atualização pelo rádio.

            - loc_torso_to_field_rotation (Matriz3x3): Rotação do torso para o campo.
            - loc_torso_to_field_transform (Matriz4x4): Transformação do torso para o campo.
            - loc_torso_roll (float): Inclinação de rolagem do torso (deg).
            - loc_torso_pitch (float): Inclinação de pitch do torso (deg).
            - loc_torso_orientation (float): Orientação do torso (deg).
            - loc_torso_inclination (float): Inclinação do torso em relação ao z do campo (deg).
            - loc_torso_position (np.array): Posição do torso (m).
            - loc_torso_velocity (np.array): Velocidade do torso (m/s).
            - loc_torso_acceleration (np.array): Aceleração do torso (m/s²).

            - cheat_abs_pos (np.array): Posição da cabeça dada pelo server (m).
            - cheat_ori (float): Orientação da cabeça dada pelo server (deg).
            - loc_CoM_position (np.array): Posição do centro de massa (m).
            - loc_CoM_velocity (np.array): Velocidade do centro de massa (m/s).

            - loc_head_z (float): Posição Z da cabeça (m).
            - loc_head_z_is_up_to_date (bool): Se o Z é atualizado.
            - loc_head_z_last_update (int): Momento (ms) da última atualização do Z da cabeça.
            - loc_head_z_vel (float): Velocidade Z da cabeça (m/s).

            - imu_torso_roll (float): Inclinação de rolagem pelo giroscópio (deg).
            - imu_torso_pitch (float): Inclinação de pitch pelo giroscópio (deg).
            - imu_torso_orientation (float): Orientação pelo giroscópio (deg).
            - imu_torso_inclination (float): Inclinação pelo giroscópio (deg).
            - imu_torso_to_field_rotation (Matriz3x3): Rotação pelo giroscópio.
            - imu_last_visual_update (int): Momento (ms) da última atualização junto às imagens.

            - imu_weak_torso_to_field_transform (Matriz4x4): Transformação fraca do torso para o campo.
            - imu_weak_head_to_field_transform (Matriz4x4): Transformação fraca da cabeça para o campo.
            - imu_weak_field_to_head_transform (Matriz4x4): Transformação fraca do campo para a cabeça.
            - imu_weak_torso_position (np.array): Posição fraca do torso (m).
            - imu_weak_torso_velocity (np.array): Velocidade fraca do torso (m/s).
            - imu_weak_torso_acceleration (np.array): Aceleração fraca do torso (m/s²).
            - imu_weak_torso_next_position (np.array): Estimativa da posição do torso no próximo passo.
            - imu_weak_torso_next_velocity (np.array): Estimativa da velocidade do torso no próximo passo.
            - imu_weak_CoM_position (np.array): Centro de massa fraco (m).
            - imu_weak_CoM_velocity (np.array): Velocidade fraca do centro de massa (m/s).

            J_HEAD_YAW, J_HEAD_PITCH, [...]:
                Constantes que representam o índice de cada junta na tabela de juntas.
        """

        robot_xml = "nao" + str(robot_type) + ".xml"  # nome do modelo 3D do robô
        self.type = robot_type
        self.beam_height = Robot.BEAM_HEIGHTS[robot_type]
        self.no_of_joints = 24 if robot_type == 4 else 22

        # Corrigir assimetrias nas juntas
        self.FIX_EFFECTOR_MASK = np.ones(self.no_of_joints)
        self.FIX_EFFECTOR_MASK[Robot.FIX_INDICES_LIST] = -1

        # Dicionário que relaciona o nome das peças às instâncias de Body_Part
        self.body_parts = dict()
        self.unum = unum  # número identificador único do robô
        self.gyro = np.zeros(3)  # velocidade angular nas 3 direções
        self.acc = np.zeros(3)  # aceleração nas 3 direções
        self.frp = dict()
        self.feet_toes_last_touch = {"lf": 0, "rf": 0, "lf1": 0, "rf1": 0}
        self.feet_toes_are_touching = {"lf": False, "rf": False, "lf1": False, "rf1": False}
        self.fwd_kinematics_list = None
        self.rel_cart_CoM_position = np.zeros(3)

        self.joints_position = np.zeros(self.no_of_joints)
        self.joints_speed = np.zeros(self.no_of_joints)
        self.joints_target_speed = np.zeros(self.no_of_joints)
        self.joints_target_last_speed = np.zeros(self.no_of_joints)
        self.joints_info = [None] * self.no_of_joints
        self.joints_transform = [Matriz4x4() for _ in range(self.no_of_joints)]  # Literalmente uma matriz 4x4 para cada junta.

        # Localização relativa à cabeça
        self.loc_head_to_field_transform = Matriz4x4()
        self.loc_field_to_head_transform = Matriz4x4()
        self.loc_rotation_head_to_field = Matriz3x3()
        self.loc_rotation_field_to_head = Matriz3x3()
        self.loc_head_position = np.zeros(3)
        self.loc_head_position_history = deque(maxlen=40)
        self.loc_head_velocity = np.zeros(3)
        self.loc_head_orientation = 0
        self.loc_is_up_to_date = False
        self.loc_last_update = 0
        self.loc_head_position_last_update = 0
        self.radio_fallen_state = False
        self.radio_last_update = 0

        # Localização relativa ao torso
        self.loc_torso_to_field_rotation = Matriz3x3()
        self.loc_torso_to_field_transform = Matriz4x4()
        self.loc_torso_roll = 0
        self.loc_torso_pitch = 0
        self.loc_torso_orientation = 0
        self.loc_torso_inclination = 0
        self.loc_torso_position = np.zeros(3)
        self.loc_torso_velocity = np.zeros(3)
        self.loc_torso_acceleration = np.zeros(3)

        # Outros
        self.cheat_abs_pos = np.zeros(3)
        self.cheat_ori = 0.0
        self.loc_CoM_position = np.zeros(3)
        self.loc_CoM_velocity = np.zeros(3)

        self.loc_head_z = 0
        self.loc_head_z_is_up_to_date = False
        self.loc_head_z_last_update = 0
        self.loc_head_z_vel = 0

        # Localização + Giroscópio
        self.imu_torso_roll = 0
        self.imu_torso_pitch = 0
        self.imu_torso_orientation = 0
        self.imu_torso_inclination = 0
        self.imu_torso_to_field_rotation = Matriz3x3()
        self.imu_last_visual_update = 0

        # Localização + Giroscópio + Acelerômetro
        self.imu_weak_torso_to_field_transform = Matriz4x4()
        self.imu_weak_head_to_field_transform = Matriz4x4()
        self.imu_weak_field_to_head_transform = Matriz4x4()
        self.imu_weak_torso_position = np.zeros(3)
        self.imu_weak_torso_velocity = np.zeros(3)
        self.imu_weak_torso_acceleration = np.zeros(3)
        self.imu_weak_torso_next_position = np.zeros(3)
        self.imu_weak_torso_next_velocity = np.zeros(3)
        self.imu_weak_CoM_position = np.zeros(3)
        self.imu_weak_CoM_velocity = np.zeros(3)

        # Juntas
        self.J_HEAD_YAW = 0
        self.J_HEAD_PITCH = 1
        self.J_LLEG_YAW_PITCH = 2
        self.J_RLEG_YAW_PITCH = 3
        self.J_LLEG_ROLL = 4
        self.J_RLEG_ROLL = 5
        self.J_LLEG_PITCH = 6
        self.J_RLEG_PITCH = 7
        self.J_LKNEE = 8
        self.J_RKNEE = 9
        self.J_LFOOT_PITCH = 10
        self.J_RFOOT_PITCH = 11
        self.J_LFOOT_ROLL = 12
        self.J_RFOOT_ROLL = 13
        self.J_LARM_PITCH = 14
        self.J_RARM_PITCH = 15
        self.J_LARM_ROLL = 16
        self.J_RARM_ROLL = 17
        self.J_LELBOW_YAW = 18
        self.J_RELBOW_YAW = 19
        self.J_LELBOW_ROLL = 20
        self.J_RELBOW_ROLL = 21
        self.J_LTOE_PITCH = 22
        self.J_RTOE_PITCH = 23

        # Carregar o modelo 3D
        dir_ = M.get_active_directory("/world/commons/robots/")
        robot_xml_root = xmlp.parse(dir_ + robot_xml).getroot()

        joint_no = 0
        for child in robot_xml_root:
            if child.tag == "bodypart":
                self.body_parts[child.attrib['name']] = BodyPart(child.attrib['mass'])

            elif child.tag == "joint":
                self.joints_info[joint_no] = JointInfo(child)
                self.joints_position[joint_no] = 0.0
                ji = self.joints_info[joint_no]

                self.body_parts[ji.anchor0_part].joints.append(Robot.MAP_PERCEPTOR_TO_INDEX[ji.perceptor])

                joint_no += 1
                if joint_no == self.no_of_joints:
                    break

            else:
                raise NotImplementedError

        assert joint_no == self.no_of_joints, "Robô e XML estão inconsistentes!"

    def get_head_abs_vel(self, history_steps: int) -> np.ndarray:
        """
        Descrição:
            Calcula a velocidade absoluta da cabeça do robô (m/s) com base nas últimas posições registradas.
            A velocidade é computada como a razão entre a variação de posição e o tempo decorrido.

            Semelhante à função que havia dentro da classe LocalizerV2, de /sobre_cpp/ambientacao.

        Parâmetros:
            history_steps : int

                Número de posições anteriores utilizadas para o cálculo.
                O valor deve estar no intervalo [1,40].

        Retorno:
            np.ndarray
                Um vetor de 3 elementos (vx, vy, vz) representando a velocidade da cabeça em m/s.
                Se o histórico de posições ainda não possuir entradas, é retornado um vetor nulo.
        """

        assert 1 <= history_steps <= 40, "O parâmetro 'history_steps' deve estar no intervalo [1,40]"

        if len(self.loc_head_position_history) == 0:
            # Se o histórico ainda não possuir posições, retorna um vetor nulo
            return np.zeros(3)

        # Utiliza o número de posições disponíveis, o menor entre o requerido e o que temos
        h_step = min(history_steps, len(self.loc_head_position_history))
        t = h_step * Robot.VISUALSTEP

        # Cálculo da velocidade como variação de posição pelo tempo decorrido
        return (self.loc_head_position - self.loc_head_position_history[h_step - 1]) / t

    def _initialize_kinematics(self):
        """
        Descrição:
            Inicializa a cinemática do robô, construindo a sequência de partes do corpo e a lista de cinemática direta.
            A função parte da cabeça ("head") e percorre recursivamente as conexões de juntas para acrescentar cada parte.
            Por fim, corrige problemas de simetria presentes nas definições de algumas juntas.

        Parâmetros:
            None.

        Retorno:
            Apenas configura o atributo `self.fwd_kinematics_list` para o robô.
        """

        # Começa a varredura a partir da cabeça
        parts = {"head"}
        sequential_body_parts = ["head"]

        while len(parts) > 0:
            part = parts.pop()

            for j in self.body_parts[part].joints:

                p = self.joints_info[j].anchor1_part

                if len(self.body_parts[p].joints) > 0:  # Adiciona a parte se ela é o ponto de ancoragem de uma nova junta
                    parts.add(p)
                    sequential_body_parts.append(p)

        self.fwd_kinematics_list = [
            (self.body_parts[part], j, self.body_parts[self.joints_info[j].anchor1_part])
            for part in sequential_body_parts for j in self.body_parts[part].joints
        ]

        # Corrige problemas de simetria nas definições de algumas juntas
        for i in Robot.FIX_INDICES_LIST:
            self.joints_info[i].axes *= -1
            aux = self.joints_info[i].min
            self.joints_info[i].min = -self.joints_info[i].max
            self.joints_info[i].max = -aux

    def update_localization(self, localization_raw: np.ndarray, time_local_ms: int):
        """
        Descrição:
            Atualiza as informações de localização do robô, a velocidade, a posição, a orientação
            e outras métricas, a partir de um pacote de dados brutos de localização.
            Realiza também o histórico de posições da cabeça para apoiar cálculos futuros de velocidade.

            Aqui estamos recebendo as informações obtivas do módulo /sobre_cpp/ambientacao.so

        Parâmetros:
            localization_raw : np.ndarray
                Dados brutos de localização, onde estão presentes transformação, posição, velocidade e flags de atualização.
                Mesma saída providenciada por ambientecao.so
            time_local_ms : int
                Timestamp atual em milissegundos, utilizado para o cálculo de velocidade a partir das variações de posição.

        Retorno:
            Atualiza os parâmetros internos do robô, como posição, velocidade, orientação, etc.
        """

        # Analisa as flags de atualização presentes nos dados brutos
        loc = localization_raw.astype(float)  # Converte para 64bits para maior precisão
        self.loc_is_up_to_date = bool(loc[32])  # Flag de posição
        self.loc_head_z_is_up_to_date = bool(loc[34])  # Flag de altura da cabeça

        if self.loc_head_z_is_up_to_date:
            time_diff = (time_local_ms - self.loc_head_z_last_update) / 1000
            self.loc_head_z_vel = (loc[33] - self.loc_head_z) / time_diff
            self.loc_head_z = loc[33]
            self.loc_head_z_last_update = time_local_ms

        # Armazena histórico de posições da cabeça a cada ciclo de visão
        # Ou seja, aquilo que tinha de histório de posições realmente era inútil?
        # Fizemos bem ao apagar? É o que descobriremos.
        self.loc_head_position_history.appendleft(np.copy(self.loc_head_position))

        if self.loc_is_up_to_date:
            time_diff = (time_local_ms - self.loc_last_update) / 1000
            self.loc_last_update = time_local_ms
            self.loc_head_to_field_transform.matriz[:] = loc[0:16].reshape((4, 4))
            self.loc_field_to_head_transform.matriz[:] = loc[16:32].reshape((4, 4))

            # Extraindo métricas da cabeça
            self.loc_rotation_head_to_field = self.loc_head_to_field_transform.obter_matriz_de_rotacao_3x3()
            self.loc_rotation_field_to_head = self.loc_field_to_head_transform.obter_matriz_de_rotacao_3x3()
            vet_trans = self.loc_head_to_field_transform.obter_vetor_de_translacao()
            self.loc_head_velocity = (vet_trans - self.loc_head_position) / time_diff
            self.loc_head_position = vet_trans
            self.loc_head_position_last_update = time_local_ms
            self.loc_head_orientation = self.loc_head_to_field_transform.get_yaw_deg()
            self.radio_fallen_state = False

            # Extraindo métricas do centro de massa
            vet_trans = self.loc_head_to_field_transform(self.rel_cart_CoM_position)
            self.loc_CoM_velocity = (vet_trans - self.loc_CoM_position) / time_diff
            self.loc_CoM_position = vet_trans

            # Extraindo métricas do torso
            t = self.get_body_part_to_field_transform('torso')
            self.loc_torso_to_field_transform = t
            self.loc_torso_to_field_rotation = t.obter_matriz_de_rotacao_3x3()
            self.loc_torso_orientation = t.get_yaw_deg()
            self.loc_torso_pitch = t.get_pitch_deg()
            self.loc_torso_roll = t.get_roll_deg()
            self.loc_torso_inclination = t.get_inclination_deg()
            vet_trans = t.get_translation()
            self.loc_torso_velocity = (vet_trans - self.loc_torso_position) / time_diff
            self.loc_torso_position = vet_trans
            self.loc_torso_acceleration = self.loc_torso_to_field_rotation.multiply(self.acc) + Robot.GRAVITY

    def head_to_body_part_transform(self, body_part_name, coords, is_batch=False):
        """
        Descrição:
            Converte posições ou transformações de pose no referencial da cabeça
            para o referencial de uma parte do corpo especificada.

        Parâmetros:
            body_part_name : str
                Nome da parte do corpo (conforme especificado pelo robô).
            coords: array-like ou objeto de transformação
                Uma posição 3D, uma lista de posições 3D ou uma matriz de transformação (Matriz4x4).
            is_batch : bool
                Indica se o parâmetro `coords` é um lote (lista) de posições ou transformações.

        Retorno:
            ndarray ou list
                Se `is_batch` for False, é retornado um ndarray ou um objeto transformado.
                Se `is_batch` for True, é retornada uma lista de transformações ou de posições transformadas.
        """
        head_to_bp_transform: Matriz4x4 = self.body_parts[body_part_name].transform.invert()

        if is_batch:
            return [head_to_bp_transform(c) for c in coords]
        else:
            return head_to_bp_transform(coords)

    def get_body_part_to_field_transform(self, body_part_name) -> Matriz4x4:
        """
        Descrição:
            Retorna a matriz de transformação que leva do referencial de uma parte do corpo para o referencial do campo.
            A matriz resultante proporciona tanto a posição quanto a rotação absolutas dessa parte do corpo.

        Parâmetros:
            body_part_name : str
                Nome da parte do corpo (conforme especificado pelo robô).

        Retorno:
            Matriz4x4
                Uma matriz de transformação que relaciona o referencial da parte do corpo ao referencial do campo.

        Observação:
        É importante que `self.loc_is_up_to_date` seja True para que essa transformação, seja consistente.
        Caso os dados de localização não estejam atualizados, os resultados poderão ser imprecisos ou inconsistentes.
        """
        return self.loc_head_to_field_transform.multiply(self.body_parts[body_part_name].transform)

    def get_body_part_abs_position(self, body_part_name) -> np.ndarray:
        """
        Descrição:
            Retorna a posição absoluta de uma parte do corpo, considerando tanto os dados de localização quanto a cinemática direta.

        Parâmetros:
            body_part_name : str
                Nome da parte do corpo (conforme especificado pelo robô).

        Retorno:
            np.ndarray
                Um array 3D (x, y, z) representando a posição absoluta da parte do corpo no referencial do campo.

        Observação:
            É importante que `self.loc_is_up_to_date` seja True para que a posição, seja consistente.
            Caso os dados de localização não estejam atualizados, os resultados poderão ser imprecisos ou inconsistentes.
        """
        return self.get_body_part_to_field_transform(body_part_name).obter_vetor_de_translacao()

    def get_joint_to_field_transform(self, joint_index: int) -> Matriz4x4:
        """
        Descrição:
            Retorna a matriz de transformação que relaciona um determinada articulação (joint) ao referencial do campo. 
            A matriz resultante incorpora tanto a cinemática quanto a localização do robô.

        Parâmetros:
            joint_index : int
                Índice da articulação que se quer obter a transformação para o referencial do campo.

        Retorno:
            Matriz4x4
                Uma matriz 4x4 que representa a posição e a orientação da articulação no referencial do campo.

        Observação:
            É importante que `self.loc_is_up_to_date` seja True para que o resultado, seja preciso. 
            Caso os dados de localização não estejam atualizados, a transformação resultante pode ser inconsistente.
        """
        return self.loc_head_to_field_transform.multiply(self.joints_transform[joint_index])

    def get_joint_abs_position(self, joint_index: int) -> np.ndarray:
        """
        Descrição:
            Retorna a posição absoluta de uma articulação, considerando tanto os dados de localização quanto a cinemática do robô.

        Parâmetros:
            joint_index : int
                Índice da articulação que se quer obter a posição.

        Retorno:
            np.ndarray
                Um array 3D (x, y, z) que representa a posição da articulação no referencial do campo.

        Observação:
            É importante que `self.loc_is_up_to_date` seja True para que o resultado, seja preciso.
            Caso os dados de localização não estejam atualizados, a posição resultante pode ser inconsistente.
        """
        return self.get_joint_to_field_transform(joint_index).obter_vetor_de_translacao()

    def update_pose(self) -> None:
        """
        Descrição:
            Atualiza a pose de cada parte do robô com base nas transformações cinemáticas diretas
            e nas posições das articulaçãos. Isso é realizado a partir das transformações
            das juntas, que estão encadeadas na árvore cinemática do robô.
            A posição do centro de massa é, ao final, recalculada como a média pesada pelas massas de cada parte do robô.

        Parâmetros:
            Não recebe parâmetros.

        Retorno:
            - Atualiza as transformações das articulaçãos e das partes do robô.
            - Recalcula a posição relativa do centro de massa do robô.

        Observação:
            É importante que a lista de transformações cinemáticas (self.fwd_kinematics_list)
            tenha sido propriamente criada antes, pelo método _initialize_kinematics().
        """

        # Faz a verificação
        if self.fwd_kinematics_list is None:
            self._initialize_kinematics()

        # Realiza as tranformações
        for body_part, j, child_body_part in self.fwd_kinematics_list:
            ji = self.joints_info[j]
            self.joints_transform[j].matriz[:] = body_part.transform.matriz
            self.joints_transform[j].translate(ji.anchor0_axes, True)
            child_body_part.transform.matriz[:] = self.joints_transform[j].matriz
            child_body_part.transform.rotate_deg(ji.axes, self.joints_position[j], True)
            child_body_part.transform.translate(ji.anchor1_axes_neg, True)

        # Posição do Centro de Massa
        self.rel_cart_CoM_position = np.average([b.transform.obter_vetor_de_translacao()
                                                 for b in self.body_parts.values()],
                                                0,
                                                [bp.mass for bp in self.body_parts.values()])

    def update_imu(self, time_local_ms: int) -> None:
        """
        Descrição:
            Atualiza o estado da unidade inercial (IMU) do robô, utilizando tanto os
            dados de localização quanto os giroscópicos. Quando os dados de
            localização estão atualizados, a IMU é sincronizada exatamente com eles.
            Quando estão desatualizados, é realizado um procedimento de extrapolação
            para atualizar posição, velocidade e orientação a partir das últimas
            medições de giroscópio e aceleração.

        Parâmetros:
            time_local_ms : int
                Timestamp atual em milissegundos.

        Retorno:
            Atualiza os seguintes parâmetros internos do robô:

            - imu_torso_roll, imu_torso_pitch, imu_torso_orientation, imu_torso_inclination
            - imu_torso_to_field_rotation
            - imu_weak_torso_to_field_transform
            - imu_weak_head_to_field_transform
            - imu_weak_field_to_head_transform
            - imu_weak_torso_position
            - imu_weak_torso_velocity
            - imu_weak_torso_next_position
            - imu_weak_torso_next_velocity
            - imu_weak_torso_acceleration
            - imu_weak_CoM_position
            - imu_weak_CoM_velocity
            - imu_last_visual_update

        Observação:
            Quando os dados de localização estão desatualizados, o modelo faz uma
            extrapolação utilizando o giroscópio, a aceleração, a velocidade atual 
            e a posição atual. Isso evita que o robô perca o rastreio do seu 
            posicionamento enquanto ele se move.
        """

        # update IMU
        if self.loc_is_up_to_date:
            self.imu_torso_roll = self.loc_torso_roll
            self.imu_torso_pitch = self.loc_torso_pitch
            self.imu_torso_orientation = self.loc_torso_orientation
            self.imu_torso_inclination = self.loc_torso_inclination
            self.imu_torso_to_field_rotation.matriz[:] = self.loc_torso_to_field_rotation.matriz
            self.imu_weak_torso_to_field_transform.matriz[:] = self.loc_torso_to_field_transform.matriz
            self.imu_weak_head_to_field_transform.matriz[:] = self.loc_head_to_field_transform.matriz
            self.imu_weak_field_to_head_transform.matriz[:] = self.loc_field_to_head_transform.matriz
            self.imu_weak_torso_position[:] = self.loc_torso_position
            self.imu_weak_torso_velocity[:] = self.loc_torso_velocity
            self.imu_weak_torso_acceleration[:] = self.loc_torso_acceleration
            self.imu_weak_torso_next_position = self.loc_torso_position + self.loc_torso_velocity * Robot.STEPTIME + self.loc_torso_acceleration * (0.5 * Robot.SQ_STEPTIME)
            self.imu_weak_torso_next_velocity = self.loc_torso_velocity + self.loc_torso_acceleration * Robot.STEPTIME
            self.imu_weak_CoM_position[:] = self.loc_CoM_position
            self.imu_weak_CoM_velocity[:] = self.loc_CoM_velocity
            self.imu_last_visual_update = time_local_ms
        else:
            g = self.gyro / 50  # convert degrees per second to degrees per step

            self.imu_torso_to_field_rotation.multiply(Matrix_3x3.from_rotation_deg(g), in_place=True, reverse_order=True)

            self.imu_torso_orientation = self.imu_torso_to_field_rotation.get_yaw_deg()
            self.imu_torso_pitch = self.imu_torso_to_field_rotation.get_pitch_deg()
            self.imu_torso_roll = self.imu_torso_to_field_rotation.get_roll_deg()

            self.imu_torso_inclination = atan(sqrt(tan(self.imu_torso_roll / 180 * pi) ** 2 + tan(self.imu_torso_pitch / 180 * pi) ** 2)) * 180 / pi

            # Update position and velocity until 0.2 seconds has passed since last visual update
            if time_local_ms < self.imu_last_visual_update + 200:
                self.imu_weak_torso_position[:] = self.imu_weak_torso_next_position
                if self.imu_weak_torso_position[2] < 0:
                    self.imu_weak_torso_position[2] = 0  # limit z coordinate to positive values
                self.imu_weak_torso_velocity[:] = self.imu_weak_torso_next_velocity * Robot.IMU_DECAY  # stability tradeoff
            else:
                self.imu_weak_torso_velocity *= 0.97  # without visual updates for 0.2s, the position is locked, and the velocity decays to zero

            # convert proper acceleration to coordinate acceleration and fix rounding bias
            self.imu_weak_torso_acceleration = self.imu_torso_to_field_rotation.multiply(self.acc) + Robot.GRAVITY
            self.imu_weak_torso_to_field_transform = Matrix_4x4.from_3x3_and_translation(self.imu_torso_to_field_rotation, self.imu_weak_torso_position)
            self.imu_weak_head_to_field_transform = self.imu_weak_torso_to_field_transform.multiply(self.body_parts["torso"].transform.invert())
            self.imu_weak_field_to_head_transform = self.imu_weak_head_to_field_transform.invert()
            p = self.imu_weak_head_to_field_transform(self.rel_cart_CoM_position)
            self.imu_weak_CoM_velocity = (p - self.imu_weak_CoM_position) / Robot.STEPTIME
            self.imu_weak_CoM_position = p

            # Next Position = x0 + v0*t + 0.5*a*t^2,   Next velocity = v0 + a*t
            self.imu_weak_torso_next_position = self.imu_weak_torso_position + self.imu_weak_torso_velocity * Robot.STEPTIME + self.imu_weak_torso_acceleration * (0.5 * Robot.SQ_STEPTIME)
            self.imu_weak_torso_next_velocity = self.imu_weak_torso_velocity + self.imu_weak_torso_acceleration * Robot.STEPTIME

    def set_joints_target_position_direct(self, indices, values: np.ndarray, harmonize: bool = True, max_speed: float = 7.03, tolerance: float = 0.012, limit_joints: bool = True) -> int:
        """
        Descrição:
            Calcula a velocidade de um conjunto de juntas para alcançarem suas posições-alvo.

            A função controla o posicionamento das juntas gradualmente até que os alvos especificados
            sean atingidos. Quando o parâmetro `harmonize` é habilitado, todas as juntas serão sincronizadas
            para completar o seu movimento junto. Quando desativado, cada junta se moverá na velocidade
            máxima até o seu destino. Se a posição já estiver dentro da tolerância especificada, o método
            querará -1.

        Parâmetros:
            indices:
                    Índice(s) das juntas que serão movidas.
                    Pode ser um `int`, uma `list` de `int`, um `slice` ou um `ndarray`.
            values:
                    Um `ndarray` do `numpy` especificando as posições-alvo para cada uma das juntas especificadas.
            harmonize:
                    Se `True` (padrão), faz com que todas as juntas alcancem o destino simultaneamente.
            max_speed:
                    Velocidade máxima para as juntas, em graus/steps. Padrão é 7.03.
            tolerance:
                    Tolerância para considerar que o alvo já foi atingido (graus). Padrão é 0.012.
            limit_joints:
                    Se `True` (padrão), limita os movimentos às amplitudes das juntas.

        Retorno:
            Um `int` representando o número de steps restantes até que o posicionamento seja atingido.
            Retorna -1 se o posicionamento já chegou ou se ele se encontrar dentro da tolerância.

        Exemplos:

            (let p[tx] be the joint position at t=x)

            Example for return value: moving joint[0] from 0deg to 10deg
                    pos[t0]: 0, speed[t0]: 7deg/step, ret=2   # target will predictedly be reached in 2 steps
                    pos[t1]: 7, speed[t1]: 3deg/step, ret=1   # target will predictedly be reached in 1 step (send final action)
                    pos[t2]: 10, speed[t2]: 0deg/step, ret=0   # target was predictedly already reached
                    pos[t3]: 10, speed[t3]: 0deg/step, ret=-1  # (best case scenario) server reported with delay, that target was reached (see tolerance)
                    pos[t?]: 10, speed[t?]: 0deg/step, ret=-1  # if there is friction, it may take some additional steps

                    If everything worked as predicted we could stop calling this function when ret==1
                    If we need precision, it is recommended to wait for ret==-1

            Example 1:
                set_joints_target_position_direct(range(2,4), np.array([10.0,5.0]),harmonize=True)
                    Joint[2]   p[t0]: 0  target pos: 10  ->  p[t1]=5,   p[t2]=10
                    Joint[3]   p[t0]: 0  target pos: 5   ->  p[t1]=2.5, p[t2]=5

            Example 2:
                set_joints_target_position_direct([2,3], np.array([10.0,5.0]),harmonize=False)
                    Joint[2]   p[t0]: 0  target pos: 10  ->  p[t1]=7,   p[t2]=10
                    Joint[3]   p[t0]: 0  target pos: 5   ->  p[t1]=5,   p[t2]=5
        """

        assert isinstance(values, np.ndarray), "'values' deve ser um ndarray do numpy"
        np.nan_to_num(values, copy=False)  # Substitue NaNs por zero e infinitos por números finitos

        # Limita o range das juntas
        if limit_joints:
            if isinstance(indices, (list, np.ndarray)):
                for i in range(len(indices)):
                    values[i] = np.clip(values[i], self.joints_info[indices[i]].min, self.joints_info[indices[i]].max)
            elif isinstance(indices, slice):
                info = self.joints_info[indices]
                for i in range(len(info)):
                    values[i] = np.clip(values[i], info[i].min, info[i].max)
            else:  # int
                values[0] = np.clip(values[0], self.joints_info[indices].min, self.joints_info[indices].max)

        # Diferença prevista: posição atual vs posição futura
        predicted_diff = self.joints_target_last_speed[indices] * 1.1459156  # rad/s para deg/steps
        predicted_diff = np.asarray(predicted_diff)
        np.clip(predicted_diff, -max_speed, max_speed, out=predicted_diff)  # limita a velocidade prevista

        # Distância relatativa até o destino
        reported_dist = values - self.joints_position[indices]
        if np.all((np.abs(reported_dist) < tolerance)) and np.all((np.abs(predicted_diff) < tolerance)):
            self.joints_target_speed[indices] = 0
            return -1

        deg_per_step = reported_dist - predicted_diff

        relative_max = np.max(np.abs(deg_per_step)) / max_speed
        remaining_steps = int(np.ceil(relative_max))

        if remaining_steps == 0:
            self.joints_target_speed[indices] = 0
            return 0

        if harmonize:
            deg_per_step /= remaining_steps
        else:
            np.clip(deg_per_step, -max_speed, max_speed, out=deg_per_step)

        self.joints_target_speed[indices] = deg_per_step * 0.87266463  # converter para rad/s

        return remaining_steps

    def get_command(self) -> bytes:
        """
        Descrição:
            Constrói uma sequência de bytes que representa um comando para o robô a partir das velocidades
            das juntas especificadas em `self.joints_target_speed`.

            Os valores serão formatados como uma sequência de tuples na forma
            (efector velocidade), sendo que o nome do efector é obtido a partir de `self.joints_info`.
            A máscara `self.FIX_EFFECTOR_MASK` é aplicada para corrigir problemas de simetria de determinados
            efetores antes de gerar o comando.
            Após a construção, o array `self.joints_target_speed` é resetado para zero, enquanto
            `self.joints_target_last_speed` passa a referenciar o array que acabou de ser utilizado.

            Muito semelhante à conforme descrito no vídeo da BahiaRT.

        Parâmetros:
            Nenhum.

        Retorno:
            cmd: bytes
                    Uma sequência de bytes já pronta para ser enviada como comando para o robô.
        """

        j_speed = self.joints_target_speed * self.FIX_EFFECTOR_MASK  # Corrige assimetria de determinados efetores
        cmd = "".join(f"({self.joints_info[i].effector} {j_speed[i]:.5f})" for i in range(self.no_of_joints)).encode('utf-8')

        self.joints_target_last_speed = self.joints_target_speed  # Agora os dois apontam para o mesma referência
        self.joints_target_speed = np.zeros_like(self.joints_target_speed)  # Zera o array para o próximo comando

        return cmd
