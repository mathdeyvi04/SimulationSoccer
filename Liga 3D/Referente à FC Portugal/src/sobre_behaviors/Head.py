from math_ops.GeneralMath import GeneralMath
from world.World import World
import numpy as np


class Head:
    """
    Descrição:
        Classe responsável pelo controle do "head movement" do agente, ou seja,
        pela lógica de movimentação e orientação da cabeça do robô para percepção do ambiente.
        - Gerencia o estado do movimento da cabeça (por exemplo, alternando entre olhar para a esquerda e para a direita).
        - Define ângulos e pontos de referência importantes para a movimentação da cabeça, como cantos do campo e postes.
        - Permite integração direta com o objeto do mundo (`World`), utilizando informações globais e de percepção.

    Variáveis de Ambiente:
        FIELD_FLAGS : list
            Lista de posições de referência para a cabeça, composta por cantos do campo e postes.
        HEAD_PITCH : float
            Ângulo fixo de inclinação da cabeça (em graus), por padrão -35º, para garantir boa visão do campo.

    Métodos Disponíveis:
        - execute
        - compute_best_direction
    """

    FIELD_FLAGS = World.FLAGS_CORNERS_POS + World.FLAGS_POSTS_POS  # Pontos de referência para olhar com a cabeça
    HEAD_PITCH = -35  # Ângulo padrão de inclinação da cabeça (pitch) para visão do campo

    def __init__(self, world: World) -> None:
        """
        Descrição:
            Inicializa o controlador da cabeça do agente.

            - Inicializa a cabeça olhando para a esquerda.
            - Define o estado inicial como 0.

        Parâmetros:
            world: World
                Instância do ambiente/mundo do agente, necessária para acessar referências globais,
                posições e informações de percepção.
        """
        self.world = world            # Referência ao objeto do mundo/percepção global
        self.look_left = True         # Começa olhando para a esquerda
        self.state = 0                # Estado inicial (pode ser usado para FSM de varredura)

    def compute_best_direction(self, can_self_locate, use_ball_from_vision=False):
        """
        Descrição:
            Calcula e retorna a melhor direção (em graus) para a cabeça do robô olhar,
            priorizando manter a bola no campo de visão e, se necessário, posicionar a
            cabeça para observar também as bandeiras de referência do campo.

            - O método busca garantir que a bola permaneça centralizada no FOV (campo de visão) do robô, respeitando margens de segurança.
            - Se possível, tenta também manter uma bandeira de referência visível, auxiliando na autolocalização.
            - Se a bola estiver muito próxima do robô, o ângulo é centralizado (0°).
            - Limita o movimento da cabeça para evitar rotações excessivas (saturação em +/-119°).

        Parâmetros:
            can_self_locate : bool
                Indica se o robô consegue se autolocalizar no momento (localização visual válida).
            use_ball_from_vision : bool, opcional
                Se True, usa a posição relativa da bola percebida pela visão; caso contrário,
                utiliza a posição absoluta da bola.

        Retorno:
            best_dir : float
                Ângulo (em graus) para o qual a cabeça do robô deve ser orientada, considerando a priorização da bola e das bandeiras de referência.
        """

        FOV_MARGIN = 15  # Margem de segurança para evitar extremos horizontais do FOV
        SAFE_RANGE = 120 - FOV_MARGIN * 2  # Faixa segura dentro do campo de visão
        HALF_RANGE = SAFE_RANGE / 2  # Metade do range seguro

        w = self.world
        r = w.robot

        # Calcula a distância 2D da bola, usando visão direta ou posição absoluta
        if use_ball_from_vision:
            ball_2d_dist = np.linalg.norm(w.ball_rel_torso_cart_pos[:2])
        else:
            ball_2d_dist = np.linalg.norm(w.ball_abs_pos[:2] - r.loc_head_position[:2])

        # Determina a direção da bola (ângulo relativo ao torso)
        if ball_2d_dist > 0.12:
            if use_ball_from_vision:
                ball_dir = GeneralMath.angle_horizontal_from_vector2D(w.ball_rel_torso_cart_pos[:2])
            else:
                ball_dir = GeneralMath.angle_rel_between_target_position(r.loc_head_position, r.imu_torso_orientation, w.ball_abs_pos)
        else:
            # Se a bola está muito perto do robô, centraliza a cabeça (0°)
            ball_dir = 0

        flags_diff = dict()

        # Itera sobre as bandeiras do campo e calcula a diferença angular em relação à bola
        for f in Head.FIELD_FLAGS:
            flag_dir = GeneralMath.angle_rel_between_target_position(r.loc_head_position, r.imu_torso_orientation, f)
            diff = GeneralMath.normalize_deg(flag_dir - ball_dir)
            # Se a bandeira estiver dentro do FOV e o robô consegue se localizar, priorize centralizar a bola
            if abs(diff) < HALF_RANGE and can_self_locate:
                return ball_dir  # Bola centralizada, retorna direção da bola
            flags_diff[f] = diff

        # Busca a bandeira mais próxima da direção da bola
        closest_flag = min(flags_diff, key=lambda k: abs(flags_diff[k]))
        closest_diff = flags_diff[closest_flag]

        if can_self_locate:
            # Se puder se localizar, calcula a diferença mínima para manter a bola centralizada e incluir a bandeira
            # Garante que não exceda o SAFE_RANGE
            final_diff = min(abs(closest_diff) - HALF_RANGE, SAFE_RANGE) * np.sign(closest_diff)
        else:
            # Se não pode se localizar, tenta aproximar a bandeira mais próxima enquanto mantém a bola visível
            final_diff = np.clip(closest_diff, -SAFE_RANGE, SAFE_RANGE)
            # Saturação para evitar rotação excessiva da cabeça
            return np.clip(ball_dir + final_diff, -119, 119)

        # Retorna o ângulo final normalizado
        return GeneralMath.normalize_deg(ball_dir + final_diff)

    def execute(self) -> None:
        """
        Descrição:
            Executa a lógica de movimentação da cabeça do robô, escolhendo a direção ideal para buscar ou acompanhar a bola,
            alternando entre modos guiados por percepção e busca aleatória conforme o contexto.

            - O método busca, prioritariamente, manter a bola no campo de visão (FOV) do robô, utilizando informações visuais e de localização própria.
            - Caso a bola não esteja visível ou o robô não consiga se localizar, alterna entre modos de busca guiada (usando informações recentes)
              e busca aleatória (random search) após atingir um limite de tentativas (TIMEOUT).
            - O método também alterna a direção da cabeça entre esquerda e direita durante a busca aleatória, para varrer amplamente o campo.

            Estados:
                0                - Ajuste preciso da cabeça: bola no FOV e localização própria disponível.

                1..TIMEOUT-1     - Busca guiada: utiliza informações recentes, incrementando o contador de tentativas.

                TIMEOUT          - Busca aleatória: varredura ampla, alternando lados, após esgotar tentativas guiadas.
        """

        TIMEOUT = 30  # Número máximo de tentativas de busca guiada antes de entrar em modo aleatório
        w = self.world
        r = w.robot

        # Determina se o robô consegue se localizar com base na atualização recente de localização visual
        can_self_locate = r.loc_last_update > w.time_local_ms - w.VISUALSTEP_MS

        # --------------------------------------- A. Bola está no FOV e o robô pode se localizar
        best_dir = None
        if w.ball_last_seen > w.time_local_ms - w.VISUALSTEP_MS:
            if can_self_locate:
                # Direção ideal baseada em visão e autolocalização
                best_dir = self.compute_best_direction(can_self_locate, use_ball_from_vision=True)
                self.state = 0  # Reseta para o estado mais confiável
            # --------------------------------------- B. Bola está no FOV, mas o robô NÃO consegue se localizar
            elif self.state < TIMEOUT:
                best_dir = self.compute_best_direction(can_self_locate, use_ball_from_vision=True)
                self.state += 1  # Incrementa o estado para busca guiada
        # --------------------------------------- C. Bola NÃO está no FOV
        elif self.state < TIMEOUT:
            best_dir = self.compute_best_direction(can_self_locate)
            self.state += 1  # Incrementa o estado para busca guiada

        # --------------------------------------- D. Busca aleatória após TIMEOUT tentativas
        if self.state == TIMEOUT:
            if w.ball_last_seen > w.time_local_ms - w.VISUALSTEP_MS:
                # Bola está no FOV: busca 45 graus para ambos os lados da bola
                ball_dir = M.vector_angle(w.ball_rel_torso_cart_pos[:2])
                targ = np.clip(ball_dir + (45 if self.look_left else -45), -119, 119)
            else:
                # Bola não está no FOV: busca 119 graus para ambos os lados do campo
                targ = 119 if self.look_left else -119

            # Tenta mover a cabeça para a posição alvo; se falhar, alterna o lado
            if r.set_joints_target_position_direct([0, 1], np.array([targ, Head.HEAD_PITCH]), False) <= 0:
                self.look_left = not self.look_left
        else:
            # Ajuste preciso ou busca guiada: move a cabeça para a melhor direção estimada
            # Muito foda movendo a cabeça
            r.set_joints_target_position_direct([0, 1], np.array([best_dir, Head.HEAD_PITCH]), False)




















