from time import sleep
from world.commons.Draw import Draw


# Apenas um ambiente para apresentarmos como funcionará a funcionalidade de desenho.

class Drawings:
    """
    Descrição:
        Classe utilitária responsável por demonstrar e facilitar o uso de primitivas gráficas (desenhos) no ambiente de simulação do agente robótico.
        Permite desenhar círculos, esferas, flechas, anotações, polígonos e linhas em tempo real, sem a necessidade de instanciar explicitamente um agente.
        É útil para depuração visual, demonstrações e marcação de referências no ambiente 3D.

    Métodos:
        __init__(self, script)
            Inicializa a classe com o script principal.

        execute(self)
            Executa um loop que desenha continuamente diversas formas e anotações no
            ambiente de simulação, demonstrando as funcionalidades gráficas disponíveis.
    """

    def __init__(self, script) -> None:
        """
        Descrição:
            Inicializa a classe Drawings, armazenando a referência ao script principal.

        Parâmetros:
            script: objeto Script
                Referência ao script principal para acesso aos argumentos de configuração.

        Retorno:
            None
        """
        self.script = script

    def execute(self) -> None:
        """
        Descrição:
            Executa um loop infinito, no qual diversas primitivas gráficas são desenhadas no ambiente de simulação.
            Demonstra o uso das funções de desenho para círculos, esferas, anotações, flechas, polígonos e linhas, com diferentes cores e estilos.
            O loop pode ser interrompido pelo usuário pressionando ctrl+c.

        Parâmetros:
            Não possui parâmetros.

        Retorno:
            None
        """

        # O objeto de desenho é criado automaticamente para cada agente,
        # mas aqui exemplificamos como desenhar sem criar um agente.
        # Normalmente, acessaríamos o objeto por player.world.draw.
        a = self.script.args
        draw = Draw(True, 0, a.i, 32769)

        print("\nPress ctrl+c to return.")

        while True:
            for i in range(100):
                sleep(0.02)

                # Desenha dois círculos concêntricos de cores diferentes
                draw.circle((0, 0), i / 10, 2, Draw.Color.green_light, "green")
                draw.circle((0, 0), i / 9, 2, Draw.Color.red, "red")

                # Desenha uma esfera simulando uma bola em movimento
                draw.sphere((0, 0, 5 - i / 20), 0.2, Draw.Color.red, "ball")

                # Adiciona uma anotação textual na posição especificada
                draw.annotation((0, 0, 1), "Hello!", Draw.Color.cyan, "text")

                # Desenha uma flecha com cor personalizada, mudando de tamanho ao longo do tempo
                draw.arrow((0, 0, 5), (0, 0, 5 - i / 25), 0.5, 4, Draw.Color.get(127, 50, 255), "my_arrow")

                # Desenha uma pirâmide usando polígonos e linhas
                draw.polygon(((2, 0, 0), (3, 0, 0), (3, 1, 0), (2, 1, 0)), Draw.Color.blue, 255, "solid", False)
                draw.line((2, 0, 0), (2.5, 0.5, 1), 2, Draw.Color.cyan, "solid", False)
                draw.line((3, 0, 0), (2.5, 0.5, 1), 2, Draw.Color.cyan, "solid", False)
                draw.line((2, 1, 0), (2.5, 0.5, 1), 2, Draw.Color.cyan, "solid", False)
                draw.line((3, 1, 0), (2.5, 0.5, 1), 2, Draw.Color.cyan, "solid", True)
