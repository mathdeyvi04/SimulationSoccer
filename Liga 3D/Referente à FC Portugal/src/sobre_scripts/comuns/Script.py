from os import path, listdir, getcwd, cpu_count
from os.path import join, dirname, isfile, isdir
import json
import sys

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

    def __init__(
            self,
            construtor_cpp=0
    ) -> None:
        









