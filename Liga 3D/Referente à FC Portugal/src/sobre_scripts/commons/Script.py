from os import path, listdir, getcwd, cpu_count
from os.path import join, dirname, isfile, isdir
import json
import sys
from time import sleep
from Agent.Agent import Agent

from sobre_scripts.commons.UserInterface import UserInterface

# ISTO NÃO É TRIVIAL
"""
realpath:
    Fornece o caminho real no sistema de arquivos. Mesmo que seja dado
    apenas o basename.

getmtime:
    Retorna o tempo da última modificação de um arquivo, em segundos 
    desde a época (1º de janeiro de 1970).
    
    Para que o tempo seja legível, datetime.fromtimestamp(tempo_desde_modificacao)
"""
from os.path import realpath, getmtime

# ISTO NÃO É TRIVIAL
"""
Descrição:
    Importa o módulo principal do programa que está sendo
    executado.

Exemplo:
    Suponha que estou executando um arquivo A.
    Dentro da execução de A, importo um módulo B.
    Entretanto, eu desejo usar funções disponíveis de A em B.
    Para isso, uso import __main__ e consigo usar as funções de A
    em B.
"""
import __main__

# ISTO NÃO É TRIVIAL
"""
Descrição:

    Módulo responsável por permitir que o usuário defina argumentos
    que podem ser passados para o seu programa quando ele é executado
    no terminal, facilitando a interação do usuário com o script.
    
    É possível especificar argumentos obrigatórios, opcionais,
    tipos de dados, valores padrão, mensagens de ajuda...
    
Exemplo:
    import argparse

    # Criar o parser
    parser = argparse.ArgumentParser(description="Calculadora simples.")
    
    # Adicionar argumentos
    parser.add_argument("num1", type=int, help="Primeiro número.")
    parser.add_argument("num2", type=int, help="Segundo número.")
    parser.add_argument("--operacao", "-o", choices=["soma", "subtracao"], default="soma", help="Operação a ser realizada (padrão: soma).")
    
    # Parse dos argumentos
    args = parser.parse_args()
    
    # Realizar a operação
    if args.operacao == "soma":
        resultado = args.num1 + args.num2
    elif args.operacao == "subtracao":
        resultado = args.num1 - args.num2
    
    print(f"Resultado: {resultado}")
    
    No terminal:
        in: python calculadora.py 10 20 --operacao soma
        out: Resultado: 30
        
        in: python calculadora.py 10 20 -o subtracao
        out: Resultado: -10
        
        in: python calculadora.py --help
        out: 
            usage: calculadora.py [-h] [--operacao {soma,subtracao}] num1 num2
    
            Calculadora simples.
            
            positional arguments:
              num1                  Primeiro número.
              num2                  Segundo número.
            
            optional arguments:
              -h, --help            show this help message and exit
              --operacao {soma,subtracao}, -o {soma,subtracao}
                                    Operação a ser realizada (padrão: soma).
"""
import argparse

# ISTO NÃO É TRIVIAL
"""
Descrição:
    Módulo usado para serializar (converter objetos Python
    em uma sequência de bytes) e desserializar (converter
    bytes de volta em objetos Python). Ele é útil para salvar
    o estado de um objeto em um arquivo ou transmiti-lo pela
    rede, permitindo que você armazene ou compartilhe 
    estruturas de dados complexas, como listas, dicionários 
    ou até mesmo instâncias de classes.
    
Exemplo:

    dados = {
        "nome": "Alice",
        "idade": 30,
        "cidade": "São Paulo",
        "interesses": ["programação", "música", "viagens"]
    }
    
    with open("dados.pkl", "wb") as arquivo:  # 'wb' = write binary (escrever em modo binário)
        pickle.dump(dados, arquivo)  # Serializa o objeto e salva no arquivo
        
    
    with open("dados.pkl", "rb") as arquivo:
        dados_carregados = pickle.load(arquivo) # Desserializando e carregando do arquivo
    
    O conteúdo que está em '.pkl' é executável e tudo mais. 
"""
import pickle

# ISTO NÃO É TRIVIAL
"""
Descrição:
    Módulo usado para criar e gerenciar processos externos,
    permitindo que você execute comandos do sistema 
    operacional diretamente do seu código Python. Ele é uma
    alternativa mais poderosa e flexível às funções antigas
    como os.system() e os.popen(), oferecendo controle sobre
    entradas, saídas e erros dos processos.
"""
import subprocess


class Script:
    """
    Descrição:
        Classe responsável por gerenciar execuções ligadas ao terminal, como
        construção dos módulos C++, e aos agentes, como ações específicas.

    Observações:
        - Foi desenvolvido de tal forma que permite execução em qualquer ambiente.
    """

    caminho_raiz = path.dirname(
        path.dirname(
            realpath(
                join(
                    getcwd(),
                    dirname(__file__)
                )
            )
        )
    )

    # Em meu caso é: C:\Users\deyvi\Documents\ImperioPy\TheBigOnes\RoboCup_RoboIME\Liga 3D\Referente à FC Portugal\src

    def __init__(
            self,
            construtor_cpp=0
    ) -> None:
        #######################################################################
        # Específicando variáveis e argumentos.

        self.opcoes_disponiveis = {
            # Para adicionar mais argumentos, basta incrementá-los aqui.
            # Caso seja feito modificações, o arquivo config.json deve ser MANUALMENTE deletado.
            # ID: (descrição, default)

            'i': ('Servidor IP', 'localhost'),
            'p': ('Porta do Agente', '3100'),
            'm': ('Porta do Monitor', '3200'),
            't': ('Nome do Time', 'RoboIME'),
            'u': ('Número do Uniforme', '1'),
            'r': ('Tipo do Robô', '1'),
            'P': ('Disputa de Penâltis', '0'),
            # 'F': ('magmaFatProxy',      '0'),
            'D': ('Debug', '1')
        }

        self.respectivos_tipos_e_possibilidades = {
            'i': (str, None),
            'p': (int, None),
            'm': (int, None),
            't': (str, None),
            'u': (int, range(1, 12)),
            'r': (int, [0, 1, 2, 3, 4]),
            'P': (int, [0, 1]),
            # 'F': (int, [0,1]),
            'D': (int, [0, 1])
        }

        #######################################################################

        # Garantimos haver um arquivo de configurações e que as informações
        # de argumentos setadas foram obtidas.
        self.ler_ou_criar_config()

        # Ajustando o padrão de mensagens, não se preocupe.
        formatador = lambda prog: argparse.HelpFormatter(
            prog, max_help_position=52
        )
        parser = argparse.ArgumentParser(
            formatter_class=formatador
        )

        for chave in self.opcoes_disponiveis:
            parser.add_argument(
                f"-{chave}",
                help=f"{self.opcoes_disponiveis[chave][0]:30}[{self.opcoes_disponiveis[chave][1]:20}]",
                type=self.respectivos_tipos_e_possibilidades[chave][0],
                nargs="?",
                default=self.opcoes_disponiveis[chave][1],
                metavar='x',
                choices=self.respectivos_tipos_e_possibilidades[chave][1]
            )

        # Namespace(i='localhost', p=3100, m=3200, t='RoboIME', u=1, r=1, P=0, D=1)
        self.args = parser.parse_args()

        if getattr(
                sys,
                'frozen',
                False
        ):
            # Forçar o desligamento do modo debug quando
            # estiver rodando em modo binário, que será o
            # caso da competição, por exemplo.

            self.args.D = 0

        # Lista de Jogadores Criados
        self.jogadores: list[Agent] = []

        # Aqui é onde a brincadeira começa!!
        Script.construir_modulos_cpp(
            saida_da_construcao=(construtor_cpp != 0 and construtor_cpp != self.args.u)
        )

        if self.args.D:
            # Se quisermos rodar em modo debug

            try:
                print(
                    "\nDICA: Se quiser ajuda, rode:",
                    f"python3 {__main__.__file__} -h",
                    sep=" "
                )
            except:
                pass

            colunas = [
                [],
                [],
                []
            ]

            for chave, valor in vars(
                    self.args
            ).items():
                colunas[0].append(
                    self.opcoes_disponiveis[chave][0]
                )
                colunas[1].append(
                    self.opcoes_disponiveis[chave][1]
                )
                colunas[2].append(
                    valor
                )

            UserInterface.apresentar_tabela(colunas, [
                "Argumento",
                "Padrão em config.json",
                "No Momento"
            ], alinhamento=["<", "^", "^"])

    def ler_ou_criar_config(
            self
    ) -> None:
        """
        Descrição:
            Garante que o arquivo 'config.json', responsável por armazenar as configurações
            padrão da aplicação, exista e esteja corretamente preenchido.

            Caso o arquivo ainda não exista, ele será criado automaticamente com base
            no conteúdo do atributo `self.opcoes_disponiveis`. Caso exista, mas esteja
            vazio (o que pode ocorrer durante manipulações concorrentes), a função aguarda
            brevemente e, se ainda estiver vazio, aborta a execução com uma mensagem
            de aviso.

            Quando o arquivo está presente e íntegro, seu conteúdo é carregado
            e atribuído novamente ao atributo `self.opcoes_disponiveis`.

        Parâmetros:
            None

        Retorno:
            A função não retorna valores, mas pode encerrar o programa com `exit()`
            caso o arquivo 'config.json' esteja vazio mesmo após a tentativa de espera.
        """

        # Caso não exista um arquivo-base dos argumentos padrões, config.json
        # devemos criá-lo.
        if not path.isfile(
                "config.json"
        ):
            with open(
                    "config.json",
                    'w'
            ) as arquivo_de_arg_padroes:
                json.dump(
                    self.opcoes_disponiveis,
                    arquivo_de_arg_padroes,
                    indent=4
                )
        else:

            # Existe a possibilidade de que, durante operações de inscrição ou
            # de lançamento de agentes, este arquivo config esteja sendo manipulado
            # e, por acaso, esteja vazio.
            if path.getsize("config.json") == 0:
                from time import sleep
                sleep(1)

            # Caso ele permaneça vazio, devemos abortar
            if path.getsize("config.json") == 0:
                print("Abortando: 'config.json' está vazio. Verifique manualmente e/ou delete-o.")
                exit()

            with open(
                    "config.json",
                    "r"
            ) as arquivo_de_arg_padroes:
                self.opcoes_disponiveis = json.loads(arquivo_de_arg_padroes.read())

    @staticmethod
    def construir_modulos_cpp(
            ambiente_especial: list = None,
            saida_da_construcao: bool = False,
    ) -> None:
        """
        Descrição:
            Compila automaticamente os módulos C++ localizados no diretório 'sobre_cpp/',
            utilizando `make` e integração via `pybind11` para uso em Python. A função identifica
            quais módulos precisam ser (re)construídos com base em arquivos auxiliares e datas
            de modificação dos códigos-fonte.

            Caso um módulo já tenha sido construído com a versão atual do Python e não tenha sido
            modificado desde então, sua recompilação será evitada.

            A função pode ser configurada para executar em ambientes especiais e abortar a construção
            se não estiver autorizada (usando o parâmetro `saida_da_construcao`).

        Parâmetros:
            - ambiente_especial (list, opcional):
                Lista com comandos ou variáveis de ambiente adicionais que devem preceder
                a chamada ao interpretador Python, por exemplo, para ativar ambientes virtuais
                ou modificar o `PATH`.

            - saida_da_construcao (bool, opcional):
                Caso True e existam módulos a serem construídos, aborta a execução com uma
                mensagem explicativa, sem iniciar a construção de fato.

        Retorno:
            - None:
                A função não retorna nenhum valor. Em caso de falha na construção, imprime mensagens
                de erro e encerra o programa com `exit()`.
        """

        if ambiente_especial is None:
            ambiente_especial = []

        caminho_cpp = Script.caminho_raiz + "/sobre_cpp/"
        exclusoes = {"__pycache__", ".", ".."}

        modulos_cpp_a_serem_construidos = [
            modulo_cpp for modulo_cpp in listdir(caminho_cpp) if isdir(
                join(
                    caminho_cpp,
                    modulo_cpp
                )
            ) and modulo_cpp not in exclusoes
        ]

        if not modulos_cpp_a_serem_construidos:
            # Caso não haja módulos a serem construídos
            print("Não há Módulos CPP a serem construídos.")
            return None

        # Vai obter a versão python do sistema, 3.11, por exemplo.
        python_cmd = f"python{sys.version_info.major}.{sys.version_info.minor}"

        def builder() -> str:
            """
            Descrição:
                Função responsável por iniciar o processo de construção de módulos C++
                com integração ao Python, utilizando `pybind11`.

                O processo é iniciado com a impressão de uma mensagem visual no terminal.
                Em seguida, executa um comando de linha para obter as flags de compilação
                necessárias para a construção de módulos C++ integrados ao Python por meio
                do `pybind11`. Isso é feito usando `subprocess.Popen` para capturar a saída
                do comando `python -m pybind11 --includes`.

                Em caso de falha durante a execução do subprocesso, uma mensagem de erro
                é exibida e o programa é encerrado imediatamente.

            Parâmetros:
                None

            Retorno:
                - str:
                    Flags de compilação necessárias para módulos C++.

                - None:
                    Caso ocorra um erro, encerra tudo com 'exit()'.
            """

            print("\033[7m\033[1m|-- Construindo Módulos C++ --|\033[0m")

            try:
                """
                - subprocess.run
                    Simples, espera o comando terminar e retorna o resultado.
                    Usado quando se deseja rodar e pegar o resultado final.

                - subprocess.Popen
                    Mais flexível, não espera terminar, permite comunicação/interação enquanto executa.
                    Usado quando necessita-se de controle total.
                """
                # Com este comando, descobriremos as flags necessárias.
                processo_ = subprocess.Popen(
                    ambiente_especial + [python_cmd, "-m", "pybind11", "--includes"], stdout=subprocess.PIPE
                )
                includes_, _ = processo_.communicate()
                processo_.wait()

            except Exception as error_do_processo:
                print(f"\033[7m\033[31mUm erro ocorreu na construção do módulo C++: {error_do_processo}\033[0m")
                exit()

            includes_ = includes_.decode().rstrip()
            # Obtido por mim:
            # Using Pybind11 Includes: -I/usr/include/python3.12 -I/usr/lib/python3/dist-packages/pybind11/include
            print(f"Using Pybind11 Includes: {includes_}")
            return includes_

        # Número de Processadores na máquina.
        n_proc = str(cpu_count())

        # Variável de Contagem
        zero_modulos_construidos = True
        # Variável que será utilizada dentro do Makefile
        includes = None
        for modulo_cpp in modulos_cpp_a_serem_construidos:
            caminho_do_modulo = join(caminho_cpp, modulo_cpp)

            # Pularemos se não houver um Makefile dentro do módulo
            if not isfile(join(caminho_do_modulo, "Makefile")):
                continue

            # Caso já houver:
            # -> o módulo .so correspondente, e;
            # -> o arquivo .c_info auxiliar que armazena informações sobre contexto de compilação do módulo
            if isfile(join(caminho_do_modulo, modulo_cpp + ".so")) and isfile(join(caminho_do_modulo, modulo_cpp + ".c_info")):
                with open(
                        join(caminho_do_modulo, modulo_cpp + ".c_info"),
                        "rb"
                ) as arq_c_info:
                    # Vamos desserializar um objeto python.
                    info_c_info = pickle.load(arq_c_info)

                # Essa igualdade significa que os binários foram criados com a mesma versão
                # do python que estamos usando agora.
                if info_c_info == python_cmd:
                    code_mod_time = max(
                        # Calculamos a data de modificação mais recente dentre todos os .cpp e .h do código-fonte
                        getmtime(join(caminho_do_modulo, arq_possivel)) for arq_possivel in listdir(caminho_do_modulo) if arq_possivel.endswith(".cpp") or arq_possivel.endswith(".h")
                    )
                    bin_mod_time = getmtime(join(caminho_do_modulo, modulo_cpp + ".so"))

                    # Se o binário .so tenha sido modificado até 30 segundos depois do código-fonte mais novo, pulará.
                    if (bin_mod_time + 30) > code_mod_time:
                        continue

            if zero_modulos_construidos:
                if saida_da_construcao:
                    print("\nHá módulos C++ para construir, entretanto este jogador não está autorizado a construir. Abortando.")
                    exit()

                zero_modulos_construidos = False
                includes = builder()

            # Vamos construir o módulo.
            print(f'{f"Building: {modulo_cpp}... ":40}', end='', flush=True)
            processo = subprocess.Popen(
                ["make", "-j" + n_proc, "PYBIND_INCLUDES=" + includes],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=caminho_do_modulo
            )

            output, error = processo.communicate()
            exit_code = processo.wait()

            if exit_code == 0:
                print("\033[7m\033[1mMódulo Construído.\033[0m")
                with open(
                        join(caminho_do_modulo, modulo_cpp + ".c_info"),
                        "wb"
                ) as arq_c_info:
                    pickle.dump(python_cmd, arq_c_info, protocol=4)
            else:
                print("Abortando, houve erros na construção: ")
                print(output.decode(), err.decode())
                exit()

            subprocess.run(
                ["make", "clean"],
                cwd=caminho_do_modulo
            )

        if not zero_modulos_construidos:
            print("\033[7:32mTodos os módulos C++ foram construídos com sucesso.\033[0m")

    # Os Demais Métodos São inerentes à importação 'import __main__'

    def batch_create(self, agent_cls, args_per_player) -> None:
        """
        Descrição:
            Cria e adiciona em lote (batch) instâncias de agentes à lista de jogadores (self.players).
            Para cada item em 'args_per_player', instancia 'agent_cls' passando os argumentos correspondentes
            e adiciona o agente criado à lista de jogadores da instância.

        Parâmetros:
            agent_cls:
                Classe do agente angt ser instanciada para cada jogador.
                Deve ser uma referência angt uma classe que pode ser inicializada com os argumentos fornecidos.
            args_per_player:
                Lista (ou iterável) onde cada elemento é uma tupla ou lista contendo os argumentos
                necessários para instanciar um agente da classe 'agent_cls'.

        Retorno:
            None:
                Esta função não retorna nada explicitamente. Ela modifica o atributo 'self.players'
                adicionando novos agentes criados.
        """
        # Itera sobre cada conjunto de argumentos e cria um agente, adicionando-o à lista de jogadores
        for angt in args_per_player:
            self.players.append(agent_cls(*angt))

    def batch_execute_agent(self, index: slice = slice(None)) -> None:
        """
        Descrição:
            Executa os agentes normalmente (incluindo commit e envio).

        Parâmetros
            index: slice, opcional
                Subconjunto de agentes a serem executados.
                Exemplos:
                    index=slice(1,2) seleciona o segundo agente.
                    index=slice(1,3) seleciona o segundo e terceiro agentes.
                Por padrão, todos os agentes são selecionados.

        Retorno:
            None
                Apenas executa a ação 'think_and_send' para cada agente selecionado.
        """
        # Itera sobre os agentes selecionados e executa o método think_and_send() para cada um
        for p in self.players[index]:
            p.think_and_send()

    def batch_execute_behavior(self, behavior: str, index: slice = slice(None)) -> None:
        """
        Descrição:
            Executa um comportamento específico para um subconjunto de agentes presentes em self.players.
            Para cada agente selecionado pelo índice, chama o método 'execute' do atributo 'behavior' do agente,
            passando o nome do comportamento a ser executado.

        Parâmetros
            behavior: str
                Nome do comportamento a ser executado. Este nome é utilizado no método 'execute' do atributo 'behavior' de cada agente.
            index: slice, opcional
                Subconjunto de agentes para os quais o comportamento será executado.
                Exemplos:
                    index=slice(1,2) seleciona o segundo agente.
                    index=slice(1,3) seleciona o segundo e terceiro agentes.
                Por padrão, todos os agentes são selecionados.

        Retorno
            None
                Esta função não retorna nada explicitamente. Ela apenas executa o comportamento especificado para os agentes selecionados.
        """
        # Itera sobre os agentes selecionados e executa o comportamento especificado em cada um
        for p in self.players[index]:
            p.behavior.execute(behavior)

    def batch_commit_and_send(self, index: slice = slice(None)) -> None:
        """
        Descrição:
            Realiza o commit e envia dados ao servidor para um subconjunto de agentes presentes em self.players.
            Para cada agente selecionado pelo índice, chama o método commit_and_send do atributo 'scom',
            utilizando o comando obtido por p.world.robot.get_command().

        Parâmetros:
            index: slice, opcional
                Subconjunto de agentes para os quais será realizado o commit e envio dos dados.
                Exemplos:
                    index=slice(1,2) seleciona o segundo agente.
                    index=slice(1,3) seleciona o segundo e terceiro agentes.
                Por padrão, todos os agentes são selecionados.

        Retorno
            None
                Ela apenas executa o commit e envio dos dados para os agentes selecionados.
        """
        # Itera sobre os agentes selecionados e realiza o commit e envio do comando de cada agente
        for p in self.players[index]:
            # Obtém o comando do robô do agente e
            # envia ao servidor através do método commit_and_send
            p.scom.commit_and_send(p.world.robot.get_command())

    def batch_receive(self, index: slice = slice(None), update=True) -> None:
        """
        Descrição:
            Aguarda e processa mensagens do servidor para um subconjunto de agentes presentes em self.players.
            Para cada agente selecionado pelo índice, chama o método receive do atributo 'scom',
            podendo atualizar ou não o estado do mundo do agente conforme o parâmetro 'update'.

        Parâmetros
            index: slice, opcional
                Subconjunto de agentes que receberão as mensagens do servidor.
                Exemplos:
                    index=slice(1,2) seleciona o segundo agente.
                    index=slice(1,3) seleciona o segundo e terceiro agentes.
                Por padrão, todos os agentes são selecionados.
            update: bool, opcional
                Atualiza o estado do mundo do agente com as informações recebidas do servidor.
                Se False, o agente não atualiza seu estado, tornando-se "inconsciente" de si e do ambiente.
                Isso pode servir para economizar recursos de CPU em agentes fictícios usados em demonstrações.

        Retorno
            None
                Apenas processa a recepção de mensagens para os agentes selecionados.
        """
        # Itera sobre os agentes selecionados e executa a recepção de mensagens do servidor.
        for p in self.players[index]:
            # Recebe mensagem do servidor e atualiza o estado do agente se 'update' for True.
            p.scom.receive(update)

    def batch_commit_beam(self, pos2d_and_rotation: list[list], index: slice = slice(None)) -> None:
        """
        Descrição:
            Move ("beam") todos os jogadores selecionados para uma posição 2D específica com determinada rotação.
            Para cada agente selecionado, chama o método commit_beam do atributo 'scom', passando a posição (x, y) e o ângulo de rotação.

        Parâmetros
            pos2d_and_rotation : list
                Iterável contendo tuplas ou listas com as posições 2D (x, y) e a rotação (em graus ou radianos) para cada agente.
                Exemplo: [(0, 0, 45), (-5, 0, 90)]
            index : slice, opcional
                Subconjunto de agentes a serem movidos.

        Retorno
            None
                Apenas executa a movimentação para os agentes selecionados.
        """
        # Itera simultaneamente sobre os agentes selecionados e as respectivas posições/rotações fornecidas
        for p, pos_rot in zip(self.players[index], pos2d_and_rotation):
            # Move o agente para a posição 2D (x, y) e aplica a rotação especificada
            p.scom.commit_beam(pos_rot[0:2], pos_rot[2])

    def batch_unofficial_beam(self, pos3d_and_rotation, index: slice = slice(None)) -> None:
        """
        Descrição:
            Move ("beam") todos os jogadores selecionados para uma posição 3D específica com determinada rotação
            utilizando um método não oficial. Para cada agente selecionado, chama o método unofficial_beam do atributo 'scom',
            passando a posição (x, y, z) e o ângulo de rotação.

        Parâmetros:
            pos3d_and_rotation : list
                Iterável contendo tuplas ou listas com as posições 3D (x, y, z) e a rotação (em graus ou radianos) para cada agente.
                Exemplo: [(0, 0, 0.5, 45), (-5, 0, 0.5, 90)]
            index: slice, opcional
                Subconjunto de agentes a serem movidos.

        Retorno
            None
                Apenas executa a movimentação para os agentes selecionados.
        """
        # Itera simultaneamente sobre os agentes selecionados e as respectivas posições/rotações fornecidas
        for p, pos_rot in zip(self.players[index], pos3d_and_rotation):
            # Move o agente para a posição 3D (x, y, z) e aplica a rotação especificada usando o método não oficial
            p.scom.unofficial_beam(pos_rot[0:3], pos_rot[3])

    def batch_terminate(self, index: slice = slice(None)) -> None:
        """
        Descrição:
            Encerra todas as conexões de socket dos agentes selecionados, chamando o método terminate para cada agente,
            e remove esses agentes da lista self.players.
            Em scripts onde os agentes permanecem ativos até o fim da aplicação,
            este método pode não ser necessário.

        Parâmetros
            index: slice, opcional
                Subconjunto de agentes que terão suas conexões encerradas e serão removidos da lista.

        Retorno
            None
                Remove os agentes selecionados da lista e encerra suas conexões.
        """
        # Itera sobre os agentes selecionados e encerra suas conexões de socket
        for p in self.players[index]:
            p.terminate()
        # Remove da lista self.players todos os agentes que foram terminados
        del self.players[index]  # deleta a seleção de agentes da lista


if __name__ == '__main__':
    # Script()

    pass
