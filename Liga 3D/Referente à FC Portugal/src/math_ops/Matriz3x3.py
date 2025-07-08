from math import asin, atan2, pi, sqrt
import numpy as np


class Matriz3x3:
    """
    Descrição:
        Classe responsável por gerir funcionalidades ligadas às matrizes 3x3.

    Métodos Disponível:
        - get_roll_deg: obtém ângulo de rotação x.
        - get_pitch_deg: obtém ângulo de rotação y.
        - get_yaw_deg: obtém ângulo de rotação z.
        - get_inclination_z_deg: obtém ângulo de inclinação do eixo Z relativo e do eixo Z canônico.
        - multiply
        - rotate_x_deg
        - rotate_y_deg
        - rotate_z_deg
        - _rotate_x_neg_rad
        - _rotate_y_neg_rad
        - _rotate_z_neg_rad
        - rotate_rad: rotaciona em torno de um vetor e de um ângulo em radianos.
        - rotate_deg: rotaciona em torno de um vetor e de ângulo em graus.
        - invert
        - create_sup_matrix_rotation: cria matriz de rotação geral.
    """

    def __init__(self, matriz: np.ndarray[float] = None) -> None:
        """
        Descrição:
            Inicializa uma matriz desta classe.

        Exemplos:
            - a = Matriz3x3()                            # Identidade
            - b = Matriz3x3( [[1,1,1],[2,2,2],[3,3,3]] ) # manually initialize matrix
            - c = Matriz3x3( [1,1,1,2,2,2,3,3,3] )       # manually initialize matrix
            - d = Matriz3x3( b )                         # copy constructor
        """

        if matriz is None:

            self.matriz = np.identity(3)
        elif isinstance(matriz, Matriz3x3):

            self.matriz = np.copy(matriz.matriz)
        else:

            self.matriz = np.asarray(matriz)
            # Apenas para garantirmos.
            self.matriz.shape = (3, 3)

        # Apenas para facilitarmos o uso.
        self.rotation_shortcuts = {(1, 0, 0): self.rotate_x_rad, (-1, 0, 0): self._rotate_x_neg_rad,
                                   (0, 1, 0): self.rotate_y_rad, (0, -1, 0): self._rotate_y_neg_rad,
                                   (0, 0, 1): self.rotate_z_rad, (0, 0, -1): self._rotate_z_neg_rad}

    # Relativo ao eixo x
    def get_roll_deg(self) -> float:
        """
        Descrição:
            Retorna o ângulo de rotação (roll) ao redor do eixo X em graus, assumindo
            a convenção de multiplicação de matrizes de rotação na ordem RotZ * RotY * RotX = Rot.
            Esse ângulo corresponde à inclinação lateral de um objeto no espaço tridimensional.

            O valor é calculado com base nos elementos da matriz de rotação `self.m`.
            Caso os elementos `m[2,1]` e `m[2,2]` sejam ambos zero, a função retorna 180 graus
            como um caso especial para evitar ambiguidade no uso do `atan2`.

        Parâmetros:
            - self: objeto que possui o atributo `self.m`, uma matriz 3x3 representando uma rotação 3D.

        Retorno:
            - float: ângulo de rotação ao redor do eixo X (roll), em graus.
        """
        if self.matriz[2, 1] == 0 and self.matriz[2, 2] == 0:
            return 180

        return atan2(self.matriz[2, 1], self.matriz[2, 2]) * 180 / pi

    # Relativo ao eixo y
    def get_pitch_deg(self) -> float:
        """
        Descrição:
            Retorna o ângulo de rotação (pitch) ao redor do eixo **y** em graus, com base na matriz de rotação `self.m`.
            A convenção assumida para a decomposição da rotação é a seguinte:
            `Rot = RotZ * RotY * RotX`, ou seja, as rotações são aplicadas primeiro em **x**, depois em **y**, e por fim em **z**.

            O ângulo de pitch corresponde à inclinação do vetor no plano x–z, ou seja, ao movimento de "olhar para cima ou para baixo"
            em sistemas tridimensionais. O cálculo usa a função `atan2` para evitar ambiguidade nos quadrantes, garantindo robustez
            mesmo em casos em que `cos(pitch)` se aproxima de zero (próximo de ±90°).

        Parâmetros:
            - self: instância de uma classe que contém o atributo `self.m`, uma matriz de rotação 3×3.

        Retorno:
            - float:
                O valor do ângulo de **pitch** em graus (float). Esse ângulo indica a inclinação ao redor do eixo y.
        """
        return atan2(-self.matriz[2, 0], sqrt(self.matriz[2, 1] * self.matriz[2, 1] + self.matriz[2, 2] * self.matriz[2, 2])) * 180 / pi

    # Relativo ao eixo z
    def get_yaw_deg(self) -> float:
        """
        Descrição:
            Retorna o ângulo de guinada (yaw) em graus a partir de uma matriz de rotação tridimensional `self.matriz`,
            assumindo a convenção de ordem de rotações Z-Y-X (isto é, RotZ * RotY * RotX = Rot).

            O yaw corresponde à rotação em torno do eixo Z, frequentemente usada para indicar orientação horizontal
            (por exemplo, direção de um agente em um plano 2D).

            Em casos onde os elementos matriz[1, 0] e matriz[0, 0] da matriz são ambos zero (situação de singularidade),
            o cálculo é feito com base em outros elementos da matriz (matriz[0, 1] e matriz[1, 1]) para evitar divisão por zero
            e garantir continuidade na medida angular.

        Parâmetros:
            - self: instância contendo o atributo `m`, uma matriz de rotação 3x3 (do tipo `np.ndarray`).

        Retorno:
            - float:
                Ângulo de guinada (yaw) em graus, no intervalo (-180°, 180°], calculado com `atan2`.
        """
        if self.matriz[1, 0] == 0 and self.matriz[0, 0] == 0:
            return atan2(self.matriz[0, 1], self.matriz[1, 1]) * 180 / pi
        return atan2(self.matriz[1, 0], self.matriz[0, 0]) * 180 / pi

    def get_inclination_z_deg(self) -> float:
        """
        Descrição:
            Retorna a inclinação do eixo z do sistema de coordenadas atual (representado pela matriz `self.matriz`)
            em relação ao eixo z de uma referência (normalmente a base global), assumindo que `self.matriz` representa
            uma matriz de rotação válida em 3D.

            A inclinação é calculada com base no cosseno do ângulo entre os eixos z (ou seja, a projeção do eixo z
            transformado sobre o eixo z original). O valor retornado é em graus e representa o desvio angular do
            vetor z transformado em relação ao vetor [0, 0, 1].

        Retorno:
            float:
                Ângulo de inclinação em graus, no intervalo [0, 90], onde:
                - 0º significa alinhamento perfeito com o eixo z de referência;
                - 90° indica que o vetor está perpendicular ao eixo z de referência.
        """

        return 90 - (asin(self.matriz[2, 2]) * 180 / pi)

    def multiply(self, mat: np.ndarray, in_place: bool = False, reverse_order: bool = False):
        """
        Descrição:
            Multiplica a matriz de rotação atual pela matriz ou vetor fornecido em `mat`.
            Permite especificar se a multiplicação deve ser feita no sentido padrão (self * mat)
            ou invertido (mat * self). Pode modificar a matriz interna da instância ou retornar
            uma nova matriz, conforme indicado pelo parâmetro `in_place`.

            Se `mat` for um vetor 3D, retorna o vetor resultante da multiplicação.

        Parâmetros:
            mat: Matriz3x3 ou array_like
                Matriz multiplicadora ou vetor 3D a ser multiplicado pela matriz atual.

            in_place: bool, opcional
                - True: modifica a matriz interna da instância diretamente.
                - False: preserva a matriz original e retorna uma nova matriz multiplicada (padrão).

            reverse_order: bool, opcional
                - False: realiza a multiplicação na ordem self * mat (padrão).
                - True: realiza a multiplicação na ordem mat * self.

        Retorno:
            result: Matriz3x3 ou array_like
                Retorna uma instância Matriz3x3 se `mat` for matriz (retorna `self` se `in_place=True`),
                ou um vetor 3D se `mat` for vetor.
        """

        # get array from matrix object or convert to numpy array (if needed)
        mat = mat.matriz if isinstance(mat, Matriz3x3) else np.asarray(mat)

        a, b = (mat, self.matriz) if reverse_order else (self.matriz, mat)

        if mat.ndim == 1:
            return np.matmul(a, b)  # multiplication by 3D vector
        elif in_place:
            np.matmul(a, b, self.matriz)  # multiplication by matrix, in place
            return self
        else:  # multiplication by matrix, return new Matriz3x3
            return Matrix3x3(np.matmul(a, b))

    def rotate_x_rad(self, rotation_rad: float, in_place: bool = False):
        """
        Descrição:
            Aplica uma rotação 3D à matriz atual ao redor do eixo X, com base em um ângulo fornecido
            em radianos. Essa rotação modifica apenas os eixos Y e Z da matriz, mantendo o eixo X fixo.

            Se `rotation_rad` for 0, a matriz original é retornada sem alterações.

            Opcionalmente, pode modificar a matriz interna da instância (`in_place=True`) ou retornar
            uma nova matriz rotacionada (`in_place=False`).

        Parâmetros:
            rotation_rad: float
                Ângulo de rotação em radianos ao redor do eixo X.

            in_place: bool, opcional
                - True: a matriz interna da instância é modificada diretamente (padrão).
                - False: a matriz original é preservada e uma nova matriz rotacionada é retornada.

        Retorno:
            result: Matriz3x3
                A própria instância (`self`) se `in_place=True`, ou uma nova matriz rotacionada se `in_place=False`.
        """

        if rotation_rad == 0:
            return self if in_place else Matriz3x3(self)

        c = np.math.cos(rotation_rad)
        s = np.math.sin(rotation_rad)

        mat = np.array([
            [1, 0, 0],
            [0, c, -s],
            [0, s, c]])

        return self.multiply(mat, in_place)

    def rotate_y_rad(self, rotation_rad: float, in_place: bool = False):
        """Veja documentação de rotate_x_rad"""
        if rotation_rad == 0:
            return self if in_place else Matriz3x3(self)

        c = np.math.cos(rotation_rad)
        s = np.math.sin(rotation_rad)

        mat = np.array([
            [c, 0, s],
            [0, 1, 0],
            [-s, 0, c]])

        return self.multiply(mat, in_place)

    def rotate_z_rad(self, rotation_rad: float, in_place: bool = False):
        """Veja documentação de rotate_x_rad"""
        if rotation_rad == 0:
            return self if in_place else Matriz3x3(self)

        c = np.math.cos(rotation_rad)
        s = np.math.sin(rotation_rad)

        mat = np.array([
            [c, -s, 0],
            [s, c, 0],
            [0, 0, 1]])

        return self.multiply(mat, in_place)

    def rotate_x_deg(self, rotation_deg: float, in_place=False):
        return self.rotate_x_rad(rotation_deg * (pi / 180), in_place)

    def rotate_y_deg(self, rotation_deg: float, in_place=False):
        return self.rotate_y_rad(rotation_deg * (pi / 180), in_place)

    def rotate_z_deg(self, rotation_deg: float, in_place=False):
        return self.rotate_z_rad(rotation_deg * (pi / 180), in_place)

    def _rotate_x_neg_rad(self, rotation_rad: float, in_place: bool = False):
        """Veja documentação de rotate_x_rad"""
        return self.rotate_x_rad(-rotation_rad, in_place)

    def _rotate_y_neg_rad(self, rotation_rad: float, in_place: bool = False):
        """Veja documentação de rotate_x_rad"""
        return self.rotate_y_rad(-rotation_rad, in_place)

    def _rotate_z_neg_rad(self, rotation_rad: float, in_place: bool = False):
        """Veja documentação de rotate_x_rad"""
        return self.rotate_z_rad(-rotation_rad, in_place)

    def rotate_rad(self, rotation_vec: np.ndarray[float | int], rotation_rad: float, in_place=False):
        """
        Descrição:
            Aplica uma rotação 3D à matriz de rotação atual, com base em um vetor de rotação arbitrário
            (eixo de rotação unitário) e um ângulo em radianos. A rotação segue a fórmula de Rodrigues
            para geração da matriz rotacional correspondente ao eixo e ângulo especificados.

            Opcionalmente, pode modificar a matriz interna da instância (`in_place=True`) ou retornar
            uma nova matriz rotacionada (`in_place=False`).

            Se o ângulo for 0, nenhuma rotação será aplicada. Se um atalho (shortcut) para rotações
            específicas for registrado, ele será utilizado em vez da rotação geral.

        Parâmetros:
            rotation_vec: array_like, comprimento 3
                Vetor unitário que representa o eixo de rotação.

            rotation_rad: float
                Ângulo de rotação em radianos. Usado na construção da matriz de rotação via
                fórmula de Rodrigues.

            in_place: bool, opcional
                - True: a matriz interna da instância é modificada diretamente (padrão).
                - False: a matriz original é preservada e uma nova matriz rotacionada é retornada.

        Retorno:
            result: Matriz3x3
                A própria instância (`self`) se `in_place=True`, ou uma nova matriz rotacionada se `in_place=False`.
        """

        if rotation_rad == 0:
            return

        # Note que podemos utilizar esta função para rotacionar os casos triviais.
        shortcut = self.rotation_shortcuts.get(
            tuple[rotation_vec[0], rotation_vec[1], rotation_vec[2]],
            None
        )
        if shortcut:
            return shortcut(rotation_rad, in_place)

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
            [xxc1 + c, xyc1 - zs, xzc1 + ys],
            [xyc1 + zs, yyc1 + c, yzc1 - xs],
            [xzc1 - ys, yzc1 + xs, zzc1 + c]])

        return self.multiply(mat, in_place)

    def rotate_deg(self, rotation_vec: np.ndarray[float | int], rotation_deg: float, in_place=False):
        """
        Descrição:
            Rotaciona a matriz de rotação atual por um determinado ângulo (em graus) ao redor de um vetor de rotação.
            Internamente, a rotação é convertida para radianos e delegada à função `rotate_rad`.

            A operação pode ser feita in-place (modificando a matriz atual) ou pode retornar uma nova instância
            rotacionada sem alterar a original.

        Parâmetros:
            rotation_vec: array_like, length 3
                Vetor em torno do qual a rotação será realizada.

            rotation_deg: float
                Ângulo da rotação, em graus.

            in_place: bool, optional
                - True: modifica a matriz de rotação atual diretamente (padrão).
                - False: retorna uma nova matriz com a rotação aplicada.

        Retorno:
            result: Matriz3x3
                Se `in_place=True`, retorna o próprio objeto `self` com a matriz modificada.
                Caso contrário, retorna uma nova instância da matriz rotacionada.
        """
        return self.rotate_rad(rotation_vec, rotation_deg * (pi / 180), in_place)

    def invert(self, in_place=False):
        """
        Descrição:
            Inverte a matriz de rotação atual (matriz 3x3). A inversão de uma matriz de rotação
            corresponde à sua transposta, dado que matrizes de rotação são ortogonais. No entanto,
            esta função utiliza inversão genérica via `np.linalg.inv` para garantir compatibilidade
            com matrizes arbitrárias (caso contenham erros numéricos).

            Pode modificar a matriz original da instância (`in_place=True`) ou retornar uma nova
            instância com a matriz invertida (`in_place=False`).

        Parâmetros:
            in_place: bool, opcional
                - True: a matriz interna da instância é substituída pela sua inversa.
                - False: a matriz original é preservada e uma nova instância com a inversa é retornada.

        Retorno:
            result: Matriz3x3
                A própria instância (`self`) se `in_place=True`, ou uma nova instância com a
                matriz invertida se `in_place=False`.
        """

        if in_place:
            self.matriz = np.linalg.inv(self.matriz)
            return self
        else:
            return Matriz3x3(np.linalg.inv(self.matriz))

    @classmethod
    def create_sup_matrix_rotation(cls, euler_vec: np.ndarray[float]):
        """
        Descrição:
            Cria uma matriz de rotação 3D a partir de ângulos de Euler fornecidos em graus, utilizando a convenção
            de rotação composta na ordem ZYX (RotZ * RotY * RotX), também conhecida como yaw-pitch-roll.

            Essa função aplica rotações sequenciais nos eixos Z, Y e X correspondentes, respectivamente,
            aos ângulos de yaw (z), pitch (y) e roll (x).

        Parâmetros:
            euler_vec: array_like, comprimento 3
                Vetor contendo os ângulos de Euler em graus no formato (x, y, z), ou seja, (roll, pitch, yaw).

        Retorno:
            result: Matriz3x3
                Instância da classe Matriz3x3 representando a matriz de rotação composta.

        Exemplo:
            Matriz3x3.from_rotation_deg((roll, pitch, yaw))
                Gera a matriz composta RotZ(yaw) * RotY(pitch) * RotX(roll)
        """

        mat = cls().rotate_z_deg(euler_vec[2], True).rotate_y_deg(euler_vec[1], True).rotate_x_deg(euler_vec[0], True)
        return mat
