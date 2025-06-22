from math import acos, asin, atan2, cos, sin, sqrt, pi
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


class GeneralMathMath:
    """
    Descrição:
        Classe responsável por aglutinar funções triviais e necessárias nas operações que serão
        amplamente realizadas.

    Métodos Disponíveis (Todos static):
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
        - target_abs_angle
        - unit_vector_by_angle
        - angle_rel_between_target_position
        - rotate_vector_2D
        - vector_projection
        - distance_point_to_line
        - distance_point_to_segment
        - distance_point_to_ray
        - closest_point_on_ray_to_point
        - does_circle_intersect_segment
        - do_segments_intersect
        - intersection_pt_segment_opp_goal
        - intersection_pt_circle_opp_goal
        - distance_point_to_opp_goal
        - intersection_pts_circle_segment
        - get_intersection_pt_lines
        - obter_diretorio_ativo

    Observações:
        - Todas as considerações de vetores são para bidimensionais.
    """

    @staticmethod
    def spherical_deg_simspark_to_cart(spherical_vec: np.ndarray[float]) -> np.ndarray:
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
    def angle_horizontal_from_vector2D(vector_2D: np.ndarray[float], is_rad: bool = False) -> float:
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
    def angle_between_vectors(vector_1: np.ndarray[float], vector_2: np.ndarray[float], is_rad: bool = False) -> float:
        """
        Descrição:
            Obtém o ângulo, seja em graus ou radianos, entre dois vetores, de qualquer dimensão.
        """

        angle_rad = acos(
            np.dot(
                GeneralMath.normalize(vector_1),
                GeneralMath.normalize(vector_2)
            )
        )

        return angle_rad if is_rad else angle_rad * 180 / pi

    @staticmethod
    def unit_vector_by_angle(angle: float, is_rad: bool = False) -> np.ndarray:
        """
        Descrição:
            Vetor Unitário com direção determinada pelo ângulo.
        """

        return np.array([cos(angle), sin(angle)], float) if is_rad else np.array([GeneralMath.cos_deg(angle), GeneralMath.sin_deg(angle)], float)

    @staticmethod
    def angle_rel_between_target_position(position_2D: np.ndarray[float], orientacao: float, target: np.ndarray[float], is_rad: bool = False) -> float:
        """
        Descrição:
            Retornará o ângulo relativo do vetor target - position_2D quando este está sob orientação específica.
        """

        if is_rad:
            return GeneralMath.normalize_rad(atan2(target[1] - position_2D[1], target[0] - position_2D[0]) - orientacao)
        else:
            return GeneralMath.normalize_deg(atan2(target[1] - position_2D[1], target[0] - position_2D[0]) * 180 / pi - orientacao)

    @staticmethod
    def target_abs_angle(pos2d, target, is_rad=False):
        """ angle (degrees or radians) of vector (target-pos2d) """
        if is_rad:
            return atan2(target[1] - pos2d[1], target[0] - pos2d[0])
        else:
            return atan2(target[1] - pos2d[1], target[0] - pos2d[0]) * 180 / pi

    @staticmethod
    def rotate_vector_2D(vector_2D: np.ndarray[float], angle: float, is_rad: bool = False) -> np.ndarray:
        """
        Descrição:
            Rotaciona um vetor 2D em antihorário por um ângulo dado.
        """

        cos_ang = cos(angle) if is_rad else cos(angle * pi / 180)
        sin_ang = sin(angle) if is_rad else sin(angle * pi / 180)

        return np.array([cos_ang * vector_2D[0] - sin_ang * vector_2D[1], sin_ang * vector_2D[0] + cos_ang * vector_2D[1]])

    @staticmethod
    def vector_projection(vector_to_be_projected: np.ndarray[float], vector_main: np.ndarray[float]) -> np.ndarray[float]:
        """
        Descrição:
            Projeção de vector_to_be_projected em vector_main
        """

        mod_sq = np.dot(vector_main, vector_main)

        return vector_main * np.dot(vector_to_be_projected, vector_main) / mod_sq if mod_sq != 0 else vector_main

    ######################################################
    # Nem vou realizar alterações, pois o nível está alto.
    ######################################################

    @staticmethod
    def distance_point_to_line(ponto: np.ndarray[float], ponto_inicial_da_linha: np.ndarray[float], ponto_final_da_linha: np.ndarray[float]) -> tuple[float, str]:
        """
        Descrição:
            Distância entre ponto e uma linha.
        Distance between point p and 2d line 'ab' (and side where p is)

        Parâmetros:
            ponto_inicial_da_linha: ndarray[float]
                2D point that defines line
            ponto_final_da_linha: ndarray[float]
                2D point that defines line
            ponto: ndarray[float]
                2D point

        Retorno:
            Distância: float
                distance between line and point
            Lado_Visitante: str
                Caso estejamos no ponto_inicial, observando o ponto_final, o ponto poderá estar do nosso lado left ou right.
        """

        line_len = np.linalg.norm(ponto_final_da_linha - ponto_inicial_da_linha)

        if line_len != 0:  # assumes vertical line
            # Em minha IDE, Pycharm, isso está como code inatingível. Oq não faz sentido algum para mim.
            sdist = np.cross(ponto_final_da_linha - ponto_inicial_da_linha, ponto - ponto_inicial_da_linha) / line_len
            dist = abs(sdist)
        else:
            dist = sdist = np.linalg.norm(ponto - ponto_inicial_da_linha)

        return dist, "left" if sdist > 0 else "right"

    @staticmethod
    def distance_point_to_segment(ponto: np.ndarray[float], ponto_inicial: np.ndarray[float], ponto_final: np.ndarray[float]) -> float:
        """
        Descrição:
            Obtém a distância entre um ponto e um segmento, ambos à 2D.
        """

        ap = ponto - ponto_inicial
        ab = ponto_final - ponto_inicial

        ad = GeneralMath.vector_projection(ap, ab)

        # Is d in ab? We can find k in (ad = k * ab) without computing any norm
        # we use the largest dimension of ab to avoid division by 0
        k = ad[0] / ab[0] if abs(ab[0]) > abs(ab[1]) else ad[1] / ab[1]

        if k <= 0:
            return np.linalg.norm(ap)
        elif k >= 1:
            return np.linalg.norm(ponto - ponto_final)
        else:
            return np.linalg.norm(ponto - (ad + ponto_inicial))  # p-d

    # Acredito que este ray se refira a SEMIRETA
    @staticmethod
    def distance_point_to_ray(p: np.ndarray[float], ray_start: np.ndarray[float], ray_direction: np.ndarray[float]) -> float:
        """
        Descrição:
            DistÂncia entre ponto para semireta 2D.
        """

        rp = p - ray_start
        rd = GeneralMath.vector_projection(rp, ray_direction)

        # Is d in ray? We can find k in (rd = k * ray_direction) without computing any norm
        # we use the largest dimension of ray_direction to avoid division by 0
        k = rd[0] / ray_direction[0] if abs(ray_direction[0]) > abs(ray_direction[1]) else rd[1] / ray_direction[1]

        if k <= 0:
            return np.linalg.norm(rp)
        else:
            return np.linalg.norm(p - (rd + ray_start))  # p-d

    @staticmethod
    def closest_point_on_ray_to_point(p: np.ndarray[float], ray_start: np.ndarray[float], ray_direction: np.ndarray[float]) -> np.ndarray:
        """
        Descrição:
            Obtém ponto na semireta mais perto do ponto p
        """

        rp = p - ray_start
        rd = GeneralMath.vector_projection(rp, ray_direction)

        # Is d in ray? We can find k in (rd = k * ray_direction) without computing any norm
        # we use the largest dimension of ray_direction to avoid division by 0
        k = rd[0] / ray_direction[0] if abs(ray_direction[0]) > abs(ray_direction[1]) else rd[1] / ray_direction[1]

        if k <= 0:
            return ray_start
        else:
            return rd + ray_start

    @staticmethod
    def does_circle_intersect_segment(centro: np.ndarray[float], raio: float, ponto_inicial: np.ndarray[float], ponto_final: np.ndarray[float]) -> bool:
        """
        Descrição:
            Retorna verdadeiro se o círculo definido por centro e raio específicos intercepta
            uma linha
        """

        ap = centro - ponto_inicial
        ab = ponto_final - ponto_inicial

        ad = GeneralMath.vector_projection(ap, ab)

        # Is d in ab? We can find k in (ad = k * ab) without computing any norm
        # we use the largest dimension of ab to avoid division by 0
        k = ad[0] / ab[0] if abs(ab[0]) > abs(ab[1]) else ad[1] / ab[1]

        if k <= 0:
            return np.dot(ap, ap) <= raio * raio
        elif k >= 1:
            return np.dot(centro - ponto_final, centro - ponto_final) <= raio * raio

        dp = centro - (ad + ponto_inicial)

        return np.dot(dp, dp) <= raio * raio

    @staticmethod
    def do_segments_intersect(a: np.ndarray[float], b: np.ndarray[float], c: np.ndarray[float], d: np.ndarray[float]) -> bool:
        """
        Descrição:
            Verifica se duas linhas 2D se interceptam, assume que são não coincidentes,
            utilizando o teste de interseção de segmentos por orientação.

            Verifica se os pontos a e b ficam de lados diferentes da linha formada por c - d e
            se c e d ficam de lados diferentes na linha formada por a - b. Caso sim, AB e CD se cruzam.

            O algoritmo é realmente complexo, sugiro que verifique:
            https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
        """

        CounterClockWise = lambda a_, b_, c_: (c_[1] - a_[1]) * (b_[0] - a_[0]) > (b_[1] - a_[1]) * (c_[0] - a_[0])

        return CounterClockWise(a, c, d) != CounterClockWise(b, c, d) and CounterClockWise(a, b, c) != CounterClockWise(a, b, d)

    @staticmethod
    def intersection_pt_segment_opp_goal(ponto_inicial: np.ndarray[float], ponto_final: np.ndarray[float]) -> np.ndarray[float] | None:
        """
        Descrição:
            Obtém ponto de interseção entre o segmento e a linha frontal do gol adversário.

            Retornará None caso:
            - Não haja interseção.
            - Ponto não esteja exatamente na linha frontal.
            - segmento tenha linha vertical.
        """
        vec_x = ponto_final[0] - ponto_inicial[0]

        # Collinear intersections are not accepted
        if vec_x == 0:
            return None

        k = (15.01 - ponto_inicial[0]) / vec_x

        # No collision
        if k < 0 or k > 1:
            return None

        intersection_pt = ponto_inicial + (ponto_final - ponto_inicial) * k

        if -1.01 <= intersection_pt[1] <= 1.01:
            return intersection_pt
        else:
            return None

    @staticmethod
    def intersection_pt_circle_opp_goal(centro: np.ndarray[float], raio: float) -> tuple[float, float] | None:
        """
        Descrição:
            Obtém pontos de interseção do círculo de centro e raio com linha frontal do gol.
            Apenas as coordenadas y são retornadas quando as coordenadas, dado que as coordenadas x
            são sempre iguais à 15.
        """

        x_dev = abs(15 - centro[0])

        if x_dev > raio:
            return None  # no intersection with x=15

        y_dev = sqrt(raio * raio - x_dev * x_dev)

        p1_y = max(centro[1] - y_dev, -1.01)
        p2_y = min(centro[1] + y_dev, 1.01)

        if p1_y == p2_y:
            return p1_y  # return the y coordinate of a single intersection point
        elif p2_y < p1_y:
            return None  # no intersection
        else:
            return p1_y, p2_y  # return the y coordinates of the intersection segment

    @staticmethod
    def distance_point_to_opp_goal(ponto: np.ndarray[float]) -> float:
        """
        Descrição:
            Distância entre ponto e linha frontal do gol.
        """

        if ponto[1] < -1.01:
            return np.linalg.norm(ponto - (15, -1.01))
        elif ponto[1] > 1.01:
            return np.linalg.norm(ponto - (15, 1.01))
        else:
            return abs(15 - ponto[0])

    @staticmethod
    def intersection_pts_circle_segment(circle_center, circle_radius, pt1, pt2, full_line=True, tangent_tol=1e-9) -> list[tuple[float, float]]:
        """
        Descrição:
            Achar pontos no círculo que interceptam um segmento de linha.
            Há 3 possibilidades: 0, 1, 2 pontos de interseção.
            Seguiremos o que está descrito em: http://mathworld.wolfram.com/Circle-LineIntersection.html

        Parâmetros:
            circle_center: The (x, y) location of the circle center
            circle_radius: The radius of the circle
            pt1: The (x, y) location of the first point of the segment
            pt2: The (x, y) location of the second point of the segment
            full_line: True to find intersections along full line - not just in the segment.  False will just return intersections within the segment.
            tangent_tol: Numerical tolerance at which we decide the intersections are close enough to consider it a tangent

        Retorno:
            Lista de comprimento 0, 1, 2, onde cada elemento é um ponto do círculo que intercepta o segmento de linha
        """

        (p1x, p1y), (p2x, p2y), (cx, cy) = pt1, pt2, circle_center
        (x1, y1), (x2, y2) = (p1x - cx, p1y - cy), (p2x - cx, p2y - cy)
        dx, dy = (x2 - x1), (y2 - y1)
        dr = (dx ** 2 + dy ** 2) ** .5
        big_d = x1 * y2 - x2 * y1
        discriminant = pow(circle_radius, 2) * pow(dr, 2) - pow(big_d, 2)

        if discriminant < 0:  # No intersection between circle and line
            return []
        else:  # There may be 0, 1, or 2 intersections with the segment
            intersections = [
                (cx + (big_d * dy + sign * (-1 if dy < 0 else 1) * dx * discriminant ** .5) / dr ** 2,
                 cy + (-big_d * dx + sign * abs(dy) * discriminant ** .5) / dr ** 2)
                for sign in ((1, -1) if dy < 0 else (-1, 1))]  # This makes sure the order along the segment is correct
            if not full_line:  # If only considering the segment, filter out intersections that do not fall within the segment
                fraction_along_segment = [
                    (xi - p1x) / dx if abs(dx) > abs(dy) else (yi - p1y) / dy for xi, yi in intersections]
                intersections = [pt for pt, frac in zip(
                    intersections, fraction_along_segment) if 0 <= frac <= 1]
            # If line is tangent to circle, return just one point (as both intersections have same location)
            if len(intersections) == 2 and abs(discriminant) <= tangent_tol:
                return [intersections[0]]
            else:
                return intersections

    @staticmethod
    def get_intersection_pt_lines(a1, a2, b1, b2):
        """
        Descrição:
            Obtém o ponto de interseção entre duas linhas.
            Adaptado a partir https://stackoverflow.com/questions/3252194/numpy-and-line-intersections
        """

        # Criamos um vetor stack vertical, no qual os arrays fornecidos são empilhados
        # gerando um array maior.
        stack = np.vstack([a1, a2, b1, b2])
        # Criamos um vetor stack horizontal
        h = np.hstack((stack, np.ones((4, 1))))
        # Obtemos o ponto de interseção
        x, y, z = np.cross(
                            np.cross(h[0], h[1]),
                            np.cross(h[2], h[3])
                          )
        if z == 0:  # lines are parallel
            return np.array([float('inf'), float('inf')])

        return np.array([x / z, y / z], float)

    @staticmethod
    def obter_diretorio_ativo(dir_: str) -> str:
        """Autoexplicativo"""
        global GLOBAL_DIR
        return GLOBAL_DIR + dir_

