from math_ops.Matriz4x4 import Matriz4x4


class BodyPart:
    """
    Descrição:
        Responsável por representar partes do corpo e suas respectivas
        juntas e massa geral.
    """

    def __init__(self, massa: int | float) -> None:
        self.mass = massa
        self.joints = []
        # Matriz de transformação da parte do corpo para cabeça
        self.transform = Matriz4x4()
