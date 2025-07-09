from sobre_behaviors.custom.Step.StepGenerator import StepGenerator
import numpy as np


class Step:

    def __init__(self, base_agent) -> None:
        """
        Descrição:
            Inicializa o controlador primitivo de passo (Skill-Set-Primitive Step), configurando os parâmetros 
            essenciais para a geração de movimentos de caminhada do robô.

            - `nao_specs` contém especificações do robô (como dimensões dos membros), tipicamente extraídas da cinemática inversa.
            - `feet_y_dev` define o desvio lateral dos pés, multiplicado por 1.2 para dar um passo mais largo que o padrão.
            - `sample_time` determina o intervalo de amostragem dos passos, obtido dos parâmetros do robô.
            - `max_ankle_z` define a altura máxima do tornozelo durante o passo.
            - O gerador de passos (`StepGenerator`) é inicializado com esses parâmetros para garantir movimentos naturais e seguros.


        Parâmetros:
            base_agent: Base_Agent
                Instância do agente base, fornecendo acesso ao mundo simulado, cinemática inversa 
                e parâmetros do robô.

        Atributos Definidos:
            world: World
                Referência ao mundo do robô, permitindo acesso ao tempo, estado, etc.
            ik: object
                Referência ao módulo de cinemática inversa do agente base, usado para calcular movimentos das pernas.
            description: str
                Descrição textual do comportamento ou primitivo.
            auto_head: bool
                Indica se a orientação automática da cabeça está habilitada neste primitivo.
            leg_length: float
                Comprimento total da perna do robô, calculado como a soma dos comprimentos da coxa e da canela.
            step_generator: StepGenerator
                Instância do gerador de passos, configurada com os parâmetros específicos do robô.
        """

        self.values_l = None
        self.world = base_agent.world
        self.ik = base_agent.inv_kinematics
        self.description = "Step (Skill-Set-Primitive)"
        self.auto_head = True

        # Especificações do robô NAO: [distância entre tornozelos, altura da coxa, ...]
        nao_specs = self.ik.NAO_SPECS
        # Comprimento da perna: soma da altura da coxa e da canela
        self.leg_length = nao_specs[1] + nao_specs[3]  # upper leg height + lower leg height

        # Desvio lateral dos pés (step width), aumentado para passos mais largos e maior estabilidade
        feet_y_dev = nao_specs[0] * 1.2  # wider step
        # Tempo de amostragem do passo (duração de cada passo)
        sample_time = self.world.robot.STEPTIME
        # Altura máxima do tornozelo durante o passo (para garantir que o pé levante suficiente)
        max_ankle_z = nao_specs[5]

        # Inicializa o gerador de passos com os parâmetros calculados
        self.step_generator = StepGenerator(feet_y_dev, sample_time, max_ankle_z)

    def execute(self, reset, ts_per_step=7, z_span=0.03, z_max=0.8):
        """
        Descrição:
            Executa um passo do comportamento de caminhada primitiva, gerando posições alvo para as pernas com base
            no gerador de passos, aplicando cinemática inversa (IK) e enviando comandos para as juntas do robô.

            - Calcula as posições alvo das pernas usando o gerador de passos, considerando os parâmetros de altura e tempo do passo.
            - Aplica cinemática inversa (IK) para converter as posições alvo em ângulos de junta para cada perna.
            - Emite avisos caso algum ângulo de junta esteja fora do alcance ou a posição seja inalcançável.
            - Define valores fixos para as juntas dos braços, garantindo uma postura estável durante o passo.
            - Envia comandos diretamente para as juntas do robô usando 'set_joints_target_position_direct'.

        Parâmetros:
            reset: bool
                Se True, reinicializa o gerador de passos para iniciar um novo ciclo de movimento.
            ts_per_step: int, opcional
                Quantidade de ciclos de controle por passo (quanto menor, mais rápido o passo). Padrão = 7.
            z_span: float, opcional
                Amplitude vertical do passo (altura máxima do pé em relação ao solo). Padrão = 0.03 metros.
            z_max: float, opcional
                Fator de escala do comprimento máximo da perna, usado para limitar a altura máxima alcançável pelo pé. Padrão = 0.8.

        Retorno:
            bool
                Sempre retorna False (interface compatível com outros comportamentos; pode ser alterado futuramente para indicar término).
        """

        # Obtém posições alvo para as pernas (y e z para cada perna) a partir do gerador de passos
        lfy, lfz, rfy, rfz = self.step_generator.get_target_positions(
            reset, ts_per_step, z_span, self.leg_length * z_max
        )

        # Aplica cinemática inversa (IK) e envia comandos para as pernas

        # Perna esquerda
        indices, self.values_l, error_codes = self.ik.leg(
            (0, lfy, lfz), (0, 0, 0), True, dynamic_pose=False
        )
        # Verifica e reporta possíveis erros de alcance das juntas
        for i in error_codes:
            print(f"Joint {i} is out of range!" if i != -1 else "Position is out of reach!")

        self.world.robot.set_joints_target_position_direct(indices, self.values_l)

        # Perna direita
        indices, self.values_r, error_codes = self.ik.leg(
            (0, rfy, rfz), (0, 0, 0), False, dynamic_pose=False
        )
        for i in error_codes:
            print(f"Joint {i} is out of range!" if i != -1 else "Position is out of reach!")

        self.world.robot.set_joints_target_position_direct(indices, self.values_r)

        # ----------------- Define postura fixa para os braços durante o passo

        # Braço esquerdo (juntas 14,16,18,20)
        indices = [14, 16, 18, 20]
        values = np.array([-80, 20, 90, 0])
        self.world.robot.set_joints_target_position_direct(indices, values)

        # Braço direito (juntas 15,17,19,21)
        indices = [15, 17, 19, 21]
        values = np.array([-80, 20, 90, 0])
        self.world.robot.set_joints_target_position_direct(indices, values)

        # Sempre retorna False (comportamento contínuo)
        return False

    def is_ready(self):
        """ Returns True if Step Behavior is ready to start under current game/robot conditions """
        return True





















