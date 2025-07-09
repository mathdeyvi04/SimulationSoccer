from Agent.BaseAgent import BaseAgent
from sobre_scripts.commons.Script import Script
from sobre_scripts.commons.UserInterface import UserInterface


class Behaviors:
    """
    Descrição:
        Gerenciamento e execução de comportamentos do agente. Permite listar, selecionar, executar e sincronizar
        comportamentos, além de utilizar o comando 'beam' para reposicionar rapidamente o robô.

        Utilizada para facilitar testes, depuração e interação manual com diferentes comportamentos
        disponíveis no agente.
    """

    def __init__(self, script: Script) -> None:
        """
        Descrição:
            Inicializa a classe Behaviors com uma referência ao script principal, permitindo acesso ao agente e ao ambiente de simulação.

        Parâmetros:
            script: Script
                Objeto principal do script que contém referências ao agente e utilitários.
        """
        self.script = script
        self.player: BaseAgent = None

    def ask_for_behavior(self):
        """
        Descrição:
            Lista todos os comportamentos disponíveis no agente e solicita ao usuário que escolha um para execução.
            Apresenta os nomes e descrições em formato de tabela, permitindo navegação interativa.

        Retorno:
            str ou valor especial:
                Nome do comportamento selecionado, ou uma opção especial caso o usuário escolha pular ou utilizar o comando 'beam'.
        """
        names, descriptions = self.player.behavior.get_all_behaviors()

        UserInterface.print_table(
            [names, descriptions],
            ["Nome do Comportamento", "Descrição"],
            numbering=[True, False]
        )
        choice, is_str_opt = UserInterface.obter_entrada(
            'Escolha o comportamento ("" to skip 2 time steps, "b" to beam, ctrl+c to return): ',
            ["", "b"], int, [0, len(names)]
        )

        return choice if is_str_opt else names[choice]

    def sync(self) -> None:
        """
        Descrição:
            Sincroniza o agente com o simulador, enviando o comando atual do robô e recebendo o novo estado do ambiente.
            Utilizado para garantir que comandos executados sejam processados e o ambiente esteja atualizado.
        """
        self.player.scom.commit_and_send(self.player.world.robot.get_command())
        self.player.scom.receive()

    def beam(self) -> None:
        """
        Descrição:
            Reposiciona instantaneamente o robô para uma posição padrão utilizando o comando 'beam',
            sincroniza o ambiente para garantir estabilidade.
            Executa múltiplas iterações de sincronização após o reposicionamento.
        """
        self.player.scom.unofficial_beam((-2.5, 0, self.player.world.robot.beam_height), 0)
        for _ in range(5):
            self.sync()

    def execute(self) -> None:
        """
        Descrição:
            Executa um loop interativo para seleção e execução de comportamentos do agente robótico em simulação.
            Inicializa o agente com os argumentos do script, faz o reposicionamento inicial (beam) e coloca o ambiente em modo "PlayOn".
            Permite ao usuário escolher um comportamento para ser executado, pular passos de tempo, ou reposicionar o robô.
            Para comportamentos especiais ("Step", "Basic_Kick", "Walk", "Dribble"), executa-os por um número definido de passos de tempo ou até sua conclusão antecipada.
            Para outros comportamentos, executa até sua conclusão automática.

        Parâmetros:
            None.

        Retorno:
            None.
        """

        a = self.script.args
        self.player = BaseAgent(a.i, a.p, a.m, a.u, a.r, a.t)  # Args: Server IP, Agent Port, Monitor Port, Uniform No., Robot Type, Team Name
        behavior = self.player.behavior

        self.beam()
        self.player.scom.unofficial_set_play_mode("PlayOn")

        # Parâmetros Padrões para os comportamentos especiais.
        special_behaviors = {
            "Step": (),
            "BasicKick": (0,),
            "Walk": ((0.5, 0), False, 0, False, None),
            "Dribble": (None, None)
        }

        while True:
            behavior_name = self.ask_for_behavior()
            if behavior_name == 0:  # skip 2 time steps (user request)
                self.sync()
                self.sync()
            elif behavior_name == 1:  # beam
                self.beam()
            else:
                if behavior_name in special_behaviors:  # not using execute_to_completion to abort behavior after a timeout
                    duration = UserInterface.obter_inteiro(
                        "Por quanto time steps [1,1000]? ", 1, 1001
                    )
                    for _ in range(duration):
                        if behavior.execute(
                                behavior_name, *special_behaviors[behavior_name]
                        ):
                            break  # break if behavior ends
                        self.sync()
                else:
                    behavior.execute_to_completion(behavior_name)

