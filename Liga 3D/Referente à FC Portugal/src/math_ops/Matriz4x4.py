from math import asin, atan2, pi, sqrt
from typing import Sequence
import numpy as np

# Consertar interseção de módulos
from GeneralMath import GeneralMath
from Matriz3x3 import Matriz3x3


class Matriz4x4:
    """
    Descrição:
        Classe responsável por ...

    Métodos Disponíveis:
        - create_matrix_translation
        - create_matrix_rot_and_trans
        - translate
        - obter_vetor_de_translacao
        - obter_matriz_de_rotacao_4x4
        - obter_matriz_de_rotacao_3x3
        - get_roll_deg
        - get_pitch_deg
        - get_yaw_deg
        - get_inclination_deg
        -
        -
        -
        -
        -
        -
        -
        -

    Metódos não contemplados:
        - get_x, get_y, get_z
        - get_distance

    """

    def __init__(self, matriz: Sequence[float] = None) -> None:
        """
        Descrição:
            Construtor da classe Matriz4x4

        Constructor examples:
            - a = Matrix_4x4( )                                           # create identity matriz
            - b = Matrix_4x4( [[1,1,1,1],[2,2,2,2],[3,3,3,3],[4,4,4,4]] ) # manually initialize matriz
            - c = Matrix_4x4( [1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4] )         # manually initialize matriz
            - d = Matrix_4x4( b )                                         # copy constructor
        """

        if matriz is None:

            self.matriz = np.identity(4)
        elif isinstance(matriz, Matriz4x4):

            self.matriz = np.copy(matriz.matriz)
        elif isinstance(matriz, Matriz3x3):

            # Criamos uma matriz 4x4 baseado em uma 3x3
            self.matriz = np.identity(4)
            self.matriz[0:3, 0:3] = matriz.matriz
        else:
            self.matriz = np.asarray(matriz)
            self.matriz.shape = (4, 4)

    @classmethod
    def create_matrix_translation(cls, translation_vec: np.ndarray[float]):
        """
        Descrição:
            Cria uma matriz de translação a partir de um vetor dado, o qual corresponderá
            à translação.

        Parâmetro:
            - translation_vec: vetor tridimensional.

        Retorno:
            Matriz4x4 correspondente à translação.
        """

        mat = np.identity(4)
        mat[0:3, 3] = translation_vec

        return cls(mat)  # Apenas usamos o construtor aqui.

    @classmethod
    def create_matrix_rot_and_trans(cls, mat3x3: Matriz3x3, translation_vec: np.ndarray[float]):
        """
        Descrição:
            Cria uma matriz 4x4 a partir de uma matriz de rotação 3x3 e um vetor de translação.

        Parâmetros:
            - mat3x3: matriz 3x3 corrrespondente à rotação.
            - translation_vec: vetor de translação tridimensional
        """
        mat = np.identity(4)
        mat[0:3, 0:3] = mat3x3.matriz
        mat[0:3, 3] = translation_vec
        return cls(mat)

    def translate(self, translation_vec: np.ndarray[float], in_place=False):
        """
        Descrição:
            Aplica uma translação 3D à matriz de transformação atual (4x4), utilizando um vetor de translação.
            A operação afeta apenas a coluna de translação da matriz homogênea, mantendo as rotações inalteradas.

            Pode modificar a matriz interna da instância (`in_place=True`) ou retornar uma nova matriz transformada
            (`in_place=False`).

        Parâmetros:
            translation_vec: array_like, comprimento 3
                Vetor de translação contendo os deslocamentos nos eixos X, Y e Z.

            in_place: bool, opcional
                - True: a matriz interna da instância é modificada diretamente.
                - False: uma nova matriz de transformação com a translação aplicada é retornada (padrão).

        Retorno:
            Matrix_4x4
                A própria instância (`self`) se `in_place=True`, ou uma nova matriz com a translação aplicada se `in_place=False`.
        """

        vec = np.array([*translation_vec, 1])
        # Note que calculamos apenas a multiplicação devido à 4° coluna.
        np.matmul(self.matriz, vec, out=vec)

        if in_place:
            self.matriz[:, 3] = vec
            return self
        else:
            ret = Matrix_4x4(self.matriz)
            ret.matriz[:, 3] = vec
            return ret

    def obter_vetor_de_translacao(self) -> np.ndarray:
        return self.matriz[0:3, 3]

    def obter_matriz_de_rotacao_4x4(self):
        """
        Descrição:
            Obtém nova matriz 4x4 sem a translação.
        """

        mat = Matriz4x4(self.matriz)
        mat.matriz[0:3, 3] = 0
        return mat

    def obter_matriz_de_rotacao_3x3(self) -> Matriz3x3:
        """
        Descrição:
            Obtém nova matriz 3x3 referente à rotação, sem translação.
        """
        self.matriz: np.ndarray[float]
        return Matriz3x3(self.matriz[0:3, 0:3])

    ################################
    # Aqui a preguiça bateu! Me perdoe
    ###############################

    def get_roll_deg(self):
        """ Get angle around the x-axis in degrees, Rotation order: RotZ*RotY*RotX=Rot """
        if self.matriz[2, 1] == 0 and self.matriz[2, 2] == 0:
            return 180
        return atan2(self.matriz[2, 1], self.matriz[2, 2]) * 180 / pi

    def get_pitch_deg(self):
        """ Get angle around the y-axis in degrees, Rotation order: RotZ*RotY*RotX=Rot """
        return atan2(-self.matriz[2, 0], sqrt(self.matriz[2, 1] * self.matriz[2, 1] + self.matriz[2, 2] * self.matriz[2, 2])) * 180 / pi

    def get_yaw_deg(self):
        """ Get angle around the z-axis in degrees, Rotation order: RotZ*RotY*RotX=Rot """
        if self.matriz[1, 0] == 0 and self.matriz[0, 0] == 0:
            return atan2(self.matriz[0, 1], self.matriz[1, 1]) * 180 / pi
        return atan2(self.matriz[1, 0], self.matriz[0, 0]) * 180 / pi

    def get_inclination_deg(self):
        ''' Get inclination of z-axis in relation to reference z-axis '''
        return 90 - (asin(np.clip(self.matriz[2, 2], -1, 1)) * 180 / pi)


if __name__ == '__main__':
    m = Matriz4x4()
    print(m.obter_matriz_de_rotacao().matriz)
