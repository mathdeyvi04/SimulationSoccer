from world.commons.OtherRobot import OtherRobot
from world.World import World
from typing import Callable
import numpy as np


class Radio:
    """
    Descrição:
        Classe utilitária para codificação e decodificação de informações
        compactadas em mensagens de rádio para comunicação entre agentes
        em ambientes simulados de futebol de robôs (RoboCup 3D).

        - Codificar a posição de um jogador ou da bola em uma string curta para enviar via rádio
        - Decodificar uma mensagem recebida para extrair informações de posição

    Limitações e convenções do simulador:
        - Limites do mapa (hardcoded):
            - Posições de companheiros/adversários (x, y) ∈ ([-16, 16], [-11, 11])
            - Posição da bola (x, y) ∈ ([-15, 15], [-10, 10])
        - Limitações conhecidas do servidor:
            - Permitido: todos caracteres ASCII de 0x20 a 0x7E, exceto ' ', '(', ')'
            - Bugs conhecidos:
                * Aspas simples (') ou duplas (") cortam a mensagem
                * Barra invertida ('\\') no final ou próxima de outra '\\' causa erro
                * Ponto-e-vírgula (';') no início da mensagem é problemático

    Constantes de codificação:
        - TP: Configurações para posição de companheiros (teammates)
        - OP: Configurações para posição de adversários (opponents)
        - BP: Configurações para posição da bola (ball)
        - SYMB: Lista de símbolos ASCII válidos para uso em mensagens de rádio
        - SLEN: Comprimento da lista de símbolos válidos
        - SYMB_TO_IDX: Mapeamento de código ordinal do caractere para índice na tabela de símbolos

    """

    # Constantes para discretização e codificação das posições de jogadores e bola
    # TP: teammates (linhas, colunas, índices do meio, escala, combinações, etc.)
    TP = 321, 221, 160, 110, 10, 10, 70941, 141882
    # OP: opponents
    OP = 201, 111, 100, 55, 6.25, 5, 22311, 44622
    # BP: ball
    BP = 301, 201, 150, 100, 10, 10, 60501

    # Caracteres válidos para mensagens de rádio
    SYMB = "!#$%&*+,-./0123456789:<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~;"
    # Comprimento da tabela de símbolos
    SLEN = len(SYMB)
    # Mapeamento de código ordinal do caractere para seu índice em SYMB
    SYMB_TO_IDX = {ord(s): i for i, s in enumerate(SYMB)}

    def __init__(self, world: World, commit_announcement: Callable) -> None:
        """
        Descrição:
            Inicializa uma instância da classe, configurando referências ao
            estado do mundo e à função de envio de anúncios.

        Parâmetros:
            world: World
                Instância do objeto World que contém referências ao agente,
                companheiros de equipe (teammates) e oponentes (opponents).
            commit_announcement: callable
                Função responsável por enviar mensagens de anúncio (broadcast)
                para outros agentes/jogadores.

        Atributos Definidos:
            - self.world: referência ao objeto World, para acesso ao estado atual do ambiente.
            - self.commit_announcement: função de envio de anúncios.
            - self.groups: tupla de grupos de jogadores, cada um contendo:

                [lista de jogadores (teammates/opponents), bool indicando se o grupo possui a bola, bool indicando se o agente faz parte do grupo]

                Grupos definidos:
                1. 2 companheiros (teammates) + 5 oponentes (opponents) (os jogadores diretamente próximos da bola) [bola presente]
                2. 7 companheiros (restante do time) [bola ausente]
                3. 2 companheiros + 6 oponentes (demais jogadores agrupados) [bola ausente]

        Observação:
            Adiciona dinamicamente ao final de cada grupo um valor booleano indicando se o
            agente faz parte daquele grupo (self in group?).
        """
        self.world = world  # Guarda a referência ao estado do ambiente
        self.commit_announcement = commit_announcement  # Função de envio de anúncios por rádio ou broadcast

        self.MIAs = None # será verificado no check_brodcast

        # Referências locais para facilitar o acesso
        r = world.robot  # O próprio agente
        t = world.teammates  # Lista de companheiros
        o = world.opponents  # Lista de oponentes

        # Define grupos de jogadores para diferentes contextos/estratégias
        # Cada grupo é composto por: ([lista de jogadores], possui bola?)
        self.groups = (
            [(t[9], t[10], o[6], o[7], o[8], o[9], o[10]), True],  # Grupo próximo da bola: 2 companheiros, 5 oponentes
            [(t[0], t[1], t[2], t[3], t[4], t[5], t[6]), False],  # Companheiros restantes: 7 companheiros
            [(t[7], t[8], o[0], o[1], o[2], o[3], o[4], o[5]), False]  # Restante: 2 companheiros, 6 oponentes
        )

        # Para cada grupo, adiciona um booleano indicando se o agente (self) está presente no grupo
        for g in self.groups:
            # g[0]: lista de jogadores do grupo
            # g.append(...): adiciona ao grupo o resultado da verificação se o agente está presente na lista
            g.append(any(i.is_self for i in g[0]))

    @staticmethod
    def get_player_combination(pos, is_unknown: bool, is_down: bool, info: tuple) -> tuple[int, int]:
        """
        Descrição:
            Codifica a posição de um jogador em um índice único (combinação),
            considerando condições especiais como posição desconhecida, jogador
            caído ou fora do mapa.

            - Se a posição for desconhecida, retorna uma combinação especial ("unknown").
            - Se a posição estiver fora dos limites do campo (mais de 1m além dos limites), retorna uma combinação especial ("out of bounds").
            - Se o jogador estiver caído, usa um offset para garantir unicidade das combinações nesse estado.
            - A função utiliza np.clip para garantir que posições discretizadas fiquem dentro dos limites do grid.


        Parâmetros:
            pos: tuple/list
                Posição (x, y, [z]) do jogador no campo.
            is_unknown: bool
                Indica se a posição do jogador é desconhecida. Se True, retorna a combinação correspondente a "desconhecido".
            is_down: bool
                Indica se o jogador está caído ("down"). Se True, utiliza o espaço de combinações reservado para essa condição.
            info: tuple
                Parâmetros de discretização e codificação (normalmente um dos conjuntos TP, OP, BP da classe Radio):
                    info[0] - número de linhas (grid X)
                    info[1] - número de colunas (grid Y)
                    info[2] - índice central linha
                    info[3] - índice central coluna
                    info[4] - escala para X (pixels por metro ou fator de discretização)
                    info[5] - escala para Y
                    info[6] - offset para estado "down"
                    info[7] - maior índice de combinação válido (posição "normal")

        Retorna:
            comb: int
                Índice inteiro (0-based) representando a combinação codificada para a posição e estado do jogador.
            n_comb: int
                Número total de combinações possíveis para o contexto.
        """
        if is_unknown:
            # Retorna o índice reservado para "posição desconhecida" e o número total de combinações possíveis
            return info[7] + 1, info[7] + 2

        x, y = pos[:2]

        # Se o jogador estiver mais de 1m fora dos limites do campo, retorna combinação "out of bounds"
        if x < -17 or x > 17 or y < -12 or y > 12:
            return info[7], info[7] + 2

        # Discretiza as coordenadas para índices de linha (l) e coluna (c) no grid, absorvendo pequenas saídas dos limites
        l = int(np.clip(round(info[4] * x + info[2]), 0, info[0] - 1))
        c = int(np.clip(round(info[5] * y + info[3]), 0, info[1] - 1))

        # Calcula o índice final, adicionando o offset caso o jogador esteja caído (is_down)
        return (l * info[1] + c) + (info[6] if is_down else 0), info[7] + 2

    @staticmethod
    def get_ball_combination(x: float, y: float) -> tuple[int, int]:
        """
        Descrição:
            Codifica a posição da bola em um índice único (combinação),
            discretizando as coordenadas contínuas para um grid fixo.

        Parâmetros:
            x: float
                Coordenada X da bola no campo.
            y: float
                Coordenada Y da bola no campo.

        Retorna:
            comb: int
                Índice inteiro (0-based) representando a combinação codificada para a posição da bola.
            n_comb: int
                Número total de combinações possíveis para a bola.
        """
        # Discretiza as coordenadas contínuas para índices de linha e coluna no grid do campo da bola
        # np.clip garante que valores fora do grid sejam trazidos para o limite
        linha = int(np.clip(round(Radio.BP[4] * x + Radio.BP[2]), 0, Radio.BP[0] - 1))
        coluna = int(np.clip(round(Radio.BP[5] * y + Radio.BP[3]), 0, Radio.BP[1] - 1))

        # Retorna o índice de combinação e o número total de combinações possíveis para a bola
        return linha * Radio.BP[1] + coluna, Radio.BP[6]

    @staticmethod
    def get_ball_position(comb: int) -> np.ndarray:
        """
        Descrição:
            Decodifica um índice de combinação para a posição (x, y, z) da bola no campo.

        Parâmetros:
            comb: int
                Índice inteiro representando a combinação codificada da posição da bola.

        Retorna:
            pos: np.ndarray
                Vetor numpy com as coordenadas (x, y, z) da bola (z fixo como altura da bola no solo).
        """
        # Recupera o índice de linha e coluna do grid a partir da combinação
        linha = comb // Radio.BP[1]
        coluna = comb % Radio.BP[1]

        # Converte os índices de volta para coordenadas contínuas no campo
        # Assume que a bola está no solo (z=0.042)
        return np.array([linha / Radio.BP[4] - 15, coluna / Radio.BP[5] - 10, 0.042])

    @staticmethod
    def get_player_position(comb, info) -> int | tuple[float, float, bool]:
        """
        Descrição:
            Decodifica um índice de combinação para a posição (x, y) e estado (caído ou não) de um jogador.

        Parâmetros:
            comb: int
                Índice inteiro representando a combinação codificada da posição do jogador.
            info: tuple
                Parâmetros de discretização e codificação (normalmente um dos conjuntos TP, OP, BP da classe Radio).

        Retorna:
            - -1 se está fora dos limites
            - -2 se a posição é desconhecida
            - tuple[float, float, bool]

                Coordenada X do jogador
                Coordenada Y do jogador
                True se o jogador está caído ("down"), false caso contrário.
        """
        # Checa condições especiais: jogador fora dos limites ou posição desconhecida
        if comb == info[7]:
            return -1  # Jogador está fora dos limites
        if comb == info[7] + 1:
            return -2  # Posição do jogador é desconhecida

        # Detecta se o jogador está caído a partir do offset nas combinações
        is_down = comb >= info[6]
        if is_down:
            comb -= info[6]

        # Recupera o índice de linha e coluna do grid a partir da combinação
        linha = comb // info[1]
        coluna = comb % info[1]

        # Converte os índices de volta para coordenadas contínuas do campo
        return linha / info[4] - 16, coluna / info[5] - 11, is_down

    def check_broadcast_requirements(self) -> bool:
        """
        Descrição:
            Verifica se o grupo de broadcast está apto a enviar informações,
            considerando o estado e a atualização recente dos membros do grupo.

            - O ciclo de broadcast é dividido em 9 fases (g0, g1, g2, ig0, ig1, ig2, iig0, iig1, iig2) que se repetem a cada 0.36s.
              - 'igx' significa 'grupo incompleto', onde até 1 membro pode estar ausente recentemente (MIA).
              - 'iigx' significa 'grupo muito incompleto', onde até 2 membros podem estar ausentes recentemente.
            - O objetivo é evitar que mensagens incompletas monopolizem o canal de broadcast, especialmente em jogos com muitos agentes.
            - O número máximo de jogadores "recentemente desaparecidos" (MIA) permitido depende da fase do ciclo.
            - Jogadores que nunca foram vistos/ouvidos não são considerados "recentemente desaparecidos" para o cálculo do limite, exceto em casos especiais.
            - Se houver apenas um membro no grupo desde o início, o grupo pode ser atualizado, exceto na 1ª rodada.
            - Isso impede que agentes "perdidos" monopolizem as primeiras rodadas do broadcast.


        Retorno:
            ready: bool
                True se todos os requisitos estiverem satisfeitos para o grupo atual.
                Retorna False caso algum requisito não seja satisfeito.

        Racional:
            - 1ª rodada: nenhum membro ausente -> grupo pode atualizar 3 vezes a cada 0.36s
            - 2ª rodada: até 1 ausente -> grupo pode atualizar 2 vezes a cada 0.36s
            - 3ª rodada: até 2 ausentes -> grupo pode atualizar 1 vez a cada 0.36s
            - >2 ausentes -> grupo não é atualizado
        """

        w = self.world
        r = w.robot
        ago40ms = w.time_local_ms - 40  # Limite de atualização recente (40ms atrás)
        ago370ms = w.time_local_ms - 370  # Limite para considerar um jogador "recentemente desaparecido" (até 2 MIAs, radio pode ter delay de 20ms)
        group: list[OtherRobot]

        # Calcula em qual fase do ciclo de broadcast estamos (0 a 8), baseado no tempo do servidor
        idx9 = int((w.time_server * 25) + 0.1) % 9
        max_MIA = idx9 // 3  # Número máximo de MIAs permitido nesta fase do ciclo (0, 1 ou 2)
        group_idx = idx9 % 3  # Índice do grupo a ser verificado (0, 1 ou 2)
        group, has_ball, is_self_included = self.groups[group_idx]

        # 0. Checa se grupo é válido

        # Se o grupo inclui a bola, ela precisa estar atualizada (última atualização < 40ms)
        if has_ball and w.ball_abs_pos_last_update < ago40ms:
            return False

        # Se o agente faz parte do grupo, precisa estar com a posição própria atualizada (última atualização < 40ms)
        if is_self_included and r.loc_last_update < ago40ms:
            return False

        # Identifica membros do grupo que estão "recentemente desaparecidos" (MIA)
        # MIA = jogador (que não é o próprio agente) cuja última atualização foi há mais de 370ms, mas já foi visto antes
        MIAs = [
            not ot.is_self and 0 < ot.state_last_update < ago370ms for ot in group
        ]
        # Inclui também jogadores que nunca foram vistos (state_last_update == 0) na lista self.MIAs
        self.MIAs = [ot.state_last_update == 0 or MIAs[i] for i, ot in enumerate(group)]

        # Se o número de MIAs exceder o limite para esta fase, o grupo não pode transmitir
        if sum(MIAs) > max_MIA:
            return False

        # Jogadores nunca vistos só impedem broadcast na 1ª rodada (max_MIA==0) ou se todos são MIA
        if (max_MIA == 0 and any(self.MIAs)) or all(self.MIAs):
            return False

        # Checa membros inválidos:
        # - Jogador não é o próprio, não é MIA e:
        #    - atualização é antiga (>40ms) OU
        #    - nunca foi atualizado OU
        #    - não possui posição de cabeça (state_abs_pos tem menos de 3 elementos)
        if any(
                (not ot.is_self and not self.MIAs[i] and
                 (ot.state_last_update < ago40ms or ot.state_last_update == 0 or len(ot.state_abs_pos) < 3)
                ) for i, ot in enumerate(group)
        ):
            return False

        # Se passou por todos os filtros, o grupo está pronto para broadcast
        return True

    def broadcast(self):
        """
        Descrição:
            Envia uma mensagem codificada para os companheiros de equipe, contendo o estado/posição de todas as entidades móveis do ambiente,
            desde que certas condições de broadcast estejam satisfeitas.

            1. Verifica se o grupo está apto ao broadcast (com base em atualizações recentes e número de membros disponíveis).
            2. Codifica o estado do grupo e da bola em um único valor inteiro ("combination"), usando índices compactados.
            3. Converte esse valor inteiro em uma string curta de símbolos válidos, escapando o ponto-e-vírgula (';') no início
               devido a um bug conhecido no servidor.
            4. Envia a mensagem para os companheiros de equipe através do método commit_announcement.

        Parâmetros:
            None

        Retorno:
            None, but:

            - A mensagem resultante contém informações suficientes para os agentes reconstruírem o estado do grupo e da bola.
            - O método depende de discretização consistente e do uso dos símbolos definidos em Radio.SYMB.
            - O tamanho máximo da combinação é limitado para garantir que o valor caiba na codificação de símbolos.
            - O envio só ocorre se todos os requisitos de broadcast forem atendidos (verificados por check_broadcast_requirements).
        """

        # Só transmite se todos os requisitos de broadcast estiverem atendidos
        if not self.check_broadcast_requirements():
            return

        w = self.world

        # Determina o índice do grupo do ciclo atual, baseado no tempo do servidor
        group_idx = int((w.time_server * 25) + 0.1) % 3
        group, has_ball, _ = self.groups[group_idx]

        # 1. Cria a combinação codificada

        # Inicializa a combinação com o número do grupo (garante unicidade por ciclo)
        combination = group_idx
        no_of_combinations = 3

        # Adiciona a combinação da bola, se o grupo incluir a bola
        if has_ball:
            c, n = Radio.get_ball_combination(w.ball_abs_pos[0], w.ball_abs_pos[1])
            combination += c * no_of_combinations
            no_of_combinations *= n

        # Adiciona as combinações dos membros do grupo (posição/estado de cada agente)
        for i, ot in enumerate(group):
            c, n = Radio.get_player_combination(
                ot.state_abs_pos,  # posição absoluta do agente
                self.MIAs[i], ot.state_fallen,  # se está desaparecido ou caído
                Radio.TP if ot.is_teammate else Radio.OP  # parâmetros de discretização
            )
            combination += c * no_of_combinations
            no_of_combinations *= n

        # Garante que a combinação não ultrapasse o limite máximo de codificação (por segurança)
        assert no_of_combinations < 9.61e38  # 88*89^19 - limite do sistema de símbolos

        # 2. Codifica a mensagem usando os símbolos válidos

        # O primeiro símbolo deve evitar o caractere ';' (bug do servidor)
        msg = Radio.SYMB[combination % (Radio.SLEN - 1)]
        combination //= (Radio.SLEN - 1)

        # Codifica os símbolos restantes
        while combination:
            msg += Radio.SYMB[combination % Radio.SLEN]
            combination //= Radio.SLEN

        # 3. Envia a mensagem para os companheiros de equipe

        self.commit_announcement(msg.encode())  # Envia a mensagem codificada

    def receive(self, msg: bytearray):
        """
        Descrição:
            Decodifica uma mensagem de broadcast recebida via rádio, atualizando o estado do mundo (world)
            com as posições e estados mais recentes da bola e dos jogadores do grupo correspondente.

            1. Decodifica a mensagem em um inteiro de combinação, usando os símbolos definidos em Radio.SYMB.
            2. Extrai o número da mensagem (ID do grupo) e separa as combinações para bola e jogadores.
            3. Atualiza o estado da bola e dos jogadores do grupo no objeto world, desde que certos critérios temporais sejam respeitados.
               - A bola só é atualizada se não foi vista recentemente.
               - O próprio agente (self) só é atualizado caso esteja "perdido" há mais de 110 ms.
               - Outros agentes só são atualizados via rádio se não forem vistos recentemente (há mais de 40 ms).

        Parâmetros:
            msg: bytearray
                Mensagem codificada recebida, contendo informações compactadas sobre a bola e jogadores do grupo.

        Retorno:
            - Usa discretização consistente para decodificar as posições.
            - Atualiza velocidades quando possível, exceto se mudanças abruptas sugerirem teletransporte (beam).
            - Não atualiza a si próprio se o sistema de localização visual for mais confiável.
            - O campo state_filtered_velocity do agente é corrigido para desacelerar o decaimento quando há nova informação.
        """

        w = self.world
        r = w.robot
        ago40ms = w.time_local_ms - 40
        ago110ms = w.time_local_ms - 110
        msg_time = w.time_local_ms - 20  # Mensagem provavelmente enviada há 20 ms (1 ciclo atrás)

        # 1. Decodifica a combinação

        # O primeiro símbolo nunca é ';' (bug do servidor),
        # então usa Radio.SLEN-1 para o primeiro dígito
        combination = Radio.SYMB_TO_IDX[msg[0]]
        total_combinations = Radio.SLEN - 1

        # Decodifica os demais símbolos (base Radio.SLEN)
        if len(msg) > 1:
            for m in msg[1:]:
                combination += total_combinations * Radio.SYMB_TO_IDX[m]
                total_combinations *= Radio.SLEN

        # 2. Extrai o ID da mensagem (grupo)

        message_no = combination % 3
        combination //= 3
        group, has_ball, _ = self.groups[message_no]

        # 3. Extrai as informações da bola e dos jogadores

        # Se o grupo inclui a bola, extrai a combinação da bola
        ball_comb = 0  # Apenas para previnirmos erros
        if has_ball:
            ball_comb = combination % Radio.BP[6]
            combination //= Radio.BP[6]

        players_combs = []
        for ot in group:
            info = Radio.TP if ot.is_teammate else Radio.OP
            players_combs.append(combination % (info[7] + 2))
            combination //= info[7] + 2

        # 4. Atualiza o estado do mundo

        # Atualiza a bola se não foi vista recentemente
        if has_ball and w.ball_abs_pos_last_update < ago40ms:
            time_diff = (msg_time - w.ball_abs_pos_last_update) / 1000
            ball = Radio.get_ball_position(ball_comb)
            w.ball_abs_vel = (ball - w.ball_abs_pos) / time_diff
            w.ball_abs_speed = np.linalg.norm(w.ball_abs_vel)
            w.ball_abs_pos_last_update = msg_time  # (erro pode ser de 0 a 40 ms)
            w.ball_abs_pos = ball
            w.is_ball_abs_pos_from_vision = False

        # Atualiza cada membro do grupo conforme regras de prioridade e confiança
        for c, ot in zip(players_combs, group):
            # Caso seja o próprio agente (self), só atualiza se não viu a si mesmo recentemente
            if ot.is_self:
                # O mecanismo de self-localization é mais preciso que a informação vinda pelo rádio
                if r.loc_last_update < ago110ms:  # "Perdido" há mais de 2 ciclos visuais
                    data = Radio.get_player_position(c, Radio.TP)
                    if isinstance(data, tuple):
                        x, y, is_down = data
                        r.loc_head_position[:2] = x, y  # Mantém z inalterado
                        r.loc_head_position_last_update = msg_time
                        r.radio_fallen_state = is_down
                        r.radio_last_update = msg_time
                continue

            # Não atualiza se o agente foi recentemente visto (prioriza visão direta)
            if ot.state_last_update >= ago40ms:
                continue

            info = Radio.TP if ot.is_teammate else Radio.OP
            data = Radio.get_player_position(c, info)

            if isinstance(data, tuple):
                x, y, is_down = data
                p = np.array([x, y])

                # Atualiza a velocidade filtrada se já havia posição anterior
                if ot.state_abs_pos is not None:
                    time_diff = (msg_time - ot.state_last_update) / 1000
                    velocity = np.append((p - ot.state_abs_pos[:2]) / time_diff, 0)  # v.z = 0
                    vel_diff = velocity - ot.state_filtered_velocity
                    if np.linalg.norm(vel_diff) < 4:  # Mudança brusca sugere beam, então ignora
                        ot.state_filtered_velocity /= (ot.vel_decay, ot.vel_decay, 1)  # Neutraliza decaimento (exceto z)
                        ot.state_filtered_velocity += ot.vel_filter * vel_diff

                ot.state_fallen = is_down
                ot.state_last_update = msg_time
                ot.state_body_parts_abs_pos = {"head": p}
                ot.state_abs_pos = p
                ot.state_horizontal_dist = np.linalg.norm(p - r.loc_head_position[:2])
                ot.state_ground_area = (p, 0.3 if is_down else 0.2)  # Estimativa grosseira do espaço ocupado
