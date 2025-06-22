import numpy as np


class Behavior:
    """
    Descrição:
        Classe gerenciadora dos comportamentos do robô.

    Métodos Disponíveis:
        - get_custom_callbacks
        - create_behaviors
        - get_custom_behavior_object
        - get_all_behaviors
        - get_current
        - get_previous
        - force_reset
        -
        -
        -
        -
        -
        -
    """

    def __init__(self, base_agent) -> None:
        """
        Descrição:
            Inicializa o gerenciador de comportamentos padrão do agente, armazenando referências ao ambiente e
            preparando módulos auxiliares fundamentais para o controle do robô, como poses, motores de slot e controle de cabeça.

            - Atribui o agente base e o ambiente (`world`) à instância para uso posterior em outros métodos.
            - Inicializa variáveis de estado para rastrear o comportamento em execução, o tempo de início e detalhes do comportamento anterior.
            - Importa e instancia os módulos auxiliares essenciais para execução de comportamentos:
                - Poses: controle de poses padronizadas do robô.
                - Slot_Engine: execução de sequências de movimentos via slots (ex: levantar, sentar).
                - Head: controle dos movimentos da cabeça (ex: olhar para bola, seguir alvo).
            - Os imports são realizados dentro do método para evitar loops de importação circular.

        Parâmetros
            base_agent: Base_Agent
                Instância do agente base, utilizada para acessar o ambiente simulado e informações do robô.
        """

        self.objects = None
        self.behaviors = None
        self.base_agent = base_agent
        self.world = self.base_agent.world  # Referência ao ambiente simulado

        # Variáveis de estado para controle dos comportamentos
        self.state_behavior_name = None  # Nome do comportamento atual
        self.state_behavior_init_ms = 0  # Tempo de início do comportamento atual (em ms)
        self.previous_behavior = None  # Nome do comportamento anterior executado
        self.previous_behavior_duration = None  # Duração do comportamento anterior (em ms)

        # Inicializa os módulos auxiliares de comportamento
        from sobre_behaviors.Poses import Poses
        from sobre_behaviors.SlotEngine import SlotEngine
        from sobre_behaviors.Head import Head

        self.poses = Poses(self.world)  # Controle de poses do robô
        self.slot_engine = SlotEngine(self.world)  # Execução de slots (sequências de movimento)
        self.head = Head(self.world)  # Controle de movimentos da cabeça

    def get_custom_callbacks(self):
        """
        Descrição:
            Retorna um dicionário de callbacks para todos os comportamentos customizados (custom behaviors)
            disponíveis no sistema. Cada comportamento deve ser adicionado manualmente à lista de classes
            para facilitar a manutenção, evitar importações dinâmicas e garantir compatibilidade na distribuição
            do código.

            - A busca automática por comportamentos customizados não é utilizada para evitar problemas
              de distribuição e importação dinâmica de código.
            - Para adicionar novos comportamentos, é necessário:
                1. Adicionar a importação da nova classe no início do método.
                2. Incluir a nova classe na lista 'classes'.
            - Cada instância de comportamento é criada e armazenada em 'self.objects'.
            - Os callbacks são construídos de forma que cada função mantém a referência correta ao objeto (via parâmetro default `o=o`).
            - O método facilita a integração modular de novos comportamentos customizados no sistema.


        Parâmetros:
            Nenhum.

        Retorno:
            dict
                Dicionário onde cada chave é o nome da classe do comportamento customizado e cada valor é uma tupla contendo:
                    - Descrição do comportamento (str)
                    - Flag para auto_head (bool)
                    - Callback de execução (função: recebe reset e argumentos adicionais)
                    - Callback de prontidão (função: verifica se o comportamento está pronto para execução)
       """

        # Declaração manual dos comportamentos customizados
        # Para adicionar um novo comportamento:
        # 1. Importe a classe abaixo.
        # 2. Adicione-a à lista 'classes'.
        from sobre_behaviors.custom.Basic_Kick.BasicKick import BasicKick
        from sobre_behaviors.custom.Dribble.Dribble import Dribble
        from sobre_behaviors.custom.Fall.Fall import Fall
        from sobre_behaviors.custom.Get_Up.GetUp import GetUp
        from sobre_behaviors.custom.Step.Step import Step
        from sobre_behaviors.custom.Walk.Walk import Walk
        classes = [BasicKick, Dribble, Fall, GetUp, Step, Walk]

        '''---- Fim das declarações manuais ----'''

        # Instancia cada comportamento customizado e armazena em 'self.objects'
        self.objects = {cls.__name__: cls(self.base_agent) for cls in classes}

        # Retorna callbacks formatados para cada comportamento
        # Cada valor da tupla inclui: descrição, flag de controle de cabeça, callback de execução, callback de prontidão
        return {
            name: (
                o.description,
                o.auto_head,
                lambda reset, *args, o_=o: o_.execute(reset, *args),  # Callback de execução do comportamento
                lambda *args, o_=o: o_.is_ready(*args)  # Callback de verificação de prontidão
            )
            for name, o in self.objects.items()
        }

    def create_behaviors(self):
        """
        Descrição:
            Cria e registra o dicionário de comportamentos disponíveis para o agente, agregando callbacks de execução
            e de verificação de prontidão a partir dos módulos de poses, slot engine e comportamentos customizados.

            - O dicionário `self.behaviors` é preenchido com comportamentos padronizados (poses), movimentos baseados em slots
              e quaisquer callbacks customizados definidos pelo usuário.
            - Cada entrada do dicionário possui:
                - Descrição textual do comportamento.
                - Flag indicando se o comando de cabeça automática deve ser usado.
                - Callback de execução do comportamento (função que recebe reset e argumentos opcionais).
                - Callback para verificar se o comportamento está pronto para uso.

        Comentários:
            - O método organiza todos os comportamentos disponíveis em um único dicionário, facilitando o acesso, chamada e automação.
            - A função `update` permite sobrepor ou adicionar comportamentos de diferentes fontes.
            - Os callbacks são funções lambda que encapsulam a lógica de execução e verificação de prontidão de cada comportamento.
            - O padrão adotado facilita a integração com máquinas de estados ou mecanismos de decisão automáticos.
        """

        # Carrega callbacks dos comportamentos de poses padrão
        self.behaviors = self.poses.get_behaviors_callbacks()
        # Adiciona/atualiza com os comportamentos de slot engine (movimentos complexos em etapas)
        self.behaviors.update(self.slot_engine.get_behaviors_callbacks())
        # Adiciona/atualiza com callbacks de comportamentos customizados definidos pelo usuário
        self.behaviors.update(self.get_custom_callbacks())

    def get_custom_behavior_object(self, name: str):
        """
        Descrição:
            Retorna a instância única do objeto de um comportamento customizado pelo nome da classe.

        Parâmetros:
            name: str
                Nome da classe do comportamento customizado (deve existir em self.objects).

        Retorno:
            object
                Instância do comportamento customizado correspondente ao nome fornecido.
        """
        # Verifica se o comportamento customizado existe
        assert name in self.objects, f"Não há comportamento customizado chamado {name}"
        # Retorna a instância do comportamento customizado
        return self.objects[name]

    def get_all_behaviors(self) -> tuple[list, list]:
        """
        Descrição:
            Retorna os nomes e descrições de todos os comportamentos registrados no gerenciador.

            - Permite listar os comportamentos disponíveis e suas descrições para fins de exibição ou seleção pelo usuário.

        Parâmetros:
            None.

        Retorno:
            tuple (list, list)
                Uma tupla contendo:
                - Lista com os nomes dos comportamentos.
                - Lista com as descrições associadas a cada comportamento.
        """
        # Retorna listas de nomes e descrições de todos os comportamentos
        return [key for key in self.behaviors], [val[0] for val in self.behaviors.values()]

    def get_current(self) -> tuple[str, float]:
        """
        Descrição:
            Retorna o nome e a duração (em segundos) do comportamento atualmente em execução.

        Parâmetros:
            Nenhum.

        Retorno:
            tuple (str, float)
                Nome do comportamento atual e sua duração em segundos.
        """
        # Calcula a duração do comportamento atual em segundos
        duration = (self.world.time_local_ms - self.state_behavior_init_ms) / 1000.0
        # Retorna nome e duração atual
        return self.state_behavior_name, duration

    def get_previous(self) -> tuple[str, float]:
        """
        Descrição:
            Retorna o nome e a duração (em segundos) do comportamento executado anteriormente.

        Parâmetros:
            Nenhum.

        Retorno:
            tuple (str, float)
                Nome do comportamento anterior e sua duração em segundos.
        """
        # Retorna nome e duração do comportamento anterior
        return self.previous_behavior, self.previous_behavior_duration

    def force_reset(self) -> None:
        """
        Descrição:
            Força o reset do próximo comportamento a ser executado, apagando o nome do comportamento atual.

            - Configurar 'state_behavior_name' como None garante que o próximo comportamento será reinicializado do zero.
            - Útil para recuperar o agente em situações de erro ou para reinicializar explicitamente o controle de comportamento.

        Parâmetros
            None

        Retorno
            None
         """
        # Limpa o nome do comportamento atual, forçando o reset na próxima execução
        self.state_behavior_name = None

    def execute(self, name: str, *args) -> bool:
        """
        Descrição:
            Executa um passo do comportamento identificado por `name`, repassando argumentos adicionais para a respectiva função de execução.
            O método gerencia a transição entre comportamentos, realizando o reset automaticamente quando um novo comportamento é iniciado e
            atualizando os estados de controle de histórico e duração. Também realiza a execução do controle de cabeça, caso o comportamento permita.

            - Caso o nome do comportamento solicitado seja diferente do atual, o método realiza o reset do estado do comportamento,
              armazena o nome e duração do comportamento anterior, e reinicializa o tempo de início do novo comportamento.
            - A execução do comportamento pode disparar o controle de cabeça automaticamente, dependendo da flag de auto_head definida.
            - O método pode ser chamado repetidamente em um loop: ele retorna False enquanto o comportamento está em andamento e True ao término.
            - Para obter o nome e a duração do comportamento atual, utilize o método `get_current()`.

        Parâmetros:
            name: str
                Nome do comportamento a ser executado (deve estar presente em self.behaviors).
            *args:
                Argumentos adicionais que serão repassados para o método de execução do comportamento.

        Retorno:
            finished: bool
                Retorna True se o comportamento foi finalizado (i.e., a função de execução retornou True),
                ou False caso o comportamento ainda esteja em andamento.

        """

        # Garante que o comportamento solicitado existe
        assert name in self.behaviors, f"Comportamento {name} não existe!"

        # Verifica se há transição entre comportamentos e realiza o reset se necessário
        reset = bool(self.state_behavior_name != name)
        if reset:
            if self.state_behavior_name is not None:
                # Armazena nome do comportamento anterior se estava em execução
                self.previous_behavior = self.state_behavior_name
            # Calcula e armazena a duração do comportamento anterior
            self.previous_behavior_duration = (self.world.time_local_ms - self.state_behavior_init_ms) / 1000.0
            # Atualiza o nome e o tempo de início do novo comportamento
            self.state_behavior_name = name
            self.state_behavior_init_ms = self.world.time_local_ms

        # Executa o controle da cabeça se o comportamento permitir (auto_head == True)
        if self.behaviors[name][1]:
            self.head.execute()

        # Executa um passo do comportamento, passando 'reset' e argumentos adicionais
        if not self.behaviors[name][2](reset, *args):
            # Comportamento ainda está em execução
            return False

        # Comportamento foi concluído
        self.previous_behavior = self.state_behavior_name  # Atualiza histórico
        self.state_behavior_name = None  # Limpa comportamento atual
        return True

    def execute_sub_behavior(self, name, reset, *args):
        """
        Descrição:
            Executa um passo de um comportamento auxiliar (subcomportamento) identificado por `name`,
            repassando argumentos adicionais conforme necessário. Esse método é útil para composições de comportamentos,
            permitindo que comportamentos customizados reutilizem outros comportamentos já implementados.

            - Realiza verificação de existência do comportamento antes de executar.
            - Caso o comportamento permita, realiza controle automático da orientação da cabeça.
            - O método não altera o estado do comportamento principal, ou seja, ao chamar `get_current()` após este método,
              será retornado o comportamento principal, não o subcomportamento.
            - Comportamentos do tipo "Poses" ignoram o argumento reset.
            - Útil para cenários em que um comportamento de alto nível depende ou compõe ações de outros comportamentos.

        Parâmetros:
            name: str
                Nome do comportamento a ser executado. Deve estar registrado em `self.behaviors`.
            reset: bool
                Indica se o comportamento deve ser reiniciado antes da execução deste passo. O reset é ignorado para poses.
            *args: tuple
                Argumentos adicionais que serão repassados para a execução do comportamento, conforme sua assinatura.

        Retorno:
            finished: bool
                Retorna True se o comportamento solicitado foi concluído, False caso ainda esteja em andamento.
        """

        # Garante que o comportamento solicitado existe
        assert name in self.behaviors, f"Comportamento {name} não existe!"

        # Se o comportamento permite, executa controle automático da cabeça
        if self.behaviors[name][1]:
            self.head.execute()

        # Executa o comportamento solicitado, repassando reset e demais argumentos
        return self.behaviors[name][2](reset, *args)

    def execute_to_completion(self, name, *args):
        """
        Descrição:
            Executa o comportamento identificado por `name` até sua conclusão, enviando comandos ao servidor a cada passo.
            O método gerencia adequadamente o envio e recebimento de comandos para garantir a execução correta do comportamento,
            respeitando as particularidades de slot behaviors, poses e comportamentos customizados.
            
            - O método garante sincronização com o servidor a cada passo, utilizando o método `commit_and_send` seguido de um `receive`.
            - O parâmetro `skip_last` determina se o último comando deve de fato ser enviado, conforme o tipo do comportamento.
            - Ao final, sempre reseta as velocidades-alvo das juntas para garantir integridade das próximas execuções.
        
        Parâmetros:
            name: str
                Nome do comportamento a ser executado. Pode ser um comportamento de slot, pose ou customizado.
            *args: tuple
                Argumentos adicionais necessários para a execução do comportamento.

        Notas
            - Para slot behaviors, considera-se o comportamento finalizado assim que o último comando é enviado.
            - Para poses, a conclusão é determinada pela resposta do servidor indicando o estado desejado do robô.
            - Para comportamentos customizados, assume-se a mesma lógica das poses, ou seja, o último comando é ignorado.
            - Antes de sair, o vetor `Robot.joints_target_speed` é resetado para evitar que comandos residuais afetem execuções futuras.
            - Caso o comportamento indique término logo na primeira chamada, nenhum comando é enviado.
            - Atenção: Caso o comportamento nunca indique término, esta função pode entrar em loop infinito.

        Retorno
            None
        """

        r = self.world.robot
        # Para slot behaviors, o último comando precisa ser enviado; para outros, é ignorado
        skip_last = name not in self.slot_engine.behaviors

        while True:
            done = self.execute(name, *args)
            # Para poses e custom behaviors, sai imediatamente se terminado (não envia último comando)
            if done and skip_last:
                break
            # Envia comando ao servidor e recebe resposta
            self.base_agent.scom.commit_and_send(r.get_command())
            self.base_agent.scom.receive()
            # Para slot behaviors, envia o último comando antes de sair
            if done:
                break

        # Reseta velocidades-alvo das juntas para evitar comandos residuais no próximo comportamento
        r.joints_target_speed = np.zeros_like(r.joints_target_speed)

    def is_ready(self, name, *args) -> bool:
        """ Checks if behavior is ready to start under current game/robot conditions """

        assert name in self.behaviors, f"Comportamento {name} não existe!"
        return self.behaviors[name][3](*args)
