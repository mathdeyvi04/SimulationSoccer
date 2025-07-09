from communication.WorldParser import WorldParser
from itertools import count
from select import select
from sys import exit
from world.World import World
import socket
import time


class ServerComm:
    """
    Descrição:
        Classe focada no gerenciamento das mensagens e conexões entre o Agente
        e o Servidor.

        Possui alguns métodos não oficiais que deverão ser utilizados somente
        em ambientes de teste.

    Métodos Disponíveis:
        - _receive_async
        - receive
        - send_immediate
        - send
        - commit
        - commit_and_send
        - clear_buffer
        - commit_announcement
        - commit_pass_command
        - commit_beam
        - unofficial_beam
        - unofficial_kill_sim
        - unofficial_move_ball
        - unofficial_set_game_time
        - unofficial_set_play_mode
        - unofficial_kill_player
        - close
                                    -

    """

    monitor_socket = None

    def __init__(
            self,
            host: str,
            agent_port: int,
            monitor_port: int,
            unum: int,
            robot_type: int,
            team_name: str,
            world_parser: WorldParser,
            world: World,
            other_players: list,  # list[BaseAgent] | list[Agent]
            wait_for_server: bool = True
    ) -> None:
        """
        Descrição:
            Inicializa uma instância de comunicação com o servidor de simulação
            para um agente robô.

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
        if ServerComm.monitor_socket is None and monitor_port is not None:
            print("Conectando à porta de monitoramento do servidor em ", host, ":", monitor_port, sep="", end=".", flush=True)
            ServerComm.monitor_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            ServerComm.monitor_socket.connect((host, monitor_port))
            print("Conexão concluída!")

    def _receive_async(
            self,
            other_players: list,  # list[BaseAgent] | list[Agent]
            first_pass: bool
    ) -> None:
        """
        Descrição:
            Função privada usada para receber informações assíncronas do servidor
            durante a fase de inicialização do agente.

        Parâmetros:
            other_players (iterable):
                Lista ou iterável contendo outros agentes/jogadores.
            first_pass (bool):
                Indica se esta é a primeira chamada do ciclo de inicialização (True) ou não (False).

        Retorno:
            - Realiza uma tentativa de recebimento de mensagem do servidor de forma não bloqueante.
            - Coordena a sincronização entre o agente atual e os outros jogadores, enviando e recebendo mensagens 'syn'.
            - Imprime mensagens de progresso para indicar o andamento da inicialização assíncrona.
            - Restaura o modo de bloqueio original do socket ao final do processo.
        """

        # Se não houver outros jogadores, realiza o recebimento normalmente e retorna
        if not other_players:
            self.receive()
            return

        # Define o socket como não bloqueante para permitir polling
        self.socket.setblocking(False)
        if first_pass:
            print("Inicializando agentes assíncronos", self.unum, end="", flush=True)

        while True:
            try:
                print(".", end="", flush=True)
                self.receive()  # Tenta receber mensagem do servidor
                break
            except:
                # Se não conseguir receber, sincroniza com os outros jogadores
                pass
            for p in other_players:
                p.scom.send_immediate(b'(syn)')
            for p in other_players:
                p.scom.receive()

        # Restaura o modo bloqueante padrão do socket
        self.socket.setblocking(True)
        if not first_pass:
            print("Concluído!")

    def receive(self, update: bool = True):
        """
        Descrição:
            Recebe e processa mensagens do servidor via socket, realizando parsing dos dados recebidos.

            O método escuta o socket associado ao agente, processando todos os pacotes disponíveis
            até que não haja mais dados a receber no momento. Em cada iteração, lê o cabeçalho do pacote
            (4 bytes que indicam o tamanho da mensagem seguinte), faz o parsing do conteúdo da mensagem,
            e, opcionalmente, aciona a atualização do estado do mundo após o processamento.

            Encerra o programa caso o socket seja fechado inesperadamente (ConnectionResetError).

        Parâmetros:
            update (bool):
                Se True (padrão), executa update do mundo após o recebimento e parsing das mensagens.

        Observações:
            - Se múltiplos pacotes forem recebidos sem update intermediário, é registrado um log indicando
              possível perda de pacotes ou configuração inadequada do modo de sincronização.
            - Se um novo pacote chegar durante a atualização do mundo, a função é chamada recursivamente
              para garantir que todos os dados sejam processados.
        """
        i = 0
        for i in count():
            # Loop infinito: processa todos os pacotes disponíveis na fila do socket
            try:
                # Recebe o cabeçalho do pacote (4 bytes com o tamanho da mensagem)
                if self.socket.recv_into(self.rcv_buff, nbytes=4) != 4:
                    raise ConnectionResetError()

                msg_size = int.from_bytes(self.rcv_buff[:4], byteorder='big', signed=False)

                # Recebe o corpo da mensagem, utilizando o tamanho informado no cabeçalho
                if self.socket.recv_into(self.rcv_buff, nbytes=msg_size, flags=socket.MSG_WAITALL) != msg_size:
                    raise ConnectionResetError()
            except ConnectionResetError:
                print("\nError: socket foi fechado por rcssserver3d!")
                exit()

            # Realiza o parsing da mensagem recebida, muito aloprado.
            self.world_parser.parse(self.rcv_buff[:msg_size])

            # Checa se ainda há dados disponíveis para leitura imediata no socket
            if len(select([self.socket], [], [], 0.0)[0]) == 0:
                break

        if update:
            # Loga aviso caso múltiplos pacotes tenham sido recebidos sem update
            if i == 1:
                self.world.log("ServerComm.py: O agente perdeu 1 pacote! O modo de sincronização está ativado?")
            if i > 1:
                self.world.log(f"ServerComm.py: O agente perdeu {i} pacotes consecutivos! O modo de sincronização está desabilitado?")

            # Atualiza o estado do mundo com as novas informações
            # O parsing permite que usemos as variáveis.
            self.world.update()

            # Se um novo pacote chegou durante o update, processa recursivamente
            # select é uma função que permite verificar se há dados prontos para leitura,
            # escrita ou erro em sockets, sem bloquear a execução do programa.
            if len(select([self.socket], [], [], 0.0)[0]) != 0:
                self.world.log("ServerComm.py: Recebeu um novo pacote em world.update()!")
                self.receive()

    def send_immediate(self, msg: bytes) -> None:
        """
        Descrição:
            Envia imediatamente uma mensagem para o servidor via socket.

            A mensagem é precedida por um cabeçalho de 4 bytes indicando o tamanho do corpo da mensagem em bytes.
            Caso o socket esteja fechado (BrokenPipeError), exibe uma mensagem de erro e encerra o programa.

        Parâmetros:
            msg (bytes): Mensagem a ser enviada, no formato de bytes.
        """
        try:
            # Envia o tamanho da mensagem (4 bytes, big-endian) seguido da mensagem propriamente dita
            self.socket.send(len(msg).to_bytes(4, byteorder='big') + msg)
        except BrokenPipeError:
            print("\nError: socket foi fechado por rcssserver3d!")
            exit()

    def send(self) -> None:
        """
        Descrição:
            Envia todas as mensagens acumuladas no buffer de envio para o servidor.

            Se não houver dados prontos para leitura no socket, adiciona um pacote de sincronização '(syn)' ao buffer,
            junta todas as mensagens do buffer e envia de uma vez. Caso contrário, registra um log indicando
            que um novo pacote foi recebido enquanto o agente ainda estava "pensando".

            Após o envio, limpa o buffer de mensagens.
        """
        # Checa se não há dados para ler no socket (ou seja, pode enviar)
        if len(select([self.socket], [], [], 0.0)[0]) == 0:
            self.send_buff.append(b'(syn)')  # Adiciona pacote de sincronização
            self.send_immediate(b''.join(self.send_buff))  # Envia todas as mensagens concatenadas
        else:
            # Se um novo pacote foi recebido enquanto o agente pensava, registra no log
            self.world.log("ServerComm.py: Recebeu um novo pacote enquanto estava pensando.")

        self.send_buff = []  # Limpa o buffer após o envio

    def commit(self, msg: bytes) -> None:
        """
        Descrição:
            Adiciona uma mensagem ao buffer de envio.

        Parâmetros:
            msg (bytes): Mensagem a ser adicionada ao buffer.
        """
        assert isinstance(msg, bytes), "A mensagem deve ser do tipo bytes.!"

        self.send_buff.append(msg)  # Adiciona a mensagem ao buffer
    def commit_and_send(self, msg: bytes = b'') -> None:
        """
        Descrição:
            Adiciona uma mensagem ao buffer e envia todas as mensagens acumuladas
            imediatamente.

        Parâmetros:
            msg (bytes): Mensagem a ser adicionada e enviada (por padrão, mensagem vazia).
        """
        self.commit(msg)
        self.send()

    def clear_buffer(self) -> None:
        """
        Descrição:
            Remove todas as mensagens pendentes no buffer de envio (`self.send_buff`),
            garantindo que o buffer fique vazio para novas mensagens.
        """
        self.send_buff = []  # Remove todas as mensagens acumuladas no buffer

    def commit_announcement(self, msg: bytes) -> None:
        """
        Descrição:
            Prepara e adiciona ao buffer uma mensagem de anúncio para todos os jogadores em campo.

        Restrições e observações:
            - Mensagem deve ter no máximo 20 caracteres.
            - Apenas caracteres ASCII entre 0x20 e 0x7E são permitidos, exceto ' ', '(', ')'.
            - Caracteres aceitos: letras, números e símbolos: !"#$%&'*+,-./:;<=>?@[\]^_`{|}~
            - Alcance da mensagem: 50 metros (campo tem 36m na diagonal, então a limitação é irrelevante).
            - Um jogador pode ouvir mensagens de um companheiro a cada 2 ciclos (0,04s).
            - Cada time possui sua própria fila de mensagens; não é possível bloquear mensagens do outro time.
            - Mensagens enviadas para si mesmo são sempre recebidas.

        Parâmetros:
            msg (bytes): Mensagem a ser anunciada, obedecendo às restrições acima.
        """
        # Valida as restrições de tamanho e tipo da mensagem
        assert len(msg) <= 20 and isinstance(msg, bytes)
        # Monta e adiciona o comando de anúncio ao buffer de envio
        self.commit(b'(say ' + msg + b')')

    def commit_pass_command(self) -> None:
        """
        Descrição:
            Prepara e adiciona ao buffer o comando de passe para o agente.

        Condições para que o passe seja executado corretamente:
            - O modo de jogo atual deve ser PlayOn.
            - O agente deve estar próximo da bola (distância padrão: 0,5m).
            - Não deve haver oponentes próximos à bola (distância padrão: 1m).
            - A bola deve estar praticamente parada (velocidade < 0,05m/s).
            - Deve haver um intervalo mínimo entre comandos de passe consecutivos.

        Observação:
            Este método apenas adiciona o comando ao buffer; o envio real depende de outros métodos.
        """
        # Adiciona o comando de passe ao buffer de envio
        self.commit(b'(pass)')

    def commit_beam(self, pos2d: list | tuple, rot) -> None:
        """
        Descrição:
            Envia o comando oficial 'beam' para reposicionar e rotacionar o agente no campo durante o jogo.

            Este comando pode ser usado em partidas oficiais (se permitido pela configuração do servidor).
            O reposicionamento pode estar sujeito à adição de ruído, dependendo do servidor.

        Parâmetros:
            pos2d: array_like
                Posição absoluta 2D [x, y] para o agente (X negativo sempre corresponde ao nosso lado do campo, independentemente do lado atual).
            rot: int ou float
                Ângulo do agente em graus (0 aponta para frente).
        """
        # Garante que a posição é 2D, conforme exigido pelo comando oficial
        assert len(pos2d) == 2, "The official beam command accepts only 2D positions!"
        # Monta e envia o comando no formato esperado pelo servidor
        self.commit(f"(beam {pos2d[0]} {pos2d[1]} {rot})".encode())

    # Os comandos a seguir devem ser usados apenas em ambiente de desenvolvimento.

    def unofficial_beam(self, pos3d: list | tuple, rot) -> None:
        """
        Descrição:
            Envia o comando 'beam' não-oficial para reposicionar e rotacionar o agente em qualquer posição 3D.

            Este comando não é permitido em partidas oficiais, sendo destinado apenas para testes ou cenários específicos.
            Permite reposicionamento em 3D e aceita qualquer valor de ângulo, sem necessidade de normalização.

        Parâmetros:
            pos3d: array_like
                Posição absoluta 3D [x, y, z] para o agente (X negativo sempre corresponde ao nosso lado do campo, independentemente do lado atual).
            rot: int ou float
                Ângulo do agente em graus (0 aponta para frente).
        """
        # Garante que a posição é 3D, conforme exigido pelo comando não-oficial
        assert len(pos3d) == 3, "The unofficial beam command accepts only 3D positions!"

        # Para times do lado esquerdo, envia a mensagem com ajuste de rotação para -90°
        # Para times do lado direito, inverte as posições X e Y e ajusta a rotação para +90°
        if self.world.team_side_is_left:
            msg = f"{self._unofficial_beam_msg_left}{pos3d[0]} {pos3d[1]} {pos3d[2]} {rot - 90}))".encode()
        else:
            msg = f"{self._unofficial_beam_msg_right}{-pos3d[0]} {-pos3d[1]} {pos3d[2]} {rot + 90}))".encode()

        # Envia a mensagem para o socket de monitoramento com o tamanho da mensagem nos primeiros 4 bytes
        self.monitor_socket.send(len(msg).to_bytes(4, byteorder='big') + msg)

    def unofficial_kill_sim(self) -> None:
        """
        Descrição:
            Envia um comando não-oficial para finalizar ("matar") imediatamente o simulador.

            Este comando só deve ser utilizado em ambientes de teste e não é permitido em partidas oficiais.
            O comando é enviado diretamente ao monitor_socket com o protocolo esperado (tamanho da mensagem nos primeiros 4 bytes).

            - Use este comando com cautela, pois encerra a simulação abruptamente.
        """
        msg = b'(killsim)'
        # Envia ao monitor_socket o comando de kill, precedido pelo tamanho (4 bytes)
        self.monitor_socket.send(len(msg).to_bytes(4, byteorder='big') + msg)

    def unofficial_move_ball(self, pos3d: list | tuple, vel3d=(0, 0, 0)) -> None:
        """
        Descrição:
            Comando não-oficial para reposicionar e definir a velocidade da bola no campo.

            Pode ser usado apenas em ambientes de teste ou monitoramento, não em partidas oficiais.
            O raio da bola é 0.042m.

        Parâmetros:
            pos3d: array_like
                Posição absoluta 3D [x, y, z] da bola (X negativo sempre corresponde ao nosso lado do campo, independentemente do lado atual).
            vel3d: array_like, opcional
                Velocidade absoluta 3D [vx, vy, vz] da bola (padrão: (0,0,0)).
        """
        # Garante que tanto a posição quanto a velocidade possuem 3 componentes
        assert len(pos3d) == 3 and len(vel3d) == 3, "To move the ball we need a 3D position and velocity"

        # Monta o comando de acordo com o lado do time
        if self.world.team_side_is_left:
            # Para o time da esquerda, usa as coordenadas como estão
            msg = f"(ball (pos {pos3d[0]} {pos3d[1]} {pos3d[2]}) (vel {vel3d[0]} {vel3d[1]} {vel3d[2]}))".encode()
        else:
            # Para o time da direita, inverte X e Y para manter a referência correta
            msg = f"(ball (pos {-pos3d[0]} {-pos3d[1]} {pos3d[2]}) (vel {-vel3d[0]} {-vel3d[1]} {vel3d[2]}))".encode()

        # Envia ao monitor_socket o comando de movimentação da bola, precedido pelo tamanho (4 bytes)
        self.monitor_socket.send(len(msg).to_bytes(4, byteorder='big') + msg)

    def unofficial_set_game_time(self, time_in_s: float) -> None:
        """
        Descrição:
            Envia um comando não-oficial para ajustar o tempo do jogo.

            Esse comando é usado para manipular o tempo da simulação em ambientes
            de teste ou monitoramento.

        Parâmetros:
            time_in_s : float
                Novo valor para o tempo de jogo, em segundos.

        Exemplo:
            unofficial_set_game_time(68.78)
        """
        # Monta o comando de ajuste de tempo
        msg = f"(time {time_in_s})".encode()
        # Envia ao monitor_socket o comando de tempo, precedido pelo tamanho (4 bytes)
        self.monitor_socket.send(len(msg).to_bytes(4, byteorder='big') + msg)

    def unofficial_set_play_mode(self, play_mode: str) -> None:
        """
        Descrição:
            Envia um comando não-oficial para alterar o modo de jogo (play mode) do simulador.

            Este comando é útil para testes, replays e manipulação direta do estado do jogo em ambiente de simulação.
            Não é permitido em partidas oficiais.

        Parâmetros:
            play_mode : str
                Novo modo de jogo a ser ativado (ex: "PlayOn", "KickOff_Left", "Goal_Right", etc.).

        Exemplo:
            self.unofficial_set_play_mode("PlayOn")
        """
        # Monta a mensagem de modo de jogo e converte para bytes
        msg = f"(playMode {play_mode})".encode()
        # Envia o tamanho da mensagem (4 bytes) seguido da mensagem para o monitor_socket
        self.monitor_socket.send(len(msg).to_bytes(4, byteorder='big') + msg)

    def unofficial_kill_player(self, unum: int, team_side_is_left: bool) -> None:
        """
        Descrição:
            Envia um comando não-oficial para remover ("matar") um jogador específico do simulador.

            Este comando é útil em testes para simular falhas, substituições, ou cenários especiais. Não é permitido em partidas oficiais.

        Parâmetros:
            unum: int
                Número da camisa (uniforme) do jogador a ser removido.
            team_side_is_left : bool
                Indica se o jogador pertence ao time da esquerda (True) ou da direita (False).

        Exemplo:
            # Remove o jogador 10 do time da esquerda
            self.unofficial_kill_player(10, True)
        """
        # Monta a mensagem indicando o jogador e o time a ser removido
        msg = f"(kill (unum {unum}) (team {'Left' if team_side_is_left else 'Right'}))".encode()
        # Envia o tamanho da mensagem (4 bytes) seguido da mensagem para o monitor_socket
        self.monitor_socket.send(len(msg).to_bytes(4, byteorder='big') + msg)

    def close(self, close_monitor_socket: bool = False) -> None:
        """
        Descrição:
            Fecha o socket do agente e, opcionalmente, o socket de monitoramento compartilhado.

        Parâmetros:
            close_monitor_socket : bool, opcional
                Se True, fecha também o monitor_socket compartilhado entre agentes rodando na mesma thread.
                (Padrão: False)

        Observação:
            O monitor_socket é fechado apenas se existir e se for solicitado explicitamente.
        """
        # Fecha o socket principal do agente
        self.socket.close()
        # Fecha o monitor_socket compartilhado, se solicitado e se existir
        if close_monitor_socket and ServerComm.monitor_socket is not None:
            ServerComm.monitor_socket.close()
            ServerComm.monitor_socket = None

