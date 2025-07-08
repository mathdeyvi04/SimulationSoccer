from os import listdir, getcwd
from os.path import isfile, join as join_path

import argparse


def main(
        conj_de_caract_a_ser_buscado: str,
        diretorio_raiz: str = None,
        conj_de_pastas_a_serem_desconsideradas: set = None,
        conj_de_extensoes_a_serem_desconsiderados: set = None
) -> None:
    """
    Descrição:
        Automação responsável por buscar em todos os arquivos referências dos caracteres
        que foram introduzidos.

        A função varre todos os caminhos possíveis presentes no diretório raiz.

    Parâmetros:
        conj_de_caract_a_ser_buscado (str):
            String a ser buscada dentro dos arquivos.

        diretorio_raiz (str):
            Local onde se deseja realizar a busca, padrão é ser o atual de execução.

        conj_de_pastas_a_serem_desconsideradas (set):
            Pastas que não deverão ser abertas para varredura.
            Padrão: {".git", "__pycache__", "..", "."}

        conj_de_extensoes_a_serem_desconsideradas (set):
            Extensões que não deverão nem ser abertas para leitura:
            Padrão: {".exe", ".png", ".mp3"}

    Retorno:
        - Apresentação de quais caminhos de arquivos possuem a referência.
        - Quantas vezes cada caminho de arquivo possue a referência.
        - Uma janela de visualização do texto próximo que contém a referência.
    """

    # É mais inteligente que varramos todos os caminhos possíveis e então façamos
    # as buscas dentro de cada arquivo aberto.

    conj_de_pastas_a_serem_desconsideradas_default = {".git", "__pycache__", "..", "."}
    if conj_de_pastas_a_serem_desconsideradas is None:
        conj_de_pastas_a_serem_desconsideradas = conj_de_pastas_a_serem_desconsideradas_default
    else:
        # Garantia que não haverá elementos repetidos.
        conj_de_pastas_a_serem_desconsideradas.update(conj_de_pastas_a_serem_desconsideradas_default)

    conj_de_extensoes_a_serem_desconsiderados_default = {
        ".exe", ".png", ".mp3"
    }
    if conj_de_extensoes_a_serem_desconsiderados is None:
        conj_de_extensoes_a_serem_desconsiderados = conj_de_extensoes_a_serem_desconsiderados_default
    else:
        conj_de_extensoes_a_serem_desconsiderados.update(conj_de_extensoes_a_serem_desconsiderados_default)

    if diretorio_raiz is None:
        diretorio_raiz = getcwd()
    else:
        # Estaremos recebendo um caminho pelo do Windows e queremos
        # usá-lo dentro do Linux.
        diretorio_raiz = diretorio_raiz.replace("\\", "/").replace("C:", "/mnt/c")

    for caminho in listdir(diretorio_raiz):

        # Para poupar erros inúteis, devemos filtrar as pastas e arquivos que serão buscados.
        if caminho in conj_de_pastas_a_serem_desconsideradas or \
                caminho[caminho.rfind('.'):] in conj_de_extensoes_a_serem_desconsiderados or \
                caminho.startswith("word_finder"):
            continue

        # Vamos ficar com o caminho direto
        caminho = join_path(diretorio_raiz, caminho)

        if isfile(caminho):
            # Como é um arquivo, podemos abri-lo.

            try:
                with open(
                        caminho,
                        "r",
                        encoding="utf-8"
                ) as arquivo_aberto:
                    texto_completo = arquivo_aberto.read()

                    # Se chegou até aqui, deu certo.
                    index = 0
                    while True:

                        index = texto_completo.find(
                            conj_de_caract_a_ser_buscado,
                            index
                        )

                        # Não encontramos mais
                        if index == -1:
                            break

                        # Caso passe, encontramos.
                        print(f"\033[1;44mDentro de {caminho} obtive:\033[0m\n")

                        # Devemos realizar cortes agora.
                        print(texto_completo[index - 50: index + 50].replace("\n", "\\n"))
                        print()
                        # Só para diminuirmos o tamanho da string completa
                        index += len(conj_de_caract_a_ser_buscado)

            except UnicodeDecodeError as error:
                # print(f"\033[1;41m Para {caminho}, obtive {error} \033[0m")
                pass

        else:
            # Vamos considerar que tudo que não é um arquivo, é uma pasta.

            main(conj_de_caract_a_ser_buscado, join_path(caminho))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "conj_de_caract_a_ser_buscado",
        type=str,
        help="String a ser buscada dentro dos arquivos."
    )
    parser.add_argument(
        "--dir_raiz",
        type=str,
        default=None,
        help="Local onde se deseja realizar a busca, padrão é ser o atual de execução."
    )
    parser.add_argument(
        "--pastas_ignoradas",
        type=str,
        default=None,
        help="Set de pastas que deverão ser ignoradas. Padrão é que sejam: {'.git', '__pycache__', '..', '.'}"
    )
    parser.add_argument(
        "--extensoes_ignoradas",
        type=str,
        default=None,
        help="Set de extensões que deverão ser ignoradas. Padrão é que sejam: {'.exe', '.png', '.mp3'}"
    )

    args = parser.parse_args()

    main(
        args.conj_de_caract_a_ser_buscado,
        args.dir_raiz,
        args.pastas_ignoradas,
        args.extensoes_ignoradas
    )
