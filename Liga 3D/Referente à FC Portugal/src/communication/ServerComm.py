from communication.WorldParser import WorldParser
from itertools import count
from select import select
from sys import exit
from world.World import World
import socket
import time


class ServerComm:
    """

    """

    def __init__(
            self,
            host: str,
            agent_port: int,
            monitor_port: int,
            unum: int,
            robot_type: int,
            team_name: str,
            world_parser: World_Parser,
            world: World,
            other_players,
            wait_for_server=True
    ) -> None:
        """
        Descrição:
            Inicializa uma instância de comunicação com o servidor de simulação para um agente robô.

        Parâmetros:
            host (str):
                Endereço do servidor.
            agent_port (int):
                Porta para conexão do agente.
            monitor_port (int):
                Porta para conexão do monitor (pode ser None).
            unum (int):
                Número único do agente.
            robot_type (int):
                Tipo do robô (identificador do modelo).
            team_name (str):
                Nome do time do agente.
            world_parser (World_Parser):
                Instância responsável pelo parsing do mundo.
            world (World):
                Instância do mundo compartilhado entre agentes.
            other_players (iterable):
                Lista ou iterável contendo os outros jogadores.
            wait_for_server (bool, opcional):
                Se True, espera o servidor estar disponível antes de conectar.

        Retorno:
            - Estabelece conexão TCP com o servidor do agente e, se necessário, com o monitor.
            - Realiza uma sequência de mensagens de inicialização e sincronização com o servidor.
            - Garante que informações essenciais (como o lado do campo) sejam recebidas antes de prosseguir.
            - Prepara buffers de envio e recebimento, e inicializa variáveis importantes do agente.
            - Em caso de falha na conexão ou na inicialização, encerra o programa com mensagens apropriadas.
        """

        # Finalmente temos a informação do tamanho do buffer. Esse maldito está aparecendo
        # desde /sobre_cpp/robovizlogger.h
        self.BUFFER_SIZE = 8192
        self.rcv_buff = bytearray(self.BUFFER_SIZE)
        self.send_buff = []
        self.world_parser = world_parser
        self.unum = unum

        # Durante a inicialização, não se sabe se o agente está do lado esquerdo ou direito
        self._unofficial_beam_msg_left = "(agent (unum " + str(unum) + ") (team Left) (move "
        self._unofficial_beam_msg_right = "(agent (unum " + str(unum) + ") (team Right) (move "
        self.world = world

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Tenta conectar ao servidor, aguardando se necessário
        if wait_for_server:
            print("Aguardando o servidor em ", host, ":", agent_port, sep="", end=".", flush=True)
        while True:
            try:
                self.socket.connect((host, agent_port))
                print(end=" ")
                break
            except ConnectionRefusedError:
                if not wait_for_server:
                    print("Servidor está fora do ar. Encerrando...")
                    exit()
                time.sleep(1)
                print(".", end="", flush=True)
        print("Agente conectado", unum, self.socket.getsockname())

        # Envia mensagem de cena para selecionar o modelo do robô
        # Observe que enviaremos a mensagem em bytes
        self.send_immediate(b'(scene rsg/agent/nao/nao_hetero.rsg ' + str(robot_type).encode() + b')')
        self._receive_async(other_players, True)

        # Envia mensagem de inicialização informando número e nome do time
        self.send_immediate(b'(init (unum ' + str(unum).encode() + b') (teamname ' + team_name.encode() + b'))')
        self._receive_async(other_players, False)

        # Repete o processo para garantir que a informação do lado do campo seja recebida
        for _ in range(3):
            # Elimina avanço de passo extra alterando a ordem do syn (bug do protocolo rcssserver3d, geralmente para o jogador 11)
            self.send_immediate(b'(syn)')  # Caso não seja necessário, o servidor descarta
            for p in other_players:
                p.scom.send_immediate(b'(syn)')
            for p in other_players:
                p.scom.receive()
            self.receive()

        if world.team_side_is_left is None:
            print("\nErro: o servidor não retornou o lado do time! Verifique o terminal do servidor!")
            exit()

        # O socket do monitor é compartilhado por todos os agentes na mesma thread
        if Server_Comm.monitor_socket is None and monitor_port is not None:
            print("Conectando à porta de monitoramento do servidor em ", host, ":", monitor_port, sep="", end=".", flush=True)
            Server_Comm.monitor_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            Server_Comm.monitor_socket.connect((host, monitor_port))
            print("Conexão concluída!")


