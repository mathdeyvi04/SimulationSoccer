from math import asin, atan2, pi, sqrt
from typing import Sequence
import numpy as np

# Consertar interseção de módulos
from math_ops.GeneralMath import GeneralMath
from math_ops.Matriz3x3 import Matriz3x3


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

    Metódos não contemplados:
        - get_x, get_y, get_z -> basta utilizar o obter_vetor_de_translacao
    """

    def __init__(self, matriz: Sequence[float] = None) -> None:
        """
        Descrição:
            Construtor da classe Matriz4x4

        Constructor examples:
            - a = Matriz4x4( )                                           # create identity matriz
            - b = Matriz4x4( [[1,1,1,1],[2,2,2,2],[3,3,3,3],[4,4,4,4]] ) # manually initialize matriz
            - c = Matriz4x4( [1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4] )         # manually initialize matriz
            - d = Matriz4x4( b )                                         # copy constructor
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
            Matriz4x4
                A própria instância (`self`) se `in_place=True`, ou uma nova matriz com a translação aplicada se `in_place=False`.
        """

        vec = np.array([*translation_vec, 1])
        # Note que calculamos apenas a multiplicação devido à 4.ª coluna.
        np.matmul(self.matriz, vec, out=vec)

        if in_place:
            self.matriz[:, 3] = vec
            return self
        else:
            ret = Matriz4x4(self.matriz)
            ret.matriz[:, 3] = vec
            return ret

    def obter_vetor_de_translacao(self) -> np.ndarray:
        return self.matriz[0:3, 3]

    def obter_modulo_vetor_de_translacao(self) -> float:
        return np.linalg.norm(self.matriz[0:3, 3])

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
            Obtém nova matriz 3x3 referente à rotação, basicamente a self.matriz sem translação.
        """
        self.matriz: np.ndarray[float]
        return Matriz3x3(self.matriz[0:3, 0:3])

    ################################
    # Aqui a preguiça bateu! Me perdoe.
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
        """ Get inclination of z-axis in relation to reference z-axis """
        return 90 - (asin(np.clip(self.matriz[2, 2], -1, 1)) * 180 / pi)

    def rotate_deg(self, rotation_vec, rotation_deg, in_place=False):
        """
        Rotates the current transformation matrix

        Parameters
        ----------
        rotation_vec : array_like, length 3
            rotation vector
        rotation_deg : float
            rotation in degrees
        in_place: bool, optional
            * True: the internal matrix is changed in-place (default)
            * False: a new matrix is returned and the current one is not changed

        Returns
        -------
        result : Matriz4x4
            self is returned if in_place is True
        """
        return self.rotate_rad(rotation_vec, rotation_deg * (pi / 180), in_place)

    def rotate_rad(self, rotation_vec, rotation_rad, in_place=False):
        """
        Rotates the current transformation matrix

        Parameters
        ----------
        rotation_vec : array_like, length 3
            rotation vector
        rotation_rad : float
            rotation in radians
        in_place: bool, optional
            * True: the internal matrix is changed in-place (default)
            * False: a new matrix is returned and the current one is not changed

        Returns
        -------
        result : Matriz4x4
            self is returned if in_place is True
        """

        if rotation_rad == 0:
            return self if in_place else Matriz4x4(self.matriz)

        # Em Matriz3x3, fazemos isso usando um set de possibilidades.
        # shortcuts for rotation around 1 axis
        if rotation_vec[0] == 0:
            if rotation_vec[1] == 0:
                if rotation_vec[2] == 1:
                    return self.rotate_z_rad(rotation_rad, in_place)
                elif rotation_vec[2] == -1:
                    return self.rotate_z_rad(-rotation_rad, in_place)
            elif rotation_vec[2] == 0:
                if rotation_vec[1] == 1:
                    return self.rotate_y_rad(rotation_rad, in_place)
                elif rotation_vec[1] == -1:
                    return self.rotate_y_rad(-rotation_rad, in_place)
        elif rotation_vec[1] == 0 and rotation_vec[2] == 0:
            if rotation_vec[0] == 1:
                return self.rotate_x_rad(rotation_rad, in_place)
            elif rotation_vec[0] == -1:
                return self.rotate_x_rad(-rotation_rad, in_place)

        c = np.math.cos(rotation_rad)
        c1 = 1 - c
        s = np.math.sin(rotation_rad)
        x = rotation_vec[0]
        y = rotation_vec[1]
        z = rotation_vec[2]
        xxc1 = x * x * c1
        yyc1 = y * y * c1
        zzc1 = z * z * c1
        xyc1 = x * y * c1
        xzc1 = x * z * c1
        yzc1 = y * z * c1
        xs = x * s
        ys = y * s
        zs = z * s

        mat = np.array([
            [xxc1 + c, xyc1 - zs, xzc1 + ys, 0],
            [xyc1 + zs, yyc1 + c, yzc1 - xs, 0],
            [xzc1 - ys, yzc1 + xs, zzc1 + c, 0],
            [0, 0, 0, 1]])

        return self.multiply(mat, in_place)

    def rotate_x_rad(self, rotation_rad, in_place=False):
        """
        Rotates the current transformation matrix around the x-axis

        Parameters
        ----------
        rotation_rad : float
            rotation in radians
        in_place: bool, optional
            * True: the internal matrix is changed in-place (default)
            * False: a new matrix is returned and the current one is not changed

        Returns
        -------
        result : Matriz4x4
            self is returned if in_place is True
        """
        if rotation_rad == 0:
            return self if in_place else Matriz4x4(self.matriz)

        c = np.math.cos(rotation_rad)
        s = np.math.sin(rotation_rad)

        mat = np.array([
            [1, 0, 0, 0],
            [0, c, -s, 0],
            [0, s, c, 0],
            [0, 0, 0, 1]])

        return self.multiply(mat, in_place)

    def rotate_y_rad(self, rotation_rad, in_place=False):
        """
        Rotates the current transformation matrix around the y-axis

        Parameters
        ----------
        rotation_rad : float
            rotation in radians
        in_place: bool, optional
            * True: the internal matrix is changed in-place (default)
            * False: a new matrix is returned and the current one is not changed

        Returns
        -------
        result : Matriz4x4
            self is returned if in_place is True
        """
        if rotation_rad == 0:
            return self if in_place else Matriz4x4(self.matriz)

        c = np.math.cos(rotation_rad)
        s = np.math.sin(rotation_rad)

        mat = np.array([
            [c, 0, s, 0],
            [0, 1, 0, 0],
            [-s, 0, c, 0],
            [0, 0, 0, 1]])

        return self.multiply(mat, in_place)

    def rotate_z_rad(self, rotation_rad, in_place=False):
        """
        Rotates the current transformation matrix around the z-axis

        Parameters
        ----------
        rotation_rad : float
            rotation in radians
        in_place: bool, optional
            * True: the internal matrix is changed in-place (default)
            * False: a new matrix is returned and the current one is not changed

        Returns
        -------
        result : Matriz4x4
            self is returned if in_place is True
        """
        if rotation_rad == 0:
            return self if in_place else Matriz4x4(self.matriz)

        c = np.math.cos(rotation_rad)
        s = np.math.sin(rotation_rad)

        mat = np.array([
            [c, -s, 0, 0],
            [s, c, 0, 0],
            [0, 0, 1, 0],
            [0, 0, 0, 1]])

        return self.multiply(mat, in_place)

    def rotate_x_deg(self, rotation_deg, in_place=False):
        """
        Rotates the current transformation matrix around the x-axis

        Parameters
        ----------
        rotation_deg : float
            rotation in degrees
        in_place: bool, optional
            * True: the internal matrix is changed in-place (default)
            * False: a new matrix is returned and the current one is not changed

        Returns
        -------
        result : Matriz4x4
            self is returned if in_place is True
        """
        return self.rotate_x_rad(rotation_deg * (pi / 180), in_place)

    def rotate_y_deg(self, rotation_deg, in_place=False):
        """
        Rotates the current transformation matrix around the y-axis

        Parameters
        ----------
        rotation_deg : float
            rotation in degrees
        in_place: bool, optional
            * True: the internal matrix is changed in-place (default)
            * False: a new matrix is returned and the current one is not changed

        Returns
        -------
        result : Matriz4x4
            self is returned if in_place is True
        """
        return self.rotate_y_rad(rotation_deg * (pi / 180), in_place)

    def rotate_z_deg(self, rotation_deg, in_place=False):
        """
        Rotates the current transformation matrix around the z-axis

        Parameters
        ----------
        rotation_deg : float
            rotation in degrees
        in_place: bool, optional
            * True: the internal matrix is changed in-place (default)
            * False: a new matrix is returned and the current one is not changed

        Returns
        -------
        result : Matriz4x4
            self is returned if in_place is True
        """
        return self.rotate_z_rad(rotation_deg * (pi / 180), in_place)

    def invert(self, in_place=False):
        """
        Descrição:
            Inverts the current transformation matrix

        Parameters:
            in_place: bool, optional
                * True: the internal matrix is changed in-place (default)
                * False: a new matrix is returned and the current one is not changed

        Returns
            result : Matriz4x4
                self is returned if in_place is True
        """

        if in_place:
            self.matriz = np.linalg.inv(self.matriz)
            return self
        else:
            return Matriz4x4(np.linalg.inv(self.matriz))

    def multiply(self, mat, in_place=False):
        """
        Multiplies the current transformation matrix by mat

        Parameters
        ----------
        mat : Matriz4x4 or array_like
            multiplier matrix or 3D vector
        in_place: bool, optional
            * True: the internal matrix is changed in-place (default)
            * False: a new matrix is returned and the current one is not changed (if mat is a 4x4 matrix)

        Returns
        -------
        result : Matriz4x4 | array_like
            Matriz4x4 is returned if mat is a matrix (self is returned if in_place is True);
            a 3D vector is returned if mat is a vector
        """
        if isinstance(mat, Matriz4x4):
            mat = mat.matriz
        else:
            mat = np.asarray(mat)  # conversion to array, if needed
            if mat.ndim == 1:  # multiplication by 3D vector
                vec = np.append(mat, 1)  # conversion to 4D vector
                return np.matmul(self.matriz, vec)[0:3]  # conversion to 3D vector

        if in_place:
            np.matmul(self.matriz, mat, self.matriz)
            return self
        else:
            return Matriz4x4(np.matmul(self.matriz, mat))

    def __call__(self, mat, is_spherical=False):
        """
        Multiplies the current transformation matrix by mat and returns a new matrix or vector

        Parameters
        ----------
        mat : Matrix_4x4 or array_like
            multiplier matrix or 3D vector
        is_spherical : bool
            only relevant if mat is a 3D vector, True if it uses spherical coordinates

        Returns
        -------
        result : Matrix_4x4 | array_like
            Matrix_4x4 is returned if mat is a matrix;
            a 3D vector is returned if mat is a vector
        """

        if is_spherical and mat.ndim == 1:
            mat = GeneralMath.spherical_deg_simspark_to_cart(mat)

        return self.multiply(mat, False)


if __name__ == '__main__':
    m = Matriz4x4()
    print(m.obter_matriz_de_rotacao().matriz)
