from Agent.BaseAgent import BaseAgent
from sobre_scripts.commons.Script import Script
from world.commons.Draw import Draw
import numpy as np


class Joints:
    """
    Descrição:
        Classe utilitária para visualização e manipulação interativa dos valores das juntas do rob. em simulação.
        Permite configurar, destacar e anotar visualmente as posições ou velocidades das juntas, além de oferecer
        opções para ativar gravidade, harmonização e exibição de labels.
        É útil tanto para depuração de movimentos quanto para demonstrações didáticas ou
        ajustes finos em testes de controle.

    Parâmetros:
        script: Script
            Referência ao script principal, utilizada para acessar argumentos e funcionalidades do ambiente de simulação.

    Métodos:
        __init__(self, script: Script) -> None
            Inicializa a classe Joints e define os parâmetros padrão.

        _draw_joints(self, player: BaseAgent)
            Realiza a visualização das juntas do robô, desenhando linhas e anotações para cada junta.
            Destaca visualmente com valores diferentes de zero.


    """
    def __init__(self, script: Script) -> None:
        """
        Descrição:
            Inicializa a classe Joints, configurando variáveis padrão para controle e visualização das juntas do robô.

        Parâmetros:
            script: Script
                Referência ao script principal para acesso ao ambiente de simulação.

        Atributos:
            joints_no: int or None
                Número total de juntas do robô (pode ser definido posteriormente).
            agent_pos: tuple
                Posição inicial do agente no campo (x, y, z).
            enable_pos: bool
                Ativa/desativa controle de posição das juntas.
            enable_gravity: bool
                Ativa/desativa gravidade no ambiente de simulação.
            enable_labels: bool
                Ativa/desativa a exibição de anotações (labels) nas juntas.
            enable_harmonize: bool
                Ativa/desativa o alinhamento/harmonização das juntas.
            active_joint: int
                Índice da junta atualmente selecionada para manipulação.
            joints_value: any
                Armazena os valores atuais das juntas (posição ou velocidade).
        """
        self.joints_no = None
        self.script = script
        self.agent_pos = (-3, 0, 0.45)
        self.enable_pos = True
        self.enable_gravity = False
        self.enable_labels = True
        self.enable_harmonize = True
        self.active_joint = 0
        self.joints_value = None  # position or speed

    def _draw_joints(self, player: BaseAgent):
        """
        Descrição:
            Desenha, para cada junta do robô, uma linha indicando sua posição no espaço,
            além de anotar seu índice e valor (caso diferente de zero).
            Labels são posicionados em diferentes alturas para evitar sobreposição visual,
            e as juntas ativas (com valor diferente de zero) são destacadas em vermelho.

        Parâmetros:
            player: BaseAgent
                Instância do agente base contendo as informações das juntas e métodos de visualização.

        Retorno:
            None
        """
        zstep = 0.05
        # Altura extra dos labels para evitar sobreposição entre anotações das juntas
        label_z = [3 * zstep, 5 * zstep, 0, 0, zstep, zstep, 2 * zstep, 2 * zstep, 0, 0, 0, 0, zstep, zstep, 0, 0, zstep, zstep, 4 * zstep, 4 * zstep, 5 * zstep, 5 * zstep, 0, 0]
        for j, transf in enumerate(player.world.robot.joints_transform):
            rp = transf.obter_vetor_de_translacao()
            pos = player.world.robot.loc_head_to_field_transform(rp, False)
            j_id = f"{j}"
            j_name = f"{j}"
            color = Draw.Color.cyan
            # Se a junta está ativa (valor diferente de zero), destaca em vermelho e exibe o valor
            if player.world.robot.joints_position[j] != 0:
                j_name += f" ({int(player.world.robot.joints_position[j])})"
                color = Draw.Color.red
            # Calcula posição do label em relação à junta
            label_rp = np.array([rp[0] - 0.0001, rp[1] * 0.5, 0])
            label_rp /= np.linalg.norm(label_rp) / 0.5  # Labels a 0.5m do corpo
            label_rp += (0, 0, label_z[j])
            label = player.world.robot.loc_head_to_field_transform(rp + label_rp, False)
            # Desenha linha entre a junta e o label
            player.world.draw.line(pos, label, 2, Draw.Color.green_light, j_id, False)
            # Anota o nome/valor da junta na simulação
            player.world.draw.annotation(label, j_name, color, j_id)

    def print_help(self):
        print(f"""
---------------------- Joints demonstration ----------------------
Command: {{action/actions/option}}
    action : [joint:{{int}}] value 
    actions: value0,value1,...,valueN
             e.g. if N=10, you control all joints from j0 to j10
    option:  {{h,s,g,l,w,r,"",.}}
Examples:
    "6 90"   - move joint 6 to 90deg or move joint 6 at 90deg/step
    "4"      - move last joint to 4deg or apply speed of 4deg/step
    "1,9,-35"- move joints 0,1,2 to 1deg, 9deg, -35deg (or speed)
    "h"      - help, display this message
    "s"      - toggle position/speed control ({"Posi" if self.enable_pos else "Spee"})
    "g"      - toggle gravity                ({self.enable_gravity})
    "l"      - toggle labels                 ({self.enable_labels})
    "w"      - toggle harmonize*             ({self.enable_harmonize})
    "r"      - reset (position mode + reset joints)
    ""       - advance 2 simulation step
    "."      - advance 1 simulation step
    "ctrl+c" - quit demonstration

    *all joints end moving at the same time when harmonize is True
------------------------------------------------------------------""")

    def _user_control_step(self, player: BaseAgent):
        """
        Descrição:
            Gerencia a interação do usuário para controle manual das juntas do robô durante a simulação.
            Permite alternar entre modos de controle (posição/velocidade), gravidade, labels, harmonização, reset de juntas,
            exibe ajuda, além de ajustar valores específicos de juntas por comando direto ou múltiplo.
            Retorna o número de passos a serem pulados na simulação, conforme o comando.

        Parâmetros:
            player: BaseAgent
                Instância do agente base, utilizada para acesso e ajuste dos valores das juntas.

        Retorno:
            int
                Número de passos a serem pulados na simulação (0 = nenhum, 1 = próximo comando, etc.).
        """
        while True:
            inp = input("Command: ")

            # Alterna entre controle por posição e velocidade
            if inp == "s":
                self.enable_pos = not self.enable_pos
                print("Using", "position" if self.enable_pos else "velocity", "control.")
                if self.enable_pos:
                    self.joints_value[:] = player.world.robot.joints_position
                else:
                    self.joints_value.fill(0)
                continue

            # Alterna gravidade
            elif inp == "g":
                self.enable_gravity = not self.enable_gravity
                print("Using gravity:", self.enable_gravity)
                continue

            # Alterna exibição de labels
            elif inp == "l":
                self.enable_labels = not self.enable_labels
                print("Using labels:", self.enable_labels)
                continue

            # Alterna harmonização dos comandos de junta
            elif inp == "w":
                self.enable_harmonize = not self.enable_harmonize
                print("Using harmonize:", self.enable_harmonize)
                continue

            # Reseta todas as juntas para zero usando controle de posição
            elif inp == "r":
                self.enable_pos = True
                self.joints_value.fill(0)
                print("Using position control. All joints are set to zero.")
                continue

            # Exibe mensagem de ajuda
            elif inp == "h":
                self.print_help()
                continue

            # Avança um passo na simulação
            elif inp == "":
                return 1
            # Não avança nenhum passo (apenas atualiza)
            elif inp == ".":
                return 0

            # Processa comando para ajuste individual/múltiplo de juntas
            try:
                # Comando no formato: índice valor (ex: "2 45")
                if " " in inp:
                    self.active_joint, value = map(float, inp.split())
                    self.joints_value[int(self.active_joint)] = value
                # Comando no formato: v1,v2,v3... (ajuste múltiplo)
                elif "," in inp:
                    values = inp.split(",")
                    self.joints_value[0:len(values)] = values
                # Comando apenas valor: ajusta a junta ativa
                else:
                    self.joints_value[self.active_joint] = float(inp)
            except Exception:
                print("Illegal command!")
                continue

    def execute(self):
        """
        Descrição:
            Executa o ciclo principal de controle manual das juntas do robô.
            Inicializa o agente, configura as variáveis de controle, realiza o posicionamento inicial (beam) e entra em um laço interativo,
            onde o usuário pode ajustar as juntas, alternar modos e visualizar labels.
            Permite controlar a posição ou velocidade de cada junta individualmente, ativar/desativar gravidade e harmonização,
            além de exibir feedback visual contínuo do estado das juntas.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """
        a = self.script.args
        # Cria agente base com parâmetros fornecidos pelo script
        player = BaseAgent(a.i, a.p, a.m, a.u, a.r, a.t)  # Args: Server IP, Agent Port, Monitor Port, Uniform No., Robot Type, Team Name

        # Descobre e inicializa número de juntas e vetor de valores das juntas
        self.joints_no = player.world.robot.no_of_joints
        self.joints_value = np.zeros(self.joints_no)  # inicialização zerada

        # Posiciona agente no campo (beam) com orientação inicial
        player.scom.commit_beam(self.agent_pos[0:2], 0)

        # Exibe mensagem de ajuda ao início
        self.print_help()

        # Inicialização: envia comandos e recebe estados para garantir consistência e visualização inicial
        for _ in range(8):
            player.scom.commit_and_send()
            player.scom.receive()
        self._draw_joints(player)

        skip_next = 0  # variável para controlar avanço de múltiplos passos

        # Loop principal de controle interativo
        while True:
            # Se não há passos a pular, processa novo comando do usuário
            if skip_next == 0:
                skip_next = self._user_control_step(player)
            else:
                skip_next -= 1

            # Exibe labels se habilitado
            if self.enable_labels:
                self._draw_joints(player)

            # Atualiza valores das juntas segundo modo ativo
            if self.enable_pos:
                player.world.robot.set_joints_target_position_direct(
                    slice(self.joints_no), self.joints_value, harmonize=self.enable_harmonize
                )
            else:
                # Converte de graus/passo para rad/s (aprox. 0.87266463)
                player.world.robot.joints_target_speed[:] = self.joints_value * 0.87266463

            # Se gravidade está desabilitada, faz "beam" contínuo para manter posição
            if not self.enable_gravity:
                player.scom.unofficial_beam(self.agent_pos, 0)

            # Envia comando e recebe atualização do estado do robô
            player.scom.commit_and_send(player.world.robot.get_command())
            player.scom.receive()
            
            
            