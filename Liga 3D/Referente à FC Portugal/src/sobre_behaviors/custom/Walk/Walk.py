from behaviors.custom.Walk.Env import Env
from math_ops.GeneralMath import GeneralMath
from math_ops.NeuralNetwork import run_mlp
import numpy as np
import pickle


class Walk:

    def __init__(self, base_agent) -> None:
        """
        Descrição:
            Inicializa o controlador de caminhada omnidirecional baseada em aprendizado por reforço (RL).

            - O modelo de Walk é carregado dinamicamente conforme o tipo do robô, garantindo compatibilidade e ajuste fino do comportamento de caminhada.

            - O arquivo de modelo é selecionado a partir de um vetor de caminhos, utilizando o tipo do robô como índice.

            - O ambiente 'Env' encapsula a interface entre o agente base e o modelo RL, permitindo fácil integração e manipulação dos estados e ações.

        Parâmetros:
            base_agent: Base_Agent
                Instância do agente base, utilizada para acessar o mundo, o robô e outros recursos compartilhados.

        Atributos Definidos:
            world: World
                Referência ao ambiente/mundo do agente, herdada do base_agent.
            description: str
                Descrição breve do comportamento implementado.
            auto_head: bool
                Indica se a orientação automática da cabeça está habilitada para este comportamento.
            env: Env
                Ambiente de simulação/env interface para integração com o agente RL.
            last_executed: int
                Guarda o timestamp do último comando executado (pode ser usado para controle de frequência).
            model: object
                Modelo de aprendizado por reforço carregado a partir de arquivo pickle, específico para o tipo de robô.
        """

        self.world = base_agent.world
        self.description = "Caminhado OminiDirecional"
        self.auto_head = True
        self.env = Env(base_agent)
        self.last_executed = 0

        # Carrega o modelo RL adequado ao tipo do robô, utilizando pickle
        with open(
                GeneralMath.obter_diretorio_ativo(
                    [
                     "/sobre_behaviors/custom/Walk/walk_R0.pkl",
                     "/sobre_behaviors/custom/Walk/walk_R1_R3.pkl",
                     "/sobre_behaviors/custom/Walk/walk_R2.pkl",
                     "/sobre_behaviors/custom/Walk/walk_R1_R3.pkl",
                     "/sobre_behaviors/custom/Walk/walk_R4.pkl"
                ][self.world.robot.type]),
                'rb'
        ) as f:
            self.model = pickle.load(f)

    def execute(
            self,
            reset: bool,
            target_2d: tuple | np.ndarray,
            is_target_absolute: bool,
            orientation: float | None,
            is_orientation_absolute: bool,
            distance: float | None
    ):
        """
        Descrição:
            Executa um passo do comportamento de caminhada omnidirecional, controlado via modelo de aprendizado por reforço (RL).
            Calcula e ajusta os parâmetros de caminhada para que o robô se aproxime do alvo 2D desejado, podendo ainda controlar a orientação
            do torso e a velocidade conforme a distância ao alvo.

            - O parâmetro 'reset' é sobrescrito se o passo anterior foi executado exatamente 20 ms antes (proteção contra "duplo reset").
            - Se o alvo está em coordenadas absolutas, converte para o referencial do torso do robô levando em conta a posição da cabeça e orientação do torso.
            - Se a distância não for especificada, é calculada automaticamente a partir da posição relativa ao alvo.
            - A orientação é suavizada (multiplicada por 0.3) quando relativa, para evitar sobrecorreção e oscilações.
            - Observação do estado é extraída via self.env.observe e a ação é gerada pelo modelo RL para ser executada no ambiente.


        Parâmetros:
            reset: bool
                Indica se o comportamento deve ser reinicializado no passo atual.
            target_2d: array_like
                Posição alvo 2D (x, y), em coordenadas absolutas ou relativas ao torso do robô (definido por is_target_absolute).
            is_target_absolute: bool
                True se target_2d está em coordenadas absolutas do campo;
                False se está em coordenadas relativas ao torso do robô.
            orientation: float ou None
                Ângulo desejado para a orientação do torso (graus). Pode ser absoluto ou relativo, conforme is_orientation_absolute.
                Se None, a orientação é ajustada automaticamente para "apontar" na direção do alvo.
            is_orientation_absolute: bool
                True se orientation é em relação ao campo (global);
                False se é relativo ao torso do robô.
                Ignorado se orientation for None.
            distance: float ou None
                Distância ao alvo final (tipicamente entre 0 e 0.5 metros). Afeta a velocidade de abordagem ao alvo.
                Se None, considera target_2d como alvo final e calcula a distância automaticamente.

        Retorno:
            bool
                Sempre retorna False (compatível com interface de comportamentos,
                pode ser alterado para indicar término futuramente).
        """

        r = self.world.robot

        # 0. Sobrescreve reset para evitar duplo reset em ciclos de 20ms (ex: subcomportamentos)
        if reset and self.world.time_local_ms - self.last_executed == 20:
            reset = False
        self.last_executed = self.world.time_local_ms

        # 1. Define parâmetros de caminhada

        if is_target_absolute:
            # Se o alvo está em coordenadas absolutas, converte para relativas ao torso (corrigidas pela orientação)
            raw_target = target_2d - r.loc_head_position[:2]
            self.env.walk_rel_target = GeneralMath.rotate_vector_2D(raw_target, -r.imu_torso_orientation)
        else:
            # Caso seja relativo, já utiliza diretamente
            self.env.walk_rel_target = target_2d

        # Determina distância ao alvo (influencia velocidade de aproximação)
        if distance is None:
            self.env.walk_distance = np.linalg.norm(self.env.walk_rel_target)
        else:
            self.env.walk_distance = distance  # valor máximo típico = 0.5

        # Calcula orientação desejada do torso
        # Se não especificada, aponta suavemente para a direção do alvo
        if orientation is None:
            self.env.walk_rel_orientation = GeneralMath.angle_horizontal_from_vector2D(self.env.walk_rel_target) * 0.3
        elif is_orientation_absolute:
            self.env.walk_rel_orientation = GeneralMath.normalize_deg(orientation - r.imu_torso_orientation)
        else:
            self.env.walk_rel_orientation = orientation * 0.3  # suavização para evitar overshoot

        # 2. Executa o comportamento de caminhada via RL

        # Observa o estado atual, gera ação via modelo RL e executa no ambiente
        obs = self.env.observe(reset)
        # Executamos a ação caminhar como um vetor.
        action = run_mlp(obs, self.model)
        self.env.execute(action)

        # Retorna False sempre (interface compatível com outros comportamentos)
        return False

    def is_ready(self):
        """ Returns True if Walk Behavior is ready to start under current game/robot conditions """
        return True
