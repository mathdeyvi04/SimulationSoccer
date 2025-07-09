from agent.BaseAgent import BaseAgent
from world.commons.Draw import Draw
from stable_baselines3 import PPO
from stable_baselines3.common.vec_env import SubprocVecEnv
from scripts.commons.Server import Server
from scripts.commons.TrainBase import TrainBase
from time import sleep
import os
import gym
import numpy as np

'''
Objetivo:
Aprender como simular quedas (exemplo mais simples):
- Classe Fall: implementa um ambiente customizado do OpenAI Gym para simular quedas.
- Classe Train: implementa algoritmos para treinar um novo modelo ou testar um modelo já existente.
'''


class Fall(gym.Env):
    """
    Descrição:
        Ambiente customizado no estilo OpenAI Gym para treinar
        agentes de aprendizado por reforço na tarefa de simular quedas de um robô.

        Ela implementa métodos para inicialização, controle do ciclo de simulação,
        interação com o agente e gerenciamento do espaço de observação e ação.

    Métodos Disponíveis:
        - observe
        - sync
        - reset
        - render
        - close
        - step
    """

    def __init__(self, ip, server_p, monitor_p, r_type, enable_draw) -> None:
        """
        Descrição:
            Inicializa o ambiente de simulação para o agente de RL, configurando o agente, os espaços de observação e ação,
            e verificando se a funcionalidade de "cheat" está habilitada para acessar informações privilegiadas.

        Parâmetros
            ip: str
                Endereço IP do servidor ao qual o agente irá se conectar.
            server_p: int
                Porta do servidor utilizada para comunicação do agente.
            monitor_p: int
                Porta do monitor utilizada para visualização ou monitoramento do agente.
            r_type: int ou str
                Tipo do robô que será utilizado pelo agente.
            enable_draw: bool
                Indica se a visualização gráfica está habilitada durante a simulação.
        """

        self.robot_type = r_type

        # Inicializa o agente com os parâmetros fornecidos e opções padrão
        # Args: Server IP, Agent Port, Monitor Port, Uniform No., Robot Type, Team Name, Enable Log, Enable Draw
        self.player = BaseAgent(ip, server_p, monitor_p, 1, self.robot_type, "Gym", True, enable_draw)
        self.step_counter = 0  # Contador de passos para limitar o tamanho do episódio

        # Espaço de observação (state space): número de juntas + altura do torso
        self.no_of_joints = self.player.world.robot.no_of_joints
        self.obs = np.zeros(self.no_of_joints + 1, np.float32)  # joints + torso height
        self.observation_space = gym.spaces.Box(
            low=np.full(len(self.obs), -np.inf, np.float32),
            high=np.full(len(self.obs), np.inf, np.float32),
            dtype=np.float32
        )

        # Espaço de ação (action space): uma ação para cada junta
        MAX = np.finfo(np.float32).max
        no_of_actions = self.no_of_joints
        # Define o espaço de ações do ambiente usando o formato Box do Gym.
        # Cada ação é representada por um número de ponto flutuante (float32) e pode assumir qualquer valor entre -MAX e +MAX,
        # onde MAX é o maior valor possível para o tipo float32. O número de ações (no_of_actions) normalmente corresponde ao número de juntas do robô.
        # Esse espaço é utilizado para indicar ao agente quais são os limites válidos para cada componente da ação que ele pode executar.
        self.action_space = gym.spaces.Box(
            low=np.full(no_of_actions, -MAX, np.float32),
            high=np.full(no_of_actions, MAX, np.float32),
            dtype=np.float32
        )

        # Verifica se as funções de "cheat" estão habilitadas no servidor
        assert np.any(self.player.world.robot.cheat_abs_pos), "Cheats não estão permitidos! Run_Player.py -> Server -> Cheats"

    def observe(self) -> np.ndarray:
        """
        Descrição:
            Coleta o vetor de observações do ambiente a partir do estado atual do robô.
            O vetor de observação é composto pelas posições normalizadas das juntas e pela altura atual do torso do robô (coordenada z).
            Esta função prepara os dados de entrada para o agente de aprendizado por reforço.

        Retorno
            obs: np.ndarray
                Vetor de observações, contendo as posições normalizadas das juntas e a altura do torso.
        """

        r = self.player.world.robot

        # Preenche o vetor de observações com as posições normalizadas das juntas
        for i in range(self.no_of_joints):
            self.obs[i] = r.joints_position[i] / 100  # Normalização simples para facilitar o aprendizado

        # Adiciona a altura do torso (coordenada z absoluta) como última observação
        self.obs[self.no_of_joints] = r.cheat_abs_pos[2]  # Alternativa: r.loc_head_z

        return self.obs

    def sync(self) -> None:
        """
        Descrição:
            Executa um único passo de simulação, enviando o comando atual do robô ao simulador e recebendo o novo estado do ambiente.
            Garante que as ações recentes do agente sejam processadas corretamente na simulação.

        Retorno
            None
        """
        r = self.player.world.robot
        # Envia o comando de controle atual do robô ao simulador
        self.player.scom.commit_and_send(r.get_command())
        # Recebe o novo estado do ambiente após executar as ações
        self.player.scom.receive()

    def reset(self) -> np.ndarray:
        """
        Descrição:
            Reinicializa e estabiliza o robô no início de um novo episódio.
            O robô é reposicionado ("beam") suspenso no ar e mantido em uma postura neutra por vários ciclos para garantir estabilidade.
            Após isso, o robô é colocado no solo, realiza um pequeno movimento para atualizar a física, e então é estabilizado novamente em contato com o solo.
            Retorna o vetor de observação inicial.

        Retorno
            obs: np.ndarray
                Vetor de observações após a estabilização do robô.
        """

        self.step_counter = 0
        r = self.player.world.robot

        # Fase 1: Reposiciona o robô acima do solo e mantém a postura inicial
        # 25 vezes realmente é necessário?
        for _ in range(25):
            self.player.scom.unofficial_beam((-3, 0, 0.50), 0)  # Suspende o robô acima do solo
            self.player.behavior.execute("Zero")  # Executa postura neutra
            self.sync()  # Sincroniza o passo de simulação

        # Fase 2: Reposiciona o robô no solo e força atualização da física
        self.player.scom.unofficial_beam((-3, 0, r.beam_height), 0)  # Posiciona o robô no solo
        r.joints_target_speed[0] = 0.01  # Move levemente a cabeça para corrigir bug do simulador
        self.sync()

        # Fase 3: Estabiliza o robô no solo
        for _ in range(7):
            self.player.behavior.execute("Zero")  # Mantém postura neutra
            self.sync()  # Sincroniza o passo de simulação

        return self.observe()

    def render(self, mode='human', close=False):
        """
        Descrição:
            Método padrão de ambientes Gym para renderização.
            Neste ambiente, não realiza nenhuma ação visual, mas pode ser sobrescrito em subclasses.
        """
        return

    def close(self) -> None:
        """
        Descrição:
            Encerra o ambiente, limpando todas as visualizações/desenhos e finalizando o agente.
            Deve ser chamado ao final do uso para evitar vazamento de recursos.
        """
        # Limpa visualizações e desenhos gráficos
        Draw.clear_all()
        # Encerra o agente e libera recursos associados
        self.player.terminate()

    def step(self, action: np.ndarray) -> tuple[np.ndarray, float, bool, dict]:
        """
        Descrição:
            Executa um passo de simulação aplicando a ação fornecida nas juntas do robô.
            As ações são amplificadas para incentivar a exploração inicial.
            Após a execução, avalia se o robô caiu (condição de sucesso), se o tempo limite foi atingido,
            ou se o episódio deve continuar.

        Parâmetros
            action: np.ndarray
                Vetor de ações a serem aplicadas nas juntas do robô.

        Retorno
            obs: np.ndarray
                Vetor de observações após a execução da ação.
            reward: float
                Recompensa do passo (1 se caiu rapidamente, 0 caso contrário).
            done: bool
                Indica se o episódio terminou.
            info: dict
                Dicionário informativo (vazio nesta implementação).
        """

        r = self.player.world.robot
        # Aplica as posições-alvo diretamente em todas as juntas, ampliando as ações para estimular a exploração
        r.set_joints_target_position_direct(
            slice(self.no_of_joints),  # Aplica a todas as juntas disponíveis
            action * 10,  # Amplifica as ações
            harmonize=False  # Não harmoniza, pois as ações mudam a cada passo
        )

        self.sync()  # Executa o passo de simulação
        self.step_counter += 1  # Incrementa o contador de passos
        self.observe()  # Atualiza o vetor de observações

        if self.obs[-1] < 0.15:  # Condição terminal: o robô caiu (altura do torso < 0.15)
            return self.obs, 1, True, {}  # Recompensa 1, episódio termina
        elif self.step_counter > 150:  # Condição terminal: atingiu o tempo limite sem cair
            return self.obs, 0, True, {}
        else:
            return self.obs, 0, False, {}  # Episódio continua sem recompensa


class Train(TrainBase):

    def __init__(self, script) -> None:
        super.__init__(script)

    def train(self, args) -> None:
        """
        Descrição:
            Executa o treinamento do agente de aprendizado por reforço para a tarefa de queda (Fall).
            Configura os parâmetros principais de aprendizado, inicializa ambientes paralelos para coleta de experiências,
            seleciona entre retreinamento de um modelo existente ou início de um novo modelo e gerencia os ciclos de treinamento
            e avaliação, além do salvamento periódico dos modelos.

        Parâmetros
            args: dict
                Dicionário de argumentos para o processo de treinamento.
                Se contiver a chave 'model_file', o treinamento será feito a partir de um modelo pré-existente (retraining);
                caso contrário, um novo modelo será criado do zero.
        """

        # --------------------------------------- Parâmetros de aprendizado
        n_envs = min(4, os.cpu_count())  # Limita a no máximo 4 ambientes paralelos ou a quantidade de CPUs disponíveis
        n_steps_per_env = 128  # Passos de rollout por ambiente (tamanho do buffer = n_envs * n_steps_per_env)
        minibatch_size = 64  # Tamanho do minibatch (deve dividir exatamente o total do buffer)
        total_steps = 50000  # Passos totais de treinamento (ajustar para >=10M em cenários reais)
        learning_rate = 30e-4  # Taxa de aprendizado (ajustar para 3e-4 recomendado em ambientes complexos)
        folder_name = f'Fall_R{self.robot_type}'  # Nome da pasta para logs/modelos
        model_path = f'./sobre_scripts/gyms/logs/{folder_name}/'  # Caminho base para salvar modelos

        print("Model path:", model_path)

        # --------------------------------------- Inicialização dos ambientes
        def init_env(i_env):
            # Função auxiliar para inicializar cada ambiente com portas distintas
            def thunk():
                return Fall(self.ip, self.server_p + i_env, self.monitor_p_1000 + i_env, self.robot_type, False)

            return thunk

        # Cria um grupo de servidores, incluindo um extra para avaliação
        servers = Server(self.server_p, self.monitor_p_1000, n_envs + 1)

        # Cria ambientes paralelos para coleta de experiência e um ambiente separado para avaliação
        env = SubprocVecEnv([init_env(i) for i in range(n_envs)])
        eval_env = SubprocVecEnv([init_env(n_envs)])

        try:
            if "model_file" in args:
                # Retreinamento: carrega modelo existente e continua treinamento
                model = PPO.load(
                    args["model_file"],
                    env=env,
                    n_envs=n_envs,
                    n_steps=n_steps_per_env,
                    batch_size=minibatch_size,
                    learning_rate=learning_rate
                )
            else:
                # Treinamento do zero: inicializa novo modelo PPO com política MLP
                model = PPO(
                    "MlpPolicy",
                    env=env,
                    verbose=1,
                    n_steps=n_steps_per_env,
                    batch_size=minibatch_size,
                    learning_rate=learning_rate
                )

            # Executa o ciclo de treinamento com avaliações e salvamentos periódicos
            model_path = self.learn_model(
                model,
                total_steps,
                model_path,
                eval_env=eval_env,
                eval_freq=n_steps_per_env * 10,
                save_freq=n_steps_per_env * 20,
                backup_env_file=__file__  # Salva backup do script de ambiente junto do modelo
            )
        except KeyboardInterrupt:
            sleep(1)  # Aguarda finalização de processos filhos
            print("\nctrl+c pressed, aborting...\n")
            servers.kill()
            return

        # Encerra os ambientes e servidores ao final do treinamento
        env.close()
        eval_env.close()
        servers.kill()

    def test(self, args) -> None:

        # Uses different server and monitor ports
        server = Server(self.server_p - 1, self.monitor_p, 1)
        env = Fall(self.ip, self.server_p - 1, self.monitor_p, self.robot_type, True)
        model = PPO.load(args["model_file"], env=env)

        try:
            self.export_model(args["model_file"], args["model_file"] + ".pkl", False)  # Export to pkl to create custom behavior
            self.test_model(model, env, log_path=args["folder_dir"], model_path=args["folder_dir"])
        except KeyboardInterrupt:
            print()

        env.close()
        server.kill()


"""
O processo de aprendizado leva cerca de 5 minutos.  
Um vídeo com os resultados pode ser visto em:  
https://imgur.com/a/KvpXS41

Espaço de estados:
- Composto por todas as posições das articulações e pela altura do torso
- O número de posições das articulações varia para o robô do tipo 4, portanto, os modelos não são intercambiáveis
- Neste exemplo, esse problema pode ser evitado utilizando apenas as primeiras 22 articulações e atuadores

Recompensa:
- A recompensa por cair é 1, ou seja, após algum tempo todo episódio terá r=1.
- Qual o incentivo para o robô cair mais rápido? O retorno descontado (discounted return).
  Em cada estado, o algoritmo buscará recompensas de curto prazo.
- Durante o treinamento, o melhor modelo é salvo de acordo com o retorno médio, que quase sempre chega a 1.
  Por isso, neste exemplo, o último modelo treinado geralmente será o superior.

Evolução esperada do tempo de episódio:
    3s|o
      |o
      | o
      |  o
      |   oo
      |     ooooo
  0.4s|          oooooooooooooooo
      |------------------------------> tempo

Este exemplo não escala bem com o número de CPUs porque:
- Utiliza um buffer de rollout pequeno (n_steps_per_env * n_envs)
- A carga de trabalho da simulação é baixa
- Por esses motivos, a sobrecarga de IPC (comunicação entre processos) se torna significativa
"""
