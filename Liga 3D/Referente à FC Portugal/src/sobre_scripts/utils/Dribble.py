from Agent.Agent import Agent
from Agent.BaseAgent import BaseAgent
from sobre_scripts.commons.Script import Script
import numpy as np


class Dribble:
    """
    Descrição:
        Simulação e demonstração do comportamento de drible.
        Instancia um agente driblador (robô base) e um agente oponente, controla o fluxo de execução para o drible com reposicionamento automático, visualização de informações e interação com o ambiente de simulação.

    Métodos:
        __init__(self, script)
            Inicializa a classe Dribble com o script principal.

        execute(self)
            Executa o loop principal de simulação do drible, controlando o agente driblador, o agente oponente, as anotações gráficas e a sincronização com o servidor.
    """

    def __init__(self, script: Script) -> None:
        """
        Descrição:
            Inicializa a classe Dribble, armazenando a referência ao script principal para acesso aos argumentos,
            gerenciamento de agentes e controle do ambiente de simulação.

        Parâmetros:
            script: Script
                Referência ao script principal da aplicação.

        Retorno:
            None
        """
        self.script = script

    def execute(self):
        """
        Descrição:
            Executa o ciclo principal do experimento de drible:
            - Instancia um agente driblador (Base_Agent) e um agente oponente.
            - Configura as opções de visualização do caminho planejado e obstáculos.
            - Reposiciona o driblador e inicia o modo de jogo.
            - Entra em um loop contínuo onde:
                - Reposiciona o driblador automaticamente em situações de "beam" ou início de jogo.
                - Executa o comportamento "Dribble" ou de recuperação ("Zero_Bent_Knees") conforme o contexto.
                - Atualiza uma anotação visual com a velocidade da cabeça do robô.
                - Executa os agentes oponentes e sincroniza todos os agentes com o servidor.
            - O loop é interrompido apenas externamente (ex: ctrl+c).

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """

        a = self.script.args

        # Cria um agente driblador (Base_Agent) e um agente oponente (Agent)
        # Batch_create permite instanciar múltiplos agentes de uma vez
        # Args: Server IP, Agent Port, Monitor Port, Uniform No., Robot Type, Team Name, Enable Log, Enable Draw
        self.script.batch_create(BaseAgent, ((a.i, a.p, a.m, a.u, a.r, a.t, True, True),))  # Driblador
        self.script.batch_create(Agent, ((a.i, a.p, a.m, u, "Opponent", False, False) for u in range(1, 2)))  # Oponente

        # Referências aos objetos do agente driblador
        p: BaseAgent = self.script.players[0]
        p.path_manager.draw_options(enable_obstacles=True, enable_path=True)  # Ativa visualização de obstáculos e caminho

        behavior = p.behavior
        w = p.world
        r = w.robot
        d = w.draw

        # Reposiciona o driblador na posição inicial e inicia o modo de jogo
        p.scom.unofficial_beam((-3, 0, r.beam_height), 0)
        p.scom.unofficial_set_play_mode("PlayOn")
        print("\nPress ctrl+c to return.")

        while True:
            # Se for o início do jogo adversário, força modo de jogo para "PlayOn"
            if w.play_mode == w.M_THEIR_KICKOFF:
                p.scom.unofficial_set_play_mode("PlayOn")

            # Executa comportamento de recuperação ou inicia drible conforme contexto de queda/beam
            if behavior.is_ready("GetUp") or w.play_mode_group in [w.MG_ACTIVE_BEAM, w.MG_PASSIVE_BEAM]:
                # Reposiciona o driblador atrás da bola e executa postura de recuperação
                p.scom.unofficial_beam((*(w.ball_abs_pos[:2] - (1, 0)), r.beam_height), 0)
                behavior.execute("Zero_Bent_Knees")
            else:
                # Executa o drible normalmente
                behavior.execute("Dribble", None, None)

            # Anota visualmente a velocidade da cabeça do robô acima do robô
            d.annotation(r.loc_head_position + (0, 0, 0.2),
                         f"{np.linalg.norm(r.get_head_abs_vel(40)[:2]):.2f}",
                         d.Color.white, "vel_annotation")

            # Envia o comando para o simulador
            p.scom.commit_and_send(r.get_command())

            # Executa os oponentes como agentes normais
            self.script.batch_execute_agent(slice(1, None))

            # Todos aguardam feedback do servidor
            self.script.batch_receive()
