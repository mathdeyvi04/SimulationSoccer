from math_ops.GeneralMath import GeneralMath
from world.Robot import Robot
from world.World import World
from typing import Callable
import numpy as np
import math


class WorldParser:
    """
    Descrição:
        Analisa o estado atual do mundo a partir de mensagens ou de um modelo de mundo já criado.
        Sendo responsável por:
            - Armazenar o modelo do mundo vigente.
            - Realizar o mapeamento de flags para suas posições, tanto para o time que joga à esquerda quanto para o que joga à direita.
            - Realizar o mapeamento de diferentes modo de jogo para um identificador inteiro, conforme o modelo de mundo.

    Métodos Disponíveis:
        - find_non_digit
        - find_char
        - read_float
        - read_int
        - read_bytes
        - read_str
        - get_next_tag
        - parse




    """

    def __init__(self, world: World, hear_callback: Callable) -> None:
        """
        Descrição:
            Construtor da classe WorldParser.

        Parâmetros:
            world: World
                Uma instância que representa o modelo vigente do mundo, que será utilizado pelo parser.
            hear_callback : Callable
                Uma função que será invocada pelo modelo para processar mensagens ou eventos que ele receber.

        Retorno:
            exp: object
                Uma expressão ou um parâmetro opcional que será utilizado pelo modelo.
            depth: object
                Profundidade ou um parâmetro opcional que ainda não é utilizado.
            LEFT_SIDE_FLAGS : dict
                Dicionário que relaciona o nome das flags às suas posições corrigidas, enquanto o time joga pelo lado esquerdo.
            RIGHT_SIDE_FLAGS : dict
                Dicionário que relaciona o nome das flags às suas posições corrigidas, enquanto o time joga pelo lado direito.
            play_mode_to_id : object
                Mapeamento opcional que relacionará o nome do modo de jogo ao seu identificador inteiro.
            LEFT_PLAY_MODE_TO_ID : dict
                Dicionário que relaciona o nome do modo de jogo para o identificador inteiro, enquanto o time joga pelo lado esquerdo.
            RIGHT_PLAY_MODE_TO_ID : dict
                Dicionário que relaciona o nome do modo de jogo para o identificador inteiro, enquanto o time joga pelo lado direito.
        """
        self.LOG_PREFIX = "WorldParser.py: "
        self.world = world
        self.hear_callback = hear_callback
        self.exp = None  # Expressão de mensagem
        self.depth = None

        # Mapping between flag names and their corrected location when playing on the left side
        self.LEFT_SIDE_FLAGS = {
            b'F2L': (-15, -10, 0),
            b'F1L': (-15, +10, 0),
            b'F2R': (+15, -10, 0),
            b'F1R': (+15, +10, 0),
            b'G2L': (-15, -1.05, 0.8),
            b'G1L': (-15, +1.05, 0.8),
            b'G2R': (+15, -1.05, 0.8),
            b'G1R': (+15, +1.05, 0.8)
        }

        # Mapping between flag names and their corrected location when playing on the right side
        self.RIGHT_SIDE_FLAGS = {
            b'F2L': (+15, +10, 0),
            b'F1L': (+15, -10, 0),
            b'F2R': (-15, +10, 0),
            b'F1R': (-15, -10, 0),
            b'G2L': (+15, +1.05, 0.8),
            b'G1L': (+15, -1.05, 0.8),
            b'G2R': (-15, +1.05, 0.8),
            b'G1R': (-15, -1.05, 0.8)
        }
        self.play_mode_to_id = None

        self.LEFT_PLAY_MODE_TO_ID = {
            "KickOff_Left": World.M_OUR_KICKOFF,
            "KickIn_Left": World.M_OUR_KICK_IN,
            "corner_kick_left": World.M_OUR_CORNER_KICK,
            "goal_kick_left": World.M_OUR_GOAL_KICK,
            "free_kick_left": World.M_OUR_FREE_KICK,
            "pass_left": World.M_OUR_PASS,
            "direct_free_kick_left": World.M_OUR_DIR_FREE_KICK,
            "Goal_Left": World.M_OUR_GOAL,
            "offside_left": World.M_OUR_OFFSIDE,
            "KickOff_Right": World.M_THEIR_KICKOFF,
            "KickIn_Right": World.M_THEIR_KICK_IN,
            "corner_kick_right": World.M_THEIR_CORNER_KICK,
            "goal_kick_right": World.M_THEIR_GOAL_KICK,
            "free_kick_right": World.M_THEIR_FREE_KICK,
            "pass_right": World.M_THEIR_PASS,
            "direct_free_kick_right": World.M_THEIR_DIR_FREE_KICK,
            "Goal_Right": World.M_THEIR_GOAL,
            "offside_right": World.M_THEIR_OFFSIDE,
            "BeforeKickOff": World.M_BEFORE_KICKOFF,
            "GameOver": World.M_GAME_OVER,
            "PlayOn": World.M_PLAY_ON
        }

        self.RIGHT_PLAY_MODE_TO_ID = {
            "KickOff_Left": World.M_THEIR_KICKOFF,
            "KickIn_Left": World.M_THEIR_KICK_IN,
            "corner_kick_left": World.M_THEIR_CORNER_KICK,
            "goal_kick_left": World.M_THEIR_GOAL_KICK,
            "free_kick_left": World.M_THEIR_FREE_KICK,
            "pass_left": World.M_THEIR_PASS,
            "direct_free_kick_left": World.M_THEIR_DIR_FREE_KICK,
            "Goal_Left": World.M_THEIR_GOAL,
            "offside_left": World.M_THEIR_OFFSIDE,
            "KickOff_Right": World.M_OUR_KICKOFF,
            "KickIn_Right": World.M_OUR_KICK_IN,
            "corner_kick_right": World.M_OUR_CORNER_KICK,
            "goal_kick_right": World.M_OUR_GOAL_KICK,
            "free_kick_right": World.M_OUR_FREE_KICK,
            "pass_right": World.M_OUR_PASS,
            "direct_free_kick_right": World.M_OUR_DIR_FREE_KICK,
            "Goal_Right": World.M_OUR_GOAL,
            "offside_right": World.M_OUR_OFFSIDE,
            "BeforeKickOff": World.M_BEFORE_KICKOFF,
            "GameOver": World.M_GAME_OVER,
            "PlayOn": World.M_PLAY_ON
        }

    def find_non_digit(self, start: int):
        """
        Descrição:
            Procura o próximo caractere na expressão self.exp a partir do índice start que não seja um dígito ('0'-'9') e nem um ponto ('.'). Retorna o índice desse caractere.

        Parâmetros:
            start (int):
                Índice inicial na expressão self.exp a partir do qual a busca será realizada.

        Retorno:
            int:
                Índice do primeiro caractere a partir de start que não seja um dígito e nem um ponto.
        """
        while True:
            if (self.exp[start] < ord('0') or self.exp[start] > ord('9')) and self.exp[start] != ord('.'):
                return start
            start += 1

    def find_char(self, start: int, char: int) -> int:
        """
        Descrição:
            Procura o próximo caractere igual ao valor char na expressão self.exp, a partir do índice start. Retorna o índice desse caractere.

        Parâmetros:
            start (int):
                Índice inicial na expressão self.exp a partir do qual a busca será realizada.
            char (int):
                Valor inteiro (ASCII) do caractere a ser procurado na expressão self.exp.

        Retorno:
            int:
                Índice do primeiro caractere igual a char encontrado a partir de start.
        """
        while True:
            if self.exp[start] == char:
                return start
            start += 1

    def read_float(self, start) -> tuple[float, int]:
        """
        Descrição:
            Lê um valor de ponto flutuante (float) da expressão self.exp a partir do índice start, convertendo a sequência de bytes correspondente. Trata especialmente valores 'nan' e erros de conversão.

        Parâmetros:
            start (int):
                Índice inicial na expressão self.exp de onde se tentará ler o número em formato float.

        Retorno:
            tuple[float, int]:
              - O valor float lido (ou float('nan') para 'nan', ou 0 em caso de erro)
              - O índice final após a leitura do float
        """
        if self.exp[start:start + 3] == b'nan':
            return float('nan'), start + 3  # handle nan values (they exist)

        # we assume the first one is a digit or minus sign
        end = self.find_non_digit(start + 1)

        try:
            retval = float(self.exp[start:end])
        except:
            self.world.log(f"{self.LOG_PREFIX}Conversão de string para float falhou: {self.exp[start:end]} em msg[{start},{end}], \nMsg: {self.exp.decode()}")
            retval = 0

        return retval, end

    def read_int(self, start: int) -> tuple[int, int]:
        """
        Descrição:
            Lê um valor inteiro da expressão self.exp a partir do índice start, convertendo a sequência de bytes correspondente.

        Parâmetros:
            start (int):
                Índice inicial na expressão self.exp de onde se tentará ler o número em formato inteiro.

        Retorno:
            tuple[int, int]:
                - O valor inteiro lido
                - O índice final após a leitura do inteiro
        """
        # we assume the first one is a digit or minus sign
        end = self.find_non_digit(start + 1)
        return int(self.exp[start:end]), end

    def read_bytes(self, start: int) -> tuple[bytes, int]:
        """
        Descrição:
            Lê uma sequência de bytes da expressão self.exp a partir do índice start até encontrar um espaço (' ') ou parêntese fechando (')'). Retorna o segmento de bytes lido e o índice final.

        Parâmetros:
            start (int):
                Índice inicial na expressão self.exp de onde se iniciará a leitura dos bytes.

        Retorno:
            tuple[bytes, int]:
                - O segmento de bytes lido
                - O índice final após a leitura
        """
        end = start
        while True:
            if self.exp[end] == ord(' ') or self.exp[end] == ord(')'):
                break
            end += 1

        return self.exp[start:end], end

    def read_str(self, start: int) -> tuple[str, int]:
        """
        Descrição:
            Lê uma sequência de bytes da expressão self.exp a partir de start
            (usando read_bytes) e a converte para string. Retorna a string e o índice final.

        Parâmetros:
            start (int):
                Índice inicial na expressão self.exp de onde se iniciará a leitura da string.

        Retorno:
            tuple[str, int]:
                Uma tupla contendo:
                  - A string lida a partir dos bytes
                  - O índice final após a leitura
        """
        b, end = self.read_bytes(start)
        return b.decode(), end

    # As tags referidas aqui se tratam dos pseudonônimos para as sequências de informações
    # Por exemplo (GS ......) -> sequência de informações de estado do jogo.
    def get_next_tag(self, start: int) -> tuple[bytes, int, int] | tuple[None, int, int]:
        """
        Descrição:
            Procura a próxima tag (delimitada por '(') na expressão self.exp a partir do índice start, monitorando o nível de profundidade dos elementos. Retorna o conteúdo da tag encontrada, o índice final e o menor nível de profundidade alcançado durante a busca.

        Parâmetros:
            start (int):
                Índice inicial na expressão self.exp de onde a busca pela próxima tag será realizada.

        Retorno:
            tuple[bytes, int, int] | tuple[None, int, int]:
                Uma tupla contendo:
                  - O conteúdo da tag (em bytes) ou None se não encontrada
                  - O índice final após a leitura
                  - O menor nível de profundidade alcançado durante a busca
        """

        min_depth = self.depth
        while True:
            if self.exp[start] == ord(")"):  # monitor xml element depth
                self.depth -= 1
                if min_depth > self.depth:
                    min_depth = self.depth
            elif self.exp[start] == ord("("):
                break
            start += 1
            if start >= len(self.exp):
                return None, start, 0

        self.depth += 1
        start += 1
        end = self.find_char(start, ord(" "))

        return self.exp[start:end], end, min_depth

    def parse(self, exp: bytes) -> None:
        """
        Descrição:
            Analisa e interpreta uma mensagem recebida, atualizando o estado interno do mundo do robô.
            Esta função realiza o parsing do conteúdo de 'exp', que contém informações sensoriais, de visão, giroscópio,
            acelerômetro, movimentação, posições dos jogadores, informações do time, placar, e outros dados relevantes para o agente.

        Parâmetros:
            exp (bytes): Mensagem recebida a ser interpretada.

        Retorno:
            - Atualiza diversos atributos de self.world, self.world.robot e outros membros relacionados ao estado do ambiente e do agente.
            - Pode registrar mensagens de log em caso de tags desconhecidas ou erros de parsing.
        """

        # Armazena a mensagem para uso por outras funções membros
        self.exp = exp

        # Reseta profundidade de parsing do XML e inicializa variáveis importantes do mundo
        self.depth = 0
        self.world.step += 1
        self.world.line_count = 0
        self.world.robot.frp = dict()
        self.world.flags_posts = dict()
        self.world.flags_corners = dict()
        self.world.vision_is_up_to_date = False
        self.world.ball_is_visible = False

        # Seta todos os dedos do pé como não tocando o solo
        self.world.robot.feet_toes_are_touching = dict.fromkeys(self.world.robot.feet_toes_are_touching, False)
        self.world.time_local_ms += World.STEPTIME_MS

        # Marca todos os jogadores como não visíveis no início do ciclo de percepção
        for p in self.world.teammates:
            p.is_visible = False
        for p in self.world.opponents:
            p.is_visible = False

        # Inicia parsing a partir do primeiro tag
        tag, end, _ = self.get_next_tag(0)

        # Percorre toda a mensagem enquanto houver conteúdo a ser processado
        while end < len(exp):

            match tag:
                case b'time':
                    # Parse dos dados de tempo
                    while True:
                        tag, end, min_depth = self.get_next_tag(end)
                        if min_depth == 0:
                            break

                        match tag:
                            case b'now':
                                # Lê o tempo atual do servidor
                                self.world.time_server, end = self.read_float(end + 1)

                                # Test server time reliability
                                # increment = self.world.time_server - last_time
                                # if increment < 0.019: print ("down",last_time,self.world.time_server)
                                # if increment > 0.021: print ("up",last_time,self.world.time_server)
                            case _:
                                self.world.log(f"{self.LOG_PREFIX}Unknown tag inside 'time': {tag} at {end}, \nMsg: {exp.decode()}")

                case b'GS':
                    # Parse das informações do estado do jogo (Game State)
                    while True:
                        tag, end, min_depth = self.get_next_tag(end)
                        if min_depth == 0:
                            break

                        match tag:
                            case b'unum':
                                _, end = self.read_int(end + 1)  # Número do agente, geralmente já conhecido
                            case b'team':
                                aux, end = self.read_str(end + 1)
                                is_left = bool(aux == "left")
                                # Atualiza lado do time se necessário
                                if self.world.team_side_is_left != is_left:
                                    self.world.team_side_is_left = is_left
                                    self.play_mode_to_id = self.LEFT_PLAY_MODE_TO_ID if is_left else self.RIGHT_PLAY_MODE_TO_ID
                                    self.world.draw.set_team_side(not is_left)
                                    self.world.team_draw.set_team_side(not is_left)
                            case b'sl':
                                # Gols marcados (score left/right)
                                if self.world.team_side_is_left:
                                    self.world.goals_scored, end = self.read_int(end + 1)
                                else:
                                    self.world.goals_conceded, end = self.read_int(end + 1)
                            case b'sr':
                                if self.world.team_side_is_left:
                                    self.world.goals_conceded, end = self.read_int(end + 1)
                                else:
                                    self.world.goals_scored, end = self.read_int(end + 1)
                            case b't':
                                self.world.time_game, end = self.read_float(end + 1)
                            case b'pm':
                                aux, end = self.read_str(end + 1)
                                if self.play_mode_to_id is not None:
                                    self.world.play_mode = self.play_mode_to_id[aux]
                            case _:
                                self.world.log(f"{self.LOG_PREFIX}Tag desconhecida dentro de 'GS': {tag} at {end}, \nMsg: {exp.decode()}")

                case b'GYR':
                    # Parse dos dados do giroscópio
                    while True:
                        tag, end, min_depth = self.get_next_tag(end)
                        if min_depth == 0:
                            break

                        """
                        O giroscópio mede a velocidade angular do torso do robô (vetor de taxa de rotação).
                        A direção do vetor de velocidade angular segue a regra da mão direita.
                        
                        Sistema de referência original:
                            X: esquerda(-) / direita(+)      Y: trás(-) / frente(+)      Z: baixo(-) / cima(+)
                        
                        Novo sistema de referência adotado:
                            X: trás(-) / frente(+)           Y: direita(-) / esquerda(+) Z: baixo(-) / cima(+)
                        
                        Observação: A conversão entre sistemas de referência é necessária para 
                        alinhar os dados do giroscópio com o referencial utilizado internamente pelo robô,
                        garantindo que os comandos de controle e as percepções sensoriais estejam consistentes.
                        """
                        match tag:
                            case b'n':
                                pass  # tag de nome/identificador (não usada no momento)
                            case b'rt':
                                # Leitura do vetor de rotação (trocando ordem e sinais para novo referencial)
                                self.world.robot.gyro[1], end = self.read_float(end + 1)
                                self.world.robot.gyro[0], end = self.read_float(end + 1)
                                self.world.robot.gyro[2], end = self.read_float(end + 1)
                                self.world.robot.gyro[1] *= -1
                            case _:
                                self.world.log(f"{self.LOG_PREFIX}Tag Desconhecida dentro de 'GYR': {tag} at {end}, \nMsg: {exp.decode()}")

                case b'ACC':
                    # Parse dos dados do acelerômetro
                    while True:
                        tag, end, min_depth = self.get_next_tag(end)
                        if min_depth == 0:
                            break

                        """
                        O acelerômetro mede a aceleração relativa à queda livre. Durante qualquer tipo de queda
                        livre, sua leitura será zero.
                        Quando o robô está em repouso em relação à superfície da Terra, o acelerômetro indica
                        uma aceleração para cima de 9,81 m/s² (no ambiente SimSpark).
                        
                        Sistema de referência original:
                            X: esquerda(-) / direita(+)
                            Y: trás(-) / frente(+)
                            Z: baixo(-) / cima(+)
                        
                        Novo sistema de referência adotado:
                            X: trás(-) / frente(+)
                            Y: direita(-) / esquerda(+)
                            Z: baixo(-) / cima(+)
                        
                        Observação: A conversão entre sistemas de referência garante que as medições do acelerômetro estejam alinhadas com o referencial interno do robô, facilitando a interpretação dos dados e a execução de algoritmos de controle e percepção.
                        """
                        match tag:
                            case b'n':
                                pass
                            case b'a':
                                self.world.robot.acc[1], end = self.read_float(end + 1)
                                self.world.robot.acc[0], end = self.read_float(end + 1)
                                self.world.robot.acc[2], end = self.read_float(end + 1)
                                self.world.robot.acc[1] *= -1
                            case _:
                                self.world.log(f"{self.LOG_PREFIX}Tag desconhecida dentro de 'ACC': {tag} at {end}, \nMsg: {exp.decode()}")

                case b'HJ':
                    # Parse das informações das juntas do robô (Head Joints)

                    # Fazemos isso para previnir um possível erro. Pois é considerado que o case b'ax' nunca
                    # virá antes do case b'n', logo joint_name e joint_index sempre estariam declarados.
                    # Entretanto, apenas Deus sabe do futuro.
                    joint_name = None
                    joint_index = 0
                    while True:
                        tag, end, min_depth = self.get_next_tag(end)
                        if min_depth == 0:
                            break

                        match tag:
                            case b'n':
                                joint_name, end = self.read_str(end + 1)
                                joint_index = Robot.MAP_PERCEPTOR_TO_INDEX[joint_name]
                            case b'ax':
                                joint_angle, end = self.read_float(end + 1)

                                # Corrige simetria para certos sensores
                                if joint_name in Robot.FIX_PERCEPTOR_SET:
                                    joint_angle = -joint_angle
                                old_angle = self.world.robot.joints_position[joint_index]
                                # Calcula a velocidade da junta
                                self.world.robot.joints_speed[joint_index] = (joint_angle - old_angle) / World.STEPTIME * math.pi / 180
                                self.world.robot.joints_position[joint_index] = joint_angle
                            case _:
                                self.world.log(f"{self.LOG_PREFIX}Tag desconhecida dentro de 'HJ': {tag} at {end}, \nMsg: {exp.decode()}")

                case b'FRP':
                    # Parse dos dados de força aplicada nos pés do robô (Foot Force Perceptor)

                    # Basta verificar o case b'HJ'
                    foot_toe_ref = None
                    while True:
                        tag, end, min_depth = self.get_next_tag(end)
                        if min_depth == 0:
                            break

                        '''
                        O sistema de referência é utilizado tanto para o ponto de contato quanto 
                        para o vetor de força aplicado nesse ponto.
                        Observação: O vetor de força é aplicado ao pé, portanto geralmente aponta para cima.

                        Sistema de referência original:
                            X: esquerda(-) / direita(+)
                            Y: trás(-) / frente(+)
                            Z: baixo(-) / cima(+)

                        Novo sistema de referência adotado:
                            X: trás(-) / frente(+)
                            Y: direita(-) / esquerda(+)
                            Z: baixo(-) / cima(+)
                        '''
                        match tag:
                            case b'n':
                                foot_toe_id, end = self.read_str(end + 1)
                                self.world.robot.frp[foot_toe_id] = foot_toe_ref = np.empty(6)
                                self.world.robot.feet_toes_last_touch[foot_toe_id] = self.world.time_local_ms
                                self.world.robot.feet_toes_are_touching[foot_toe_id] = True
                            case b'c':
                                # Coordenada do ponto de contato
                                foot_toe_ref[1], end = self.read_float(end + 1)
                                foot_toe_ref[0], end = self.read_float(end + 1)
                                foot_toe_ref[2], end = self.read_float(end + 1)
                                foot_toe_ref[1] *= -1
                            case b'f':
                                # Vetor de força
                                foot_toe_ref[4], end = self.read_float(end + 1)
                                foot_toe_ref[3], end = self.read_float(end + 1)
                                foot_toe_ref[5], end = self.read_float(end + 1)
                                foot_toe_ref[4] *= -1
                            case _:
                                self.world.log(f"{self.LOG_PREFIX}Unknown tag inside 'FRP': {tag} at {end}, \nMsg: {exp.decode()}")

                case b'See':
                    # Parse das informações de visão (percepção visual)
                    self.world.vision_is_up_to_date = True
                    self.world.vision_last_update = self.world.time_local_ms

                    is_teammate = None
                    player_id = 0
                    while True:
                        tag, end, min_depth = self.get_next_tag(end)
                        if min_depth == 0:
                            break

                        # Necessário para uso como chave em dicionário
                        tag_bytes = bytes(tag)

                        match tag:
                            # Genial demais filho
                            case b'G1R' | b'G2R' | b'G1L' | b'G2L':
                                # Flags dos gols (posts)
                                _, end, _ = self.get_next_tag(end)
                                c1, end = self.read_float(end + 1)
                                c2, end = self.read_float(end + 1)
                                c3, end = self.read_float(end + 1)
                                aux = self.LEFT_SIDE_FLAGS[tag_bytes] if self.world.team_side_is_left else self.RIGHT_SIDE_FLAGS[tag_bytes]
                                self.world.flags_posts[aux] = (c1, c2, c3)

                            case b'F1R' | b'F2R' | b'F1L' | b'F2L':
                                # Flags dos cantos do campo
                                _, end, _ = self.get_next_tag(end)
                                c1, end = self.read_float(end + 1)
                                c2, end = self.read_float(end + 1)
                                c3, end = self.read_float(end + 1)
                                aux = self.LEFT_SIDE_FLAGS[tag_bytes] if self.world.team_side_is_left else self.RIGHT_SIDE_FLAGS[tag_bytes]
                                self.world.flags_corners[aux] = (c1, c2, c3)

                            case b'B':
                                # Bola detectada
                                _, end, _ = self.get_next_tag(end)
                                self.world.ball_rel_head_sph_pos[0], end = self.read_float(end + 1)
                                self.world.ball_rel_head_sph_pos[1], end = self.read_float(end + 1)
                                self.world.ball_rel_head_sph_pos[2], end = self.read_float(end + 1)
                                self.world.ball_rel_head_cart_pos = M.deg_sph2cart(self.world.ball_rel_head_sph_pos)
                                self.world.ball_is_visible = True
                                self.world.ball_last_seen = self.world.time_local_ms

                            case b'mypos':
                                # Posição absoluta do robô (cheat)
                                self.world.robot.cheat_abs_pos[0], end = self.read_float(end + 1)
                                self.world.robot.cheat_abs_pos[1], end = self.read_float(end + 1)
                                self.world.robot.cheat_abs_pos[2], end = self.read_float(end + 1)

                            case b'myorien':
                                # Orientação absoluta do robô (cheat)
                                self.world.robot.cheat_ori, end = self.read_float(end + 1)

                            case b'ballpos':
                                # Posição absoluta da bola (cheat)
                                c1, end = self.read_float(end + 1)
                                c2, end = self.read_float(end + 1)
                                c3, end = self.read_float(end + 1)
                                self.world.ball_cheat_abs_vel[0] = (c1 - self.world.ball_cheat_abs_pos[0]) / World.VISUALSTEP
                                self.world.ball_cheat_abs_vel[1] = (c2 - self.world.ball_cheat_abs_pos[1]) / World.VISUALSTEP
                                self.world.ball_cheat_abs_vel[2] = (c3 - self.world.ball_cheat_abs_pos[2]) / World.VISUALSTEP
                                self.world.ball_cheat_abs_pos[0] = c1
                                self.world.ball_cheat_abs_pos[1] = c2
                                self.world.ball_cheat_abs_pos[2] = c3

                            case b'P':
                                # Informações de jogadores visíveis (teammates e opponents)
                                while True:
                                    previous_depth = self.depth
                                    previous_end = end
                                    tag, end, min_depth = self.get_next_tag(end)
                                    if min_depth < 2:
                                        # Se sair da profundidade do jogador, restaura estado e sai do loop
                                        end = previous_end
                                        self.depth = previous_depth
                                        break

                                    match tag:
                                        case b'team':
                                            player_team, end = self.read_str(end + 1)
                                            is_teammate = bool(player_team == self.world.team_name)
                                            # Registra nome do time oponente, se ainda não conhecido
                                            if self.world.team_name_opponent is None and not is_teammate:
                                                self.world.team_name_opponent = player_team
                                        case b'id':
                                            player_id, end = self.read_int(end + 1)
                                            player = self.world.teammates[player_id - 1] if is_teammate else self.world.opponents[player_id - 1]
                                            player.body_parts_cart_rel_pos = dict()
                                            player.is_visible = True
                                        case b'llowerarm' | b'rlowerarm' | b'lfoot' | b'rfoot' | b'head':
                                            tag_str = tag.decode()
                                            _, end, _ = self.get_next_tag(end)
                                            c1, end = self.read_float(end + 1)
                                            c2, end = self.read_float(end + 1)
                                            c3, end = self.read_float(end + 1)
                                            # Atualiza posições das partes do corpo do jogador
                                            if is_teammate:
                                                self.world.teammates[player_id - 1].body_parts_sph_rel_pos[tag_str] = (c1, c2, c3)
                                                self.world.teammates[player_id - 1].body_parts_cart_rel_pos[tag_str] = GeneralMath.spherical_deg_simspark_to_cart((c1, c2, c3))
                                            else:
                                                self.world.opponents[player_id - 1].body_parts_sph_rel_pos[tag_str] = (c1, c2, c3)
                                                self.world.opponents[player_id - 1].body_parts_cart_rel_pos[tag_str] = GeneralMath.spherical_deg_simspark_to_cart((c1, c2, c3))
                                        case _:
                                            self.world.log(f"{self.LOG_PREFIX}Tag desconhecida dentro de 'P': {tag} at {end}, \nMsg: {exp.decode()}")

                            case b'L':
                                # Linhas do campo detectadas
                                linhas = self.world.lines[self.world.line_count]
                                _, end, _ = self.get_next_tag(end)
                                linhas[0], end = self.read_float(end + 1)
                                linhas[1], end = self.read_float(end + 1)
                                linhas[2], end = self.read_float(end + 1)
                                _, end, _ = self.get_next_tag(end)
                                linhas[3], end = self.read_float(end + 1)
                                linhas[4], end = self.read_float(end + 1)
                                linhas[5], end = self.read_float(end + 1)
                                if np.isnan(linhas).any():
                                    self.world.log(f"{self.LOG_PREFIX}Received field line with NaNs {linhas}")
                                else:
                                    self.world.line_count += 1

                            case _:
                                self.world.log(f"{self.LOG_PREFIX}Tag Desconhecida dentro de 'See': {tag} at {end}, \nMsg: {exp.decode()}")

                case b'hear':
                    # Parse das mensagens de áudio (comunicação entre robôs)
                    team_name, end = self.read_str(end + 1)

                    if team_name == self.world.team_name:  # ignora mensagens de outros times
                        timestamp, end = self.read_float(end + 1)
                        if self.exp[end + 1] == ord('s'):
                            # Mensagem enviada pelo próprio robô
                            direction, end = "self", end + 5
                        else:
                            # Mensagem enviada por um companheiro
                            direction, end = self.read_float(end + 1)
                        msg, end = self.read_bytes(end + 1)
                        self.hear_callback(msg, direction, timestamp)

                    tag, end, _ = self.get_next_tag(end)

                case _:
                    # Caso encontre tag desconhecida na raiz da mensagem
                    self.world.log(f"{self.LOG_PREFIX}Tag Desconhecida na Mensagem Raiz: {tag} at {end}, \nMsg: {exp.decode()}")
                    tag, end, min_depth = self.get_next_tag(end)
