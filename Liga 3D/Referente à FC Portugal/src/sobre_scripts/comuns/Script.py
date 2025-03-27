# Desenvolvido por:
#   Matheus Deyvisson, 2025
#
# Descrição:
#     Código responsável por gerenciar comunicação e execução do código com terminal
#     e construir módulos cpp.


from os import path, listdir, getcwd, cpu_count
from os.path import join, dirname, isfile, isdir
import json
import sys
from time import sleep
from sobre_scripts.comuns.UserInterface import UserInterface

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
        Classe responsável por gerenciar a execução
        de programas no terminal e permitir o uso de argumentos.
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

            'i': (
                "Servidor IP",
                "localhost"
            ),

            'p': (
                "Porta dos Agentes",
                "3100"
            ),

            'm': (
                "Porta do Monitor",
                "3200"
            ),

            't': (
                "Nome do Time",
                "RoboIME"
            ),

            'u': (
                "Numero do Uniforme",
                '1'
            ),

            'r': (
                "Tipo do Robo",
                '1'
            ),

            'P': (
                "Disputa de Penaltis",
                '0'
            ),

            # 'F': (
            #     "magmaFatProxy",
            #     '0'
            # ),

            'D': (
                "Debug",
                '1'
            )
        }

        self.respectivos_tipos_e_possibilidades = {
            'i': (
                str,
                None
            ),

            'p': (
                int,
                None
            ),

            'm': (
                int,
                None
            ),

            't': (
                str,
                None
            ),

            'u': (
                int,
                {i for i in range(1, 12)}
            ),

            'r': (
                int,
                {i for i in range(0, 5)}
            ),

            'P': (
                int,
                [0, 1]
            ),

            # 'F': (
            #     int,
            #     [0, 1]
            # ),

            'D': (
                int,
                [0, 1]
            )
        }

        #######################################################################

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

        self.args = parser.parse_args()

        if getattr(
                sys,
                'frozen',
                False
        ):
            # Forçar o desligamento do modo debug quando
            # estiver rodando em modo binário.

            self.args.D = 0

        # Lista de Jogadores Criados
        self.jogadores = []

        Script.construir_modulos_cpp()

        if self.args.D:
            # Se quisermos rodar em modo debug

            try:
                print(
                    "\nDICA: Se quiser ajuda, rode:",
                    f"python {__main__.__file__} -h",
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

            UserInterface.apresentar_tabela(
                colunas,
                [
                    "Argumento",
                    "Padrão em config.json",
                    "Ativar"
                ],
                alinhamento=["<", "^", "^"]
            )

    def ler_ou_criar_config(
            self
    ) -> None:
        """
        Descrição:
            Garantir que config.json esteja disponível.
        """

        # Caso não exista um arquivo base dos argumentos padrões
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
            tempo_total_de_espera = 0
            while path.getsize(
                    "config.json"
            ) == 0 and tempo_total_de_espera < 5:
                print(
                    f"Esperando que config.json não esteja vazio."
                )

                sleep(0.1)

                tempo_total_de_espera += 0.1

            if tempo_total_de_espera >= 5:
                print(
                    "O tempo de espera foi ultrapassado e o arquivo config.json ainda estava vazio. Verifique manualmente."
                )

            with open(
                    "config.json",
                    'r'
            ) as arq:
                # Pode ser que tenhamos feito modificações específicas nos argumentos padrões.
                # Para isso, importamos direto do arquivo.
                self.opcoes_disponiveis = json.loads(
                    arq.read()
                )

    @staticmethod
    def construir_modulos_cpp(
            ambiente_especial: list = None,
            saida_da_construcao: bool = False,
    ):
        """
        Descrição:
            Construir módulos C++ da pasta /sobre_cpp usando
            o módulo Pybind11.

        Parâmetros:
            -> ambiente_especial
                Comando de Prefixo para rodar um programa específico
                em um determinado ambiente.

                Útil para compilar c++ em diferentes interpretadores python.

                Exemplo: ['conda', 'run', ...]

                se Nada for dado, usa o interpretador padrão.

            -> saida_da_construcao
                Saída se há alguma coisa para construir.
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
            print("Não há nenhum módulo cpp a ser construído.")
            return None  # Não há nada para construir
            pass

        comando_relativo_ao_python = f"python{sys.version_info.major}.{sys.version_info.minor}"

        # Em meu caso, python3.11

        def iniciar_construcao() -> str:
            """
            Descrição:
                Obtém os caminhos de inclusão, includes, para compilar o código C++.
            """

            print(
                "-" * 100
            )
            print("Módulos C++ sendo construidos:", modulos_cpp_a_serem_construidos)

            try:
                # ISTO NÃO É TRIVIAL
                """
                Lembre-se de MultiProcessing e de programação não linear.
                Cria um novo processo para executar um comando no sistema.
                """

                processo = subprocess.Popen(
                    ambiente_especial + [
                        comando_relativo_ao_python,
                        "-m",
                        "pybind11",

                        # Executa o módulo pybind11 com o argumento --includes
                        # que retorna os caminhos de inclusão necessários para
                        # compilar extensões C++.
                        "--includes"
                    ],
                    # Redireciona a saída padrão (stdout) do comando para um
                    # pipe, permitindo que seja capturada no código Python.
                    stdout=subprocess.PIPE
                )

                # Captura as saídas e erros do processo.
                # Esse includes será bytes de informação
                includes_, error = processo.comunicate()

                processo.wait()  # Aguarda o término do processo.

            except Exception as Error:

                print(
                    f"Obtive {Error} ao executar '{python_cmd} -m pybind11 --includes'"
                )

                exit()

            includes_ = includes_.decode().rstrip()
            print(
                f"Usando Pybind11 os : '{includes_}'"
            )

            # Esses são os caminhos de inclusão que o compilador C++
            # precisa para encontrar os headers do PyBind11 e do
            # Python.

            return includes_

        n_processadores = str(
            # Número de CPUs, físicos e lógicos(Threads), disponíveis no sistema onde o código está sendo
            # executado.
            cpu_count()
        )

        for modulo in modulos_cpp_a_serem_construidos:
            caminho_modulo = join(
                caminho_cpp,
                modulo
            )

            # Pula o módulo se não houver Makefile
            if not isfile(
                    join(
                        caminho_modulo,
                        "Makefile"
                    )
            ):
                continue

            # Cenário Específico
            if isfile(
                    join(
                        caminho_modulo,
                        modulo + ".so"
                    )
            ) and isfile(
                join(
                    caminho_modulo,
                    modulo + ".c_info"
                )
            ):
                with open(
                        join(
                            caminho_modulo,
                            modulo + ".c_info"
                        ),
                        'rb'
                ) as arq:
                    # Vamos carregar informações em binário
                    # a partir do arquivo .c_info.
                    info = pickle.load(
                        arq
                    )

                if info == comando_relativo_ao_python:
                    maior_tempo_desde_ultima_modificacao_nos_arquivos = max(
                        # Vamos procurar o arquivo de código que possui maior tempo
                        # desde última modificação.
                        getmtime(
                            join(
                                caminho_modulo,
                                arquivo_de_codigo
                            )
                        ) for arquivo_de_codigo in listdir(
                            caminho_modulo
                        ) if arquivo_de_codigo.endswith(".cpp") or arquivo_de_codigo.endswith(".h")
                    )

                    tempo_desde_ultima_modificacao_no_binario = getmtime(
                        join(
                            caminho_modulo,
                            modulo + ".so"
                        )
                    )

                    if (maior_tempo_desde_ultima_modificacao_nos_arquivos - tempo_desde_ultima_modificacao_no_binario) < 30:
                        """
                        Não buildar com uma margem de 30 segundos.
                        Cenário descrito pela equipe:
                        'we unzip the fcpy project, including the binaries, the modification times are all similar'
                        
                        """

                        continue

            if saida_da_construcao:
                print(
                    "Existem módulos a serem construídos. Este jogador não é permitido a ser construídor. Abortando..."
                )
                exit()

            includes = iniciar_construcao()

            print(
                # Fazemos dessa forma devido ao parâmetro 40 que desejamos no print.
                f"{f'Buildando: {modulo}.....':40}",
                end='',
                flush=True
            )

            processo = subprocess.Popen(
                [
                    "make",
                    f"-j{n_processadores}",  # Para usarmos os processadores de forma inteligente
                    f"PYBIND_INCLUDES={includes}"
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=caminho_modulo
            )

            saida, erros = processo.communicate()
            codigo_de_resultado = processo.wait()

            if codigo_de_resultado == 0:
                print(
                    f"Construção do módulo {modulo} efetivada!"
                )
                with open(
                        join(
                            caminho_modulo,
                            modulo + ".c_info"
                        ),
                        "wb"
                ) as arq:
                    pickle.dump(
                        comando_relativo_ao_python,
                        arq,
                        protocol=4  # Rapaz, se eu te contar oq é isso...
                    )
            else:
                print(
                    f"Aborto! Construção do módulo {modulo} não foi concluída."
                )

                print(
                    saida.decode(), erros.decode()
                )

                exit()

            # Se chegou até aqui, está tudo bem.
            print(
                "Todos os módulos construídos!\n"
            )
            print("-" * 50)

    # Os Demais Métodos São inerentes à importação 'import __main__'

