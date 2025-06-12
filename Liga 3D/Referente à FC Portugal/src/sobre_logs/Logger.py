from pathlib import Path
from datetime import datetime
from random import choices
# Estamos literalmente importando as letras ascii maiuscúlas
from string import ascii_uppercase


class Logger:
    """
    Descrição:

    Métodos Disponíveis:

    Variáveis de Ambiente:

    """

    _folder = None

    def __init__(self, is_enabled: bool, topic: str) -> None:
        """
        Descrição:
            Iniciaremos uma instância com os valores padrões para quantidade
            de entradas e parâmetros dados.

        Parâmetros:
            - is_enabled:

                Se a escritura é permitida.
            - topic:

                Nome do parante responsável, possibilitando a modularização dos logs.
        """
        # Quantas vezes teremos escrito na pasta correspondente
        self.quantidade_de_entradas_na_pasta = 0

        self.enabled = is_enabled
        self.topic = topic

    def escrever(self, msg: str, timestamp: bool = True, step: int = None) -> None:
        """
        Descrição:
            Escreve uma mensagem no arquivo de log associado ao tópico definido pela instância. O nome do arquivo é dado por `self.topic`,
            e os logs são armazenados em uma pasta única identificada por data, hora e um código aleatório.

            A mensagem pode ser precedida por um carimbo de tempo (timestamp) e/ou por uma etapa da simulação (`step`), conforme configurado.
            A pasta de logs só é criada quando a primeira mensagem é escrita, o que evita a criação desnecessária de diretórios.

            Se o logger estiver desabilitado (`self.enabled=False`), nada é registrado.

        Parâmetros:
            - msg: str
                  Mensagem a ser escrita no arquivo de log.

            - timestamp: bool, opcional (padrão: True)
                  Indica se o carimbo de tempo atual deve ser adicionado antes da mensagem.

            - step: int, opcional
                  Número da etapa da simulação. Se fornecido, ele será incluído na linha do log logo após o timestamp (se ativado).

        Retorno:
            None
        """

        if not self.enabled:
            return

        # A pasta de logs info será criada apenas se necessário.
        if Logger._folder is None:
            # Vamos gerar um nome alteatório para a pasta
            # o que é extremamente útil para caso tenhamos múltiplos servidores em treinamento.
            nome_alteratorio = ''.join(choices(ascii_uppercase, k=6))

            # Alteramos de None para um nome alteratorio com informação de data.
            Logger._folder = "./sobre_logs/" + datetime.now().strftime("%Y-%m-%d_%H.%M.%S__") + nome_alteratorio + "/"
            print(f"\n\033[1;7;36mPasta Logger Info Criada, verifique em: {Logger._folder}\033[0m")

            # parants=True: garante que os caminhos superiores sejam criados, caso não exista.
            # exist_ok=True: não levantará erro se o diretório já existir.
            Path(Logger._folder).mkdir(parents=True, exist_ok=True)

        self.quantidade_de_entradas_na_pasta += 1

        with open(
                Logger._folder + self.topic + ".log",
                'a+'
        ) as arq_log:

            prefix = ""
            write_step = step is not None
            # Fornecemos mais informações
            if timestamp or write_step:
                prefix = "{"
                if timestamp:
                    prefix += datetime.now().strftime("%a %H:%M:%S")
                    if write_step:
                        prefix += " "
                if write_step:
                    prefix += f'Step:{step}'
                prefix += "} "

            # Escrevemos a mensagem.
            arq_log.write(prefix + msg + "\n")


if __name__ == '__main__':
    # Vai criar uma pasta neste diretório, isto é, sobre_logs/sobre_logs.
    # Quando executarmos de fora, funcionará corretamente.
    Logger(True, "arroaz").escrever("sexo")