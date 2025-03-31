"""
Matheus Deyvisson, 2025

Observe que este código não é reutilizado em momento algum.
Servindo apenas para a criação do arquivo obtendo_possibilidades.h,
o qual será amplamente utilizado pelo algoritmo A*.

Logo, o desempenho neste código não importa tanto, pois ele
será executado apenas uma vez.

Objetivo:
    Possibilitar ao algoritmo cpp como varrer as proximidades
    do agente.
"""

from os import getcwd
from os.path import join, dirname
from math import sqrt

"""
Esses números não são aleatórios. Trata-se da quantidade
de linhas e colunas do campo.

Alterar os seguintes parâmetros afetará substancialmente 
a perfomance:

MAIOR_DISTANCIA -> Quanto de área devemos cobrir.
PRECISAO_DE_PIXELS -> Incremento de cada ponto que será considerado.
"""
MAIOR_DISTANCIA = 5
LINHAS = 321
COLUNAS = 221
PRECISAO_DE_PIXELS = 1
PRECISAO_NUMERICA = 6

caminho = join(
    getcwd(),
    dirname(__file__)
)


def criacao_da_tabela_de_possibilidades() -> tuple[dict, list, int]:
    """
    Descrição:
        Implementação do método para varrermos a área circular
        ao redor do agente.

        Na minha cabeça, poderia ser melhor implementado.

    Retorno:
        Possibilidades_:
            Dicionário com distâncias de um ponto (klinha, kcoluna)
            sendo a chave e lista de tuplas (klinha, kcoluna) que representam
            possibilidades de gerar essa distância.

            Essencialmente, como se fosse os aneis de um disco.

        Distâncias_crescentes_:
            Lista com os raios dos aneis de forma crescente.

        Quantidade_de_possibilidades_:
            inteiro que representa quantidade de anéis.

    """

    possibilidades_ = {}

    for linha in range(
            1 + pow(10, PRECISAO_DE_PIXELS) * MAIOR_DISTANCIA
    ):
        for coluna in range(
                1 + pow(10, PRECISAO_DE_PIXELS) * MAIOR_DISTANCIA
        ):

            distancia = sqrt(
                linha * linha + coluna * coluna
            ) * pow(10, - PRECISAO_DE_PIXELS)

            distancia = round(
                distancia,
                PRECISAO_NUMERICA
            )
            # Observe que estamos iterando usando range, forçando o uso de inteiros.
            # Dito isso, devemos lembrar de dividir por 10, para mantermos os floats.

            if distancia <= MAIOR_DISTANCIA:
                if distancia not in possibilidades_:
                    possibilidades_[
                        distancia
                    ] = []

                possibilidades_[
                    distancia
                ].append(
                    (
                        linha,
                        coluna
                    )
                )

                # Adicionamos elementos espelhados que terão a mesma distância.
                if coluna > 0:
                    possibilidades_[
                        distancia
                    ].append(
                        (
                            linha,
                            -coluna
                        )
                    )

                if linha > 0:
                    possibilidades_[
                        distancia
                    ].append(
                        (
                            -linha,
                            coluna
                        )
                    )

                if linha > 0 and coluna > 0:
                    possibilidades_[
                        distancia
                    ].append(
                        (
                            -linha,
                            -coluna
                        )
                    )

    distancias_crescentes_ = sorted(
        possibilidades_
    )
    quantidade_de_possibilidades_ = len(distancias_crescentes_)

    return possibilidades_, distancias_crescentes_, quantidade_de_possibilidades_


aneis_e_seus_pontos, raio_dos_aneis_crescentes, quantidade_de_aneis = criacao_da_tabela_de_possibilidades()


def criacao_de_header_para_cpp() -> list[str]:
    """
    Descrição:
        Cria lista de strings que representará o header usado pelo algoritmo.
        Este Header representará uma forma mais otimizada de se obter distâncias e posições.
    """

    """
    Observe que conforme alteramos o parâmetro de precisão,
    essa quantidade cresce absurdamente.
    
    Para MAIOR_DISTANCIA = 5:
    
    PRECISÃO = 1 -> 317
    PRECISAO = 2 -> 31417
    PRECISAO = 3 -> maior que 3 milhões.
    """
    quantidade_de_pontos_totais_dentro_do_disco_de_possibilidades = sum(
        len(pontos_em_anel) for pontos_em_anel in aneis_e_seus_pontos.values()
    )

    lista_de_strings_para_header = [
        f"const int quantidade_de_pontos_disponiveis = {quantidade_de_pontos_totais_dentro_do_disco_de_possibilidades};\n\n",
        f"const float aneis_e_pontos_disponiveis[{quantidade_de_pontos_totais_dentro_do_disco_de_possibilidades}] = {{"  # Esse {{ é para conseguirmos colocar o { na string.
    ]

    respectivas_distancias = []
    respectivas_linhas = []
    respectivas_colunas = []
    for distancia in raio_dos_aneis_crescentes:
        for ponto in aneis_e_seus_pontos[distancia]:
            respectivas_distancias.append(
                distancia
            )

            respectivas_linhas.append(
                ponto[0]
            )

            respectivas_colunas.append(
                ponto[1]
            )

    respectivas_distancias = str(respectivas_distancias).replace(
        "[",
        ""
    ).replace(
        "]",
        ""
    )

    respectivas_linhas = str(respectivas_linhas).replace(
        "[",
        ""
    ).replace(
        "]",
        ""
    )

    respectivas_colunas = str(respectivas_colunas).replace(
        "[",
        ""
    ).replace(
        "]",
        ""
    )

    lista_de_strings_para_header.append(
        f"{respectivas_distancias}}};\n\nconst int linhas_de_cada_ponto[{quantidade_de_pontos_totais_dentro_do_disco_de_possibilidades}] = {{"
    )
    lista_de_strings_para_header.append(
        f"{respectivas_linhas}}};\n\nconst int colunas_de_cada_ponto[{quantidade_de_pontos_totais_dentro_do_disco_de_possibilidades}] = {{"
    )
    lista_de_strings_para_header.append(
        f"{respectivas_colunas}}};\n"
    )

    return lista_de_strings_para_header


colecao_do_header = criacao_de_header_para_cpp()

with open(
    join(
        caminho,
        "obtendo_possibilidades.h"
    ),
    "w"
) as arq:
    arq.write(
        ''.join(
            colecao_do_header
        )
    )
