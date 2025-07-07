from agent.BaseAgent import BaseAgent
from pathlib import Path
from scripts.commons.Server import Server
from scripts.commons.Train_Base import TrainBase
from stable_baselines3 import PPO
from stable_baselines3.common.base_class import BaseAlgorithm
from stable_baselines3.common.vec_env import SubprocVecEnv
from time import sleep
from world.commons.Draw import Draw
import gym
import numpy as np
import os

"""
Objetivo:
Ensinar o robô a se levantar (4 variantes, veja a linha 157).
Otimizar cada quadro-chave (keyframe) dos comportamentos já existentes para diferentes situações.

----------
- Classe Get_Up: implementa um ambiente customizado no padrão OpenAI Gym para a tarefa de levantar-se.
- Classe Train: implementa os algoritmos necessários para treinar um novo modelo ou testar um modelo existente.
"""


class GetUp(gym.Env):

    def __init__(self, ip, server_p, monitor_p, r_type, fall_direction, enable_draw) -> None:
        """
        Descrição:
            Inicializa o ambiente de simulação para a tarefa de levantar-se após uma queda.

        Parâmetros
            ip: str
                Endereço IP do servidor ao qual o agente irá se conectar.
            server_p: int
                Porta do servidor utilizada para comunicação do agente.
            monitor_p: int
                Porta do monitor utilizada para visualização/monitoramento do agente.
            r_type: int
                Tipo do robô utilizado na simulação.
            fall_direction: int
                Direção da queda: 0 = frente, 1 = lado esquerdo, 2 = lado direito, 3 = costas.
            enable_draw: bool
                Se True, ativa a visualização gráfica durante a simulação.
        """
        self.robot_type = r_type
        self.fall_direction = fall_direction  # 0: frente, 1: lado esquerdo, 2: lado direito, 3: costas

        # Inicializa o agente de simulação com os parâmetros fornecidos
        self.player = BaseAgent(ip, server_p, monitor_p, 1, self.robot_type, "Gym", True, enable_draw, [])

        # Dicionário para mapear direções de queda aos nomes dos comportamentos de levantar
        self.get_up_names = {0: "Get_Up_Front", 1: "Get_Up_Side_Left", 2: "Get_Up_Side_Right", 3: "Get_Up_Back"}

        # Backup dos slots originais do comportamento de levantar, para permitir restauração posterior
        self.original_slots = []
        for delta_ms, indices, angles in self.player.behavior.slot_engine.behaviors[self.get_up_names[self.fall_direction]]:
            self.original_slots.append((delta_ms, indices, np.array(angles)))

        # Observação é uma codificação one-hot indicando o slot atual
        self.obs = np.identity(len(self.original_slots))
        self.current_slot = 0  # Índice do slot atualmente ativo

        # Espaço de ação: vetor de 11 floats, valores entre -MAX e +MAX
        MAX = np.finfo(np.float32).max
        self.action_space = gym.spaces.Box(
            low=np.full(11, -MAX, np.float32),
            high=np.full(11, MAX, np.float32),
            dtype=np.float32
        )

        # Espaço de observação: vetor one-hot de tamanho igual ao número de slots
        self.observation_space = gym.spaces.Box(
            low=np.zeros(len(self.obs), np.float32),
            high=np.ones(len(self.obs), np.float32),
            dtype=np.float32
        )

    def fall(self) -> None:
        """
        Descrição:
            Executa uma ação de queda do robô na direção especificada ao inicializar a classe.
            Define os ângulos-alvo das articulações dos pés e pernas para provocar a queda escolhida.

        Retorno:
            Envia o comando ao simulador e aguarda a atualização do estado do ambiente.
        """
        r = self.player.world.robot
        # Índices das articulações relevantes para provocar a queda
        joint_indices = [
            r.J_LFOOT_PITCH,  # Articulação do tornozelo esquerdo
            r.J_RFOOT_PITCH,  # Articulação do tornozelo direito
            r.J_LLEG_ROLL,  # Rolagem da perna esquerda
            r.J_RLEG_ROLL  # Rolagem da perna direita
        ]

        if self.fall_direction == 0:  # Frente
            r.set_joints_target_position_direct(joint_indices, np.array([50, 50, 0, 0]))
        elif self.fall_direction == 1:  # Lado esquerdo
            r.set_joints_target_position_direct(joint_indices, np.array([0, 0, -20, 20]))
        elif self.fall_direction == 2:  # Lado direito
            r.set_joints_target_position_direct(joint_indices, np.array([0, 0, 20, -20]))
        elif self.fall_direction == 3:  # Costas
            r.set_joints_target_position_direct(joint_indices, np.array([-20, -20, 0, 0]))
        else:
            raise ValueError("Direção de queda inválida.")

        # Envia o comando para o simulador e atualiza o estado
        self.player.scom.commit_and_send(r.get_command())
        self.player.scom.receive()

    def get_up(self) -> bool:
        """
        Descrição:
            Executa a sequência de levantar do chão conforme a direção da queda.
            Utiliza o comportamento apropriado (slot) cadastrado no agente e envia o comando ao simulador.
            Aguarda a resposta do ambiente e retorna se o comportamento foi finalizado.

        Retorno:
            finished: bool
                Indica se o comportamento de levantar foi concluído.
        """
        r = self.player.world.robot
        # Executa o comportamento de levantar correspondente à direção da queda
        finished = self.player.behavior.execute(self.get_up_names[self.fall_direction])

        # Envia o comando para o simulador e atualiza o estado
        self.player.scom.commit_and_send(r.get_command())
        self.player.scom.receive()
        return finished

    def other(self, behavior_name: str) -> None:
        """
        Descrição:
            Executa um comportamento arbitrário do agente, especificado pelo nome.
            Envia o comando ao simulador e atualiza o estado do ambiente.

        Parâmetros:
            behavior_name: str
                Nome do comportamento a ser executado (deve estar registrado no agente).
        """
        r = self.player.world.robot
        # Executa o comportamento especificado
        self.player.behavior.execute(behavior_name)
        # Envia o comando para o simulador e atualiza o estado
        self.player.scom.commit_and_send(r.get_command())
        self.player.scom.receive()

    def reset(self):
        """
        Reinicializa o ambiente para o início de um novo episódio.

        - Posiciona o robô em uma posição inicial com o comando 'beam'.
        - Executa a sequência de queda diversas vezes para garantir que o robô esteja caído.
        - Garante que o robô permaneça no chão, repetindo a queda caso necessário.
        - Executa o comportamento "Zero" por um número aleatório de passos para estabilizar o robô após a queda.
        - Reinicia o índice do slot atual para o início da sequência de levantar.
        - Retorna a observação correspondente ao slot inicial (one-hot encoding).

        Retorno:
            obs: np.ndarray
                Vetor de observação correspondente ao slot inicial.
        """
        # Posiciona o robô na coordenada inicial usando o comando 'beam'
        self.player.scom.commit_beam((-3, 0), 0)

        # Executa a ação de queda múltiplas vezes para garantir que o robô caia
        for _ in range(30):
            self.fall()

        # Garante que o robô permaneça caído: repete a queda enquanto ele estiver acima de uma altura mínima
        while self.player.world.robot.cheat_abs_pos[2] > 0.32:
            self.fall()

        # Executa o comportamento "Zero" para estabilizar o robô após a queda
        import random
        t = random.randint(7, 17) if self.fall_direction == 0 else random.randint(10, 20)
        for _ in range(t):
            self.other("Zero")

        # Reseta o índice do slot atual para o início da sequência de levantar
        self.current_slot = 0

        # Retorna a observação correspondente ao slot inicial
        return self.obs[self.current_slot]

    def render(self, mode='human', close=False):
        """
        Método padrão do Gym para renderização do ambiente.
        Não implementa renderização visual neste ambiente, mas pode ser sobrescrito em subclasses.
        """
        return

    def close(self):
        """
        Encerra e limpa o ambiente, removendo desenhos e fechando a conexão com o simulador.
        Deve ser chamado ao final do uso do ambiente para liberar recursos.
        """
        # Limpa visualizações/desenhos
        Draw.clear_all()
        # Encerra a comunicação com o simulador
        self.player.scom.close()

    @staticmethod
    def scale_action(action: np.ndarray):
        """
        Escala e expande o vetor de ações fornecido para adequação aos comandos do robô.

        - O primeiro elemento é multiplicado por 10 (corresponde geralmente ao tempo ou parâmetro principal).
        - Os demais elementos são multiplicados por 3 e duplicados para criar simetria entre os lados do robô.

        Parâmetros:
            action: np.ndarray
                Vetor de ações original.

        Retorno:
            new_action: np.ndarray
                Vetor de ações escalado e expandido para ser utilizado como comando.
        """
        new_action = np.zeros(len(action) * 2 - 1, action.dtype)
        new_action[0] = action[0] * 10
        new_action[1:] = np.repeat(action[1:] * 3, 2)  # Expande ações simétricas para ambos os lados do robô

        return new_action

    @staticmethod
    def get_22_angles(angles, indices):
        """
        Preenche um vetor de 22 posições com os ângulos fornecidos nos índices apropriados.

        - Assegura que todas as juntas do robô (exceto dedos dos pés) estejam representadas,
          mesmo que nem todas tenham valores definidos (preenchendo com zero onde necessário).

        Parâmetros:
            angles: np.ndarray
                Vetor de ângulos a serem aplicados.
            indices: list[int] ou np.ndarray
                Índices das juntas cujos ângulos foram definidos.

        Retorno:
            new_angles: np.ndarray
                Vetor de 22 ângulos, com zeros nos índices não definidos.
        """
        new_angles = np.zeros(22, np.float32)  # Todos os ângulos das juntas (exceto dedos dos pés)
        new_angles[indices] = angles  # Preenche apenas os índices definidos com os ângulos fornecidos

        return new_angles





























