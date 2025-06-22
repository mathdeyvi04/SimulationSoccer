from collections import deque
import numpy as np


class GetUp:
    """
    Descrição:
        Classe responsável por implementar o comportamento autônomo de levantar do chão para um robô humanoide.

        Esta classe monitora os sensores de orientação, aceleração e estabilidade do robô para decidir, de forma robusta,
        qual é a melhor estratégia de levantar (de frente, de costas ou realizando um movimento de flip) a partir de qualquer posição de queda.
        O processo é gerenciado por uma máquina de estados interna, que avança conforme o robô atinge estabilidade, executa poses intermediárias,
        e finaliza o comportamento assim que o robô está novamente de pé.

    Principais características:
        - Seleciona automaticamente a estratégia de levantar mais adequada com base nas leituras dos sensores inerciais.
        - Realiza verificações de estabilidade antes de iniciar a sequência de levantar, evitando comandos prematuros.
        - Usa subcomportamentos já definidos para executar os movimentos específicos (ex: "Get_Up_Front", "Get_Up_Back", "Flip").
        - Possui mecanismos de proteção, como o uso de watchdog, para evitar loops infinitos caso o robô esteja tremendo ou com sensores instáveis.
        - Permite fácil integração com outros sistemas de comportamento, por métodos `execute`, `reset` e `is_ready`.
    """
    def __init__(self, base_agent) -> None:
        """
        Descrição:
            Inicializa o comportamento de "levantar" (Get Up), configurando os parâmetros e referências necessárias.

        Parâmetros:
            base_agent: Base_Agent
                Instância do agente base, que fornece acesso ao mundo simulado, comportamentos e sensores do robô.

        Atributos Definidos:
            behavior: object
                Referência ao gerenciador de comportamentos do agente, usado para executar subcomportamentos.
            world: object
                Referência ao mundo simulado e ao robô.
            description: str
                Texto descritivo do comportamento.
            auto_head: bool
                Indica se o controle automático da cabeça está ativado (aqui é False).
            MIN_HEIGHT: float
                Altura mínima aceitável para considerar que a cabeça do robô está de pé.
            MAX_INCLIN: float
                Inclinação máxima do torso (em graus) antes de considerar o robô caído.
            STABILITY_THRESHOLD: int
                Quantidade de leituras do giroscópio usadas para avaliar estabilidade antes de levantar.
        """

        self.watchdog = None
        self.gyro_queue = None
        self.state = None
        self.behavior = base_agent.behavior
        self.world = base_agent.world
        self.description = "Levantar Usando os Comportamentos Apropriados"
        self.auto_head = False
        self.MIN_HEIGHT = 0.3  # altura mínima da cabeça para considerar que está em pé
        self.MAX_INCLIN = 50  # inclinação máxima do torso para considerar que está em pé
        self.STABILITY_THRESHOLD = 4  # leituras para avaliar estabilidade

    def reset(self):
        """
        Descrição:
            Reseta o estado interno do comportamento de levantar, preparando para uma nova tentativa.

        Retorno:
            - Reinicia o estado para o passo inicial.
            - Limpa a fila de leituras do giroscópio.
            - Reinicia o watchdog.
        """
        self.state = 0
        self.gyro_queue = deque(maxlen=self.STABILITY_THRESHOLD)
        self.watchdog = 0  # usado para evitar loops infinitos caso o robô "treme" e nunca estabilize

    def execute(self, reset: bool) -> bool:
        """
        Descrição:
            Executa o comportamento de levantar, progredindo por uma máquina de estados:
            0: Vai para pose 'Zero' e verifica estabilidade.
            1: Executa a sequência para levantar de frente.
            2: Executa a sequência para levantar de costas.
            3: Executa a sequência de 'Flip' (virar).

            - Usa sensores de aceleração e giroscópio para decidir qual estratégia de levantar é mais adequada.
            - Utiliza um watchdog para evitar ficar indefinidamente tentando estabilizar.
            - Chama subcomportamentos conforme necessário.


        Parâmetros:
            reset: bool
                Reinicia o comportamento caso True.

        Retorno:
            bool
                True se o robô conseguiu levantar (comportamento completo), False caso contrário.
        """

        r = self.world.robot
        execute_sub_behavior = self.behavior.execute_sub_behavior

        if reset:
            self.reset()

        if self.state == 0:  # Estado 0: Ir para pose "Zero" e aguardar estabilidade
            self.watchdog += 1
            self.gyro_queue.append(max(abs(r.gyro)))  # armazena os últimos valores de giroscópio

            # Avança se o subcomportamento terminou, estabilizou ou passou do watchdog
            if (execute_sub_behavior("Zero", None) and
                    len(self.gyro_queue) == self.STABILITY_THRESHOLD and
                    all(g < 10 for g in self.gyro_queue)) or self.watchdog > 100:

                # Decide qual sequência de levantar executar com base na aceleração (posição no chão)
                if r.acc[0] < -4 and abs(r.acc[1]) < 2 and abs(r.acc[2]) < 3:
                    execute_sub_behavior("Get_Up_Front", True)  # inicia levantar de frente
                    self.state = 1
                elif r.acc[0] > 4 and abs(r.acc[1]) < 2 and abs(r.acc[2]) < 3:
                    execute_sub_behavior("Get_Up_Back", True)  # inicia levantar de costas
                    self.state = 2
                elif r.acc[2] > 8:
                    # Caso especial: já está em pé, não precisa levantar
                    return True
                else:
                    execute_sub_behavior("Flip", True)  # inicia flip (virar)
                    self.state = 3

        elif self.state == 1:
            # Estado 1: Executando levantar de frente
            if execute_sub_behavior("Get_Up_Front", False):
                return True

        elif self.state == 2:
            # Estado 2: Executando levantar de costas
            if execute_sub_behavior("Get_Up_Back", False):
                return True

        elif self.state == 3:
            # Estado 3: Executando flip (virar)
            if execute_sub_behavior("Flip", False):
                self.reset()  # volta ao início caso o flip termine

        return False

    def is_ready(self) -> bool:
        """
        Descrição:
            Indica se o comportamento de levantar está pronto para ser executado,
            ou seja, se o robô está caído.

            - Considera o robô caído se a aceleração z é baixa, a magnitude da aceleração é alta, e a cabeça está baixa ou o torso está muito inclinado.

        Retorno:
            bool
                True se o robô está caído (pronto para levantar), False caso contrário.
        """
        r = self.world.robot
        # Checa se o robô está caído: z < 5, aceleração total > 8, cabeça baixa ou torso muito inclinado
        return (r.acc[2] < 5 and
                np.dot(r.acc, r.acc) > 64 and
                (r.loc_head_z < self.MIN_HEIGHT or r.imu_torso_inclination > self.MAX_INCLIN))
