from agent.BaseAgent import BaseAgent
from math_ops.Matriz3x3 import Matriz3x3
from math_ops.Matriz4x4 import Matriz4x4
from sobre_scripts.commons.Script import Script
from world.commons.Draw import Draw
from world.Robot import Robot
import numpy as np

"""
Descrição:
    Demonstrar a precisão do IMU (Unidade de Medida Inercial) do robô.
    
    As variáveis Robot.imu_(...) são determinadas principalmente pelo algoritmo de localização visual. Quando não há dados visuais disponíveis, o sistema recorre ao IMU para estimar os valores.
    
    Se os dados visuais não estiverem disponíveis por mais de 0,2 segundos, a posição do robô é congelada e a velocidade decai gradualmente até zero.
    
    A orientação (rotação) calculada pelo IMU é extremamente precisa e nunca é congelada, independentemente do tempo sem dados visuais. Por isso, é quase sempre seguro confiar nos dados do IMU para rotação.

Limitações conhecidas:
    - O acelerômetro apresenta baixa confiabilidade em situações de picos de aceleração "instantâneos", devido à sua baixa taxa de amostragem (50Hz).
    - Essa limitação afeta principalmente a estimativa de translação durante quedas ou colisões com outros jogadores.
"""


class IMU:
    """
    Descrição:
        Classe utilitária para demonstração, análise e visualização de dados provenientes do IMU (Unidade de Medida Inercial) do robô.
        Gerencia o ciclo de movimentação de membros do robô para gerar diferentes estímulos ao IMU, armazena e manipula transformações e dados de posição, velocidade e aceleração do torso e do centro de massa.
        Pode ser utilizada como base para testes de precisão, resposta e integração do IMU ao sistema de localização e controle do agente.
    """

    def __init__(self, script: Script) -> None:
        """
        Descrição:
            Inicializa a classe IMU, definindo variáveis para controle de ciclos,
            armazenamento de transformações, posições, velocidades e acelerações do torso, além de preparar cores para visualização.

        Parâmetros:
            script: Script
                Referência ao script principal da aplicação.

        Retorno:
            None
        """
        self.script = script
        self.player: BaseAgent = None  # Referência ao agente utilizado
        self.cycle = 0  # Contador de ciclos para alternar movimentos

        # Matrizes de transformação e rotação do torso e da cabeça em relação ao campo (mantém histórico dos últimos 3 ciclos)
        self.imu_torso_to_field_rotation = [Matriz3x3() for _ in range(3)]
        self.imu_torso_to_field_transform = [Matriz4x4() for _ in range(3)]
        self.imu_head_to_field_transform = [Matriz4x4() for _ in range(3)]

        # Armazenam posição, velocidade e aceleração do torso nos três eixos (histórico dos últimos 3 ciclos)
        self.imu_torso_position = np.zeros((3, 3))
        self.imu_torso_velocity = np.zeros((3, 3))
        self.imu_torso_acceleration = np.zeros((3, 3))
        self.imu_torso_next_position = np.zeros((3, 3))
        self.imu_torso_next_velocity = np.zeros((3, 3))
        self.imu_CoM_position = np.zeros((3, 3))  # Centro de massa

        # Cores para uso em visualizações/plots
        self.colors = [Draw.Color.green_light, Draw.Color.yellow, Draw.Color.red]

    def act(self):
        """
        Descrição:
            Executa um ciclo de movimentação do robô, alternando posturas dos membros inferiores e superiores para gerar diferentes estímulos ao IMU.
            O objetivo é variar a orientação, velocidade e aceleração do torso, permitindo observar o comportamento e precisão do IMU em diferentes situações.
            O método alterna entre diferentes configurações de juntas ao longo de 200 ciclos, reiniciando o padrão ao final.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """
        r = self.player.world.robot

        # Índices das juntas a serem manipuladas (pernas e braços)
        joint_indices = [
            r.J_LLEG_PITCH,
            r.J_LKNEE,
            r.J_LFOOT_PITCH,
            r.J_LARM_ROLL,
            r.J_RLEG_PITCH,
            r.J_RKNEE,
            r.J_RFOOT_PITCH,
            r.J_RARM_ROLL
        ]

        # Amplitude de movimento, ajustada conforme o tipo de robô
        amplitude = [1, 0.93, 1, 1, 1][r.type]

        # Atualiza ciclo de execução
        self.cycle += 1

        # Alterna entre diferentes padrões de movimento a cada 50 ciclos
        if self.cycle < 50:
            r.set_joints_target_position_direct(
                joint_indices,
                np.array([32 + 10, -64, 32, 45, 40 + 10, -80, 40, 0]) * amplitude
            )
        elif self.cycle < 100:
            r.set_joints_target_position_direct(
                joint_indices,
                np.array([-10, 0, 0, 0, -10, 0, 0, 0]) * amplitude
            )
        elif self.cycle < 150:
            r.set_joints_target_position_direct(
                joint_indices,
                np.array([40 + 10, -80, 40, 0, 32 + 10, -64, 32, 45]) * amplitude
            )
        elif self.cycle < 200:
            r.set_joints_target_position_direct(
                joint_indices,
                np.array([-10, 0, 0, 0, -10, 0, 0, 0]) * amplitude
            )
        else:
            self.cycle = 0  # Reinicia o ciclo

        # Envia o comando para o simulador e aguarda o retorno do novo estado
        self.player.scom.commit_and_send(r.get_command())
        self.player.scom.receive()

    def act2(self):
        """
        Descrição:
            Executa um ciclo simples de movimentação do agente, comandando o robô para andar
            para frente a uma velocidade determinada.
            Utiliza o comportamento "Walk" com alvo relativo, sem orientação específica, para
            percorrer uma distância fixa.
            Após enviar o comando, sincroniza o agente com o simulador para garantir a execução
            do movimento.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """
        r = self.player.world.robot
        # Executa o comportamento de caminhar para frente
        self.player.behavior.execute("Walk", (0.2, 0), False, 5, False, None)  # Parâmetros: alvo, is_target_abs, ori, is_ori_abs, distância
        self.player.scom.commit_and_send(r.get_command())
        self.player.scom.receive()

    def draw_player_reference_frame(self, i):
        """
        Descrição:
            Desenha o referencial local do agente (frame do torso baseado no IMU) no
            ambiente de simulação, para o índice de histórico especificado.
            Adiciona setas representando os eixos x, y e z, anotações de cada eixo e uma
            esfera indicando a posição do centro de massa.
            Útil para visualização da orientação e posição estimadas pelo IMU ao longo do tempo.

        Parâmetros:
            i: int
                Índice do histórico a ser desenhado (0, 1 ou 2 tipicamente).

        Retorno:
            None
        """
        pos = self.imu_torso_position[i]
        # Calcula a posição dos eixos x, y, z após rotação pelo IMU
        xvec = self.imu_torso_to_field_rotation[i].multiply((1, 0, 0)) + pos
        yvec = self.imu_torso_to_field_rotation[i].multiply((0, 1, 0)) + pos
        zvec = self.imu_torso_to_field_rotation[i].multiply((0, 0, 1)) + pos
        # Desenha as setas de cada eixo do referencial
        self.player.world.draw.arrow(pos, xvec, 0.2, 2, self.colors[i], "IMU" + str(i), False)
        self.player.world.draw.arrow(pos, yvec, 0.2, 2, self.colors[i], "IMU" + str(i), False)
        self.player.world.draw.arrow(pos, zvec, 0.2, 2, self.colors[i], "IMU" + str(i), False)
        # Adiciona anotações para cada eixo
        self.player.world.draw.annotation(xvec, "x", Draw.Color.white, "IMU" + str(i), False)
        self.player.world.draw.annotation(yvec, "y", Draw.Color.white, "IMU" + str(i), False)
        self.player.world.draw.annotation(zvec, "z", Draw.Color.white, "IMU" + str(i), False)
        # Desenha uma esfera no centro de massa
        self.player.world.draw.sphere(self.imu_CoM_position[i], 0.04, self.colors[i], "IMU" + str(i), True)

    def compute_local_IMU(self):
        """
        Descrição:
            Calcula a estimativa local de posição, velocidade, aceleração e orientação do torso do robô utilizando exclusivamente os dados do IMU.
            Integra a orientação (rotação) proveniente do giroscópio e atualiza as transformações e posições do torso e da cabeça em relação ao campo.
            Aplica equações básicas de cinemática para prever a próxima posição e velocidade do torso, incorporando a aceleração medida e a gravidade.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """
        r = self.player.world.robot
        # Converte a leitura do giroscópio de graus/s para graus por ciclo de simulação
        g = r.gyro / 50
        # Atualiza a matriz de rotação do torso para o campo usando o giro do IMU
        self.imu_torso_to_field_rotation[2].multiply(
            Matrix_3x3.from_rotation_deg(g), in_place=True, reverse_order=True
        )
        # Atualiza posição e velocidade do torso
        self.imu_torso_position[2][:] = self.imu_torso_next_position[2]
        if self.imu_torso_position[2][2] < 0:
            self.imu_torso_position[2][2] = 0  # Limita z a valores positivos
        self.imu_torso_velocity[2][:] = self.imu_torso_next_velocity[2]
        # Corrige aceleração do IMU para coordenadas globais e soma gravidade
        self.imu_torso_acceleration[2] = self.imu_torso_to_field_rotation[2].multiply(r.acc) + Robot.GRAVITY
        # Atualiza transformações para torso e cabeça
        self.imu_torso_to_field_transform[2] = Matrix_4x4.from_3x3_and_translation(
            self.imu_torso_to_field_rotation[2], self.imu_torso_position[2]
        )
        self.imu_head_to_field_transform[2] = self.imu_torso_to_field_transform[2].multiply(
            r.body_parts["torso"].transform.invert()
        )
        # Atualiza posição do centro de massa (CoM)
        self.imu_CoM_position[2][:] = self.imu_head_to_field_transform[2](r.rel_cart_CoM_position)
        # Aplica equações de cinemática para prever próxima posição e velocidade
        self.imu_torso_next_position[2] = (
                self.imu_torso_position[2]
                + self.imu_torso_velocity[2] * 0.02
                + self.imu_torso_acceleration[2] * 0.0002
        )
        self.imu_torso_next_velocity[2] = (
                self.imu_torso_velocity[2] + self.imu_torso_acceleration[2] * 0.02
        )
        # Aplica fator de decaimento para estabilidade numérica
        self.imu_torso_next_velocity[2] *= Robot.IMU_DECAY

    def compute_local_IMU_rotation_only(self):
        """
        Descrição:
            Calcula apenas a orientação (rotação) do torso com base nos dados do IMU, sem atualizar posição, velocidade ou aceleração.
            Atualiza as transformações de torso e cabeça em relação ao campo, além da posição do centro de massa, utilizando a posição local do torso.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """
        r = self.player.world.robot
        # Converte leitura do giroscópio de graus/s para graus por ciclo de simulação
        g = r.gyro / 50
        # Atualiza matriz de rotação do torso para o campo usando apenas o giro do IMU
        self.imu_torso_to_field_rotation[1].multiply(
            Matrix_3x3.from_rotation_deg(g), in_place=True, reverse_order=True
        )
        # Atualiza posição do torso (usa localização visual/localizer)
        self.imu_torso_position[1][:] = r.loc_torso_position
        # Atualiza transformações para torso e cabeça
        self.imu_torso_to_field_transform[1] = Matrix_4x4.from_3x3_and_translation(
            self.imu_torso_to_field_rotation[1], self.imu_torso_position[1]
        )
        self.imu_head_to_field_transform[1] = self.imu_torso_to_field_transform[1].multiply(
            r.body_parts["torso"].transform.invert()
        )
        # Atualiza posição do centro de massa
        self.imu_CoM_position[1][:] = self.imu_head_to_field_transform[1](r.rel_cart_CoM_position)

    def update_local_IMU(self, i):
        """
        Descrição:
            Atualiza os dados locais do IMU para o índice especificado, copiando as estimativas e transformações internas do robô.
            Sincroniza as matrizes de rotação, transformações, posições, velocidades, acelerações e a posição do centro de massa.
            Utiliza as estimativas "weak" do IMU, que podem combinar dados inerciais e visuais.

        Parâmetros:
            i: int
                Índice do histórico de IMU a ser atualizado (ex: 0, 1 ou 2).

        Retorno:
            None
        """
        r = self.player.world.robot
        # Copia transformações de rotação, torso->campo e cabeça->campo do IMU do robô
        self.imu_torso_to_field_rotation[i].m[:] = r.imu_torso_to_field_rotation.m
        self.imu_torso_to_field_transform[i].m[:] = r.imu_weak_torso_to_field_transform.m
        self.imu_head_to_field_transform[i].m[:] = r.imu_weak_head_to_field_transform.m
        # Copia posição, velocidade e aceleração do torso
        self.imu_torso_position[i][:] = r.imu_weak_torso_position
        self.imu_torso_velocity[i][:] = r.imu_weak_torso_velocity
        self.imu_torso_acceleration[i][:] = r.imu_weak_torso_acceleration
        # Calcula próxima posição e velocidade usando cinemática
        self.imu_torso_next_position[i] = self.imu_torso_position[i] + self.imu_torso_velocity[i] * 0.02 + self.imu_torso_acceleration[i] * 0.0002
        self.imu_torso_next_velocity[i] = self.imu_torso_velocity[i] + self.imu_torso_acceleration[i] * 0.02
        # Copia posição do centro de massa (CoM)
        self.imu_CoM_position[i][:] = r.imu_weak_CoM_position

    def execute(self):
        """
        Descrição:
            Executa a sequência de experimentos e demonstrações para análise do IMU, incluindo posicionamento inicial, visualização de referenciais e integração de diferentes modos de estimativa (IMU puro, IMU para rotação, IMU + localizador).
            Realiza ciclos de movimentação, atualização e visualização dos referenciais do agente, alternando entre modos de utilização dos dados do IMU.
            Após a sequência, inicia um laço contínuo onde o agente começa a andar,
            permitindo observar a estimativa do IMU durante movimento.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """
        a = self.script.args
        # Instancia o agente principal
        self.player = Agent(a.i, a.p, a.m, a.u, a.r, a.t)  # Args: Server IP, Agent Port, Monitor Port, Uniform No., Robot Type, Team Name

        # Reposiciona o agente para a posição inicial desejada (beam)
        self.player.scom.unofficial_beam((-3, 0, self.player.world.robot.beam_height), 15)

        # Realiza múltiplos ciclos de envio para garantir reposicionamento
        for _ in range(10):  # beam to place
            self.player.scom.commit_and_send()
            self.player.scom.receive()

        # Adiciona anotação visual para modo IMU + localizador
        self.player.world.draw.annotation(
            (-3, 1, 1.1), "IMU + Localizer", self.colors[0], "note_IMU_1", True
        )

        # Executa ciclo de demonstração do IMU + localizador (referencial 0)
        for _ in range(150):
            self.act()
            self.update_local_IMU(0)
            self.draw_player_reference_frame(0)

        # Adiciona anotação visual para modo IMU apenas rotação
        self.player.world.draw.annotation(
            (-3, 1, 0.9), "IMU for rotation", self.colors[1], "note_IMU_2", True
        )
        self.update_local_IMU(1)

        # Executa ciclo de demonstração do IMU para rotação (referenciais 0 e 1)
        for _ in range(200):
            self.act()
            self.update_local_IMU(0)
            self.draw_player_reference_frame(0)
            self.compute_local_IMU_rotation_only()
            self.draw_player_reference_frame(1)

        # Adiciona anotação visual para modo IMU para rotação e posição
        self.player.world.draw.annotation(
            (-3, 1, 0.7), "IMU for rotation & position", self.colors[2], "note_IMU_3", True
        )
        self.update_local_IMU(2)

        # Executa ciclo de demonstração do IMU para rotação e posição (referenciais 0, 1, e 2)
        for _ in range(200):
            self.act()
            self.update_local_IMU(0)
            self.draw_player_reference_frame(0)
            self.compute_local_IMU_rotation_only()
            self.draw_player_reference_frame(1)
            self.compute_local_IMU()
            self.draw_player_reference_frame(2)

        print("\nPress ctrl+c to return.")

        # Loop contínuo: mantém atualização dos referenciais enquanto o agente anda
        # Mostra simultaneamente: IMU+Localizer (0), IMU Rotação (1), IMU Rotação+Posição (2)
        self.update_local_IMU(2)
        while True:
            self.act2()
            self.update_local_IMU(0)
            self.draw_player_reference_frame(0)
            self.compute_local_IMU_rotation_only()
            self.draw_player_reference_frame(1)
            self.compute_local_IMU()
            self.draw_player_reference_frame(2)



