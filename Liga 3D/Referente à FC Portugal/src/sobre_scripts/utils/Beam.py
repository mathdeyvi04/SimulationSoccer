from Agent.BaseAgent import BaseAgent
from sobre_scripts.commons.Script import Script
from sobre_scripts.commons.UserInterface import UserInterface
from sobre_scripts.utils.Testing import Testing
from time import sleep

import threading as th


class Beam(Testing):
    """
    Descrição:
        Classe utilitária para manipulação do posicionamento instantâneo ("beam").
        Permite solicitar entradas do usuário, realizar o reposicionamento do robô no campo e atualizar a simulação
        para garantir estabilidade após o reposicionamento.

        Útil para modificar configurações de posicionamentos de time.
    """

    def __init__(self, script: Script) -> None:
        """
        Descrição:
            Inicializa a classe Beam com uma referência ao script principal,
            fornecendo acesso ao agente e ao ambiente de simulação.

        Parâmetros:
            script: Script
                Objeto principal de script que contém referências ao agente, ambiente e métodos utilitários.
        """
        self.player = None
        self.script = script

    # Antes havia uma função aqui. Entretanto, acredito que algo melhor possa ser feito.

    def beam_and_update(self, x: float, y: float, rot: float) -> None:
        """
        Descrição:
            Reposiciona instantaneamente o robô nas coordenadas especificadas e atualiza a simulação para ajustar possíveis colisões ou instabilidades.

            - Adiciona uma anotação visual no ambiente indicando a nova posição.
            - Realiza o comando de 'beam' para transportar o robô para (x, y, altura padrão) com a rotação especificada.
            - Executa múltiplos ciclos de simulação após o reposicionamento para garantir que o robô se estabilize, inclusive em casos de colisão com obstáculos.

        Parâmetros:
            x: float
                Coordenada x de destino para o robô.
            y: float
                Coordenada y de destino para o robô.
            rot: float
                Rotação (em radianos ou graus, conforme o simulador) a ser aplicada ao robô.

        Retorno:
            None
        """
        r = self.player.world.robot
        d = self.player.world.draw

        # Adiciona uma anotação visual na nova posição
        d.annotation((x, y, 0.7), f"x:{x} y:{y} r:{rot}", d.Color.yellow, "pos_label")

        # Executa o comando 'beam' para reposicionar o robô
        self.player.scom.unofficial_beam((x, y, r.beam_height), rot)

        # Realiza múltiplos ciclos de simulação para estabilizar o robô após o beam
        for _ in range(10):
            sleep(0.03)
            self.player.behavior.execute("Zero")
            self.player.scom.commit_and_send(r.get_command())
            self.player.scom.receive()

        return None

    @staticmethod
    def receber_entrada_paralela(
        running: list[bool],
        tripla_de_estados: list[float],
    ):
        """

        """

        while running[0]:
            if tripla_de_estados[-1] < 3:
                tripla_de_estados[tripla_de_estados[-1]] = UserInterface.obter_float(
                    f"Entre com a {tripla_de_estados[-1] + 1}° coordenada ('' para o resultado anterior, ctrl+c para sair): ", default=tripla_de_estados[tripla_de_estados[-1]]
                )

                print()

                tripla_de_estados[-1] += 1
            else:
                sleep(0.1)

        print("- Saindo do Loop de Entradas")

    def execute(self):
        """
        Descrição:
            Executa o fluxo principal de posicionamento interativo do robô no ambiente de simulação.

            - Inicializa o agente com os argumentos fornecidos no script (IP, portas, tipo de robô, nome do time, etc.).
            - Define o modo de jogo para "PlayOn" na simulação.
            - Desenha uma grade de referência no campo, marcando pontos de -15 a 15 no eixo x e de -10 a 10 no eixo y.
            - Realiza alguns ciclos iniciais de comunicação para estabilizar o ambiente.
            - Entra em um loop interativo onde o usuário pode informar coordenadas (x, y) e orientação (a) para posicionar o robô utilizando o comando "beam".
            - Após cada entrada, o robô é reposicionado e a simulação é atualizada, permitindo ao usuário testar diferentes posições e orientações de forma rápida.

            O loop pode ser interrompido com ctrl+c para retornar ao fluxo principal do script.
        """

        a = self.script.args
        # Inicializa o agente com os parâmetros do script
        self.player = BaseAgent(a.i, a.p, a.m, a.u, a.r, a.t)  # Args: Server IP, Agent Port, Monitor Port, Uniform No., Robot Type, Team Name
        d = self.player.world.draw

        # Define o modo de jogo para "PlayOn"
        self.player.scom.unofficial_set_play_mode("PlayOn")

        print("\n\033[1;7mAguarde alguns segundos...\033[0m")

        # Desenha uma grade de referência no campo
        for x in range(-15, 16):
            for y in range(-10, 11):
                d.point((x, y), 6, d.Color.red, "grid", False)
                self.player.scom.send()
                self.player.scom.receive()
        d.flush("grid")

        print("\nTeletransporte usando (x, y, orientação): \n")

        tripla_de_estados = [0, 0, 0, 0]  # O quarto elemento servirá como index.
        running = [True]
        r = self.player.world.robot

        paralela = th.Thread(target=Beam.receber_entrada_paralela, args=(running, tripla_de_estados))
        paralela.start()

        try:
            while True:

                # Se desejamos que o loop não seja bloqueado pela espera da entrada, 
                # devemos utilizar threads.
                if tripla_de_estados[-1] == 3:
                    self.beam_and_update(tripla_de_estados[0], tripla_de_estados[1], tripla_de_estados[2])
                    tripla_de_estados[-1] = 0

                # Apenas para servidor não ficar bloqueado.
                sleep(0.02)
                self.player.scom.commit_and_send(r.get_command())
                self.player.scom.receive()
        except KeyboardInterrupt:
            print("- Saindo do Loop Principal")
            running[0] = False
            paralela.join()

