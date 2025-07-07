from Agent.BaseAgent import BaseAgent
from itertools import count
from scripts.commons.Script import Script

"""
Como funciona a comunicação?

O comando `say` permite que um jogador transmita uma mensagem para todos os jogadores em campo.

• Alcance da mensagem: 50 metros  
  (Nota: o campo possui 36 metros na diagonal, então essa limitação pode ser ignorada na prática)

O perceptor `hear` informa três dados:
    - A mensagem recebida
    - O time de origem da mensagem
    - O ângulo absoluto de origem da mensagem (definido como `"self"` se o jogador ouviu a 
    própria mensagem)

Regras de funcionamento:
-------------------------

• As mensagens são recebidas no **próximo ciclo** após serem enviadas.

• As mensagens só podem ser **enviadas a cada 2 ciclos** (ou seja, a cada 0.04s).

• As mensagens enviadas em ciclos "mudos" (nos quais não é permitido comunicar) só são 
**ouvidas por quem as enviou**.

• Em um único ciclo, um jogador pode ouvir **no máximo uma mensagem de outro jogador**,
além da própria mensagem.

• Caso dois jogadores diferentes emitam mensagens no mesmo ciclo, o jogador só ouvirá **a 
primeira** delas.

• Essa limitação é aplicada separadamente para **cada time**:
    Portanto, em teoria, um jogador pode ouvir:
        - sua **própria mensagem**
        - a **primeira mensagem de um colega de equipe**
        - a **primeira mensagem de um adversário**

• Na prática, no entanto, **as mensagens de adversários não importam**, pois o parser do 
nosso time **ignora mensagens de outros times**.

Características das mensagens:
------------------------------

• Comprimento máximo: **20 caracteres**

• Conjunto de caracteres permitido:
    - Qualquer caractere ASCII no intervalo `0x20` a `0x7E`
    - **Exceto**: espaço (`' '`), parênteses (`'('` e `')'`)

• Exemplos de caracteres aceitos:
    Letras, números e símbolos como:  
    `!"#$%&'*+,-./:;<=>?@[\]^_`{|}~`

• **Importante**:  
  Devido a um bug no servidor, o envio de aspas simples (`'`) ou duplas (`"`) **encerra 
  a mensagem prematuramente**.
"""


class Team_Communication:
    """
    Descrição:
        Classe responsável por simular a comunicação entre três jogadores em um ambiente de simulação 3D (RoboCup 3D).
        Cada jogador envia e recebe mensagens utilizando o sistema de comunicação do servidor SimSpark.

        Esta classe permite testar o canal auditivo dos robôs, verificando o envio, recepção,
        e identificação da direção da origem de cada mensagem.
    """

    def __init__(self, script: Script) -> None:
        """
        Descrição:
            Inicializa a comunicação com o objeto principal de controle do experimento.

        Parâmetros:
            script: Script
                Objeto que contém a lógica de criação, envio e recepção de pacotes entre agentes.
        """
        self.script = script

    def player1_hear(self, msg: bytes, direction, timestamp: float) -> None:
        """
        Descrição:
            Callback chamado quando o jogador 1 recebe uma mensagem.

        Parâmetros:
            msg: bytes
                Mensagem recebida.
            direction: float
                Ângulo (graus) indicando a direção de onde veio a mensagem.
            timestamp: float
                Tempo (em segundos) do recebimento da mensagem.
        """
        print(f"Player 1 heard: {msg.decode():20}  from:{direction:7}  timestamp:{timestamp}")

    def player2_hear(self, msg: bytes, direction, timestamp: float) -> None:
        """
        Callback chamado quando o jogador 2 recebe uma mensagem.
        """
        print(f"Player 2 heard: {msg.decode():20}  from:{direction:7}  timestamp:{timestamp}")

    def player3_hear(self, msg: bytes, direction, timestamp: float) -> None:
        """
        Callback chamado quando o jogador 3 recebe uma mensagem.
        """
        print(f"Player 3 heard: {msg.decode():20}  from:{direction:7}  timestamp:{timestamp}")

    def execute(self):
        """
        Descrição:
            Executa o experimento de comunicação em equipe:
            - Cria três agentes no campo com suas respectivas callbacks de audição.
            - Posiciona os jogadores lado a lado.
            - Envia mensagens de identificação continuamente.
            - Imprime o conteúdo e o formato hexadecimal das mensagens enviadas.
            - Aguarda entrada do usuário entre as iterações.
        """
        a = self.script.args  # Argumentos de inicialização fornecidos via Script

        # Define as funções de callback de audição para cada jogador
        hear_callbacks = (self.player1_hear, self.player2_hear, self.player3_hear)

        # Cria os 3 jogadores com configurações definidas
        # Parâmetros: IP, porta agente, porta monitor, número de uniforme, tipo robô, nome time, log, desenho, modo auto, espera servidor, callback ouvir
        self.script.batch_create(
            Agent,
            (
                (a.i, a.p, a.m, i + 1, 0, a.t, True, True, False, True, clbk)
                for i, clbk in enumerate(hear_callbacks)
            )
        )

        # Referência aos objetos dos jogadores para facilitar o uso
        p1: BaseAgent = self.script.players[0]
        p2: BaseAgent = self.script.players[1]
        p3: BaseAgent = self.script.players[2]

        # Posiciona os jogadores em linha no campo, separados verticalmente (eixo Y)
        self.script.batch_commit_beam([(-2, i, 45) for i in range(3)])

        # Loop principal da simulação
        for i in count():
            # Constrói mensagens em bytes com número sequencial
            msg1 = b"I_am_p1!_no:" + str(i).encode()
            msg2 = b"I_am_p2!_no:" + str(i).encode()
            msg3 = b"I_am_p3!_no:" + str(i).encode()

            # Cada jogador envia sua própria mensagem
            p1.scom.commit_announcement(msg1)
            p2.scom.commit_announcement(msg2)
            p3.scom.commit_announcement(msg3)

            # Envia os pacotes de comunicação ao servidor
            self.script.batch_commit_and_send()

            # Mostra o conteúdo das mensagens enviadas no terminal
            print(f"Player 1 sent:  {msg1.decode()}      HEX: {' '.join([f'{m:02X}' for m in msg1])}")
            print(f"Player 2 sent:  {msg2.decode()}      HEX: {' '.join([f'{m:02X}' for m in msg2])}")
            print(f"Player 3 sent:  {msg3.decode()}      HEX: {' '.join([f'{m:02X}' for m in msg3])}")

            # Recebe todas as mensagens disponíveis no ciclo atual
            self.script.batch_receive()

            # Aguarda confirmação manual para prosseguir
            input("Pressione Enter para continuar ou Ctrl+C para sair.")





























