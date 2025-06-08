from math import acos, asin, atan2, cos, sin, sqrt, pi
from typing import Sequence
import numpy as np
import sys

try:
    # Note que estamos tentando acessar um elemento protegido da classe, podendo não fazer sentido.
    # Entretanto, quando se cria um executável com Pyinstaller, há extração dos arquivos necessários
    # para uma pasta temporária e sys._MEIPASS aponta para essa pasta.
    # Dessa forma, durante a execução do programa binário-empacotado, você pode acessar arquivos/diretórios
    # auxiliares usando esse caminho.
    GLOBAL_DIR = sys._MEIPASS
except Exception as error:
    # Caso esteja rodando em python puro normalmente
    # não gerará erro!
    GLOBAL_DIR = "."


class General:
    """
    Descrição:
        Classe responsável por aglutinar funções triviais e necessárias nas operações que serão
        amplamente realizadas.

    Métodos Disponíveis (Todos static):
        Matemáticos:
        - spherical_deg_simspark_to_cart
        - sin_deg
        - cos_deg
        - to_3D
        - to_2D_escrito_como_3D
        - normalize
        - acos
        - asin
        - normalize_deg
        - normalize_rad
        - deg_to_rad
        - rad_to_deg
        - angle_horizontal_from_vector2D

        Não matemáticos:
        - obter_diretorio_ativo



    """

    @staticmethod
    def spherical_deg_simspark_to_cart(spherical_vec: Sequence[float]) -> np.ndarray:
        """
        Descrição:
            Converte coordenadas esféricas expressas em graus (no estilo utilizado pelo SimSpark)
            para coordenadas cartesianas tridimensionais.

        Parâmetros:
            spherical_vec: array_like
                Vetor contendo três elementos: [raio, ângulo_horizontal_em_graus, ângulo_vertical_em_graus].

        Retorno:
            np.array
                Vetor tridimensional de coordenadas cartesianas correspondente às coordenadas esféricas fornecidas.
        """
        # Sim, eu sei que poderia ser tudo junto do return, mas a que custo!

        r = spherical_vec[0]
        h = spherical_vec[1] * pi / 180
        v = spherical_vec[2] * pi / 180

        return np.array([r * cos(v) * cos(h), r * cos(v) * sin(h), r * sin(v)])

    @staticmethod
    def sin_deg(angle_deg: float) -> float:
        """
        Descrição:
            Apenas retornará o seno de um ângulo em degraus.
        """

        return sin(angle_deg * pi / 180)

    @staticmethod
    def cos_deg(angle_deg: float) -> float:
        """
        Descrição:
            Apenas retornará o cosseno de um ângulo em degraus.
        """

        return cos(angle_deg * pi / 180)

    @staticmethod
    def to_3D(vector_2D: np.ndarray, new_value_to_z: float = 0) -> np.ndarray:
        """
        Descrição:
            Converte um vetor 2D em um vetor 3D a partir de outro valor dado.
            Note que não é criado uma nova instância, a alocação é feita in-place.
        """

        return np.append(vector_2D, new_value_to_z)

    @staticmethod
    def to_2D_escrito_como_3D(vector_3D: np.ndarray) -> np.ndarray:
        """
        Descrição:
            Uma nova instância de np.ndarray é criada copiando o vetor dado
            e zerando sua componente z correspondente.

            Por isso o nome é '2D escrito como 3D'.
        """

        vector_2D_escrito_como_3D = np.copy(vector_3D)
        vector_2D_escrito_como_3D[2] = 0
        return vector_2D_escrito_como_3D

    @staticmethod
    def normalize(vector: np.ndarray) -> np.ndarray:
        """
        Descrição:
            Divide um vetor pelo seu módulo.
            Caso o módulo seja 0, retorna o próprio vetor.
        """

        mod = np.linalg.norm(vector)

        return vector if mod == 0 else vector / mod

    @staticmethod
    def acos(valor: float) -> float:
        """
        Descrição:
            Arco-cosseno que limita a entrada entre -1 e 1.
        """

        return acos(np.clip(valor, -1, 1))

    @staticmethod
    def asin(valor: float) -> float:
        """
        Descrição:
            Arco-seno que limita a entrada entre -1 e 1.
        """

        return asin(np.clip(valor, -1, 1))

    @staticmethod
    def normalize_deg(angle_deg: float) -> float:
        """
        Descrição:
            Normalizerá o ângulo entre [-180, 180[
        """

        return (angle_deg + 180.0) % 360 - 180.0

    @staticmethod
    def normalize_rad(angle_rad: float) -> float:
        """
        Descrição:
            Normalizerá o ângulo entre [-pi, pi[
        """

        return (angle_rad + pi) % (2 * pi) - pi

    @staticmethod
    def deg_to_rad(angle_deg: float) -> float:
        """
        Descrição:
            Converte de degraus para radianos.
        """
        return angle_deg * 0.01745329251994330

    @staticmethod
    def rad_to_deg(angle_rad: float) -> float:
        """
        Descrição:
            Converte de radianos para degraus.
        """
        return angle_rad * 57.29577951308232

    @staticmethod
    def angle_horizontal_from_vector2D(vector_2D: np.ndarray, is_rad: bool = False) -> float:
        """
        Descrição:
            Calcula o ângulo do vetor 2D, no caso arctan(y/x).

        Parâmetros:
            is_rad: bool
                Se o retorno estará em radianos ou não.

        Retorno:
            arctan(y / x) em graus ou radianos.
        """
        return atan2(vector_2D[1], vector_2D[0]) if is_rad else atan2(vector_2D[1], vector_2D[0]) * 180 / pi

    @staticmethod
    def angle_between_vectors(vector_1: np.ndarray, vector_2: np.ndarray, is_rad: bool = False) -> float:
        """
        Descrição:
            Obtém o ângulo, seja em graus ou radianos, entre dois vetores, de qualquer dimensão.
        """

        angle_rad = acos(
            np.dot(
                General.normalize(vector_1),
                General.normalize(vector_2)
            )
        )

        return angle_rad if is_rad else angle_rad * 180 / pi

    @staticmethod
    def obter_diretorio_ativo(dir_: str) -> str:
        """Autoexplicativo"""
        global GLOBAL_DIR
        return GLOBAL_DIR + dir_
