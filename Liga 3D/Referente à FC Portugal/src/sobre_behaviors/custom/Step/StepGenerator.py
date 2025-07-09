import math


class StepGenerator:
    """
    Descrição:
        Gerador de caminhos de passo para robôs bípedes.

        Implementa um modelo simples para gerar a posição das pernas (pé esquerdo e direito)
        ao dar um passo, considerando o desvio em y, a elevação do pé e a duração do passo.

        A posição é gerada de acordo com uma onda sinusoidal, enquanto o robô se desloca para a frente,
        a gravidade é usada para o modelo dinâmico de oscilação.

    Métodos Disponíveis:
        - get_target_positions

    Variáveis de Ambiente
        GRAVITY : float, opcional (valor padrão = 9.81)
            A gravidade usada nas equações de oscilação.
        Z0 : float, opcional (valor padrão = 0.2)
            Altura base do centro de massa.
    """

    GRAVITY = 9.81
    Z0 = 0.2

    def __init__(self, feet_y_dev, sample_time, max_ankle_z) -> None:
        """
        Inicializa o gerador de caminhos de passo para um robô bípede.

        Parâmetros:
            feet_y_dev : float
                Desvio lateral dos pés em relação ao centro do robô.
            sample_time : float
                Duração de um passo de tempo, em segundos.
            max_ankle_z : float
                Elevação máxima que o pé consegue atingir.

        Retorno:
            Inicialização dos seguintes atributos:

            feet_y_dev : float
                Desvio lateral dos pés em relação ao centro do robô.
            sample_time : float
                Duração de um passo de tempo.
            max_ankle_z : float
                Elevação máxima que o pé consegue atingir.
            state_is_left_active : bool
                Indica se a perna que se moverá neste passo é a perna esquerda.
            state_current_ts : int
                Quantidade de instantes já processados neste passo.
            switch: bool
                Sinaliza que é preciso trocar de perna no próximo passo.
            external_progress : float
                Progresso externo do passo, variando de 0.0 a 1.0.
        """

        self.feet_y_dev = feet_y_dev
        self.sample_time = sample_time
        self.state_is_left_active = False
        self.state_current_ts = 0
        self.switch = False  # switch legs
        self.external_progress = 0  # non-overlaped progress
        self.max_ankle_z = max_ankle_z

        # Apenas para não termos problemas.
        self.ts_per_step = 0
        self.swing_height = 0
        self.max_leg_extension = 0

    def get_target_positions(
            self,
            reset: bool,
            ts_per_step: int,
            z_span: float,
            z_extension: float
    ) -> tuple[float, float, float, float]:
        """
            Descrição:
                Calcula as posições-alvo para cada perna do robô bípede em um passo de marcha.

            Parâmetros:
                reset: bool
                    Se True, reinicia o passo, resetando o estado atual.
                ts_per_step : int
                    Duração de um passo em números de instantes de tempo.
                z_span : float
                    Altura do passo, ou a elevação que o pé deve atingir enquanto se mover.
                z_extension : float
                    Extensão máxima da perna, ou a distância do tornozelo ao centro das articulações dos quadris.

            Retorno:
                target: `tuple`
                    Uma tupla com quatro elementos:
                    (Posição y perna esquerda, elevação perna esquerda,
                     Posição y perna direita, elevação perna direita).
            """

        assert isinstance(ts_per_step, int) and ts_per_step > 0, "ts_per_step deve ser um inteiro maior que zero!"

        # -------------------------- Avança um passo de tempo
        if reset:
            self.ts_per_step = ts_per_step  # Duração do passo em instantes de tempo
            self.swing_height = z_span  # Altura do passo
            self.max_leg_extension = z_extension  # Extensão máxima da perna
            self.state_current_ts = 0  # Zera o timestamp atual
            self.state_is_left_active = False  # Começa com a perna direita como perna de apoio
            self.switch = False  # Desativa a mudança de perna
        elif self.switch:
            self.state_current_ts = 0  # Reinicia o timestamp
            self.state_is_left_active = not self.state_is_left_active  # Realiza a mudança de perna
            self.switch = False
        else:
            self.state_current_ts += 1  # Avança o timestamp

        # -------------------------- Cálculo do COM em y
        W = math.sqrt(self.Z0 / self.GRAVITY)

        step_time = self.ts_per_step * self.sample_time
        time_delta = self.state_current_ts * self.sample_time

        y0 = self.feet_y_dev  # Desvio lateral dos pés
        y_swing = y0 + y0 * (math.sinh((step_time - time_delta) / W) + math.sinh(time_delta / W)) / math.sinh(-step_time / W)

        # -------------------------- Limitando a extensão máxima e a elevação do passo
        z0 = min(-self.max_leg_extension, self.max_ankle_z)  # Altura base limitada
        zh = min(self.swing_height, self.max_ankle_z - z0)  # Elevação do passo limitada

        # -------------------------- Cálculo do Z do pé que se move
        progress = self.state_current_ts / self.ts_per_step
        self.external_progress = self.state_current_ts / (self.ts_per_step - 1)
        active_z_swing = zh * math.sin(math.pi * progress)

        # -------------------------- Aplicação das configurações para o próximo passo
        if self.state_current_ts + 1 >= self.ts_per_step:
            self.ts_per_step = ts_per_step  # Duração do próximo passo
            self.swing_height = z_span  # Altura do próximo passo
            self.max_leg_extension = z_extension  # Extensão da perna para o próximo passo
            self.switch = True  # Realiza a mudança de perna no próximo passo

        # -------------------------- Distinção da perna ativa
        if self.state_is_left_active:
            return y0 + y_swing, active_z_swing + z0, -y0 + y_swing, z0
        else:
            return y0 - y_swing, z0, -y0 - y_swing, active_z_swing + z0
