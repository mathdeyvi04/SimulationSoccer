# tilizado para comunicação entre computadores por meio de redes.
# Ela fornece uma interface para criar conexões de rede usando o protocolo TCP/IP
# (e outros), permitindo que programas Python atuem como clientes ou servidores na
# comunicação entre máquinas.
import socket
from math_ops.GeneralMath import GeneralMath
import numpy as np


class Draw:
    """
    Descrição:
        Semelhante à classe RobovizDraw construída em /sobre/ambientacao/,
        criamos métodos que permitirão o envio de buffers específicos para o
        software RobovizDraw.


    Métodos Disponíveis:
        - set_team_side
        - _send
        - line
        - point
        - circle
        - polygon
        - annotation
        - arrow
        - flush
        - clear
        - clear_player
        - clear_all

    Classes Disponíveis:
        - Color: organiza e possibilita a criação de cores em bytes.
    """

    _socket = None

    def __init__(self, is_enabled: bool, unum: int, host: str, port: int) -> None:
        """
        Descrição:
            Construtor da classe.
            Inicializa o envio de mensagens de desenho pelo socket UDP, se habilitado.

        Parâmetros:
            - is_enabled (bool):

                  Define se o envio de mensagens será habilitado.
            - unum (int):

                  Número identificador do robô.
            - host (str):

                  Endereço IP ou nome do host para o qual serão enviados os pacotes UDP.
            - port (int):

                  Porta UDP usada para o envio de pacotes.

        Retorno:
            None
        """

        self.enabled = is_enabled
        self._is_team_right = None
        self._unum = unum

        # Prefixo temporário que nunca deve ser usado em circunstâncias normais
        # O encode transforma para bytes
        self._prefix = f'?{unum}_'.encode()

        # Cria um socket compartilhado para todas as instâncias
        if Draw._socket is None:
            Draw._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            Draw._socket.connect((host, port))
            Draw.clear_all()

    def set_team_side(self, is_right):
        """
        Descrição:
            Define o lado do time ao qual o robô pertence e atualiza o prefixo
            utilizado nas mensagens. Isso é importante para que o RoboViz
            não confunda diferentes robôs que compartilham o nome de buffer.

        Parâmetros:
            - is_right (bool):
                  Define se o robô é do time da direita (True) ou da esquerda (False).

        Retorno:
            None
        """
        self._is_team_right = is_right

        # Gera um identificador de jogador apropriado
        # RoboViz possui um bug/funcionalidade: ele faz a mesma substituição
        # em todos os buffers que contêm o nome. Então, para evitar essa confusão,
        # é utilizado um separador ('-') para números de dois dígitos.
        self._prefix = f"{'r' if is_right else 'l'}{'_' if self._unum < 10 else '-'}{self._unum}_".encode()

    @staticmethod
    def _send(msg, id_, flush) -> None:
        """
        Descrição:
            Método privado que envia uma mensagem para o RoboViz, se ele estiver acessível.
            Ele acrescenta o identificador junto à mensagem e controla se o envio deve ser realizado
            com um "flush" ou não. Em caso de erro de conexão, ele é silenciado.

            Exatamente como fizemos em RobovizLogger, no qual há uma função específica para tal.

        Parâmetros:
            - msg (bytes):

                  A mensagem a ser enviada pelo socket.
            - id_ (bytes):

                  O identificador que acompanha a mensagem.
            - flush (bool):

                  Se True, acrescenta um delimitador extra para forçar o envio pelo RoboViz.

        Retorno:
            Nenhum
        """
        try:
            if flush:
                Draw._socket.send(msg + id_ + b'\x00\x00\x00' + id_ + b'\x00')
            else:
                Draw._socket.send(msg + id_ + b'\x00')
        except ConnectionRefusedError:
            pass

    def circle(self, pos2d: list | np.ndarray, radius: float, thickness: float, color: bytes, id_: str, flush: bool = True) -> None:
        """
        Descrição:
            Desenha um círculo na tela do RoboViz, nas coordenadas especificadas.
            Útil para destacar posições, caminhos ou marcar um ponto de interesse.

        Parâmetros:
            - pos2d (array-like, length 2):

                  Posição (x, y) do centro do círculo, relativa ao robô.
            - radius (float):

                  Raio do círculo, em metros.
            - thickness (float):

                  Espessura da linha do círculo.
            - color (bytes):

                  A cor do círculo, especificada como um objeto `bytes`.
                  Por ex.: vermelho -> b'\xFF\x00\x00'
            - id (str):

                  Um identificador único para o círculo.
            - flush (bool):

                  Se True, faz o envio junto com um sinal de "flush".

        Retorno:
            Nenhum
        """
        if not self.enabled:
            return
        assert isinstance(color, bytes), "A cor deve ser um objeto do tipo byte, por ex.: b'\\xFF\\x00\\x00'"
        assert not np.isnan(pos2d).any(), "O parâmetro 'pos2d' contém um ou mais NaNs"

        if self._is_team_right:
            pos2d = (-pos2d[0], -pos2d[1])

        msg = b'\x01\x00' + (
            f'{f"{pos2d[0]  :.4f}":.6s}'
            f'{f"{pos2d[1]  :.4f}":.6s}'
            f'{f"{radius    :.4f}":.6s}'
            f'{f"{thickness :.4f}":.6s}').encode() + color

        Draw._send(msg, self._prefix + id_.encode(), flush)

    def line(self, p1: list | np.ndarray, p2: list | np.ndarray, thickness, color: bytes, id_: str, flush: bool = True) -> None:
        """
        Descrição:
            Desenha uma linha na tela do RoboViz, dada por dois pontos. Útil para destacar caminhos,
            vetores ou conexão entre dois pontos de interesse.

        Parâmetros:
            - p1 (array-like, length 2 ou 3):

                  Ponto de início da linha. Se possuir apenas 2 dimensões, o z será considerado 0.
            - p2 (array-like, length 2 ou 3):

                  Ponto final da linha. Se possuir apenas 2 dimensões, o z será considerado 0.
            - thickness (float):

                  Espessura da linha.
            - color (bytes):

                  A cor da linha, especificada como um objeto `bytes`.
                  Por ex.: vermelho -> b'\\xFF\\x00\\x00'
            - id (str):

                  Um identificador único para a linha.
            - flush (bool

                  Se True, faz o envio com um sinal de "flush".

        Retorno:
            Nenhum
        """

        if not self.enabled:
            return
        assert isinstance(color, bytes), "A cor deve ser um objeto do tipo byte, por ex.: b'\\xFF\\x00\\x00'"
        assert not np.isnan(p1).any(), "O parâmetro 'p1' contém um ou mais NaNs"
        assert not np.isnan(p2).any(), "O parâmetro 'p2' contém um ou mais NaNs"

        z1 = p1[2] if len(p1) == 3 else 0
        z2 = p2[2] if len(p2) == 3 else 0

        if self._is_team_right:
            p1 = (-p1[0], -p1[1], p1[2]) if len(p1) == 3 else (-p1[0], -p1[1])
            p2 = (-p2[0], -p2[1], p2[2]) if len(p2) == 3 else (-p2[0], -p2[1])

        msg = b'\x01\x01' + (
            f'{f"{p1[0]  :.4f}":.6s}'
            f'{f"{p1[1]  :.4f}":.6s}'
            f'{f"{z1     :.4f}":.6s}'
            f'{f"{p2[0]  :.4f}":.6s}'
            f'{f"{p2[1]  :.4f}":.6s}'
            f'{f"{z2     :.4f}":.6s}'
            f'{f"{thickness :.4f}":.6s}').encode() + color

        Draw._send(msg, self._prefix + id_.encode(), flush)

    def point(self, pos: list | np.ndarray, size, color: bytes, id_: str, flush: bool = True) -> None:
        """
        Descrição:
            Desenha um ponto na tela do RoboViz nas coordenadas especificadas.

        Parâmetros:
            - pos (array-like, length 2 ou 3):

                  Posição do ponto. Se possuir apenas 2 dimensões, o z será considerado 0.
            - size (float):

                  Tamanho do ponto.
            - color (bytes):

                  A cor do ponto, especificada como um objeto `bytes`.
                  Por ex.: vermelho -> b'\\xFF\\x00\\x00'
            - id (str):

                  Um identificador único para o ponto.
            - flush (bool

                  Se True, faz o envio com um sinal de "flush".

        Retorno:
            Nenhum
        """
        if not self.enabled:
            return
        assert isinstance(color, bytes), "A cor deve ser um objeto do tipo byte, por ex.: b'\\xFF\\x00\\x00'"
        assert not np.isnan(pos).any(), "O parâmetro 'pos' contém um ou mais NaNs"

        z = pos[2] if len(pos) == 3 else 0

        if self._is_team_right:
            pos = (-pos[0], -pos[1], pos[2]) if len(pos) == 3 else (-pos[0], -pos[1])

        msg = b'\x01\x02' + (
            f'{f"{pos[0]  :.4f}":.6s}'
            f'{f"{pos[1]  :.4f}":.6s}'
            f'{f"{z       :.4f}":.6s}'
            f'{f"{size      :.4f}":.6s}').encode() + color

        Draw._send(msg, self._prefix + id_.encode(), flush)

    def polygon(self, vertices: list[tuple] | np.ndarray[tuple], color: bytes, alpha: int, id_: str, flush: bool = True) -> None:
        """
        Descrição:
            Desenha um polígono nas coordenadas especificadas pelo conjunto de vértices.

        Parâmetros:
            - vertices (list de tuples ou array-like de tuples):

                  Uma sequência de vértices que formam o polígono.
                  Cada vértice deve possuir 3 dimensões (x, y, z).
            - color (bytes):

                  A cor do polígono, especificada como um objeto `bytes`.
                  Por ex.: vermelho -> b'\\xFF\\x00\\x00'
            - alpha (int):

                  Nível de opacidade do polígono, um inteiro de 0 (transparente) a 255 (opaco).
            - id (str):

                  Um identificador único para o polígono.
            - flush (bool):

                  Se True, faz o envio com um sinal de "flush".

        Retorno:
            Nenhum
        """
        if not self.enabled:
            return
        assert isinstance(color, bytes), "A cor deve ser um objeto do tipo byte, por ex.: b'\\xFF\\x00\\x00'"
        assert 0 <= alpha <= 255, "O parâmetro alpha deve estar no intervalo [0, 255]"

        if self._is_team_right:
            vertices = [(-v[0], -v[1], v[2]) for v in vertices]

        msg = b'\x01\x04' + bytes([len(vertices)]) + color + alpha.to_bytes(1, 'big')

        for v in vertices:
            msg += (
                f'{f"{v[0]  :.4f}":.6s}'
                f'{f"{v[1]  :.4f}":.6s}'
                f'{f"{v[2]  :.4f}":.6s}').encode()

        Draw._send(msg, self._prefix + id_.encode(), flush)

    def annotation(self, pos, text, color: bytes, id_: str, flush: bool = True) -> None:
        """
        Descrição:
            Desenha uma anotação textual nas coordenadas especificadas.

        Parâmetros:
            - pos (tuple ou array-like de 2 ou 3 elementos):

                  A posição onde a anotação será exibida.
                  Se possuir 2 elementos, o z é considerado 0.
            - text (str ou bytes):

                  O texto que será mostrado na anotação.
                  Se for uma `str`, ele será automaticamente codificado como `bytes`.
            - color (bytes):

                  A cor do texto da anotação, especificada como um objeto `bytes`.
                  Por ex.: vermelho -> b'\\xFF\\x00\\x00'
            - id (str):

                  Um identificador único para a anotação.
            - flush (bool):

                  Se True, faz o envio com um sinal de "flush".

        Retorno:
            Nenhum
        """
        if not self.enabled:
            return
        if not isinstance(text, bytes):
            text = str(text).encode()
        assert isinstance(color, bytes), "A cor deve ser um objeto do tipo byte, por ex.: b'\\xFF\\x00\\x00'"

        z = pos[2] if len(pos) == 3 else 0

        if self._is_team_right:
            pos = (-pos[0], -pos[1], pos[2]) if len(pos) == 3 else (-pos[0], -pos[1])

        msg = b'\x02\x00' + (
            f'{f"{pos[0]  :.4f}":.6s}'
            f'{f"{pos[1]  :.4f}":.6s}'
            f'{f"{z       :.4f}":.6s}').encode() + color + text + b'\x00'

        Draw._send(msg, self._prefix + id_.encode(), flush)

    def arrow(self, p1, p2, arrowhead_size, thickness, color: bytes, id_: str, flush: bool = True):
        """
        Descrição:
            Desenha uma seta a partir de um ponto de origem até um ponto de destino,
            com uma cabeça de seta na ponta.

        Parâmetros:
            - p1 (tuple ou array-like de 2 ou 3 elementos):

                  Ponto de origem da seta. Se possuir 2 elementos, o z é considerado 0.
            - p2 (tuple ou array-like de 2 ou 3 elementos):

                  Ponto de destino da seta. Se possuir 2 elementos, o z é considerado 0.
            - arrowhead_size (float):

                  Tamanho da cabeça da seta.
            - thickness (float):

                  Espessura das linhas que formam a seta.
            - color (bytes):

                  A cor da seta, especificada como um objeto `bytes`.
                  Por ex.: vermelho -> b'\\xFF\\x00\\x00'
            - id (str):

                  Um identificador único para a seta.
            - flush (bool):

                  Se True, faz o envio com um sinal de "flush".

        Retorno:
            Nenhum
        """
        if not self.enabled:
            return
        assert isinstance(color, bytes), "A cor deve ser um objeto do tipo byte, por ex.: b'\\xFF\\x00\\x00'"

        # Converte para 3D se os vetores estão em 2D
        if len(p1) == 2:
            p1 = M.to_3d(p1)
        else:
            p1 = np.asarray(p1)
        if len(p2) == 2:
            p2 = M.to_3d(p2)
        else:
            p2 = np.asarray(p2)

        # Cálculo do tamanho e da posição da cabeça da seta
        vec = p2 - p1
        vec_size = np.linalg.norm(vec)
        if vec_size == 0:
            return
        if arrowhead_size > vec_size:
            arrowhead_size = vec_size

        ground_proj_perpendicular = np.array([vec[1], -vec[0], 0])

        if np.all(ground_proj_perpendicular == 0):
            ground_proj_perpendicular = np.array([arrowhead_size / 2, 0, 0])
        else:
            ground_proj_perpendicular *= arrowhead_size / 2 / np.linalg.norm(ground_proj_perpendicular)

        head_start = p2 - vec * (arrowhead_size / vec_size)
        head_pt1 = head_start + ground_proj_perpendicular
        head_pt2 = head_start - ground_proj_perpendicular

        self.line(p1, p2, thickness, color, id_, False)
        self.line(p2, head_pt1, thickness, color, id_, False)
        self.line(p2, head_pt2, thickness, color, id_, flush)

    def flush(self, id_: str) -> None:
        """
        Descrição:
            Realiza o envio das alterações pendentes para um desenho específico, identificado pelo seu ID.

        Parâmetros:
            - id (str): Identificador único do desenho que deve ser atualizado.

        Retorno:
            Nenhum
        """
        if not self.enabled:
            return

        Draw._send(b'\x00\x00', self._prefix + id_.encode(), False)

    def clear(self, id_: str) -> None:
        """
        Descrição:
            Remove um desenho específico, identificado pelo seu ID, da exibição.
            Isso é realizado pelo envio de um comando que faz o buffer ser atualizado.

        Parâmetros:
            - id (str): Identificador único do desenho que deve ser removido.

        Retorno:
            Nenhum
        """
        if not self.enabled:
            return

        # Realiza a mesma saída que o clear_player
        Draw._send(b'\x00\x00', self._prefix + id_.encode(), True)

    def clear_player(self):
        """
        Descrição:
            Remove todos os desenhos criados pelo jogador atual.
            Isso é realizado pelo envio de um comando que faz o buffer ser atualizado por completo.

        Retorno:
            Nenhum
        """
        if not self.enabled:
            return

        # Realiza uma atualização que limpa todos os conteúdos
        Draw._send(b'\x00\x00', self._prefix, True)

    @staticmethod
    def clear_all():
        """ Clear all drawings of all players """
        if Draw._socket is not None:
            Draw._send(b'\x00\x00\x00\x00\x00', b'', False)  # swap buffer twice using no id

    class Color:
        """
        Descrição:
            Define uma paleta de cores RGB baseada nas cores X11.

            A classe é composta por uma série de constantes que representam cores
            na forma de bytes RGB. Os nomes das cores estão categorizados para facilitar
            a seleção e o uso, sendo uma alternativa conveniente às especificações de cor
            por tupla RGB.

            A paleta cobre uma ampla gama de tonalidades, do vermelho ao violeta,
            passando pelo laranja, amarelo, verde, ciano, azul, roxo, cinzas, marrom,
            preto e branco.

            A classe também oferece um método utilitário para criar uma cor a partir de
            um triplo RGB de inteiros (de 0 a 255).
        """
        pink_violet = b'\xC7\x15\x85'
        pink_hot = b'\xFF\x14\x93'
        pink_violet_pale = b'\xDB\x70\x93'
        pink = b'\xFF\x69\xB4'
        pink_pale = b'\xFF\xB6\xC1'

        red_dark = b'\x8B\x00\x00'
        red = b'\xFF\x00\x00'
        red_brick = b'\xB2\x22\x22'
        red_crimson = b'\xDC\x14\x3C'
        red_indian = b'\xCD\x5C\x5C'
        red_salmon = b'\xFA\x80\x72'

        orange_red = b'\xFF\x45\x00'
        orange = b'\xFF\x8C\x00'
        orange_ligth = b'\xFF\xA5\x00'

        yellow_gold = b'\xFF\xD7\x00'
        yellow = b'\xFF\xFF\x00'
        yellow_light = b'\xBD\xB7\x6B'

        brown_maroon = b'\x80\x00\x00'
        brown_dark = b'\x8B\x45\x13'
        brown = b'\xA0\x52\x2D'
        brown_gold = b'\xB8\x86\x0B'
        brown_light = b'\xCD\x85\x3F'
        brown_pale = b'\xDE\xB8\x87'

        green_dark = b'\x00\x64\x00'
        green = b'\x00\x80\x00'
        green_lime = b'\x32\xCD\x32'
        green_light = b'\x00\xFF\x00'
        green_lawn = b'\x7C\xFC\x00'
        green_pale = b'\x90\xEE\x90'

        cyan_dark = b'\x00\x80\x80'
        cyan_medium = b'\x00\xCE\xD1'
        cyan = b'\x00\xFF\xFF'
        cyan_light = b'\xAF\xEE\xEE'

        blue_dark = b'\x00\x00\x8B'
        blue = b'\x00\x00\xFF'
        blue_royal = b'\x41\x69\xE1'
        blue_medium = b'\x1E\x90\xFF'
        blue_light = b'\x00\xBF\xFF'
        blue_pale = b'\x87\xCE\xEB'

        purple_violet = b'\x94\x00\xD3'
        purple_magenta = b'\xFF\x00\xFF'
        purple_light = b'\xBA\x55\xD3'
        purple_pale = b'\xDD\xA0\xDD'

        white = b'\xFF\xFF\xFF'
        gray_10 = b'\xE6\xE6\xE6'
        gray_20 = b'\xCC\xCC\xCC'
        gray_30 = b'\xB2\xB2\xB2'
        gray_40 = b'\x99\x99\x99'
        gray_50 = b'\x80\x80\x80'
        gray_60 = b'\x66\x66\x66'
        gray_70 = b'\x4C\x4C\x4C'
        gray_80 = b'\x33\x33\x33'
        gray_90 = b'\x1A\x1A\x1A'
        black = b'\x00\x00\x00'

        @staticmethod
        def get(r, g, b) -> bytes:
            """
            Descrição:
                Obtém RGB color em bytes.
            """
            ''' Get RGB color (0-255) '''
            return bytes([int(r), int(g), int(b)])
