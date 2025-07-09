import numpy as np
import pickle
from math_ops.GeneralMath import GeneralMath
from math_ops.NeuralNetwork import run_mlp


class Fall:
    """
    Descrição:
        Classe responsável por implementar o comportamento de queda controlada para robôs humanoides.

        Utiliza um modelo de Rede Neural treinada para gerar comandos de ação que conduzem o robô a uma posição de queda desejada,
        a partir do estado atual de suas juntas e da altura da cabeça. O objetivo é simular ou treinar o robô para cair de forma segura
        ou estratégica, seja para evitar danos em situações de queda inevitável ou para fins de experimentação e aprendizado.

    Características principais:
        - Carrega automaticamente o modelo de política de ações para queda.
        - Normaliza e prepara as observações do robô (posições das juntas e altura da cabeça) para alimentar a rede neural.
        - Executa os comandos sugeridos pela política, atuando apenas nas juntas compatíveis.
        - Considera o comportamento finalizado quando a cabeça está suficientemente próxima ao solo, indicando que a queda foi completada.
    """
    def __init__(self, base_agent) -> None:
        """
        Descrição:
            Inicializa a classe responsável pelo comportamento de "queda controlada" (Fall),
            carregando o modelo de Rede Neural que define a política de ação e configurando
            os parâmetros necessários para controle dos atuadores do robô.

        Parâmetros:
            base_agent: Base_Agent
                Instância do agente base, usada para acessar o ambiente simulado, o robô e seus sensores.

        Atributos Definidos:
            world: object
                Referência ao mundo (ambiente simulado) do agente.
            description: str
                Descrição breve do comportamento.
            auto_head: bool
                Indica se o controle automático da cabeça está ativado (aqui é False).
            model: object
                Modelo de Rede Neural carregado via pickle, responsável por definir a política de ação.
            action_size: int
                Número de saídas (ações) do modelo, deduzido pelo tamanho do último vetor de bias da rede.
            obs: np.ndarray
                Vetor de observação, composto pelas posições das juntas normalizadas e a altura da cabeça.
            controllable_joints: int
                Número de juntas que podem ser controladas, garantindo compatibilidade entre diferentes tipos de robôs.
        """

        self.world = base_agent.world
        self.description = "Exemplo de Queda Controlada"
        self.auto_head = False

        # Carrega o modelo de política (Rede Neural) para o comportamento de queda
        with open(
                GeneralMath.obter_diretorio_ativo("/sobre_behaviors/custom/Fall/fall.pkl"),
                'rb'
        ) as f:
            self.model = pickle.load(f)

        # Define o número de ações com base no tamanho do último layer do modelo
        self.action_size = len(self.model[-1][0])  # tamanho do bias da última camada

        # Observação: posições das juntas + altura da cabeça
        self.obs = np.zeros(self.action_size + 1, np.float32)

        # Garante compatibilidade com diferentes tipos de robôs (nem todos têm o mesmo número de juntas)
        self.controllable_joints = min(self.world.robot.no_of_joints, self.action_size)

    def observe(self) -> None:
        """
        Descrição:
            Atualiza o vetor de observação com as informações mais recentes do robô:
            - Posições das juntas (normalizadas)
            - Altura absoluta da cabeça do robô

            - As posições das juntas são normalizadas por 100 para manter os valores em uma faixa adequada para a rede neural.
            - A última posição do vetor de observação representa a altura da cabeça, utilizada como informação de estado crítico na queda.
        """
        r = self.world.robot

        # Preenche a observação com as posições das juntas, normalizadas por 100
        for i in range(self.action_size):
            self.obs[i] = r.joints_position[i] / 100

        # A última posição guarda a altura absoluta da cabeça (pode alternar para r.loc_head_z se necessário)
        self.obs[self.action_size] = r.cheat_abs_pos[2]

    def execute(self, reset) -> bool:
        """
        Descrição:
            Executa um passo do comportamento de queda:
            - Atualiza a observação do estado atual do robô.
            - Calcula a ação ótima usando a Rede Neural carregada.
            - Envia os comandos para as juntas do robô.
            - Retorna se o estado de "queda finalizada" foi atingido.

        Parâmetros:
            reset: bool
                (Não utilizado diretamente aqui, mas pode ser usado para reinicializar estado em implementações futuras.)

        Retorno:
            bool
                True se a queda foi finalizada (altura da cabeça < 0.15m), False caso contrário.

            - A ação retornada pela rede é multiplicada por 10 para acelerar a exploração/movimentação.
            - O comando é enviado apenas para as juntas controláveis, garantindo compatibilidade.
            - O parâmetro `harmonize=False` acelera a resposta, dado que as metas de movimento mudam a cada passo.
        """
        self.observe()
        # Calcula a ação ótima com base na política do modelo RL
        action = run_mlp(self.obs, self.model)

        # Envia comando para as juntas: apenas para as controláveis, e com escala ampliada para mais movimento
        self.world.robot.set_joints_target_position_direct(
            slice(self.controllable_joints),  # aplica apenas nas juntas compatíveis
            action * 10,  # escala ações para maior efeito
            harmonize=False  # não harmoniza, pois as metas mudam a cada iteração
        )

        # Considera finalizada quando a cabeça está suficientemente próxima ao solo
        return self.world.robot.loc_head_z < 0.15
