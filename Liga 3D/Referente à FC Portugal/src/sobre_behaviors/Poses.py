import numpy as np
from world.World import World


class Poses:
    """
    Descrição:
        Classe responsável por gerenciar todas as juntas ou apenas um subgrupo delas.

        Pose -> ângulos em degrais para juntas específicas.

    Métodos Disponíveis:
        - get_behaviors_callbacks
        - execute
    """

    def __init__(self, world: World) -> None:
        """
        Descrição:
            Inicializa o controlador de poses do robô, armazenando o ambiente de execução e
            configurando poses pré-definidas para uso em comportamentos como postura neutra e quedas.

        Parâmetros:
            world: World
                Instância do objeto World, que contém informações sobre o robô e o ambiente.

        Atributos Definidos:
            world: World
                Referência ao ambiente do robô, utilizada para acessar propriedades do agente.
            tolerance: float
                Tolerância de erro angular (em radianos ou graus, conforme contexto), usada para considerar um comportamento como finalizado.
            poses: dict
                Dicionário de poses disponíveis. Cada chave é o nome de um comportamento/pose (string),
                e cada valor é uma tupla contendo:
                    - Descrição da pose (str)
                    - Ativação automática da cabeça (bool)
                    - Índices das juntas a serem configuradas (np.array)
                    - Valores alvo das juntas (np.array)

        Para adicionar uma nova pose:
            1. Insira uma nova entrada no dicionário `self.poses` com um nome único.
            2. Preencha a tupla conforme o padrão das demais poses.
            3. Não são necessárias outras alterações no código.

        O código também remove automaticamente as juntas dos "toes" (índices 22 e 23) caso o robô não seja do tipo 4,
        garantindo compatibilidade com diferentes modelos de hardware.
        """

        self.world = world
        self.tolerance = 0.05  # Tolerância para o erro angular ao considerar o comportamento como finalizado

        # Dicionário de poses pré-definidas.
        # Cada entrada define: (descrição, auto-head, índices das juntas, valores das juntas)
        self.poses = {
            "Zero": (
                "Pose neutra, incluindo a cabeça",  # descrição traduzida
                False,  # desabilita orientação automática da cabeça
                np.array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23]),  # índices
                np.array([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -90, -90, 0, 0, 90, 90, 0, 0, 0, 0])  # valores
            ),
            "Zero_Legacy": (
                "Pose neutra, incluindo a cabeça, cotovelos causam colisão (legado)",  # descrição traduzida
                False,
                np.array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23]),
                np.array([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -90, -90, 0, 0, 0, 0, 0, 0, 0, 0])
            ),
            "Zero_Bent_Knees": (
                "Pose neutra, incluindo a cabeça, joelhos dobrados",  # descrição traduzida
                False,
                np.array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23]),
                np.array([0, 0, 0, 0, 0, 0, 30, 30, -60, -60, 30, 30, 0, 0, -90, -90, 0, 0, 90, 90, 0, 0, 0, 0])
            ),
            "Zero_Bent_Knees_Auto_Head": (
                "Pose neutra, cabeça automática, joelhos dobrados",  # descrição traduzida
                True,
                np.array([2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23]),
                np.array([0, 0, 0, 0, 30, 30, -60, -60, 30, 30, 0, 0, -90, -90, 0, 0, 90, 90, 0, 0, 0, 0])
            ),
            "Fall_Back": (
                "Inclina os pés para cair para trás",  # descrição traduzida
                True,
                np.array([10, 11]),
                np.array([-20, -20])
            ),
            "Fall_Front": (
                "Inclina os pés para cair para frente",  # descrição traduzida
                True,
                np.array([10, 11]),
                np.array([45, 45])
            ),
            "Fall_Left": (
                "Inclina as pernas para cair para a esquerda",  # descrição traduzida
                True,
                np.array([4, 5]),
                np.array([-20, 20])
            ),
            "Fall_Right": (
                "Inclina as pernas para cair para a direita",  # descrição traduzida
                True,
                np.array([4, 5]),
                np.array([20, -20])
            ),
        }

        # Remove juntas dos "toes" (22 e 23) se o robô não for do tipo 4,
        # garantindo que as poses sejam compatíveis com outros modelos
        if world.robot.type != 4:
            for key, val in self.poses.items():
                idxs = np.where(val[2] >= 22)[0]  # Encontra índices das juntas dos toes
                if len(idxs) > 0:
                    # Remove os índices e valores correspondentes dos arrays
                    self.poses[key] = (
                        val[0], val[1],
                        np.delete(val[2], idxs),
                        np.delete(val[3], idxs)
                    )

    def get_behaviors_callbacks(self) -> dict:
        """
        Descrição:
            Retorna um dicionário de callbacks para cada comportamento de pose, utilizado internamente
            para facilitar a execução modular dos comportamentos.

        Parâmetros:
            None
        Retorno:
            dict
                Um dicionário onde cada chave é o nome do comportamento e cada valor é uma tupla contendo:
                    - Descrição da pose (str)
                    - Flag de ativação automática da cabeça (bool)
                    - Callback de execução da pose (função)
                    - Callback de condição de término (função sempre retorna True)
        """
        # Gera dicionário de callbacks para cada pose, com lambdas capturando o nome da pose
        return {
            key: (
                val[0], val[1], lambda reset, key_=key: self.execute(key_), lambda: True
            ) for key, val in self.poses.items()
        }

    def execute(self, name: str) -> bool:
        """
        Descrição:
            Executa a pose especificada pelo nome, ajustando as juntas do robô para os valores alvo definidos.

        Parâmetros:
            name : str
                Nome da pose a ser executada (deve ser uma chave válida em self.poses).

        Retorno:
            bool
                True se o comportamento terminou (movimento foi completamente executado),
                False caso contrário.
        """

        # Extrai índices e valores da pose
        _, _, indices, values = self.poses[name]

        # Comanda os motores para a pose e verifica se terminou
        remaining_steps = self.world.robot.set_joints_target_position_direct(indices, values, True, tolerance=self.tolerance)

        return bool(remaining_steps == -1)

