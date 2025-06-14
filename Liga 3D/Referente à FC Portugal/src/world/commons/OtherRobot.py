import numpy as np


class OtherRobot:
    """
    Descrição:
        Representa outro robô detectado no ambiente, com informações sobre sua identidade, visibilidade,
        posições relativas e absolutas de partes do corpo, bem como estado e velocidade filtrada.
        Essa classe é usada para rastrear jogadores visíveis, sejam companheiros de equipe ou oponentes.

        - Importante: Quando um novo robô é detectado, todas as posições anteriores de partes do corpo são descartadas.
        - Exemplo:

                 - se 5 partes foram vistas no instante 0s -> `body_parts_cart_rel_pos` terá 5 entradas.
                 - se 1 parte for vista no instante 1s -> `body_parts_cart_rel_pos` terá apenas 1 entrada.

    Parâmetros do __init__:
        - numero_da_camisa: int

              Número da camisa (uniforme) do robô, também usado como índice de identificação.
        - is_teammate: bool

              Indica se o robô pertence ao mesmo time.
    """

    def __init__(self, numero_da_camisa: int, is_teammate: bool) -> None:
        self.unum = numero_da_camisa  # Variável prática para identificar o número da camisa (igual ao índice + 1 do robô)
        self.is_self = False  # Flag prática para indicar se este objeto representa o próprio robô
        self.is_teammate = is_teammate  # Indica se o robô é um companheiro de equipe
        self.is_visible = False  # True se o robô foi visto na última mensagem do servidor (não implica localização absoluta conhecida)

        # Posições relativas (no referencial do observador)
        self.body_parts_cart_rel_pos = dict()  # Posições cartesianas relativas das partes visíveis do corpo
        self.body_parts_sph_rel_pos = dict()  # Posições esféricas relativas das partes visíveis do corpo

        # Filtros e decaimento para velocidade
        self.vel_filter = 0.3  # Coeficiente de filtro EMA aplicado à velocidade filtrada
        self.vel_decay = 0.95  # Fator de decaimento da velocidade a cada ciclo de visão (anulado se houver atualização)

        # Variáveis de estado (atualizadas quando o robô é visível e quando a autolocalização está disponível)
        self.state_fallen = False  # True se o robô está caído (atualizado quando a cabeça é visível)
        self.state_last_update = 0  # Timestamp local (`World.time_local_ms`) da última atualização do estado
        self.state_horizontal_dist = 0  # Distância horizontal da cabeça se visível; caso contrário, média da distância das partes visíveis
        # Essa distância pode ser atualizada por visão ou rádio, mesmo se o robô não estiver visível, assumindo a última posição

        self.state_abs_pos = None  # Posição 3D da cabeça se visível; senão, média 2D das partes visíveis ou posição vinda por rádio
        self.state_orientation = 0  # Orientação baseada no par de braços inferiores ou pés, ou média de ambos (pode estar desatualizada em relação a `state_last_update`)
        self.state_ground_area = None  # (ponto_2d, raio) da projeção da área do jogador no chão (círculo); não precisa ser precisa se estiver a mais de 3m; usada para evitar obstáculos, especialmente em quedas
        self.state_body_parts_abs_pos = dict()  # Posição absoluta 3D de cada parte do corpo (quando visíveis)
        self.state_filtered_velocity = np.zeros(3)  # Velocidade 3D filtrada (m/s); se a cabeça não for visível, apenas o componente 2D é atualizado e v.z decai
