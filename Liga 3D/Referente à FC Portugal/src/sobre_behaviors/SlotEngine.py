from math_ops.GeneralMath import GeneralMath
from os import listdir
from os.path import isfile, join
from world.World import World
import numpy as np
import xml.etree.ElementTree as xmlp


class SlotEngine:

    def __init__(self, world: World) -> None:
        """
        Descrição:
            Inicializa o gerenciador de comportamentos de slot para o robô, carregando sequências de movimentos (slots)
            a partir de arquivos XML específicos do robô e de uso comum.

            - Realiza a leitura de arquivos XML de slots presentes em diretórios comuns e específicos do tipo de robô.
            - Cada arquivo XML pode conter várias etapas ("slots"), cada uma descrevendo índices e ângulos das juntas e o tempo de execução.
            - Garante unicidade dos nomes dos comportamentos; falha caso encontre duplicatas.
            - Os diretórios são montados conforme o tipo de robô fornecido em world.robot.type.
            - Atributos adicionais (descrição, auto_head) são lidos dos atributos do nó raiz XML quando presentes.


        Parâmetros:
            world: World
                Instância do objeto World contendo informações sobre o robô e o ambiente.

        Atributos Definidos
            world: World
                Referência ao ambiente/mundo do robô.
            state_slot_number : int
                Índice do slot atualmente em execução.
            state_slot_start_time : float
                Momento em que o slot atual foi iniciado (em ms).
            state_slot_start_angles : list ou None
                Ângulos das juntas no início do slot (usado para interpolação ou controle fino).
            state_init_zero : bool
                Indica se o estado inicial da máquina de slots é a pose neutra (Zero).
            behaviors: dict
                Dicionário de comportamentos de slot, cada chave é o nome do comportamento
                e cada valor é uma lista de tuplas (delta_ms, indices, angles) para cada etapa do slot.
            descriptions: dict
                Dicionário de descrições dos comportamentos, indexado pelo nome do comportamento.
            auto_head_flags : dict
                Dicionário indicando se a cabeça deve ser controlada automaticamente para cada comportamento.

        Exemplo de uso:
            sm = SlotEngine(world)
            sm.behaviors["Levantar"]
        """

        self.state_slot_start_time_ms = None
        self.world = world
        self.state_slot_number = 0
        self.state_slot_start_time = 0
        self.state_slot_start_angles = None
        self.state_init_zero = True

        # Obtém o diretório ativo para comportamentos de slot
        dir_slots = GeneralMath.obter_diretorio_ativo("/sobre_behaviors/slot/")

        # Carrega arquivos XML comuns a todos os robôs
        common_dir = f"{dir_slots}common/"
        files = [(f, join(common_dir, f)) for f in listdir(common_dir) if isfile(join(common_dir, f)) and f.endswith(".xml")]

        # Carrega arquivos XML específicos do tipo de robô
        robot_dir = f"{dir_slots}r{world.robot.type}"
        files += [(f, join(robot_dir, f)) for f in listdir(robot_dir) if isfile(join(robot_dir, f)) and f.endswith(".xml")]

        self.behaviors = dict()  # Dicionário de comportamentos por nome
        self.descriptions = dict()  # Descrição textual de cada comportamento
        self.auto_head_flags = dict()  # Flag para orientação automática da cabeça

        for fname, file in files:
            robot_xml_root = xmlp.parse(file).getroot()
            slots = []
            bname = fname[:-4]  # Remove extensão ".xml"

            # Para cada etapa (slot) definida no XML
            for xml_slot in robot_xml_root:
                assert xml_slot.tag == 'slot', f"Elemento XML inesperado em slot behavior {fname}: '{xml_slot.tag}'"
                indices, angles = [], []

                # Lê as ações do slot (índices das juntas e ângulos alvo)
                for action in xml_slot:
                    indices.append(int(action.attrib['id']))
                    angles.append(float(action.attrib['angle']))

                # Tempo de execução da etapa (slot), convertido para ms
                delta_ms = float(xml_slot.attrib['delta']) * 1000
                assert delta_ms > 0, f"Delta inválido <=0 encontrado em Slot Behavior {fname}"
                slots.append((delta_ms, indices, angles))

            # Garante não haver duplicidade de nomes de comportamento
            assert bname not in self.behaviors, f"Encontrados pelo menos 2 slot behaviors com o mesmo nome: {fname}"

            # Lê descrição e flag de auto_head dos atributos do nó raiz, se existirem
            self.descriptions[bname] = robot_xml_root.attrib["description"] if "description" in robot_xml_root.attrib else bname
            self.auto_head_flags[bname] = (robot_xml_root.attrib["auto_head"] == "1")
            self.behaviors[bname] = slots

    def get_behaviors_callbacks(self) -> dict:
        """
        Descrição:
            Retorna callbacks para cada comportamento de slot (slot behavior),
            permitindo a execução modular de sequências de movimentos.

        Retorno:
            dict
                Dicionário onde cada chave é o nome do slot behavior e cada valor é uma tupla contendo:
                    - Descrição do comportamento (str)
                    - Flag para controle automático da cabeça (bool)
                    - Callback de execução (função: recebe reset e executa o slot)
                    - Callback de prontidão (função: verifica se o slot está pronto para execução)
        """
        # Cria callbacks para cada slot behavior, capturando corretamente o nome de cada slot
        return {
            key: (
                self.descriptions[key],
                self.auto_head_flags[key],
                lambda reset, key_=key: self.execute(key_, reset),  # Callback de execução
                lambda key_=key: self.is_ready(key_)  # Callback de prontidão
            )
            for key in self.behaviors
        }

    @staticmethod
    def is_ready(name: str) -> bool:
        """
        Descrição:
            Verifica se o comportamento de slot está pronto para ser executado.

        Parâmetros
            name: str
                Nome do comportamento (slot behavior).

        Retorno:
            bool
                Sempre retorna True (pode ser expandido futuramente para lógica real de prontidão).
        """
        # Implementação padrão: sempre pronto. (Personalizar conforme necessidade)
        return True

    def reset(self, name):
        """
        Descrição:
            Inicializa ou reseta o comportamento de slot nomeado, preparando estado interno para nova execução.

        Parâmetros:
            name: str
                Nome do comportamento de slot a ser resetado.

        Retorno:
            - Reseta o número do slot para 0.
            - Atualiza o tempo de início do slot para o tempo local atual.
            - Salva a posição atual das juntas como referência para a nova execução.
            - Valida que o nome do comportamento existe nos comportamentos carregados.
        """
        # Reseta o índice do slot e o tempo de início
        self.state_slot_number = 0
        self.state_slot_start_time_ms = self.world.time_local_ms
        # Salva a posição atual das juntas como referência
        self.state_slot_start_angles = np.copy(self.world.robot.joints_position)
        # Garante que o slot solicitado existe
        assert name in self.behaviors, f"Slot Behavior requisitado não existe: {name}"

    def execute(self, name: str, reset: bool) -> bool:
        """
        Descrição:
            Executa um passo da sequência de um comportamento de slot (slot behavior), interpolando os ângulos das juntas
            ao longo do tempo definido para cada etapa (slot). Permite reinicialização do comportamento.

            - Realiza interpolação linear dos ângulos das juntas entre o início do slot e o valor alvo, respeitando o tempo (delta_ms) definido.
            - Garante que ao término de todos os slots o método retorna True, indicando que o comportamento terminou.
            - Reseta o comportamento caso 'reset' seja True.
            - Inclui proteção contra execução redundante após término da sequência.
        Parâmetros:
            name: str
                Nome do comportamento de slot a ser executado (deve estar presente em self.behaviors).
            reset: bool
                Se True, reinicializa o estado interno para o início do comportamento antes de executar o passo.

        Retorno:
            bool
                Retorna True se o comportamento foi concluído (último slot executado e tempo esgotado),
                ou False caso o comportamento ainda esteja em andamento.
        """

        # Se solicitado, reinicializa o comportamento
        if reset:
            self.reset(name)

        # Calcula o tempo decorrido desde o início do slot atual
        elapsed_ms = self.world.time_local_ms - self.state_slot_start_time_ms
        delta_ms, indices, angles = self.behaviors[name][self.state_slot_number]

        # Checa se é hora de avançar para o próximo slot
        if elapsed_ms >= delta_ms:
            # Atualiza os ângulos de início para as juntas deste slot, garantindo continuidade suave
            self.state_slot_start_angles[indices] = angles

            # Proteção para dois cenários raros:
            # 1 - A função é chamada após o comportamento terminar e reset==False.
            # 2 - Estamos no último slot e perdemos o último passo (ex: syncmode não está ativo).
            if self.state_slot_number + 1 == len(self.behaviors[name]):
                # Retorna True indicando que o comportamento terminou; só reinicia se reset for enviado
                return True

            # Avança para o próximo slot e reinicializa contadores
            self.state_slot_number += 1
            elapsed_ms = 0
            self.state_slot_start_time_ms = self.world.time_local_ms
            delta_ms, indices, angles = self.behaviors[name][self.state_slot_number]

        # Executa interpolação linear dos ângulos das juntas para esta etapa
        # Usa (elapsed_ms + 20) para considerar discretização/frequência do ciclo de controle (~20ms)
        progress = (elapsed_ms + 20) / delta_ms
        target = (angles - self.state_slot_start_angles[indices]) * progress + self.state_slot_start_angles[indices]

        # Envia comando para os motores/juntas do robô
        self.world.robot.set_joints_target_position_direct(indices, target, False)

        # Retorna True se a sequência está concluída (última etapa e tempo esgotado)
        return bool(elapsed_ms + 20 >= delta_ms and self.state_slot_number + 1 == len(self.behaviors[name]))
