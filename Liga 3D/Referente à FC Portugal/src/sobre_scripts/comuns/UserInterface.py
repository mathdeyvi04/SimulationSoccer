# Desenvolvido por:
#   Matheus Deyvisson, 2025
#

from itertools import zip_longest
from math import ceil
from numpy import clip
from shutil import get_terminal_size
from typing import Callable


class UserInterface:
    """
    Descrição:
        Classe responsável por criar uma interface de interação
        do usuário com o terminal amigável, com
        tratamento de erros e formatação automática.

        Inclui funcionalidades interessantes como leitura de
        entradas específicas, como *ler_particula* e *ler_inteiro*,
        e a exibição de listas e tabelas formatadas, *apresentar_lista*.

        A ideia é que não há reutilização de partes do código, logo
        é desnecessário a fragmentação em subfunções.

    """

    comprimento_console = 80
    largura_console = 24

    """
    Oq significa @staticmethod?
        * Um método estático é um método que não depende da instância da classe (self) nem da classe em si (cls).

        * Ele é como uma função comum, mas está organizado dentro de uma classe por questões de organização ou semântica.

        * Como não acessa self ou cls, ele não pode modificar o estado da instância ou da classe.
    """

    @staticmethod
    def ler_entrada(
            prompt: str,
            lista_de_opcoes_de_string: list[str],
            tipo_da_entrada: Callable = str,
            intervalo=None
    ) -> tuple[int, bool]:
        """
        Descrição:
            Ler entrada do usuário com base em um tipo de
            dado ou uma lista de opções de strings.

        Parâmetros:
            -> prompt
                Mensagem exibida ao usuário antes de solicitar
                a entrada.

            -> lista_de_opcoes_de_string
                Autoexplicativo.

            -> tipo_de_entrada
                Se for str, o usuário deve escolher uma opção da
                lista_de_opcoes_de_string.

                Caso não, pode inserir um valor do tipo especificado.

                Deve ser colocado como o ponteiro para as funções:

                str() -> str.
                int() -> int

            -> intervalo:
                Para tipos numéricos.

        Retorno:
            -> escolha_do_usuario: int/tipo_de_entrada
                index da lista_de_opcoes_de_string ou o valor selecionado de uma vez.

            -> eh_opcao_da_lista: bool
                True se escolha_do_usuario faz parte da lista_de_opcoes_de_string.
        """

        if intervalo is None:
            """
            No código original tinhamos:
            
            ...
            def read_particle(... interval=[-math.inf, math.inf]):
                pass
            
            * O termo math.inf é desnecessário.
            * O argumento padrão mutável torna o código sucessível a erros.
            """
            intervalo = [-99999, 99999]

        # Verificações para o caso de não haver opções disponíveis
        # para o usuário.
        if isinstance(
                tipo_da_entrada,
                str
        ) and len(lista_de_opcoes_de_string) == 1:
            print(
                prompt,
                lista_de_opcoes_de_string[0],
                sep=""  # Não usar separador entre os itens, é mais rápido que construir uma string.
            )

            return 0, True

        if isinstance(
                tipo_da_entrada,
                int
        ) and (intervalo[1] - intervalo[0]) == 1:
            print(
                prompt,
                intervalo[0],
                sep=""  # Não usar separador entre os itens, é mais rápido que construir uma string.
            )

            return intervalo[0], False

        while True:
            entrada_do_usuario = input(
                prompt
            )

            if entrada_do_usuario in lista_de_opcoes_de_string:
                return lista_de_opcoes_de_string.index(
                    entrada_do_usuario
                ), True

            if not isinstance(
                    tipo_da_entrada,
                    str
            ):

                try:
                    # Força que a entrada esteja no tipo.
                    entrada_do_usuario = tipo_da_entrada(
                        entrada_do_usuario
                    )

                    if intervalo[0] <= entrada_do_usuario < intervalo[1]:
                        return entrada_do_usuario, False

                except:
                    pass

            print(
                f"Error: Entrada Inválida! Opções: {lista_de_opcoes_de_string} ou {tipo_da_entrada if tipo_da_entrada != str else ''}"
            )

    @staticmethod
    def ler_inteiro(
            prompt: str,
            minimo: int,
            maximo: int
    ) -> int:
        """
        Descrição:
            Lê um inteiro a partir da entrada do usuário pelo terminal.

        Parâmetros:
            Autoexplicativos.

        Retorno:
            Autoexplicativo.
        """

        while True:
            entrada_do_usuario = input(
                prompt
            )

            try:

                entrada_do_usuario = int(entrada_do_usuario)

                """
                O assert em Python é uma ferramenta de
                depuração que verifica se uma condição é
                verdadeira.
                """
                assert minimo <= entrada_do_usuario < maximo

                return entrada_do_usuario

            except:
                print(
                    f"Error: Entrada Inválida! Escolha um número entre {minimo} e {maximo - 1}"
                )

    @staticmethod
    def apresentar_tabela(
            dados: list[list],
            titulos: list[str] = None,
            alinhamento: list = None,
            comprimento_das_colunas: list[int] = None,
            max_num_de_colunas_por_titulo: list[int] = None,
            margens: list[int] = None,
            enumerando: list[bool] = None,
            prompt: str = None
    ) -> None | tuple[int, int, int]:
        """
        Descrição:
            Apresentará uma tabela formatada.

        Parâmetros:
            -> dados
                Lista de colunas, sendo cada coluna uma lista
                de itens.

                Tenha atenção neste parâmetro.
                Por exemplo, uma matrix como:
                A B C
                1 2 3
                @ # $

                Deve ser representada pelas colunas, não pelas linhas.

                dados = [
                    [A, 1, @],
                    ...
                ]

                Oq não é muito intuitivo. Podemos aprimorar isso no futuro.

            -> titulos
                Lista de títulos para cada coluna.

                Caso nada seja fornecido, não terá títulos.

            -> alinhamento
                Lista de alinhamentos para cada coluna.

                '<' --- Alinhado com Esquerda.
                '^' --- Alinhado com Centro.
                ...

                Caso nada seja fornecido, será alinhado com a esquerda.

            -> comprimento_das_colunas
                Lista de comprimento para cada coluna.

                Caso nada seja fornecido, cada coluna terá o comprimento
                mínimo necessário.

                Caso um valor dentro da lista seja 0, o comprimento da
                respectiva coluna será o mínimo necessário.

            -> max_num_de_colunas_por_titulo
                Máximo número de subcolunas que estarão sob mesmo título.

                Caso nada seja fornecido, 1 subcoluna por título.

            -> numbering
                Especifica o número de espaços adicionais à esquerda
                (leading) e à direita (trailing) para cada coluna da
                tabela.

                Caso nada seja fornecido, assume-se uma margem de 2 espaços
                para todas as colunas.

            -> enumerando
                Sendo uma lista de booleanos, define se cada coluna deve ter suas opções numeradas.

                Por exemplo, enumerando = [True, False] indica que a primeira coluna
                estará númerada.

            -> prompt
                string a ser printada no final da tabela.
                Caso nada seja fornecido, nada será apresentado.

        Retorno:
            -> index
                Index global de um item selecionado, relativo à tabela.

            -> col_index
                Index Local do item selecionado, relativo à coluna.

            -> coluna
                Número da coluna do item selecionado, começa com 0.

            Se 'enumerando' ou 'prompt' são None, None é retornado.
        """

        numero_de_colunas = len(dados)

        # Estabelecendo o default
        if alinhamento is None:
            alinhamento = [
                "<" for _ in range(numero_de_colunas)
            ]

        if comprimento_das_colunas is None:
            comprimento_das_colunas: list = [
                0 for _ in range(numero_de_colunas)
            ]

        if enumerando is None:
            enumerando = [
                False for _ in range(numero_de_colunas)
            ]
            qualquer_numeracao = False
        else:
            qualquer_numeracao = True

        if margens is None:
            margens = [
                2 for _ in range(numero_de_colunas)
            ]

        #######################################################

        def criar_tabela_e_estabelecer_comprimento_de_colunas():
            subcol: list[list] = []

            for index_de_coluna in range(
                    numero_de_colunas
            ):
                subcol.append(
                    [
                        # Sujeito a alterações?
                        [] for _ in range(len(dados[0]))
                    ]
                )

                # Somente caso a coluna esteja setada para ter o
                # tamanho MÍNIMO necessário, executará este if.
                if comprimento_das_colunas[
                    index_de_coluna
                ] == 0:
                    # Segue um algoritmo para determinar o tamanho mínimo.

                    comp_adicional_devido_numeracao = 4 if enumerando[index_de_coluna] else 0

                    if max_num_de_colunas_por_titulo is None or max_num_de_colunas_por_titulo[index_de_coluna] < 2:

                        comprimento_das_colunas[
                            index_de_coluna
                        ] = max(
                            [
                                len(str(item)) + comp_adicional_devido_numeracao for item in dados[index_de_coluna]
                            ]
                        ) + 2 * margens[index_de_coluna]

                    else:

                        subcol[
                            index_de_coluna
                        ][
                            0
                        ] = ceil(
                            len(dados[index_de_coluna]) / max_num_de_colunas_por_titulo[index_de_coluna]
                        )

                        max_num_de_colunas_por_titulo[
                            index_de_coluna
                        ] = ceil(
                            len(dados[index_de_coluna]) / subcol[
                                index_de_coluna
                            ][
                                0
                            ]
                        )

                        comprimento_das_colunas[
                            index_de_coluna
                        ] = margens[
                                index_de_coluna
                            ] * (1 + max_num_de_colunas_por_titulo[
                            index_de_coluna
                        ]) - (
                                1 if enumerando[index_de_coluna] else 0
                            )

                        for quant_de_subcol_dentro_do_titulo in range(
                                max_num_de_colunas_por_titulo[
                                    index_de_coluna
                                ]
                        ):
                            comprimento_da_subcoluna = max(
                                [
                                    len(
                                        str(item)
                                    ) + comp_adicional_devido_numeracao for item in dados[
                                                                                        index_de_coluna
                                                                                    ][
                                                                                    quant_de_subcol_dentro_do_titulo * subcol[index_de_coluna][0]: (quant_de_subcol_dentro_do_titulo + 1) *
                                                                                                                                                   subcol[index_de_coluna][0]
                                                                                    ]
                                ]
                            )

                            comprimento_das_colunas[
                                index_de_coluna
                            ] += comprimento_da_subcoluna

                            subcol[
                                index_de_coluna
                            ][
                                1
                            ].append(
                                comprimento_da_subcoluna
                            )

                    if titulos is not None:
                        comprimento_das_colunas[
                            index_de_coluna
                        ] = max(
                            comprimento_das_colunas[index_de_coluna],
                            len(titulos[index_de_coluna]) + 2 * margens[index_de_coluna]
                        )

            return subcol, comprimento_das_colunas

        subcolunas, comprimento_das_colunas = criar_tabela_e_estabelecer_comprimento_de_colunas()
        #######################################################

        # Se não colocamos numeração, devemos providenciar.
        numero_de_itens = 0
        local_index = [0]
        if qualquer_numeracao:
            for index_de_coluna in range(
                    numero_de_colunas
            ):

                assert isinstance(dados[index_de_coluna], list), "É obrigatório que seja uma lista de lista!."

                if enumerando[
                    index_de_coluna
                ]:
                    dados[
                        index_de_coluna
                    ] = [
                        f"{index_da_linha + numero_de_itens : 3} - {linha_de_dados}" for index_da_linha, linha_de_dados in enumerate(
                            dados[
                                index_de_coluna
                            ]
                        )
                    ]

                    numero_de_itens += len(
                        dados[index_de_coluna]
                    )

                local_index.append(
                    numero_de_itens
                )

        #######################################################

        comprimento_da_tabela = sum(comprimento_das_colunas) + numero_de_colunas - 1

        # Títulos das Colunas

        print("=" * comprimento_da_tabela)

        if titulos is not None:
            for index_de_coluna in range(
                    numero_de_colunas
            ):
                print(
                    f"{titulos[index_de_coluna]:^{comprimento_das_colunas[index_de_coluna]}}", end="|" if index_de_coluna < (numero_de_colunas - 1) else ''
                )
            print()  # Com isso, pulamos apenas uma linha.

            for index_de_coluna in range(
                    numero_de_colunas
            ):
                print(
                    "-" * comprimento_das_colunas[index_de_coluna], end="+" if index_de_coluna < (numero_de_colunas - 1) else ""
                )
            print()

        #######################################################

        if max_num_de_colunas_por_titulo is not None:

            for index_de_coluna, coluna in enumerate(dados):
                if max_num_de_colunas_por_titulo[
                    index_de_coluna
                ] < 2:
                    # Se for apenas uma subcoluna para cada título,
                    # então oq estamos prestes a fazer é inútil.
                    continue

                for indice_perigoso in range(
                        # Caso exista max_num_de_colunas_por_titulo, subcolunas não será uma lista de listas vazias.
                        # Ter-se-á números específicos para o algoritmo seguinte.
                        # Tente verificar a linha 332 para mais explicações
                        subcolunas[
                            index_de_coluna
                        ][0]
                ):
                    # Esse é um superalgoritmo para anexar subcolunas em um único título
                    # Entrada: ['  0 - John', '  1 - Graciete']
                    coluna[
                        indice_perigoso
                    ] = (" " * margens[index_de_coluna]).join(
                        f"{coluna[item]:{alinhamento[index_de_coluna]}{subcolunas[index_de_coluna][1][subcol_idx]}}" for subcol_idx, item in enumerate(  # Esse enumarate é bizarro mesmo, cuidado.
                            range(
                                indice_perigoso,
                                len(coluna),
                                subcolunas[index_de_coluna][0]
                            )
                        )
                    )

                # Saída: ['  0 - John     1 - Graciete', '  1 - Graciete']
                # Note que agora há elementos repetidos.
                # Vamos retirá-los
                del coluna[subcolunas[index_de_coluna][0]:]

        #######################################################

        """
        Diferença entre:
        
        zip -> Para quando chegamos no array mais curto.
        
        zip_longest -> Para quando chegamos ao final do array mais longo,
                       preenchendo os elementos faltantes do array menor
                       com algo desejado. (fillvalue='')
        """
        for linha in zip_longest(
                *dados,
                fillvalue=""
        ):
            """
            Sem o *: 
                (['  0 - John', '  1 - Graciete'],)
                ([10, 5],)
            
            Com o *:
                ('  0 - John', 10)
                ('  1 - Graciete', 5)
            """

            for index, item in enumerate(
                    linha
            ):
                comp_margem = margens[
                                  index
                              ] - 2 if enumerando[index] else margens[index]

                # Não usamos o método .center devido às possíveis numerações
                item = f"{' ' * comp_margem}{item}{' ' * margens[index]}"
                print(
                    f"{item:{alinhamento[index]}{comprimento_das_colunas[index]}}",
                    end=""
                )

                if index < (numero_de_colunas - 1):
                    print(end="|")

            print(end="\n")

        print("=" * comprimento_da_tabela)

        #######################################################

        if prompt is None:
            return None

        if not qualquer_numeracao:
            print(prompt)
            return None

        index = UserInterface.ler_inteiro(
            prompt, 0, numero_de_itens
        )

        for i, n in enumerate(
                local_index
        ):
            if index < n:
                # Rapaz, não consigo entender porque essas informações são úteis.
                return index, index - local_index[i - 1], i - 1

        # Se chegar até aqui
        raise ValueError(
            "Falha em pegar entrada ilegal."
        )

    @staticmethod
    def apresentar_lista(
            dados: list,
            enumerando=True,
            prompt=None,
            divisor=" | ",
            alinhamento="<",
            min_por_col=6
    ):
        """
        Descrição:
            Apresenta uma lista usando a maior quantidade de
            colunas possível

        Parâmetros:
            Veja os parâmetros de apresentar_tabela.

        Retorno:
            Index do elemento e o elemento selecionado.
        """

        comprimento_do_terminal = get_terminal_size()[0]  # Do shutil
        quantidade_de_itens = len(dados)
        comp_divisor = len(divisor)
        itens = []
        comprimento_dos_itens = []

        ##########################################################

        for index in range(
                quantidade_de_itens
        ):
            numeracao = f"{index}-" if enumerando else ""

            itens.append(
                f"{divisor}{numeracao}{dados[index]}"
            )

            comprimento_dos_itens.append(
                len(
                    itens[-1]
                )
            )

        """
        clip é usado para limitar os valores do array dentro de um intervalo especificado.
        É fato que não é um array, mas mesmo assim o clip faz a limitação
        da melhor forma possível.
        """
        maximo_de_colunas = clip(
            (comprimento_do_terminal + comp_divisor) // min(comprimento_dos_itens),
            1,
            ceil(quantidade_de_itens / max(min_por_col, 1))
        )

        #################################################################################

        # Algoritmo para deixar tudo alinhado da melhor forma,

        comp_da_tabela = 0
        ultimo_valor_assumido = 0
        coluna_dos_itens = []
        comp_das_colunas = []
        for i in range(
                maximo_de_colunas,
                0,
                -1  # Decrescente, vê?
        ):
            comp_das_colunas.clear()
            coluna_dos_itens.clear()  # Deve estar zerado sempre iniciar um novo loop

            a, b = divmod(quantidade_de_itens, i)
            # a = quantidade_de_itens // i
            # b = quantidade_de_itens % i

            for coluna in range(
                    i
            ):
                start = a * coluna + min(b, coluna)
                end = start + a + (1 if coluna < b else 0)

                coluna_dos_itens.append(
                    itens[start:end]
                )

                max_comp_de_uma_coluna = max(comprimento_dos_itens[start:end])
                comp_das_colunas.append(
                    max_comp_de_uma_coluna
                )

                comp_da_tabela += max_comp_de_uma_coluna

            ultimo_valor_assumido = i

            if comp_da_tabela <= comprimento_do_terminal + comp_divisor:
                break

        comp_da_tabela -= comp_divisor

        ###############################################################################

        print("=" * comp_da_tabela)

        for linha in range(
                ceil(
                    quantidade_de_itens / ultimo_valor_assumido
                )
        ):
            for coluna in range(
                    ultimo_valor_assumido
            ):
                # Caso não tenha item, vamos printar o divisor.
                conteudo_a_ser_printado = coluna_dos_itens[
                    coluna
                ][
                    linha
                ] if len(coluna_dos_itens[coluna]) > linha else divisor

                if coluna == 0:
                    # A primeira coluna não deve ter o divisor?
                    print(
                        end=f"{conteudo_a_ser_printado[comp_divisor:]:{alinhamento}{comp_das_colunas[coluna] - 1}}"
                    )
                else:
                    print(
                        end=f"{conteudo_a_ser_printado:{alinhamento}{comp_das_colunas[coluna] - 1}}"
                    )

            print()

        print("=" * comp_da_tabela)

        ###############################################################

        if prompt is None:
            return None

        if enumerando is None:
            return None

        idx = UserInterface.ler_inteiro(
            prompt, 0, quantidade_de_itens
        )

        return idx, dados[idx]


if __name__ == '__main__':
    # Testes Unitários? Devem ser providenciados no futuro.

    # exemplo = UserInterface()

    # exemplo.apresentar_tabela(
    #     dados=[
    #         ["John", "Graciete", "Galvão"],
    #         [30, 29, 50],
    #         ["SP", "SP", "CE"]
    #     ],
    #     titulos=["Nome", "Idade", "Cidade"],
    #     alinhamento=["<", "^", ">"],
    #     comprimento_das_colunas=[0, 0, 0],
    #     margens=[3, 3, 2],
    #     enumerando=[True, False, False],
    #     prompt="Escolha alguém:"
    # )

    # exemplo.apresentar_lista(
    #     dados=[
    #         "Lima",
    #         "Salgados",
    #         "SegFem"
    #     ]
    # )

    pass
