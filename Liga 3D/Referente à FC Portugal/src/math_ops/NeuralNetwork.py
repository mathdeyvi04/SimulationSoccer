import numpy as np


# Nunca havia feito isso, simplesmente insano de foda.
def run_mlp(
        obs: np.ndarray,
        weights: list[tuple[np.ndarray, np.ndarray]],
        activation_function="tanh"
):
    """
    Descrição:
        Executa uma MLP (Perceptron Multicamadas) utilizando NumPy.

        A função simula a passagem de dados por uma rede neural do tipo MLP, onde cada camada é composta por um par
        (bias, kernel), e as ativações das camadas ocultas podem ser controladas via o parâmetro `activation_function`.

    Parâmetros:
        obs: np.ndarray
            Vetor de entrada (input) da rede neural, do tipo float32.
            Representa o estado observado ou dados a serem processados pela MLP.

        weights: list[tuple[np.ndarray, np.ndarray]]
            Lista contendo os pesos da rede.
            Cada elemento da lista representa uma camada na forma de uma tupla (bias, kernel), onde:
                - bias: vetor de bias da camada (shape: [units])
                - kernel: matriz de pesos da camada (shape: [units, input_dim])
            A última camada (output) é aplicada sem ativação.

            Exemplo:
                Camada 1 (entrada -> oculta1)
                Bias: [0.1, -0.2, 0.3]
                Pesos:
                [
                    [0.25, -0.40, 0.15],   # Neurônio 1
                    [0.10, 0.20, -0.30],   # Neurônio 2
                    [-0.05, 0.07, 0.05]    # Neurônio 3
                ]

                Camada 2 (oculta1 -> oculta2)
                Bias: [0.05, 0.12]
                Pesos:
                [
                    [0.50, -0.10, 0.20], # Neurônio 1
                    [-0.15, 0.30, 0.40]   # Neurônio 2
                ]

                Camada 3 (oculta2 -> saída)
                Bias: [0.02]
                Pesos:
                [
                    [0.20, -0.25]
                ]

        activation_function: str, default="tanh"
            Função de ativação usada nas camadas ocultas.
            - "tanh" aplica a tangente hiperbólica.
            - "none" desativa a função de ativação (linear).
            - Qualquer outro valor resultará em erro (NotImplementedError).

    Retorno:
        np.ndarray
            A saída da rede neural após todas as camadas serem processadas.
            É um vetor `np.float32` representando a predição final da MLP.
    """

    # Apenas setamos que o tipo dos valores deve ser float32
    # o 'copy=False' garante que não haverá cópia.
    obs = obs.astype(np.float32, copy=False)
    saida = obs  # Copiamos a referência

    # Cada elemento de weights, com exceção da última camada, é uma tupla
    # que contém o vetor de bias, index 0, e a matriz de pesos, index 1.
    # Verifique a documentação.
    for peso in weights[:-1]:

        # Multiplica a matriz de pesos w[1] pelo vetor de entrada saida.
        # Somando o bias ao resultado.
        saida = np.matmul(peso[1], saida) + peso[0]

        # A função de ativação não linear
        # O parâmetro out apenas define que será uma operação in-place, econominzando memória
        if activation_function == "tanh":
            np.tanh(saida, out=saida)

        # Vamos aprimorar e utilizar outra função de ativação
        elif activation_function == "relu":
            np.maximum(saida, 0, out=saida)

        # Caso não seja uma função registrada
        # nem seja 'none', daremos um erro.
        elif activation_function != "none":
            raise NotImplementedError("Função de Ativação inexistente, verifique math_ops/Neural_Network.py")

    # Produto matricial do último conjunto de pesos pela saída da última camada oculta.
    # Soma bias da última camada. Note que não é aplicado a função de ativação.
    return np.matmul(weights[-1][1], saida) + weights[-1][0]
