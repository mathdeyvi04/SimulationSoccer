from Agent.BaseAgent import BaseAgent
from sobre_scripts.commons.Script import Script
from world.commons.Draw import Draw
import numpy as np


class Fwd_Kinematics:
    """
    Descrição:
        Classe utilitária para visualização e análise da cinemática direta (forward kinematics) de múltiplos agentes robóticos em ambiente de simulação.
        Permite desenhar e acompanhar visualmente, ao longo dos ciclos de simulação, a posição, orientação e nomeação das partes do corpo e juntas do robô,
        facilitando a depuração, estudo e ensino da estrutura cinemática do agente.

    Métodos Disponíveis:
        draw_cycle(self)
            Executa uma sequência de ciclos de simulação, desenhando:
                - A posição das partes do corpo do robô;
                - A posição das juntas;
                - A orientação dos eixos principais das partes do corpo.
            Limpa as anotações visuais ao final de cada etapa.

        execute(self)
            Instancia múltiplos agentes, posiciona-os em campo e, em laço contínuo, executa a visualização da cinemática direta, até ser interrompido externamente.
    """

    def __init__(self, script: Script) -> None:
        """
        Descrição:
            Inicializa a classe Fwd_Kinematics com a referência ao script principal e define a duração padrão dos ciclos de visualização.

        Parâmetros:
            script: Script
                Referência ao script principal da aplicação.

        Retorno:
            None
        """
        self.script = script
        self.cycle_duration = 200  # Número de steps para cada ciclo de visualização

    def draw_cycle(self):
        """
        Descrição:
            Executa três etapas de visualização gráfica para todos os agentes:
                1. Desenha a posição e o nome das partes do corpo do robô;
                2. Desenha a posição e o nome das juntas;
                3. Desenha a orientação dos eixos (x, y, z) das principais partes do corpo.
            Cada etapa é executada durante um ciclo de passos de simulação e, ao final, as anotações são limpas.
            Utiliza comportamento "Squat" para garantir postura conhecida dos agentes durante a visualização.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """

        # --------- Etapa 1: Desenhar posição das partes do corpo ---------
        for _ in range(self.cycle_duration):
            self.script.batch_execute_behavior("Squat")
            self.script.batch_commit_and_send()
            self.script.batch_receive()

            for p in self.script.players:
                # Se não há dados visuais suficientes, avisa o usuário (usando IMU)
                if p.world.vision_is_up_to_date and not p.world.robot.loc_is_up_to_date:
                    p.world.draw.annotation(
                        p.world.robot.cheat_abs_pos,
                        "Not enough visual data! Using IMU",
                        Draw.Color.red,
                        "ambientacao"
                    )

                # Para cada parte do corpo, desenha linha e anota o nome
                for key, val in p.world.robot.body_parts.items():
                    rp = val.transform.obter_vetor_de_translacao()
                    pos = p.world.robot.loc_head_to_field_transform(rp, False)
                    label_rp = np.array([rp[0] - 0.0001, rp[1] * 0.5, 0])
                    label_rp /= np.linalg.norm(label_rp) / 0.4  # Posicionamento do label a 0.4m
                    label = p.world.robot.loc_head_to_field_transform(rp + label_rp, False)
                    p.world.draw.line(pos, label, 2, Draw.Color.green_light, key, False)
                    p.world.draw.annotation(label, key, Draw.Color.red, key)

                # Desenha marcadores extras no pé esquerdo (lfoot)
                rp = p.world.robot.body_parts['lfoot'].transform((0.08, 0, 0))
                ap = p.world.robot.loc_head_to_field_transform(rp, False)
                p.world.draw.line(ap, ap + (0, 0, 0.1), 1, Draw.Color.red, "soup", False)

                rp = p.world.robot.body_parts['lfoot'].transform((-0.08, 0, 0))
                ap = p.world.robot.loc_head_to_field_transform(rp, False)
                p.world.draw.line(ap, ap + (0, 0, 0.1), 1, Draw.Color.red, "soup", False)

                rp = p.world.robot.body_parts['lfoot'].transform((0, 0.04, 0))
                ap = p.world.robot.loc_head_to_field_transform(rp, False)
                p.world.draw.line(ap, ap + (0, 0, 0.1), 1, Draw.Color.red, "soup", False)

                rp = p.world.robot.body_parts['lfoot'].transform((0, -0.04, 0))
                ap = p.world.robot.loc_head_to_field_transform(rp, False)
                p.world.draw.line(ap, ap + (0, 0, 0.1), 1, Draw.Color.red, "soup", True)

        Draw.clear_all()  # Limpa os desenhos antes da próxima etapa

        # --------- Etapa 2: Desenhar posição das juntas ---------
        for _ in range(self.cycle_duration):
            self.script.batch_execute_behavior("Squat")
            self.script.batch_commit_and_send()
            self.script.batch_receive()

            for p in self.script.players:
                if p.world.vision_is_up_to_date and not p.world.robot.loc_is_up_to_date:
                    p.world.draw.annotation(
                        p.world.robot.cheat_abs_pos,
                        "Not enough visual data! Using IMU",
                        Draw.Color.red,
                        "ambientacao"
                    )

                zstep = 0.05
                label_z = [
                    0, 0, 0, 0, zstep, zstep, 2 * zstep, 2 * zstep, 0, 0, 0, 0,
                    zstep, zstep, 0, 0, zstep, zstep, 2 * zstep, 2 * zstep, 3 * zstep, 3 * zstep, 0, 0
                ]
                for j, transf in enumerate(p.world.robot.joints_transform):
                    rp = transf.obter_vetor_de_translacao()
                    pos = p.world.robot.loc_head_to_field_transform(rp, False)
                    j_name = str(j)
                    label_rp = np.array([rp[0] - 0.0001, rp[1] * 0.5, 0])
                    label_rp /= np.linalg.norm(label_rp) / 0.4
                    label_rp += (0, 0, label_z[j])
                    label = p.world.robot.loc_head_to_field_transform(rp + label_rp, False)
                    p.world.draw.line(pos, label, 2, Draw.Color.green_light, j_name, False)
                    p.world.draw.annotation(label, j_name, Draw.Color.cyan, j_name)

        Draw.clear_all()  # Limpa os desenhos antes da próxima etapa

        # --------- Etapa 3: Desenhar orientação dos eixos das partes do corpo ---------
        for _ in range(self.cycle_duration):
            self.script.batch_execute_behavior("Squat")
            self.script.batch_commit_and_send()
            self.script.batch_receive()

            for p in self.script.players:
                if p.world.vision_is_up_to_date and not p.world.robot.loc_is_up_to_date:
                    p.world.draw.annotation(
                        p.world.robot.cheat_abs_pos,
                        "Not enough visual data! Using IMU",
                        Draw.Color.red,
                        "ambientacao"
                    )

                for key in p.world.robot.body_parts:
                    # Seleciona apenas partes principais
                    if key not in [
                        'head', 'torso', 'llowerarm', 'rlowerarm', 'lthigh', 'rthigh',
                        'lshank', 'rshank', 'lfoot', 'rfoot'
                    ]:
                        continue
                    bpart_abs_pos = p.world.robot.get_body_part_to_field_transform(key).translate((0.1, 0, 0))  # 10cm à frente
                    x_axis = bpart_abs_pos((0.05, 0, 0), False)
                    y_axis = bpart_abs_pos((0, 0.05, 0), False)
                    z_axis = bpart_abs_pos((0, 0, 0.05), False)
                    axes_0 = bpart_abs_pos.obter_vetor_de_translacao()
                    p.world.draw.line(axes_0, x_axis, 2, Draw.Color.green_light, key, False)
                    p.world.draw.line(axes_0, y_axis, 2, Draw.Color.blue, key, False)
                    p.world.draw.line(axes_0, z_axis, 2, Draw.Color.red, key)

        Draw.clear_all()  # Limpa todos os desenhos ao final

    def execute(self):
        """
        Descrição:
            Instancia múltiplos agentes robóticos, posiciona-os em campo e executa continuamente a visualização da cinemática direta.
            Permite observar, em tempo real, as transformações do corpo e juntas dos agentes durante um comportamento padrão ("Squat").
            O laço principal é interrompido apenas externamente (ex: ctrl+c).

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """

        a = self.script.args

        # Instancia 5 agentes com uniformes diferentes
        # Args: Server IP, BaseAgent Port, Monitor Port, Uniform No., Robot Type, Team Name, Enable Log, Enable Draw
        self.script.batch_create(
            BaseAgent,
            ((a.i, a.p, a.m, u, u - 1, a.t, True, True) for u in range(1, 6))
        )

        # Posiciona os jogadores em linha, espaçados no campo, com rotações variadas
        self.script.batch_unofficial_beam([(-2, i * 4 - 10, 0.5, i * 45) for i in range(5)])

        print("\nPress ctrl+c to return.")

        while True:
            self.draw_cycle()
