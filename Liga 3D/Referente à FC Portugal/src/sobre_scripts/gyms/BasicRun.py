from agent.BaseAgent import BaseAgent
from behaviors.custom.Step.Step import Step
from world.commons.Draw import Draw
from stable_baselines3 import PPO
from stable_baselines3.common.vec_env import SubprocVecEnv
from time import sleep
import os, gym
import numpy as np

from scripts.commons.Server import Server
from scripts.commons.TrainBase import TrainBase

"""
    Objetivo:
        Aprender como executar o movimento de avanço (andar para frente) utilizando a primitiva de passo.

    - Classe BasicRun: implementa um ambiente customizado do OpenAI Gym.
    - Classe TrainBase: implementa algoritmos para treinar um novo modelo ou testar um modelo já existente.
"""


class BasicRun(gym.Env):
    """
    Descrição:
        Ambiente customizado compatível com o OpenAI Gym para simulação e controle de robôs bípedes,
        focado em tarefas de locomoção utilizando comportamentos parametrizáveis de caminhada ("Step").

        Ela gerencia a interface de observação e ação para o agente de aprendizado por reforço,
        processa comandos, sincroniza a simulação e calcula recompensas baseadas no desempenho do robô.

    Métodos Disponíveis:
        - observe
        - sync
        - reset
        - render
        - close
        - step
    """

    def __init__(self, ip: str, server_p: int, monitor_p: int, r_type: int | str, enable_draw: bool) -> None:
        """
        Descrição:
            Inicializa uma instância do ambiente de simulação para controle de robôs, configurando o agente, os espaços de observação e ação,
            além dos parâmetros padrão para o comportamento de passo ("Step"). Também posiciona a bola longe do robô para garantir que os marcos
            de referência permaneçam no campo de visão durante o uso do comportamento de Step.

        Parâmetros
            ip: str
                Endereço IP do servidor ao qual o agente irá se conectar.
            server_p: int
                Porta do servidor utilizada para comunicação do agente.
            monitor_p: int
                Porta do monitor utilizada para visualização ou monitoramento do agente.
            r_type: int ou str
                Tipo do robô a ser utilizado pelo agente.
            enable_draw: bool
                Se True, ativa a visualização gráfica (desenho) durante a simulação.
        """

        self.act = None
        self.lastx = None
        self.robot_type = r_type

        # Inicializa o agente com os parâmetros fornecidos e opções padrão
        # Args: Server IP, Agent Port, Monitor Port, Uniform No., Robot Type, Team Name, Enable Log, Enable Draw
        self.player = BaseAgent(ip, server_p, monitor_p, 1, self.robot_type, "Gym", True, enable_draw)
        self.step_counter = 0  # Contador de passos para limitar o tamanho do episódio

        # Obtém o objeto de comportamento personalizado "Step" do agente
        self.step_obj: Step = self.player.behavior.get_custom_behavior_object("Step")

        # Espaço de observação (state space): vetor de 70 floats
        obs_size = 70
        self.obs = np.zeros(obs_size, np.float32)
        self.observation_space = gym.spaces.Box(
            low=np.full(obs_size, -np.inf, np.float32),
            high=np.full(obs_size, np.inf, np.float32),
            dtype=np.float32
        )

        # Espaço de ação (action space): vetor de 22 floats, valores entre -MAX e +MAX
        MAX = np.finfo(np.float32).max
        self.no_of_actions = act_size = 22
        self.action_space = gym.spaces.Box(
            low=np.full(act_size, -MAX, np.float32),
            high=np.full(act_size, MAX, np.float32),
            dtype=np.float32
        )

        # Parâmetros padrão do comportamento de Step
        self.step_default_dur = 7  # Duração padrão do passo
        self.step_default_z_span = 0.035  # Amplitude vertical padrão
        self.step_default_z_max = 0.70  # Altura máxima padrão

        # Posiciona a bola longe do robô para manter os marcos de referência visíveis
        # (a cabeça do robô segue a bola ao usar o comportamento de Step)
        self.player.scom.unofficial_move_ball((14, 0, 0.042))

    def observe(self, init=False) -> np.ndarray:
        """
        Descrição:
            Coleta e atualiza o vetor de observações do agente, contendo informações do robô e do ambiente em formato compatível com a interface Gym.
            As observações incluem variáveis do torso, sensores inerciais, forças nos pés, posições e velocidades das articulações, além de parâmetros do passo.
            Os valores são normalizados de forma simplificada ("naive normalization") para facilitar o aprendizado.

            Quando init=True, utiliza os valores padrão do passo; caso contrário, utiliza os valores atuais do gerador de passos.

        Parâmetros:
            init: bool, opcional
                Se True, utiliza os parâmetros padrão de caminhada (após reset); se False (padrão), utiliza os parâmetros atuais do passo.

        Retorno:
            obs: np.ndarray
                Vetor de observações normalizado, pronto para ser utilizado pelo agente.
        """

        r = self.player.world.robot

        # index       observation              naive normalization
        self.obs[0] = self.step_counter / 100  # simple counter: 0,1,2,3...
        self.obs[1] = r.loc_head_z * 3  # z coordinate (torso)
        self.obs[2] = r.loc_head_z_vel / 2  # z velocity (torso)
        self.obs[3] = r.imu_torso_orientation / 50  # absolute orientation in deg
        self.obs[4] = r.imu_torso_roll / 15  # absolute torso roll  in deg
        self.obs[5] = r.imu_torso_pitch / 15  # absolute torso pitch in deg
        self.obs[6:9] = r.gyro / 100  # gyroscope
        self.obs[9:12] = r.acc / 10  # accelerometer

        self.obs[12:18] = r.frp.get('lf', (0, 0, 0, 0, 0, 0))  # left foot: relative point of origin (p) and force vector (f) -> (px,py,pz,fx,fy,fz)*
        self.obs[18:24] = r.frp.get('rf', (0, 0, 0, 0, 0, 0))  # right foot: relative point of origin (p) and force vector (f) -> (px,py,pz,fx,fy,fz)*
        self.obs[15:18] /= 100  # naive normalization of force vector
        self.obs[21:24] /= 100  # naive normalization of force vector
        self.obs[24:44] = r.joints_position[2:22] / 100  # position of all joints except head & toes (for robot type 4)
        self.obs[44:64] = r.joints_speed[2:22] / 6.1395  # speed of    all joints except head & toes (for robot type 4)
        # *if foot is not touching the ground, then (px=0,py=0,pz=0,fx=0,fy=0,fz=0)

        if init:  # the walking parameters refer to the last parameters in effect (after a reset, they are pointless)
            self.obs[64] = self.step_default_dur / 10  # step duration in time steps
            self.obs[65] = self.step_default_z_span * 20  # vertical movement span
            self.obs[66] = self.step_default_z_max  # relative extension of support leg
            self.obs[67] = 1  # step progress
            self.obs[68] = 1  # 1 if left  leg is active
            self.obs[69] = 0  # 1 if right leg is active
        else:
            self.obs[64] = self.step_obj.step_generator.ts_per_step / 10  # step duration in time steps
            self.obs[65] = self.step_obj.step_generator.swing_height * 20  # vertical movement span
            self.obs[66] = self.step_obj.step_generator.max_leg_extension / self.step_obj.leg_length  # relative extension of support leg
            self.obs[67] = self.step_obj.step_generator.external_progress  # step progress
            self.obs[68] = float(self.step_obj.step_generator.state_is_left_active)  # 1 if left  leg is active
            self.obs[69] = float(not self.step_obj.step_generator.state_is_left_active)  # 1 if right leg is active

        '''
        Expected observations for walking parameters/state (example):
        Time step        R  0  1  2  0   1   2   3  4
        Progress         1  0 .5  1  0 .25  .5 .75  1
        Left leg active  T  F  F  F  T   T   T   T  T
        Parameters       A  A  A  B  B   B   B   B  C
        Example note: (A) has a step duration of 3ts, (B) has a step duration of 5ts
        '''

        return self.obs

    def sync(self) -> None:
        """
        Descrição:
            Executa um único passo de simulação, enviando o comando atual do robô para o simulador e recebendo a resposta do ambiente.
            Garante que as ações do agente sejam processadas e que o estado do ambiente seja atualizado.

        Retorno
            Apenas sincroniza o estado do agente e do ambiente simulador.
        """
        r = self.player.world.robot
        # Envia o comando atual do robô para o simulador
        self.player.scom.commit_and_send(r.get_command())
        # Recebe o novo estado do ambiente após processar o comando
        self.player.scom.receive()

    def reset(self) -> np.ndarray:
        """
        Descrição:
            Reinicializa o estado do robô e o estabiliza no início de um novo episódio.
            O agente é reposicionado ("beam") e colocado em uma postura padrão, primeiro flutuando no ar e depois suavemente no chão,
            para garantir estabilidade inicial. Por fim, executa comandos de estabilização e atualiza variáveis internas.

            - Para alguns comportamentos, pode ser interessante reduzir o tempo de estabilização ou adicionar ruído.

        Retorno
            obs: np.ndarray
                Vetor de observação inicial do agente, após a estabilização.
        """

        self.step_counter = 0
        r = self.player.world.robot

        # Fase 1: Reposiciona o robô acima do solo e mantém postura estável (flutuando)
        for _ in range(25):
            self.player.scom.unofficial_beam((-14, 0, 0.50), 0)  # posiciona o robô suspenso acima do solo
            self.player.behavior.execute("Zero_Bent_Knees")  # executa postura padrão de joelhos flexionados
            self.sync()  # sincroniza comandos e atualiza o ambiente

        # Fase 2: Posiciona o robô no solo
        self.player.scom.unofficial_beam((-14, 0, r.beam_height), 0)  # posiciona o robô no solo
        r.joints_target_speed[0] = 0.01  # move levemente a cabeça para forçar atualização física (corrige bug do simulador)
        self.sync()

        # Fase 3: Estabiliza o robô no solo
        for _ in range(7):
            self.player.behavior.execute("Zero_Bent_Knees")  # mantém postura estável
            self.sync()  # sincroniza comandos e atualiza o ambiente

        # Atualiza variáveis de memória e ações
        self.lastx = r.cheat_abs_pos[0]
        self.act = np.zeros(self.no_of_actions, np.float32)

        # Retorna observação inicial do agente após estabilização
        return self.observe(True)

    def render(self, mode='human', close=False):
        """
        Descrição:
            Método padrão de ambientes Gym para renderização.
            Neste ambiente, a função é um placeholder e não executa nenhuma ação visual.
            Pode ser sobrescrita em subclasses para visualização customizada.

        Parâmetros
            mode: str, opcional
                Modo de renderização (padrão 'human'). Não utilizado nesta implementação.
            close: bool, opcional
                Indica se deve fechar a renderização. Não utilizado aqui.

        Retorno
            None
        """
        return

    def close(self) -> None:
        """
        Descrição:
            Finaliza o ambiente, limpando todas as visualizações/desenhos e encerrando o agente.
            Deve ser chamado ao final do uso do ambiente para evitar vazamento de recursos.

        Retorno
            None
        """
        # Limpa visualizações/desenhos
        Draw.clear_all()
        # Encerra o agente e libera recursos associados
        self.player.terminate()

    def step(self, action: np.ndarray) -> tuple[np.ndarray, float, bool, dict]:
        """
        Descrição:
            Executa um passo de simulação no ambiente com a ação fornecida.
            Processa a ação do agente, atualiza os parâmetros do comportamento de passo, 
            aplica ajustes nos comandos das articulações e avança a simulação.
            Calcula a recompensa, verifica condição terminal (queda ou tempo limite) e retorna o novo estado.

        Parâmetros
            action: np.ndarray
                Vetor de ações a serem aplicadas no robô, incluindo ajustes de postura e parâmetros do passo.

        Retorno
            observation: np.ndarray
                Novo vetor de observações do ambiente após a ação.
            reward: float
                Recompensa obtida neste passo (distância avançada).
            done: bool
                Indica se o episódio terminou (queda ou atingiu número máximo de passos).
            info: dict
                Dicionário informativo (vazio nesta implementação).
        """

        r = self.player.world.robot

        # Aplica média móvel exponencial às ações para suavizar comandos
        self.act = 0.4 * self.act + 0.6 * action

        # Executa o comportamento Step para atualizar posições-alvo das pernas
        if self.step_counter == 0:
            '''
            No primeiro passo, utiliza parâmetros padrão para antecipar a próxima pose gerada.
            Isso permite que o agente decida os parâmetros durante o passo anterior.
            '''
            self.player.behavior.execute("Step", self.step_default_dur, self.step_default_z_span, self.step_default_z_max)
        else:
            # Calcula parâmetros do passo ajustados pela ação do agente (exploração)
            step_zsp = np.clip(self.step_default_z_span + self.act[20] / 300, 0, 0.07)
            step_zmx = np.clip(self.step_default_z_max + self.act[21] / 30, 0.6, 0.9)
            self.player.behavior.execute("Step", self.step_default_dur, step_zsp, step_zmx)

        # Aplica ações residuais nas articulações (excluindo a cabeça)
        new_action = self.act[:20] * 2  # estimula exploração ampliando a ação
        new_action[[0, 2, 4, 6, 8, 10]] += self.step_obj.values_l  # ajustes para perna esquerda
        new_action[[1, 3, 5, 7, 9, 11]] += self.step_obj.values_r  # ajustes para perna direita
        new_action[12] -= 90  # braços para baixo
        new_action[13] -= 90  # braços para baixo
        new_action[16] += 90  # destorce braços
        new_action[17] += 90  # destorce braços
        new_action[18] += 90  # cotovelos a 90 graus
        new_action[19] += 90  # cotovelos a 90 graus

        # Aplica as posições-alvo diretamente nas articulações (exceto cabeça e dedos dos pés)
        r.set_joints_target_position_direct(
            slice(2, 22),  # articulações a serem comandadas
            new_action,  # posições-alvo
            harmonize=False  # harmonização desativada, pois os alvos mudam a cada passo
        )

        self.sync()  # executa o passo de simulação
        self.step_counter += 1  # incrementa o contador de passos

        reward = r.cheat_abs_pos[0] - self.lastx  # recompensa: avanço em x
        self.lastx = r.cheat_abs_pos[0]  # atualiza posição anterior

        # Verifica condição de término: queda (z < 0.3) ou número máximo de passos
        terminal = r.cheat_abs_pos[2] < 0.3 or self.step_counter > 300

        # Retorna observação, recompensa, flag terminal e dicionário info vazio
        return self.observe(), reward, terminal, {}


class Train(TrainBase):
    """
    Descrição:
        Classe que gerencia o processo de treinamento e teste de agentes de aprendizado por reforço,
        estendendo as funcionalidades da TrainBase.

    Métodos
        train(args)
            Realiza o treinamento do agente, podendo iniciar do zero ou continuar o treinamento de um modelo existente.
        test(args)
            Executa o teste de um modelo treinado em ambiente controlado, exportando resultados e logs.
    """

    def __init__(self, script: str) -> None:
        """
        Parâmetros:
            script: str
                Caminho ou referência ao script principal utilizado na configuração do treinamento.
        """
        super().__init__(script)

    def train(self, args):
        """
        Descrição:
            Executa o processo de treinamento do agente de RL.

        Parâmetros
            args: dict
                Dicionário de argumentos para treinamento. Pode incluir 'model_file' para retreinamento.

        Fluxo geral:
            1. Define parâmetros principais de aprendizado, como número de ambientes paralelos, tamanho do buffer e taxa de aprendizado.
            2. Inicializa servidores e ambientes paralelos (SubprocVecEnv) para coleta de experiências.
            3. Se 'model_file' estiver presente em args, carrega o modelo existente para retreinamento; caso contrário, inicia novo modelo.
            4. Treina o modelo usando learn_model, salvando checkpoints e backups periodicamente.
            5. Fecha ambientes e servidores ao final ou em caso de interrupção manual.
        """

        # --------------------------------------- Parâmetros de aprendizado
        n_envs = min(16, os.cpu_count())  # número de ambientes paralelos
        n_steps_per_env = 1024  # passos de rollout por ambiente
        minibatch_size = 64  # tamanho do minibatch para atualização
        total_steps = 30000000  # total de passos de treinamento
        learning_rate = 3e-4  # taxa de aprendizado
        folder_name = f'BasicRun_R{self.robot_type}'  # nome da pasta de logs
        model_path = f'./sobre_scripts/gyms/logs/{folder_name}/'  # caminho para salvar os modelos

        print("Caminho do Modelo:", model_path)

        # --------------------------------------- Inicialização dos ambientes
        def init_env(i_env):
            # Função auxiliar para inicializar ambientes paralelos
            def thunk():
                return BasicRun(self.ip, self.server_p + i_env, self.monitor_p_1000 + i_env, self.robot_type, False)

            return thunk

        servers = Server(self.server_p, self.monitor_p_1000, n_envs + 1)  # inclui um servidor extra para avaliação

        env = SubprocVecEnv([init_env(i) for i in range(n_envs)])  # ambientes paralelos para treinamento
        eval_env = SubprocVecEnv([init_env(n_envs)])  # ambiente para avaliação

        try:
            if "model_file" in args:  # retreina modelo existente
                model = PPO.load(
                    args["model_file"],
                    env=env,
                    device="cpu",
                    n_envs=n_envs,
                    n_steps=n_steps_per_env,
                    batch_size=minibatch_size,
                    learning_rate=learning_rate
                )
            else:  # inicia treinamento de novo modelo
                model = PPO(
                    "MlpPolicy",
                    env=env,
                    verbose=1,
                    n_steps=n_steps_per_env,
                    batch_size=minibatch_size,
                    learning_rate=learning_rate,
                    device="cpu"
                )

            # Executa o treinamento propriamente dito, salvando checkpoints e backups
            model_path = self.learn_model(
                model,
                total_steps,
                model_path,
                eval_env=eval_env,
                eval_freq=n_steps_per_env * 20,
                save_freq=n_steps_per_env * 200,
                backup_env_file=__file__
            )
        except KeyboardInterrupt:
            sleep(1)  # aguarda processos filhos serem finalizados
            print("\nctrl+c pressed, aborting...\n")
            servers.kill()
            return

        # Finaliza ambientes e servidores ao término do treinamento
        env.close()
        eval_env.close()
        servers.kill()

    def test(self, args):
        """
        Executa o teste de um modelo treinado.

        Parâmetros
            args: dict
                Dicionário de argumentos para teste. Deve incluir:
                    - "model_file": caminho do modelo a ser testado
                    - "folder_dir": diretório para salvar logs/resultados

        Fluxo geral:
            1. Inicializa servidor e ambiente de teste, com portas diferenciadas.
            2. Carrega o modelo treinado.
            3. Exporta o modelo para formato .pkl para uso como comportamento customizado.
            4. Executa o teste do modelo, salvando logs no diretório especificado.
            5. Finaliza o ambiente e servidor ao final ou em caso de interrupção manual.
        """

        # Inicializa servidor e ambiente de teste (usando portas diferentes das do treinamento)
        server = Server(self.server_p - 1, self.monitor_p, 1)
        env = Basic_Run(self.ip, self.server_p - 1, self.monitor_p, self.robot_type, True)
        model = PPO.load(args["model_file"], env=env)

        try:
            # Exporta o modelo treinado para .pkl (usado para criar comportamento customizado)
            self.export_model(args["model_file"], args["model_file"] + ".pkl", False)
            # Executa o teste propriamente dito, salvando resultados e logs
            self.test_model(model, env, log_path=args["folder_dir"], model_path=args["folder_dir"])
        except KeyboardInterrupt:
            print()

        # Finaliza ambiente e servidor
        env.close()
        server.kill()


"""
O processo de aprendizado leva várias horas para ser concluído.  
Um vídeo com os resultados pode ser visualizado em:  
https://imgur.com/a/dC2V6Et

Estatísticas:
- Recompensa média:     7,7  
- Duração média do episódio: 5,5s (o episódio é limitado a 6s)
- Recompensa máxima:    9,3  (velocidade: 1,55 m/s)

Espaço de estados:
- Composto por todas as posições das articulações e pela altura do torso
- Inclui o estágio atual do comportamento de caminhada (Step)

Recompensa:
- Baseada no deslocamento ao longo do eixo x (pode ser negativa)
- Importante: os dados "cheat" e visuais são atualizados apenas a cada 3 passos
"""
