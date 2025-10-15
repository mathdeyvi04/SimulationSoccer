from Agent.BaseAgent import BaseAgent
from itertools import count
from math_ops.InverseKinematics import InverseKinematics
from sobre_scripts.commons.Script import Script
from world.commons.Draw import Draw
import numpy as np

class Inv_Kinematics:
    """
    Descrição:
        Classe utilitária para demonstração e ajuste interativo de cinemática inversa das pernas do robô. Permite ao usuário controlar, em tempo real,
        as posições dos tornozelos e rotações dos pés (em coordenadas relativas), visualizando o impacto dessas mudanças nas juntas das pernas,
        além de comparar com os resultados da cinemática direta (forward kinematics).
        Oferece recursos para alternar a ação da gravidade, visualizar anotações e receber feedback sobre comandos inválidos ou posições fora do alcance.

    Métodos:

        _user_control(self)
            Gerencia a interface de entrada do usuário para comandos de ajuste das posições e rotações dos tornozelos/pés e opções auxiliares.

        _draw_labels(self, player: Agent)
            Desenha anotações visuais no ambiente para facilitar o entendimento das posições e rotações dos tornozelos, com comparação entre cinemática inversa e direta.

        print_help(self)
            Exibe instruções detalhadas sobre os comandos disponíveis para o usuário durante a demonstração.

        execute(self)
            Inicia o ciclo principal de demonstração, permitindo o ajuste contínuo das posições dos tornozelos/pés, processando comandos do usuário
            e atualizando o robô na simulação, com feedback visual e textual.
    """

    def __init__(self, script: Script) -> None:
        """
        Descrição:
            Inicializa a classe Inv_Kinematics, armazenando os argumentos do script, configurando variáveis de controle,
            ativando a gravidade e definindo a postura inicial dos pés do robô na posição neutra.

        Parâmetros:
            script: Script
                Referência ao script principal para obter argumentos e contexto.
        """
        self.state = None
        self.args = script.args
        self.last_action = (0, 0, 0)
        self.gravity = True

        # Postura inicial neutra: todas as articulações com ângulo 0
        leg_y_dev, upper_leg_height, upper_leg_depth, lower_leg_len, _, _ = InverseKinematics.NAO_SPECS_PER_ROBOT[self.args.r]
        leg_height = upper_leg_height + lower_leg_len
        # feet_pose: [ [posição, rotação] para cada pé ]
        # posição: [x, y, z]; rotação: [roll, pitch, yaw]
        self.feet_pose = [
            [[upper_leg_depth, leg_y_dev, -leg_height], [0, 0, 0]],  # Pé esquerdo
            [[upper_leg_depth, -leg_y_dev, -leg_height], [0, 0, 0]]  # Pé direito
        ]

    def _user_control(self):
        """
        Descrição:
            Gerencia a interface de entrada do usuário para controlar a posição/rotação dos tornozelos e alternar opções como gravidade.
            Interpreta comandos como seleção de lado (esquerdo/direito), eixo, valor e opções rápidas.
            Fornece feedback para comandos inválidos e permite repetição rápida do último comando.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            int
                Quantidade de passos de simulação a avançar, conforme o comando do usuário.
        """
        while True:
            inp = input("Command:")
            if inp == "":
                return 2  # Avança 2 steps na simulação
            elif inp == ".":
                return 1  # Avança 1 step
            elif inp == "h":
                self.print_help()
                continue
            elif inp == "g":
                self.gravity = not self.gravity
                print("Using gravity:", self.gravity)
                if self.gravity:
                    return 6  # Passos extra para o beam ter efeito
                else:
                    return 1

            # Checa se input é valor numérico para repetição do último comando
            try:
                val = float(inp)
                self.feet_pose[self.last_action[0]][self.last_action[1]][self.last_action[2]] = val
                continue
            except Exception:
                pass

            # Checa formato do comando de ação
            if inp[0] not in ['l', 'r'] or inp[1] not in ['x', 'y', 'z', 'X', 'Y', 'Z']:
                print("Illegal command!")
                continue

            side = 0 if inp[0] == 'l' else 1
            pos_rot = 0 if inp[1].islower() else 1
            axis = {'x': 0, 'y': 1, 'z': 2}[inp[1].lower()]
            self.last_action = (side, pos_rot, axis)

            try:
                val = float(inp[2:])
                self.feet_pose[side][pos_rot][axis] = val
            except Exception:
                print("Illegal value conversion!")

    def _draw_labels(self, player: BaseAgent):
        """
        Descrição:
            Desenha, para cada tornozelo, anotações e linhas que mostram as posições/rotações definidas pelo usuário (cinemática inversa)
            e compara com os valores calculados pela cinemática direta, facilitando a análise visual no ambiente de simulação.

        Parâmetros:
            player: Agent
                Instância do agente no ambiente de simulação.

        Retorno:
            None
        """
        r = player.world.robot
        robot_pos = r.loc_head_position
        for i, body_part in enumerate(['lankle', 'rankle']):
            pos = r.get_body_part_abs_position(body_part)
            label_rel_pos = np.array([-0.2, (0.5 - i), 0])
            label_rel_pos /= np.linalg.norm(label_rel_pos) / 1.0  # Coloca os labels a 1m de distância

            # Linha indicando a posição do label
            player.world.draw.line(pos, pos + label_rel_pos, 2, Draw.Color.green_light, body_part, False)

            # Mostra valores definidos pelo usuário (cinemática inversa)
            p = self.feet_pose[i]
            pose_text = (
                f"x:{p[0][0]:.4f}    y:{p[0][1]:.4f}    z:{p[0][2]:.4f}",
                f"rol:{p[1][0]:.2f} (bias)   pit:{p[1][1]:.2f} (bias)   yaw:{p[1][2]:.2f} "
            )
            player.world.draw.annotation(pos + label_rel_pos + [0, 0, 0.2], pose_text[0], Draw.Color.cyan, body_part, False)
            player.world.draw.annotation(pos + label_rel_pos + [0, 0, 0.1], pose_text[1], Draw.Color.cyan, body_part, False)

            # Mostra valores resultantes da cinemática direta para comparação
            p_fk = player.inv_kinematics.get_body_part_pos_relative_to_hip(body_part)
            foot_rel_torso = r.head_to_body_part_transform("torso", r.body_parts[['lfoot', 'rfoot'][i]].transform)
            w = foot_rel_torso.get_roll_deg(), foot_rel_torso.get_pitch_deg(), foot_rel_torso.get_yaw_deg()
            pose_text_fk = (
                f"x:{p_fk[0]:.4f}    y:{p_fk[1]:.4f}    z:{p_fk[2]:.4f}",
                f"rol:{w[0]:.4f}    pit:{w[1]:.4f}    yaw:{w[2]:.4f}"
            )
            player.world.draw.annotation(pos + label_rel_pos + [0, 0, -0.2], pose_text_fk[0], Draw.Color.red, body_part, False)
            player.world.draw.annotation(pos + label_rel_pos + [0, 0, -0.3], pose_text_fk[1], Draw.Color.red, body_part, False)
            player.world.draw.annotation(pos + label_rel_pos + [0, 0, -0.4], "(forward kinematics data)", Draw.Color.red, body_part, True)

        # Mostra ângulos do torso, úteis para depuração
        note = f"Torso roll: {r.imu_torso_roll:.2f}   Torso pitch: {r.imu_torso_pitch:.2f}"
        player.world.draw.annotation(robot_pos + [0, 0, 0.10], note, Draw.Color.red, "Torso")

    def print_help(self):
        """
        Descrição:
            Exibe instruções detalhadas sobre os comandos aceitos pelo sistema interativo de cinemática inversa, incluindo exemplos
            e explicações sobre cada opção disponível ao usuário.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """
        print("""
---------------- Inverse kinematics demonstration ----------------
INPUT: ankle positions + feet rotations  (relative coordinates)
OUTPUT: angular positions of both legs' joints 
------------------------------------------------------------------
Command: {action/option}
    action: [side:{l/r} axis*:{x/y/z/X/Y/Z}] value 
            *for position use x/y/z, for rotation use X/Y/Z
    option: {"",.,g,h}
Examples:
    "lz-0.12" - move left ankle to -0.1m in the z-axis
    "rX30.5"  - rotate right foot to 30.5 deg in the x-axis (roll)
    "20"      - repeat last action but change value to 20
    ""        - advance 2 simulation step
    "."       - advance 1 simulation step
    "g"       - toggle gravity
    "h"       - help, display this message
    "ctrl+c"  - quit demonstration
------------------------------------------------------------------""")

    def execute(self):
        """
        Descrição:
            Executa o ciclo principal da demonstração de cinemática inversa:
            - Cria um agente e posiciona-o na posição inicial.
            - Mostra instruções de uso.
            - Permite ao usuário ajustar, em tempo real, as posições e rotações dos tornozelos/pés via comandos interativos.
            - Atualiza a pose do robô, executa a cinemática inversa para as pernas e fornece feedback visual e textual sobre a pose.
            - Informa o usuário sobre posições fora do alcance ou juntas fora dos limites.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """

        self.state = 0
        a = self.args

        self.print_help()
        # Cria agente para controle do robô
        player = BaseAgent(a.i, a.p, a.m, a.u, a.r, a.t)  # Args: Server IP, Agent Port, Monitor Port, Uniform No., Robot Type, Team Name

        # Posiciona agente na posição inicial usando 'beam'
        player.scom.unofficial_beam((-3, 0, 0.42), 0)

        next_control_step = 20

        # Loop principal de demonstração
        for i in count():

            if self.gravity:
                # Reaplica 'beam' para garantir posição estável sob gravidade
                player.scom.unofficial_beam((-3, 0, 0.42), 0)

            # Desenha labels com valores atuais e feedback visual
            self._draw_labels(player)

            # Realiza controle do usuário na etapa agendada
            if i == next_control_step:
                next_control_step += self._user_control()

            # Atualiza cada perna usando cinemática inversa baseada na pose definida
            for i in range(2):  # Pé esquerdo e direito
                indices, values, error_codes = player.inv_kinematics.leg(
                    self.feet_pose[i][0], self.feet_pose[i][1], bool(i == 0), False
                )

                if -1 in error_codes:
                    print("Position is out of reach!")
                    error_codes.remove(-1)
                for j in error_codes:
                    print(f"Joint {j} is out of range!")

                player.world.robot.set_joints_target_position_direct(indices, values)

            # Envia comandos para o simulador e atualiza estado
            for _ in range(20):
                player.scom.commit_and_send(player.world.robot.get_command())
                player.scom.receive()






















