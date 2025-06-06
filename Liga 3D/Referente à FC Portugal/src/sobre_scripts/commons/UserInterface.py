# Desenvolvido por:
#   Matheus Deyvisson, 2025
#
# Descrição:
#       Código responsável por prover ferramentas de visualização
#       em terminal

from itertools import zip_longest
from math import ceil
from numpy import clip
from shutil import get_terminal_size
from typing import Callable

from prettytable import PrettyTable


class UserInterface:
    """
    Descrição:
        Classe responsável por criar uma interface de interação
        do usuário com o terminal amigável, com
        tratamento de erros e formatação automática.

        Inclui funcionalidades interessantes como:



        A ideia é não haver reutilização de partes do código, logo
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
    def obter_entrada(
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
                Somente para tipos numéricos.

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
            * O argumento padrão mutável torna o código sucetível a erros.
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
    def obter_inteiro(
            prompt: str,
            minimo: int,
            maximo: int
    ) -> int:
        """
        Descrição:
            Solicita ao usuário um número inteiro em um intervalo específico.

        Parâmetros:
            prompt (str):
                Mensagem exibida ao usuário no terminal antes da entrada.

            minimo (int):
                Limite inferior do intervalo (inclusivo).

            maximo (int):
                Limite superior do intervalo (exclusivo).

        Retorno:
            int:
                O valor inteiro fornecido pelo usuário que está no intervalo [minimo, maximo).
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

    # Vamos desenvolver novas funcionalidades com PrettyTable
    @staticmethod
    def apresentar_tabela(
            dados: list[list],
            titulos: [str] = None,
            alinhamento: list[str] = None,
            numbering: list[bool] = None,
            prompt: str = None
    ):
        """
        Descrição:
            Exibe uma tabela formatada no terminal usando a biblioteca PrettyTable.
            Ainda há a versão original chamada 'print_table', pois ela permite
            subcolunas, enquanto esta não.

        Parâmetros:
            data: list[list[str]]
                Representa o conteúdo da tabela.

                Cada sublista representa uma coluna completa da tabela,
                onde cada elemento da sublista é uma célula daquela coluna.

                Ou seja:
                    - `data[0]` → coluna 0
                    - `data[1]` → coluna 1
                    - ...
                    - `zip(*data)` monta as linhas da tabela automaticamente.

                Exemplo simples:
                    data = [
                        ["Ana", "João", "Maria"],       # Coluna 0 (nomes)
                        ["25", "30", "22"],             # Coluna 1 (idades)
                        ["Engenheira", "Designer", "Professora"] # Coluna 2
                    ]

                Isso será exibido como:

                    +--------+-------+-------------+
                    | Nome   | Idade | Profissão   |
                    +--------+-------+-------------+
                    | Ana    | 25    | Engenheira  |
                    | João   | 30    | Designer    |
                    | Maria  | 22    | Professora  |
                    +--------+-------+-------------+

            titles: list[str], opcional
                Nomes das colunas. Deve ter o mesmo número de elementos que `data`.

            alignment: list[str], opcional
                Alinhamento de cada coluna:
                    - "<" para alinhamento à esquerda
                    - ">" para alinhamento à direita
                    - "^" para centralizado

                Exemplo:
                    alignment = ["<", ">", "^"]

            numbering: list[bool], opcional
                Lista de flags indicando se a coluna deve ser numerada automaticamente (para seleção).
                Exemplo:
                    numbering = [True, False, False]

                A numeração será exibida no formato:
                    "0-Valor", "1-Valor", etc.

            prompt: str, opcional
                Mensagem exibida ao usuário para selecionar um item numerado da tabela.

                Se fornecido, e se alguma coluna estiver com `numbering = True`,
                o usuário poderá digitar o índice do item desejado.

        Retorno:
            tuple[int, int, int] ou None
                Se o `prompt` for fornecido e houver colunas numeradas,
                retorna uma tupla com:

                    (índice global, índice local na coluna, índice da coluna)

                Caso contrário, retorna None.
        """

        numero_de_colunas = len(dados)

        table = PrettyTable()

        if titulos:
            table.field_names = titulos
        else:
            table.field_names = [f"Col {i}" for i in range(numero_de_colunas)]

        if numbering is None:
            any_numbering = False
        else:
            any_numbering = True

        if alinhamento is None:
            alinhamento = ["^" for _ in range(numero_de_colunas)]

        acumulativo_de_itens_por_col = [0]
        if any_numbering:
            no_of_items = 0
            for i in range(numero_de_colunas):
                if numbering[i]:
                    dados[i] = [f"{n + no_of_items} - {d}" for n, d in enumerate(dados[i])]
                    no_of_items += len(dados[i])
                acumulativo_de_itens_por_col.append(no_of_items)

        # Transformará as informações que estão em colunas
        # para linhas, exemplo:
        # [['John', 'Graciete'], [30, 50]]
        # [('John', 30), ('Graciete', 50)]         -> Sem numbering
        # [('0  -John', 30), ('1  -Graciete', 50)] -> Com numbering
        linhas = list(zip(*dados))

        for linha in linhas:
            table.add_row(linha)

        # Alinhamento por coluna
        if alinhamento:
            for i, align in enumerate(alinhamento):
                table.align[
                    table.field_names[i]
                ] = {"<": "l", "^": "c", ">": "r"}.get(align, "l")

        print(table)

        # Prompt de seleção
        if prompt and any_numbering:
            index = int(input(prompt))
            for i, n in enumerate(acumulativo_de_itens_por_col):
                if index < n:
                    return index, index - acumulativo_de_itens_por_col[i - 1], i - 1

        return None

    # @staticmethod
    # def print_table(
    #         data,
    #         titles=None,
    #         alignment=None,
    #         cols_width=None,
    #         cols_per_title=None,
    #         margins=None,
    #         numbering=None,
    #         prompt=None
    # ) -> tuple[int, int, int] | None:
    #
    #     cols_no = len(data)
    #
    #     if alignment is None:
    #         alignment = ["<"] * cols_no
    #
    #     if cols_width is None:
    #         cols_width = [0] * cols_no
    #
    #     if numbering is None:
    #         numbering = [False] * cols_no
    #         any_numbering = False
    #     else:
    #         any_numbering = True
    #
    #     if margins is None:
    #         margins = [2] * cols_no
    #
    #     # Fit column to content + margin, if required
    #     subcol = []  # subcolumn length and widths
    #     for i in range(cols_no):
    #         subcol.append([[], []])
    #         if cols_width[i] == 0:
    #             numbering_width = 4 if numbering[i] else 0
    #             if cols_per_title is None or cols_per_title[i] < 2:
    #                 cols_width[i] = max([len(str(item)) + numbering_width for item in data[i]]) + margins[i] * 2
    #             else:
    #                 subcol[i][0] = ceil(len(data[i]) / cols_per_title[i])  # subcolumn maximum length
    #                 cols_per_title[i] = ceil(len(data[i]) / subcol[i][0])  # reduce number of columns as needed
    #                 cols_width[i] = margins[i] * (1 + cols_per_title[i]) - (1 if numbering[i] else 0)  # remove one if numbering, same as when printing
    #                 for j in range(cols_per_title[i]):
    #                     subcol_data_width = max([len(str(item)) + numbering_width for item in data[i][j * subcol[i][0]:j * subcol[i][0] + subcol[i][0]]])
    #                     cols_width[i] += subcol_data_width  # add subcolumn data width to column width
    #                     subcol[i][1].append(subcol_data_width)  # save subcolumn data width
    #
    #             if titles is not None:  # expand to acomodate titles if needed
    #                 cols_width[i] = max(cols_width[i], len(titles[i]) + margins[i] * 2)
    #
    #     if any_numbering:
    #         no_of_items = 0
    #         cumulative_item_per_col = [0]  # useful for getting the local index
    #         for i in range(cols_no):
    #             assert type(data[i]) == list, "In function 'print_table', 'data' must be a list of lists!"
    #
    #             if numbering[i]:
    #                 data[i] = [f"{n + no_of_items:3}-{d}" for n, d in enumerate(data[i])]
    #                 no_of_items += len(data[i])
    #             cumulative_item_per_col.append(no_of_items)
    #
    #     table_width = sum(cols_width) + cols_no - 1
    #
    #     # --------------------------------------------- col titles
    #     print(f'{"=" * table_width}')
    #     if titles is not None:
    #         for i in range(cols_no):
    #             print(f'{titles[i]:^{cols_width[i]}}', end='|' if i < cols_no - 1 else '')
    #         print()
    #         for i in range(cols_no):
    #             print(f'{"-" * cols_width[i]}', end='+' if i < cols_no - 1 else '')
    #         print()
    #
    #     # --------------------------------------------- merge subcolumns
    #     if cols_per_title is not None:
    #         for i, col in enumerate(data):
    #             if cols_per_title[i] < 2:
    #                 continue
    #             for k in range(subcol[i][0]):  # create merged items
    #                 col[k] = (" " * margins[i]).join(f'{col[item]:{alignment[i]}{subcol[i][1][subcol_idx]}}'
    #                                                  for subcol_idx, item in enumerate(range(k, len(col), subcol[i][0])))
    #             del col[subcol[i][0]:]  # delete repeated items
    #
    #     # --------------------------------------------- col items
    #     for line in zip_longest(*data):
    #         for i, item in enumerate(line):
    #             l_margin = margins[i] - 1 if numbering[i] else margins[i]  # adjust margins when there are numbered options
    #             item = "" if item is None else f'{" " * l_margin}{item}{" " * margins[i]}'  # add margins
    #             print(f'{item:{alignment[i]}{cols_width[i]}}', end='')
    #             if i < cols_no - 1:
    #                 print(end='|')
    #         print(end="\n")
    #     print(f'{"=" * table_width}')
    #
    #     # --------------------------------------------- prompt
    #     if prompt is None:
    #         return None
    #
    #     if not any_numbering:
    #         print(prompt)
    #         return None
    #
    #     index = UserInterface.obter_inteiro(prompt, 0, no_of_items)
    #
    #     for i, n in enumerate(cumulative_item_per_col):
    #         if index < n:
    #             return index, index - cumulative_item_per_col[i - 1], i - 1
    #
    #     raise ValueError('Failed to catch illegal input')

    @staticmethod
    def apresentar_lista(
            data: list[str],
            numbering=True,
            prompt=None,
            divider=" | ",
            alignment="<",
            min_per_col=6
    ) -> tuple[int, str] | None:
        """
        Descrição:
            Imprime uma lista de itens organizados em colunas, ajustadas à largura do terminal,
            utilizando PrettyTable.
            Ainda está mantido a versão original, 'print_list'.

        Parâmetros:
            data: list[str]
                Lista de strings a serem impressas.

            Numbering: bool, padrão=True
                Se True, cada item será numerado (ex: "0 - Item").

            Prompt: str, opcional
                Se fornecido, solicita ao usuário que selecione um dos itens numerados.

            divider: str, padrão=" | "
                Separador entre colunas.

            alignment: str, padrão="<"
                Alinhamento dos itens nas colunas:
                    "<" = esquerda, ">" = direita, "^" = centralizado

            min_per_col: int, padrão=6
                Número mínimo de itens por coluna (controle da densidade vertical).

        Retorno:
            tuple[int, str] | None
                Retorna índice e valor do item selecionado, se `prompt` for fornecido.
        """

        comprimento = get_terminal_size().columns
        quantidade_de_dados = len(data)

        # Adiciona numeração se necessário
        itens_formatados = [
            f"{i}-{item}" if numbering else item
            for i, item in enumerate(data)
        ]

        itens_len = list(map(len, itens_formatados))

        # Determina o número máximo de colunas baseado na largura do terminal
        max_colunas = min(
            (comprimento + len(divider)) // (min(itens_len) + len(divider)),
            ceil(quantidade_de_dados / min_per_col)
        )
        max_colunas = max(max_colunas, 1)

        # Reorganiza os dados em colunas
        # (cada coluna será uma lista)
        altura_da_lista = ceil(quantidade_de_dados / max_colunas)
        columns = [[] for _ in range(max_colunas)]

        for i, item in enumerate(itens_formatados):
            col_idx = i // altura_da_lista
            columns[col_idx].append(item)

        # Ajusta o número de linhas por coluna (com padding)
        for col in columns:
            while len(col) < altura_da_lista:
                col.append("")

        table = PrettyTable()
        table.header = False
        table.align = {"<": "l", "^": "c", ">": "r"}.get(alignment, "l")

        for row in zip(*columns):
            table.add_row(row)

        print(table)

        if prompt is not None and numbering:
            while True:
                try:
                    index = int(input(prompt))
                    if 0 <= index < len(data):
                        return index, data[index]
                    else:
                        print(f"Erro: escolha um número entre 0 e {len(data) - 1}")
                except ValueError:
                    print("Entrada inválida! Digite um número válido.")

        return None

    # @staticmethod
    # def print_list(data, numbering=True, prompt=None, divider=" | ", alignment="<", min_per_col=6):
    #
    #     WIDTH = get_terminal_size()[0]
    #
    #     data_size = len(data)
    #     items = []
    #     items_len = []
    #
    #     # --------------------------------------------- Add numbers, margins and divider
    #     for i in range(data_size):
    #         number = f"{i}-" if numbering else ""
    #         items.append(f"{divider}{number}{data[i]}")
    #         items_len.append(len(items[-1]))
    #
    #     max_cols = clip((WIDTH + len(divider)) // min(items_len), 1, ceil(data_size / max(min_per_col, 1)))  # width + len(divider) because it is not needed in last col
    #
    #     # --------------------------------------------- Check maximum number of columns, considering content width (min:1)
    #     for i in range(max_cols, 0, -1):
    #         cols_width = []
    #         cols_items = []
    #         table_width = 0
    #         a, b = divmod(data_size, i)
    #         for col in range(i):
    #             start = a * col + min(b, col)
    #             end = start + a + (1 if col < b else 0)
    #             cols_items.append(items[start:end])
    #             col_width = max(items_len[start:end])
    #             cols_width.append(col_width)
    #             table_width += col_width
    #         if table_width <= WIDTH + len(divider):
    #             break
    #     table_width -= len(divider)
    #
    #     # --------------------------------------------- Print columns
    #     print("=" * table_width)
    #     for row in range(ceil(data_size / i)):
    #         for col in range(i):
    #             content = cols_items[col][row] if len(cols_items[col]) > row else divider  # print divider when there are no items
    #             if col == 0:
    #                 l = len(divider)
    #                 print(end=f"{content[l:]:{alignment}{cols_width[col] - l}}")  # remove divider from 1st col
    #             else:
    #                 print(end=f"{content    :{alignment}{cols_width[col]}}")
    #         print()
    #     print("=" * table_width)
    #
    #     # --------------------------------------------- Prompt
    #     if prompt is None:
    #         return None
    #
    #     if numbering is None:
    #         return None
    #     else:
    #         idx = UserInterface.obter_inteiro(prompt, 0, data_size)
    #         return idx, data[idx]


if __name__ == '__main__':
    # Desenvolvimento de testes unitários para cada método.
    # UserInterface.obter_inteiro("Informe inteiro: ", 5, 10)

    # data = [
    #     ["Pizza", "Lasanha", "Hambúrguer"],
    #     ["R$40", "R$35", "R$20"]
    # ]
    # titles = ["Prato", "Preço"]
    # alignment = ["<", ">"]
    # numbering = [True, False]
    # prompt = "Choose a person:"
    # print(UserInterface.apresentar_tabela(data, titles, alignment, numbering, prompt))

    # frutas = [
    #     "Maçã", "Banana", "Laranja", "Uva", "Pera",
    #     "Abacaxi", "Melancia", "Manga", "Morango", "Kiwi",
    #     "Limão", "Goiaba", "Jabuticaba", "Cereja"
    # ]
    # idx, fruta = UserInterface.apresentar_lista(frutas, prompt="Escolha a fruta pelo número: ")

    pass
