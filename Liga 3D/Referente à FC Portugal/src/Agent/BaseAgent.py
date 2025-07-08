# Esse decorador é usado para marcar métodos em uma classe como abstratos,
# indicando que eles devem obrigatoriamente ser implementados por qualquer
# subclasse concreta.
from abc import abstractmethod

# Importamos tudo que já construímos.
from sobre_behaviors.Behavior import Behavior
from communication.Radio import Radio
from communication.ServerComm import ServerComm
from communication.WorldParser import WorldParser
from sobre_logs.Logger import Logger
from math_ops.InverseKinematics import InverseKinematics
from world.commons.PathManager import PathManager
from world.World import World


class BaseAgent:
    """
    Descrição:
        Classe fundamental para agentes, responsável por inicializar, gerenciar e finalizar
        todos os subsistemas essenciais do agente, como comunicação, comportamentos, mensagens e
        controle do mundo simulado.

    Métodos Disponíveis:
        - think_and_send
        - hear_message
        - terminate
        - terminate_all
    """
    all_agents = []

    def __init__(
            self,
            host: str,
            agent_port: int,
            monitor_port: int,
            unum: int,
            robot_type: int,
            team_name: str,
            enable_log: bool = True,
            enable_draw: bool = True,
            apply_play_mode_correction: bool = True,
            wait_for_server: bool = True,
            hear_callback=None
    ) -> None:
        """
        Descrição
            Inicializa uma instância do agente base, configurando todos os subsistemas necessários para o funcionamento do agente robótico,
            incluindo comunicação com o servidor, parser de mundo, controle de comportamento, cinemática inversa, rádio (mensagens),
            gerenciamento de caminho, e logging.
            
            - Registra o agente na lista global de agentes (`BaseAgent.all_agents`).
            - Permite flexibilidade na configuração do ambiente e integração de callbacks customizados para mensagens.
            - O rádio é inicializado após a comunicação para garantir que receba anúncios corretamente.
            - O agente pode ser removido/terminado de forma segura via o método `terminate`.

        Parâmetros:
            host: str
                Endereço do servidor (hostname ou IP).
            agent_port: int
                Porta de comunicação do agente.
            monitor_port: int
                Porta de monitoramento.
            unum: int
                Número único do agente (identificação do robô).
            robot_type: int
                Tipo do robô controlado.
            team_name: str
                Nome do time ao qual o agente pertence.
            enable_log: bool, opcional
                Habilita/desabilita logging detalhado (padrão: True).
            enable_draw: bool, opcional
                Habilita/desabilita recursos gráficos e de desenho (padrão: True).
            apply_play_mode_correction: bool, opcional
                Aplica correções de modo de jogo ao ambiente (padrão: True).
            wait_for_server: bool, opcional
                Aguarda servidor estar disponível antes de iniciar (padrão: True).
            hear_callback: callable, opcional
                Função alternativa de callback para mensagens de rádio (padrão: None, usa método interno).
        """
        self.radio = None  # Inicializa como None, pois a recepção pode ocorrer durante a instância de ServerComm
        self.logger = Logger(enable_log, f"{team_name}_{unum}")
        self.world = World(robot_type, team_name, unum, apply_play_mode_correction, enable_draw, self.logger, host)
        self.world_parser = WorldParser(self.world, self.hear_message if hear_callback is None else hear_callback)
        self.scom = ServerComm(host, agent_port, monitor_port, unum, robot_type, team_name, self.world_parser,
                                self.world, BaseAgent.all_agents, wait_for_server)
        self.inv_kinematics = Inverse_Kinematics(self.world.robot)
        self.behavior = Behavior(self)
        self.path_manager = Path_Manager(self.world)
        # Inicialização do rádio após comunicação e parser
        self.radio = Radio(self.world, self.scom.commit_announcement)
        self.behavior.create_behaviors()
        # Registra o agente para controle coletivo
        BaseAgent.all_agents.append(self)

    @abstractmethod
    def think_and_send(self):
        """
        Descrição
            Método abstrato que deve ser implementado por subclasses para definir o ciclo de decisão do agente.
            Responsável por realizar cálculos, tomar decisões e enviar comandos ao servidor a cada ciclo de controle.

            - Deve conter a lógica principal de atuação do agente no ambiente.
        """
        pass

    def hear_message(self, msg: bytearray, direction, timestamp: float) -> None:
        """
        Descrição:
            Método de callback para processamento de mensagens recebidas via rádio.
            Encaminha mensagens recebidas (que não sejam do próprio agente) para o
            sistema de rádio.

        Parâmetros:
            msg: bytearray
                Mensagem de rádio recebida.
            direction
                Direção da mensagem (ex: "self", "teammate", etc).
            timestamp: float
                Momento em que a mensagem foi recebida.
        """
        if direction != "self" and self.radio is not None:
            self.radio.receive(msg)

    def terminate(self):
        """
        Descrição:
            Termina a execução do agente, fechando canais de comunicação e removendo-o da lista global de agentes.

            - Fecha o socket de monitoramento compartilhado se este for o último agente ativo.
            - Remove o agente da lista global para liberar recursos.
        """
        # Fecha o monitoramento compartilhado se este for o último agente
        self.scom.close(close_monitor_socket=(len(BaseAgent.all_agents) == 1))
        # Remove o agente da lista global
        BaseAgent.all_agents.remove(self)

    @staticmethod
    def terminate_all():
        """
        Descrição:
            Termina todos os agentes registrados, fechando suas conexões e limpando a lista global.

        - Fecha o socket de monitoramento compartilhado para todos.
        - Limpa a lista de agentes garantindo que não haja vazamento de recursos.
        """
        for o in BaseAgent.all_agents:
            o.scom.close(True)  # Fecha o monitoramento compartilhado, se existir
        BaseAgent.all_agents = []
