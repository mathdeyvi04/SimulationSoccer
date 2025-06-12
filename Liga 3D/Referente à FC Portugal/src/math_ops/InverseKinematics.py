from math import asin, atan, atan2, pi, sqrt
from math_ops.Matriz3x3 import Matriz3x3
from math_ops.GeneralMath import GeneralMath
import numpy as np


class InverseKinematics:
    """
    Descrição:
        Responsável por calcular movimentos e poses das pernas de um robô humanoide (como o NAO),
        com base em posições tridimensionais dos tornozelos e orientações dos pés.
        Ela implementa os principais métodos de cinemática inversa para geração de trajetórias
        e cálculo dos ângulos articulares necessários para alcançar poses desejadas.

        A classe utiliza parâmetros morfológicos específicos de diferentes versões do robô
        (armazenados em `NAO_SPECS_PER_ROBOT`) e considera deslocamentos estruturais entre
        partes do corpo (como torso e quadril). Entre suas principais funcionalidades estão:

        - Conversão de coordenadas entre diferentes referências do corpo (cabeça, torso, quadril).
        - Cálculo da posição de partes corporais em relação aos quadris.
        - Geração de trajetórias para movimentar os tornozelos entre dois pontos no espaço.
        - Cálculo da cinemática inversa para uma pose específica do tornozelo (posição + orientação).

        Essa classe é projetada para ser integrada a um objeto `robot`, o qual fornece a estrutura de partes do corpo e estados articulares.

    Métodos Disponíveis:
        - torso_to_hip_transform
        - head_to_hip_transform
        - get_body_part_pos_relative_to_hip
        - get_ankle_pos_relative_to_hip
        - leg
        - get_linear_leg_trajectory

    Variáveis de Ambiente:
        - NAO_SPECS_PER_ROBOT:
            Cada tupla representa as especificações estruturais de um robô NAO. Cada valor dentro da tupla corresponde a:
            (leg_y_dev, upper_leg_height, upper_leg_depth, lower_leg_length, knee_extra_angle, max_ankle_z)

            1. leg_y_dev:
               Deslocamento lateral da perna no eixo Y. Ex: 0.055 m.

            2. upper_leg_height:
               Altura da coxa (do quadril ao joelho). Varia conforme o modelo do robô. Ex: 0.12, 0.13832, 0.147868424 m.

            3. upper_leg_depth:
               Deslocamento em profundidade (eixo Z) entre o quadril e o joelho, usado para compensação estrutural. Ex: 0.005 m.

            4. lower_leg_length:
               Comprimento da perna inferior (do joelho até o tornozelo). Ex: 0.1, 0.11832, 0.127868424 m.

            5. knee_extra_angle:
               Ângulo extra calculado como atan(upper_leg_depth / upper_leg_height), usado para corrigir a inclinação da junta do joelho.

            6. max_ankle_z:
               Posição mais baixa que o tornozelo pode atingir no eixo Z. Valor negativo em relação ao solo. Ex: -0.091, -0.106, -0.114 m.

        - TORSO_HIP_Z:
            Distância vertical (eixo Z) entre o centro do torso e o quadril. Constante para todos os robôs: 0.115 m.

        - TORSO_HIP_X:
            Deslocamento horizontal (eixo X) entre o centro do torso e o quadril. O quadril está 1 cm atrás do torso: 0.01 m.

    """

    # VERIFIQUE A DESCRIÇÃO ANTERIOR
    # leg y deviation, upper leg height, upper leg depth, lower leg length, knee extra angle, max ankle z
    NAO_SPECS_PER_ROBOT = ((0.055, 0.12, 0.005, 0.1, atan(0.005 / 0.12), -0.091),
                           (0.055, 0.13832, 0.005, 0.11832, atan(0.005 / 0.13832), -0.106),
                           (0.055, 0.12, 0.005, 0.1, atan(0.005 / 0.12), -0.091),
                           (0.072954143, 0.147868424, 0.005, 0.127868424, atan(0.005 / 0.147868424), -0.114),
                           (0.055, 0.12, 0.005, 0.1, atan(0.005 / 0.12), -0.091))

    TORSO_HIP_Z = 0.115  # distance in the z-axis, between the torso and each hip (same for all robots)
    TORSO_HIP_X = 0.01  # distance in the x-axis, between the torso and each hip (same for all robots) (hip is 0.01m to the back)

    def __init__(self, robot) -> None:
        """
        Descrição:
            Inicializa as específicações do robô.

        Parâmetros:
            - robot:
                Classe representadora do robô.
        """

        self.robot = robot
        # Tratando esse erro aqui, podemos não pensar nele
        try:
            self.NAO_SPECS = InverseKinematics.NAO_SPECS_PER_ROBOT[robot.type]

        except:
            print("O tipo fornecido ao InverseKinematics não é compatível.")

    @staticmethod
    def torso_to_hip_transform(coords: np.ndarray | list[np.ndarray], is_batch: bool = False) -> list | np.ndarray:
        """
        Descrição:
            Converte coordenadas cartesianas relativas ao torso do robô para coordenadas
            relativas ao centro entre as articulações dos quadris.
            A transformação é feita por uma translação fixa nas direções x e z,
            com base nas constantes `TORSO_HIP_X` e `TORSO_HIP_Z`.

            Suporta tanto coordenadas únicas (um vetor 3D) quanto lotes de coordenadas
            (lista de vetores 3D).

        Parâmetros:
            - coords: array_like

                Uma posição 3D (x, y, z) ou uma lista de posições 3D.

            - is_batch: bool

                Define se `coords` representa um lote de posições.
                - True: `coords` é uma lista de vetores 3D.
                - False: `coords` é um único vetor 3D.

        Retorno:
            - coord: ndarray ou list

                Um array NumPy com a coordenada transformada se `is_batch=False`, ou uma lista de arrays com as coordenadas transformadas se `is_batch=True`.
        """

        if is_batch:
            return [c + (Inverse_Kinematics.TORSO_HIP_X, 0, Inverse_Kinematics.TORSO_HIP_Z) for c in coords]
        else:
            return coords + (Inverse_Kinematics.TORSO_HIP_X, 0, Inverse_Kinematics.TORSO_HIP_Z)

    def head_to_hip_transform(self, coords: np.ndarray | list[np.ndarray], is_batch: bool = False):
        """
        Descrição:
            Converte coordenadas cartesianas relativas à cabeça do robô para coordenadas relativas ao centro entre as articulações dos quadris.
            Suporta tanto uma coordenada única quanto um lote de coordenadas.

        Parâmetros:
            - coords:
                array_like
                Uma posição 3D (x, y, z) ou uma lista de posições 3D relativas à cabeça do robô.

            - is_batch:
                bool
                Define se `coords` representa um lote de posições.
                - True: `coords` é uma lista de vetores 3D.
                - False: `coords` é um único vetor 3D.

        Retorno:
            - coord:
                ndarray ou list
                Um array NumPy com a coordenada transformada se `is_batch=False`, ou uma lista de arrays com as coordenadas transformadas se `is_batch=True`.
        """

        coords_rel_torso = self.robot.head_to_body_part_transform("torso", coords, is_batch)
        return self.torso_to_hip_transform(coords_rel_torso, is_batch)

    def get_body_part_pos_relative_to_hip(self, body_part_name: str):
        """
        Descrição:
            Obtém a posição de uma parte do corpo relativa ao centro entre as articulações dos quadris do robô.
            A posição da parte do corpo é inicialmente considerada no referencial da cabeça, sendo transformada
            para o referencial dos quadris por meio da função `head_to_hip_transform`.

        Parâmetros:
            - body_part_name:
                str
                Nome da parte do corpo cujo vetor de posição será obtido.
                Deve corresponder a uma chave válida do dicionário `self.robot.body_parts`.

        Retorno:
            - coord:
                ndarray
                Vetor 3D representando a posição da parte do corpo no referencial do centro entre as articulações dos quadris.
        """
        bp_rel_head = self.robot.body_parts[body_part_name].transform.get_translation()
        return self.head_to_hip_transform(bp_rel_head)

    def get_ankle_pos_relative_to_hip(self, is_left: bool):
        """
        Descrição:
            Apenas chama o `get_body_part_pos_relative_to_hip` para o ankle.
        """
        return self.get_body_part_pos_relative_to_hip("lankle" if is_left else "rankle")

    # Ambas funções a seguir são não-triviais, experimente ler as respectivas
    # documentações antes de qualquer coisa.
    def leg(
            self,
            ankle_pos3d: np.ndarray,
            foot_ori3d: np.ndarray,
            is_left: bool,
            dynamic_pose: bool
    ) -> tuple[list, np.ndarray, list]:
        """
        Descrição:
            Executa a cinemática inversa para uma das pernas do robô, determinando os ângulos articulares necessários
            para posicionar o tornozelo em uma posição 3D específica (`ankle_pos3d`) com uma orientação desejada (`foot_ori3d`).
            A posição do tornozelo é fornecida em coordenadas relativas ao centro entre os dois quadris.

            A orientação do pé pode ser definida com rotações em torno dos eixos x (roll), y (pitch) e z (yaw). No entanto, as rotações
            em x e y são consideradas desvios em relação a uma pose vertical estática ou dinâmica, dependendo do parâmetro `dynamic_pose`.

            O algoritmo verifica limites articulares e alcançabilidade da pose, retornando códigos de erro apropriados quando necessário.

        Parâmetros:
            - ankle_pos3d: array_like, comprimento 3

                Posição 3D do tornozelo (x, y, z), relativa ao centro entre as articulações dos quadris.

            - foot_ori3d: array_like, comprimento 3

                Orientação do pé em torno dos eixos (x, y, z).
                As rotações em x e y são tratadas como desvios (biases) em relação a uma pose vertical padrão ou dinâmica.
                A rotação em z (yaw) pode ser controlada diretamente.

            - is_left: bool

                Define se a perna usada é a esquerda (`True`) ou a direita (`False`).

            - dynamic_pose: bool

                Se `True`, ativa o ajuste dinâmico da orientação do pé com base nos dados da IMU, para manter o pé paralelo ao solo.

        Retorno:
            - indices: list

                Lista com os índices das articulações da perna selecionada.

            - values: list

                Lista com os valores calculados (em radianos) para as articulações.

            - error_codes: list

                Lista com códigos de erro associados ao cálculo:
                  - `-1`: posição do pé está fora do alcance (pose inalcançável).
                  - `x`: A articulação de índice `x` excedeu seus limites permitidos.
        """

        error_codes = []
        leg_y_dev, upper_leg_height, upper_leg_depth, lower_leg_len, knee_extra_angle, _ = self.NAO_SPECS
        sign = -1 if is_left else 1

        # Then we translate to origin of leg by shifting the y coordinate
        ankle_pos3d = np.asarray(ankle_pos3d) + (0, sign * leg_y_dev, 0)

        # First we rotate the leg, then we rotate the coordinates to abstract from the rotation
        ankle_pos3d = Matriz3x3().rotate_z_deg(-foot_ori3d[2]).multiply(ankle_pos3d)

        # Use geometric solution to compute knee angle and foot pitch
        dist = np.linalg.norm(ankle_pos3d)  # dist hip <-> ankle
        sq_dist = dist * dist
        sq_upper_leg_h = upper_leg_height * upper_leg_height
        sq_lower_leg_l = lower_leg_len * lower_leg_len
        sq_upper_leg_l = upper_leg_depth * upper_leg_depth + sq_upper_leg_h
        upper_leg_len = sqrt(sq_upper_leg_l)
        knee = GeneralMath.acos((sq_upper_leg_l + sq_lower_leg_l - sq_dist) / (2 * upper_leg_len * lower_leg_len)) + knee_extra_angle  # Law of cosines
        foot = GeneralMath.acos((sq_lower_leg_l + sq_dist - sq_upper_leg_l) / (2 * lower_leg_len * dist))  # foot perpendicular to vec(origin->ankle_pos)

        # Check if target is reachable
        if dist > upper_leg_len + lower_leg_len:
            error_codes.append(-1)

        # Knee and foot
        knee_angle = pi - knee
        foot_pitch = foot - atan(ankle_pos3d[0] / np.linalg.norm(ankle_pos3d[1:3]))
        foot_roll = atan(ankle_pos3d[1] / min(-0.05, ankle_pos3d[2])) * -sign  # avoid instability of foot roll (not relevant above -0.05m)

        # Raw hip angles if all joints were straightforward
        raw_hip_yaw = foot_ori3d[2]
        raw_hip_pitch = foot_pitch - knee_angle
        raw_hip_roll = -sign * foot_roll

        # Rotate 45deg due to yaw joint orientation, then rotate yaw, roll and pitch
        m = Matriz3x3().rotate_y_rad(raw_hip_pitch).rotate_x_rad(raw_hip_roll).rotate_z_deg(raw_hip_yaw).rotate_x_deg(-45 * sign)

        # Get actual hip angles considering the yaw joint orientation
        hip_roll = (pi / 4) - (sign * asin(m.m[1, 2]))  # Add pi/4 due to 45deg rotation
        hip_pitch = - atan2(m.m[0, 2], m.m[2, 2])
        hip_yaw = sign * atan2(m.m[1, 0], m.m[1, 1])

        # Convert rad to deg
        values = np.array([hip_yaw, hip_roll, hip_pitch, -knee_angle, foot_pitch, foot_roll]) * 57.2957795  # rad to deg

        # Set feet rotation bias (based on vertical pose, or dynamic_pose)
        values[4] -= foot_ori3d[1]
        values[5] -= foot_ori3d[0] * sign

        indices = [2, 4, 6, 8, 10, 12] if is_left else [3, 5, 7, 9, 11, 13]

        if dynamic_pose:
            # Rotation of torso in relation to foot
            m: Matriz3x3 = Matriz3x3.create_sup_matrix_rotation((self.robot.imu_torso_roll, self.robot.imu_torso_pitch, 0))
            m.rotate_z_deg(foot_ori3d[2], True)

            roll = m.get_roll_deg()
            pitch = m.get_pitch_deg()

            # Simple balance algorithm
            correction = 1  # correction to motivate a vertical torso (in degrees)
            roll = 0 if abs(roll) < correction else roll - np.copysign(correction, roll)
            pitch = 0 if abs(pitch) < correction else pitch - np.copysign(correction, pitch)

            values[4] += pitch
            values[5] += roll * sign

        # Check and limit range of joints
        for i in range(len(indices)):
            if values[i] < self.robot.joints_info[indices[i]].min or values[i] > self.robot.joints_info[indices[i]].max:
                error_codes.append(indices[i])
                values[i] = np.clip(values[i], self.robot.joints_info[indices[i]].min, self.robot.joints_info[indices[i]].max)

        return indices, values, error_codes

    def get_linear_leg_trajectory(
            self,
            is_left: bool,
            p1: np.ndarray | list,
            p2: np.ndarray | list = None,
            foot_ori3d=(0, 0, 0),
            dynamic_pose: bool = True,
            resolution=100
    ) -> tuple[list[int], list]:
        """
        Descrição:
            Gera uma trajetória linear para o movimento da perna de um robô, de forma que o tornozelo se desloque
            linearmente entre dois pontos 3D especificados no referencial dos quadris. A trajetória é amostrada
            com uma resolução definida e convertida em valores articulares por meio da função cinemática inversa
            da perna (`self.leg`). A orientação do pé pode ser ajustada manualmente ou de forma dinâmica
            com base na IMU.

            Se apenas `p1` for fornecido, o ponto inicial da trajetória será a posição atual do tornozelo da perna
            selecionada (esquerda ou direita).

        Parâmetros:
            - is_left: bool

                Define se a perna usada é a esquerda (`True`) ou a direita (`False`).

            - p1: array_like, comprimento 3

                Se `p2` for `None`: ponto final da trajetória. O ponto inicial será a posição atual do tornozelo.
                Se `p2` for fornecido: ponto inicial da trajetória.

            - p2: array_like, comprimento 3 ou `None`

                Ponto final da trajetória. Se `None`, `p1` será tratado como ponto final.

            - foot_ori3d: array_like, comprimento 3

                Orientação do pé em torno dos eixos x, y e z (em radianos).
                As rotações em x e y são tratadas como desvios em relação a uma pose vertical (ou pose dinâmica, se habilitada).

            - dynamic_pose: bool

                Se `True`, ajusta dinamicamente a orientação do pé com base nos dados da IMU para mantê-lo paralelo ao solo.

            - resolution: int

                Número de amostras na interpolação da trajetória linear.
                Resoluções maiores produzem trajetórias mais suaves, mas aumentam o custo computacional.
                Durante a otimização analítica, pontos redundantes são removidos.

        Retorno:
            - trajectory: tuple
                Uma tupla contendo:
                1. Lista de índices das articulações relevantes.
                2. Lista de pares `[valores_articulares, códigos_de_erro]` ao longo da trajetória interpolada.
                   Consulte a função `leg()` para mais detalhes sobre o formato dos valores e códigos.
        """

        if p2 is None:
            p2 = np.asarray(p1, float)
            p1 = self.get_body_part_pos_relative_to_hip('lankle' if is_left else 'rankle')
        else:
            p1 = np.asarray(p1, float)
            p2 = np.asarray(p2, float)

        vec = (p2 - p1) / resolution

        hip_points = [p1 + vec * i for i in range(1, resolution + 1)]
        interpolation = [self.leg(p, foot_ori3d, is_left, dynamic_pose) for p in hip_points]

        indices = [2, 4, 6, 8, 10, 12] if is_left else [3, 5, 7, 9, 11, 13]

        last_joint_values = self.robot.joints_position[indices[0:4]]  # exclude feet joints to compute ankle trajectory
        next_step = interpolation[0]
        trajectory = []

        for p in interpolation[1:-1]:
            if np.any(np.abs(p[1][0:4] - last_joint_values) > 7.03):
                trajectory.append(next_step[1:3])
                last_joint_values = next_step[1][0:4]
                next_step = p
            else:
                next_step = p

        trajectory.append(interpolation[-1][1:3])

        return indices, trajectory
