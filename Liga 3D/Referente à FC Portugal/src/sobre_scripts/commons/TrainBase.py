from datetime import datetime, timedelta
from itertools import count
from os import listdir
from os.path import isdir, join, isfile
from scripts.commons.UserInterace import UserInterace as Ui
from shutil import copy
from typing import Callable
from world.World import World
from xml.dom import minidom
import numpy as np
import os, time, math, csv, select, sys
import pickle
import xml.etree.ElementTree as xmt

from stable_baselines3 import PPO
from stable_baselines3.common.base_class import BaseAlgorithm
from stable_baselines3.common.callbacks import EvalCallback, CheckpointCallback, CallbackList, BaseCallback


class TrainBase:

    def __init__(self, script) -> None:
        """
        Descrição:
            Inicializa a instância da classe, configurando variáveis relacionadas à comunicação com o servidor,
            portas para monitoramento, informações do robô, equipe e parâmetros para controle de tempo.
            Ao utilizar múltiplos ambientes (multiprocessamento) para treinamento, as portas do servidor
            e do monitor são incrementadas para evitar conflitos. Durante o teste, as portas iniciais são utilizadas.

        Parâmetros:
            script: objeto
                Objeto script que contém os argumentos de configuração necessários (tipicamente do argparse).
        """
        # Extrai os argumentos do objeto script
        args = script.args
        # Armazena o objeto script para uso posterior
        self.script = script
        # Endereço IP do servidor
        self.ip = args.i
        # Porta inicial do servidor para conexão
        self.server_p = args.p  # (initial) server port
        # Porta de monitoramento utilizada durante os testes
        self.monitor_p = args.m  # monitor port when testing
        # Porta de monitoramento inicial incrementada em 1000 para treinos com múltiplos ambientes
        self.monitor_p_1000 = args.m + 1000  # initial monitor port when training
        # Tipo de robô a ser utilizado
        self.robot_type = args.r
        # Número ou lado do time
        self.team = args.t
        # Uniforme do robô (identificador visual)
        self.uniform = args.u
        # Variáveis de controle para estimativa de tempo (utilizadas em simulação ou controle de frequência)
        self.cf_last_time = 0
        self.cf_delay = 0
        # Período alvo da simulação durante o teste (por padrão, tempo real)
        self.cf_target_period = World.STEPTIME  # target simulation speed while testing (default: real-time)

    @staticmethod
    def prompt_user_for_model():
        """
        Descrição:
            Permite ao usuário selecionar interativamente um modelo salvo em formato .zip a partir das pastas de logs do Gym.
            Lista as pastas de logs disponíveis, permite ao usuário escolher uma, então lista os modelos (.zip) nela,
            permitindo a seleção. Retorna um dicionário com caminhos e nomes selecionados ou None se o usuário cancelar.

            - Requer que o diretório './scripts/gyms/logs/' exista e contenha subpastas de logs.
            - Apenas arquivos '.zip' são considerados modelos válidos.
            - A ordenação das listas (pastas e modelos) é feita pela data de modificação (mais recentes primeiro).

        Retorno
            dict ou None
                Um dicionário com:
                    - 'folder_dir': caminho absoluto da pasta escolhida
                    - 'folder_name': nome da pasta escolhida
                    - 'model_file': caminho absoluto do arquivo .zip do modelo escolhido
                Ou None caso o usuário pressione Ctrl+C para cancelar a operação.
        """
        gyms_logs_path = "./sobre_scripts/gyms/logs/"
        # Lista as pastas presentes no diretório de logs, considerando apenas diretórios
        folders = [f for f in listdir(gyms_logs_path) if isdir(join(gyms_logs_path, f))]
        # Ordena as pastas pela data de modificação, mais recentes primeiro
        folders.sort(key=lambda f: os.path.getmtime(join(gyms_logs_path, f)), reverse=True)

        model_name = ""
        while True:
            try:
                # Mostra a lista de pastas e solicita ao usuário que escolha uma
                folder_name = Ui.apresentar_lista(folders, prompt="Escolha uma pasta de modelos salvos (ctrl+c to return): ")[1]
            except KeyboardInterrupt:
                print()
                return None  # Retorna None se o usuário cancelar com Ctrl+C

            folder_dir = os.path.join(gyms_logs_path, folder_name)
            # Lista todos os arquivos .zip na pasta escolhida (modelos salvos)
            models = [m[:-4] for m in listdir(folder_dir) if isfile(join(folder_dir, m)) and m.endswith(".zip")]

            if not models:
                print("A pasta escolhida não possui arquivo .zip!!")
                continue  # Volta a pedir para escolher outra pasta caso não haja modelos

            # Ordena os modelos pela data de modificação, mais recentes primeiro
            models.sort(key=lambda m: os.path.getmtime(join(folder_dir, m + ".zip")), reverse=True)

            try:
                # Mostra a lista de modelos e solicita ao usuário que escolha um
                model_name = Ui.apresentar_lista(models, prompt="Escolhe um modelo (ctrl+c to return): ")[1]
                break  # Sai do loop ao escolher um modelo com sucesso
            except KeyboardInterrupt:
                print()  # Permite ao usuário cancelar a escolha de modelo e voltar para pasta

        # Retorna as informações necessárias sobre o modelo selecionado
        return {
            "folder_dir": folder_dir,
            "folder_name": folder_name,
            "model_file": os.path.join(folder_dir, model_name + ".zip")
        }

    def control_fps(self, read_input=False) -> None:
        """
        Descrição:
            Controla a velocidade da simulação ajustando o intervalo entre os frames (FPS - Frames Per Second).
            Permite ao usuário definir a velocidade da simulação manualmente, inclusive pausando ou acelerando,
            e ajusta o delay entre os ciclos do loop para manter o ritmo desejado.

        Parâmetros
            read_input: bool, opcional
                Se True, solicita ao usuário que insira a velocidade da simulação via input.
                Se False, apenas executa o controle de FPS com a configuração atual.

        Retorno
            None
                O método ajusta o tempo de espera entre os frames, mas não retorna valor.
        """

        # Se for solicitado, lê o input do usuário para ajustar a velocidade da simulação
        if read_input:
            speed = input("Insira uma velocidade: ")
            if speed == '':
                # Se o input for vazio, define a velocidade máxima (sem delay)
                self.cf_target_period = 0
                print(f"Velocidade de Simulação alterada para MAX")
            else:
                if speed == '0':
                    # Pausa a simulação e permite ao usuário definir nova velocidade
                    inp = input("Pausado, digite uma nova velocidade ou '' para usar a velocidade anterior: ")
                    if inp != '':
                        speed = inp

                try:
                    # Converte o input para inteiro, garante que é não-negativo e calcula novo período alvo
                    speed = int(speed)
                    assert speed >= 0
                    self.cf_target_period = World.STEPTIME * 100 / speed
                    print(f"Velocidade da Simulação Aterada para {speed}%")
                except:
                    # Mensagem de erro caso o input seja inválido
                    print("""TrainBase.py: 
        Error: To control the simulation speed, enter a non-negative integer.
        To disable this control module, use test_model(..., enable_FPS_control=False) in your gym environment.""")

        # Calcula o tempo decorrido desde o último frame
        now = time.time()
        period = now - self.cf_last_time
        self.cf_last_time = now
        # Ajusta o delay acumulado para compensar variações no tempo de execução do frame
        self.cf_delay += (self.cf_target_period - period) * 0.9
        if self.cf_delay > 0:
            # Adiciona delay para manter a simulação na velocidade desejada
            time.sleep(self.cf_delay)
        else:
            # Garante que o delay não seja negativo
            self.cf_delay = 0

    def test_model(
            self,
            model: BaseAlgorithm,
            env,
            log_path: str = None,
            model_path: str = None,
            max_episodes=0,
            enable_FPS_control=True,
            verbose=1
    ) -> None:
        """
        Descrição:
            Testa um modelo de Aprendizagem por Reforço (RL) em um ambiente Gym-like, registra estatísticas de desempenho,
            permite controle da velocidade da simulação e pode exibir avaliações e salvar logs dos testes.
            O método executa episódios sucessivos até atingir o número máximo especificado ou até que o usuário aborte.

        Parâmetros:
            model: BaseAlgorithm
                Modelo treinado (por exemplo, PPO, DQN, etc. do stable-baselines3) a ser testado.
            env: Env
                Ambiente compatível com Gym onde o modelo será avaliado.
            log_path: str, opcional
                Pasta onde o arquivo de estatísticas dos testes será salvo.
                Se None, não salva o arquivo.
            model_path: str, opcional
                Pasta onde busca o arquivo 'evaluations.npz' para exibir avaliações e criar um arquivo CSV correspondente.
                Se None, não exibe avaliações nem gera CSV.
            max_episodes: int, opcional
                Número máximo de episódios a serem testados.
                O padrão é 0, o que significa que os testes continuam até interrupção manual.
            enable_FPS_control: bool, opcional
                Se True, permite controle interativo da velocidade da simulação durante o teste.
            verbose: int, opcional
                Nível de verbosidade:
                    0 - sem saída (exceto mensagens de controle de FPS)
                    1 - imprime estatísticas de cada episódio

        Retorno:
            O método executa os testes e registra estatísticas.
        """
        # Se fornecido, exibe avaliações do modelo a partir do diretório especificado
        if model_path is not None:
            assert os.path.isdir(model_path), f"{model_path} is not a valid path"
            TrainBase.display_evaluations(model_path)

        if log_path is not None:
            # Valida o diretório do log
            assert os.path.isdir(log_path), f"{log_path} is not a valid path"

            # Evita sobrescrever arquivos de log existentes
            if os.path.isfile(log_path + "/test.csv"):
                for i in range(1000):
                    p = f"{log_path}/test_{i:03}.csv"
                    if not os.path.isfile(p):
                        log_path = p
                        break
            else:
                log_path += "/test.csv"

            # Cria o arquivo de log e escreve o cabeçalho
            with open(log_path, 'w') as f:
                f.write("reward,ep. length,rew. cumulative avg., ep. len. cumulative avg.\n")
            print("Estatísticas de Treino salvas em:", log_path)

        # Informa sobre o controle de velocidade da simulação
        if enable_FPS_control:
            print("\nA velocidade de simulação pode ser alterada por um inteiro não negativo\n"
                  "(e.g. '50' seta velocidade para 50%, '0' pausa simulação, '' seta velocidade para MAX)\n")

        # Inicializa variáveis de estatísticas dos episódios
        ep_reward = 0
        ep_length = 0
        rewards_sum = 0
        reward_min = math.inf
        reward_max = -math.inf
        ep_lengths_sum = 0
        ep_no = 0

        # Reseta o ambiente para obter a observação inicial
        obs = env.reset()
        while True:
            # O modelo faz uma predição de ação baseada na observação atual
            action, _states = model.predict(obs, deterministic=True)
            # Executa a ação no ambiente
            obs, reward, done, info = env.step(action)
            # Acumula recompensa e comprimento do episódio
            ep_reward += reward
            ep_length += 1

            # Se habilitado, controla a velocidade da simulação de forma interativa
            if enable_FPS_control:
                self.control_fps(
                    bool(select.select([sys.stdin], [], [], 0)[0])
                )

            if done:
                # Episódio finalizado: reseta o ambiente e atualiza estatísticas
                obs = env.reset()
                rewards_sum += ep_reward
                ep_lengths_sum += ep_length
                reward_max = max(ep_reward, reward_max)
                reward_min = min(ep_reward, reward_min)
                ep_no += 1
                avg_ep_lengths = ep_lengths_sum / ep_no
                avg_rewards = rewards_sum / ep_no

                # Exibe estatísticas do episódio se verbose > 0
                if verbose > 0:
                    print(
                        f"\rEpisode: {ep_no:<3}  Ep.Length: {ep_length:<4.0f}  Reward: {ep_reward:<6.2f}                                                             \n",
                        end=f"--AVERAGE--   Ep.Length: {avg_ep_lengths:<4.0f}  Reward: {avg_rewards:<6.2f}  (Min: {reward_min:<6.2f}  Max: {reward_max:<6.2f})",
                        flush=True
                    )

                # Salva estatísticas no arquivo de log, se especificado
                if log_path is not None:
                    with open(log_path, 'a') as f:
                        writer = csv.writer(f)
                        writer.writerow([ep_reward, ep_length, avg_rewards, avg_ep_lengths])

                # Verifica se atingiu o número máximo de episódios
                if ep_no == max_episodes:
                    return

                # Reseta contadores do episódio
                ep_reward = 0
                ep_length = 0

    @staticmethod
    def learn_model(
            model: BaseAlgorithm,
            total_steps: int,
            path: str,
            eval_env=None,
            eval_freq=None,
            eval_eps=5,
            save_freq=None,
            backup_env_file=None,
            export_name=None
    ) -> str:
        """
        Descrição:
            Treina um modelo de Aprendizagem por Reforço (RL) por um número especificado de passos de ambiente (env steps),
            salva checkpoints, realiza avaliações periódicas e gera backups, conforme especificado pelos parâmetros.
            O método evita sobrescrita de diretórios existentes, organiza callbacks para avaliação, checkpoint e exportação,
            e exibe informações de treinamento ao final.

        Parâmetros:
            model: BaseAlgorithm
                Modelo de RL a ser treinado.
            total_steps: int
                Número total de passos de ambiente para treinar o modelo.
            path: str
                Caminho onde o modelo treinado será salvo. Caso o diretório exista, um sufixo incremental é adicionado.
            eval_env: Env, opcional
                Ambiente de avaliação usado para testar periodicamente o modelo durante o treinamento.
            eval_freq: int, opcional
                Frequência (em passos de ambiente) para avaliação periódica do modelo.
            eval_eps: int, opcional
                Número de episódios usados durante cada avaliação (default: 5).
            save_freq: int, opcional
                Frequência (em passos de ambiente) para salvar checkpoints do modelo.
            backup_env_file: str, opcional
                Caminho para arquivo do ambiente a ser copiado como backup na pasta do modelo.
            export_name: str, opcional
                Nome usado ao exportar modelos periodicamente (requer save_freq).

        Retorno
            path: str
                Caminho final onde o modelo foi salvo (considerando possível sufixo incremental).
        """
        start = time.time()
        start_date = datetime.now().strftime("%d/%m/%Y %H:%M:%S")

        # Se o diretório de destino já existe, adiciona sufixo incremental para não sobrescrever
        if os.path.isdir(path):
            for i in count():
                p = path.rstrip("/") + f'_{i:03}/'
                if not os.path.isdir(p):
                    path = p
                    break
        os.makedirs(path)

        # Faz backup do arquivo de ambiente, se especificado
        backup_file = ""
        if backup_env_file is not None:
            backup_file = os.path.join(path, os.path.basename(backup_env_file))
            copy(backup_env_file, backup_file)

        # Verifica se a avaliação periódica está habilitada
        evaluate = bool(eval_env is not None and eval_freq is not None)

        # Cria callback de avaliação, se aplicável
        eval_callback = None if not evaluate else EvalCallback(
            eval_env,
            n_eval_episodes=eval_eps,
            eval_freq=eval_freq,
            log_path=path,
            best_model_save_path=path,
            deterministic=True,
            render=False
        )

        # Cria callback customizado para exibir avaliações periodicamente
        custom_callback = None if not evaluate else Cyclic_Callback(
            eval_freq, lambda: TrainBase.display_evaluations(path, True)
        )

        # Cria callback de checkpoint, se aplicável
        checkpoint_callback = None if save_freq is None else CheckpointCallback(
            save_freq=save_freq, save_path=path, name_prefix="model", verbose=1
        )

        # Cria callback de exportação, se aplicável
        export_callback = None if save_freq is None or export_name is None else Export_Callback(
            save_freq, path, export_name
        )

        # Agrupa callbacks ativos em uma lista
        callbacks = CallbackList([
            c for c in [
                eval_callback, custom_callback, checkpoint_callback, export_callback
            ] if c is not None
        ])

        # Inicia o treinamento do modelo usando os callbacks configurados
        model.learn(total_timesteps=total_steps, callback=callbacks)
        # Salva o modelo final com o nome 'last_model'
        model.save(os.path.join(path, "last_model"))

        # Exibe avaliações finais, se houver
        if evaluate:
            TrainBase.display_evaluations(path)

        # Exibe datas e duração do treinamento, além do caminho do modelo
        end_date = datetime.now().strftime('%d/%m/%Y %H:%M:%S')
        duration = timedelta(seconds=int(time.time() - start))
        print(f"Train start:     {start_date}")
        print(f"Train end:       {end_date}")
        print(f"Train duration:  {duration}")
        print(f"Model path:      {path}")

        # Adiciona informações de data e duração ao backup do ambiente, se houver
        if backup_env_file is not None:
            with open(backup_file, 'a') as f:
                f.write(f"\n# Train start:    {start_date}\n")
                f.write(f"# Train end:      {end_date}\n")
                f.write(f"# Train duration: {duration}")

        return path

    @staticmethod
    def display_evaluations(path, save_csv=False):
        """
        Descrição:
            Exibe, no terminal, gráficos simplificados de evolução dos resultados de avaliação (recompensa média e comprimento médio dos episódios)
            a partir do arquivo 'evaluations.npz' localizado no diretório fornecido. Os gráficos são desenhados com caracteres Unicode.
            Opcionalmente, salva os resultados atuais em um arquivo 'evaluations.csv'.

        Parâmetros:
            path: str
                Caminho do diretório onde está o arquivo 'evaluations.npz'.
            save_csv: bool, opcional
                Se True, salva os resultados da avaliação em 'evaluations.csv' no mesmo diretório.
        """

        eval_npz = os.path.join(path, "evaluations.npz")

        # Se não existir o arquivo de avaliações, retorna sem fazer nada
        if not os.path.isfile(eval_npz):
            return

        # Configurações de largura e altura do "console gráfico"
        console_width = 80
        console_height = 18
        symb_x = "\u2022"  # símbolo para recompensa
        symb_o = "\u007c"  # símbolo para tamanho do episódio
        symb_xo = "\u237f"  # símbolo para sobreposição

        # Carrega os dados do arquivo .npz (timesteps, recompensas, comprimentos dos episódios)
        with np.load(eval_npz) as data:
            time_steps = data["timesteps"]
            results_raw = np.mean(data["results"], axis=1)
            ep_lengths_raw = np.mean(data["ep_lengths"], axis=1)
        sample_no = len(results_raw)

        # Interpola os dados para preencher a largura do console
        xvals = np.linspace(0, sample_no - 1, console_width)
        results = np.interp(xvals, range(sample_no), results_raw)
        ep_lengths = np.interp(xvals, range(sample_no), ep_lengths_raw)

        # Define limites dos gráficos (mínimo e máximo)
        results_limits = np.min(results), np.max(results)
        ep_lengths_limits = np.min(ep_lengths), np.max(ep_lengths)

        # Discretiza os dados para alinhar às linhas do "console gráfico"
        results_discrete = np.digitize(results, np.linspace(results_limits[0] - 1e-5, results_limits[1] + 1e-5, console_height + 1)) - 1
        ep_lengths_discrete = np.digitize(ep_lengths, np.linspace(0, ep_lengths_limits[1] + 1e-5, console_height + 1)) - 1

        # Inicializa matriz para desenhar os gráficos (camada 0: rewards, camada 1: length)
        matrix = np.zeros((console_height, console_width, 2), int)
        matrix[results_discrete[0]][0][0] = 1  # desenha 1ª coluna recompensa
        matrix[ep_lengths_discrete[0]][0][1] = 1  # desenha 1ª coluna comprimento
        rng = [[results_discrete[0], results_discrete[0]], [ep_lengths_discrete[0], ep_lengths_discrete[0]]]

        # Preenche as linhas dos gráficos (cria linhas contínuas nas duas curvas)
        for k in range(2):
            for i in range(1, console_width):
                x = [results_discrete, ep_lengths_discrete][k][i]
                if x > rng[k][1]:
                    rng[k] = [rng[k][1] + 1, x]
                elif x < rng[k][0]:
                    rng[k] = [x, rng[k][0] - 1]
                else:
                    rng[k] = [x, x]
                for j in range(rng[k][0], rng[k][1] + 1):
                    matrix[j][i][k] = 1

        # Desenha o gráfico no console, linha por linha (de cima para baixo)
        print(f'{"-" * console_width}')
        for l in reversed(range(console_height)):
            for c in range(console_width):
                if np.all(matrix[l][c] == 0):
                    print(end=" ")  # vazio
                elif np.all(matrix[l][c] == 1):
                    print(end=symb_xo)  # ambos sobrepostos
                elif matrix[l][c][0] == 1:
                    print(end=symb_x)  # apenas recompensa
                else:
                    print(end=symb_o)  # apenas comprimento
            print()
        print(f'{"-" * console_width}')
        print(f"({symb_x})-reward          min:{results_limits[0]:11.2f}    max:{results_limits[1]:11.2f}")
        print(f"({symb_o})-ep. length      min:{ep_lengths_limits[0]:11.0f}    max:{ep_lengths_limits[1]:11.0f}    {time_steps[-1] / 1000:15.0f}k steps")
        print(f'{"-" * console_width}')

        # Salva os dados no CSV, se solicitado
        if save_csv:
            eval_csv = os.path.join(path, "evaluations.csv")
            with open(eval_csv, 'a+') as f:
                writer = csv.writer(f)
                if sample_no == 1:
                    writer.writerow(["time_steps", "reward ep.", "length"])
                writer.writerow([time_steps[-1], results_raw[-1], ep_lengths_raw[-1]])

    @staticmethod
    def generate_slot_behavior(path: str, slots: list, auto_head: bool, XML_name: str):
        """
        Descrição:
            Gera um arquivo XML que descreve o comportamento otimizado dos slots de um sistema (por exemplo, robôs ou autômatos).
            O arquivo gerado sobrescreve qualquer arquivo XML anterior com o mesmo nome no diretório especificado.
            Cada slot representa uma configuração temporal e angular de movimentos em juntas específicas, e pode conter o atributo auto_head.

        Parâmetros
            path: str
                Diretório onde o arquivo XML será salvo.
            slots: list
                Lista de slots, onde cada slot é uma tupla contendo:
                    - s[0]: delta de tempo (em milissegundos)
                    - s[1]: lista de índices das juntas a serem movimentadas
                    - s[2]: dicionário {id: ângulo} para cada junta movimentada
            auto_head: bool
                Se True, adiciona o atributo auto_head="1" ao XML; caso contrário, auto_head="0".
            XML_name: str
                Nome do arquivo XML a ser criado.

        Retorno
            Apenas gera o arquivo XML e exibe uma mensagem de sucesso na tela.
        """

        # Monta o caminho completo do arquivo XML
        file = os.path.join(path, XML_name)

        # Cria a estrutura base do XML, com o atributo auto_head
        auto_head = '1' if auto_head else '0'
        EL_behavior = xmt.Element('behavior', {
            'description': 'Adicione uma descrição!',
            "auto_head": auto_head
        }
                                  )

        # Para cada slot, adiciona um elemento <slot> com atributo delta (tempo em segundos)
        for i, s in enumerate(slots):
            EL_slot = xmt.SubElement(EL_behavior, 'slot', {'delta': str(s[0] / 1000)})
            # Para cada junta do slot, adiciona um elemento <move> com id e ângulo
            for j in s[1]:  # Percorre todos os índices das juntas
                xmt.SubElement(EL_slot, 'move', {'id': str(j), 'angle': str(s[2][j])})

        # Converte a árvore XML para string formatada e salva no arquivo
        xml_rough = xmt.tostring(EL_behavior, 'utf-8')
        xml_pretty = minidom.parseString(xml_rough).toprettyxml(indent="    ")
        with open(file, "w") as x:
            x.write(xml_pretty)

        # Informa que o arquivo foi criado com sucesso
        print(file, "foi criado!")

    @staticmethod
    def linear_schedule(initial_value: float) -> Callable[[float], float]:
        """
        Descrição:
            Cria uma função de agendamento (schedule) linear para taxa de aprendizado ("learning rate").
            Essa função retorna outra função que, dada a fração de progresso restante no treinamento (de 1 a 0),
            calcula o valor atual da taxa de aprendizado de forma linear a partir do valor inicial.

        Parâmetros
            initial_value: float
                Valor inicial da taxa de aprendizado (learning rate) no início do treinamento.

        Retorno
            schedule: Callable[[float], float]
                Função que recebe o progresso restante (float entre 1 e 0) e retorna o learning rate correspondente (float).
        """

        def func(progress_remaining: float) -> float:
            """
                Calcula a taxa de aprendizado conforme o progresso restante.

            Parâmetros
                progress_remaining: float
                    Progresso restante, variando de 1 (início do treinamento) até 0 (fim do treinamento).

            Retorno
                learning_rate: float
                    Valor da taxa de aprendizado correspondente ao progresso atual.
            """
            return progress_remaining * initial_value

        return func

    @staticmethod
    def export_model(input_file, output_file, add_sufix=True):
        """
        Descrição:
            Exporta os pesos do modelo PPO treinado para um arquivo binário (.pkl), no formato compatível para posterior uso ou análise.
            Não sobrescreve arquivos já existentes: se add_sufix=True e o arquivo de saída já existe, será adicionado um sufixo incremental ao nome do arquivo.
            O arquivo exportado conterá os pesos e biases das camadas ocultas e da camada de saída, organizados em uma lista.

        Parâmetros:
            input_file : str
                Caminho do arquivo de entrada, compatível com o algoritmo PPO do stable-baselines3 (formato .zip normalmente).
            output_file : str
                Caminho do arquivo de saída (inclui diretório e nome base, sem extensão obrigatória).
            add_sufix : bool, opcional
                Se True (padrão), adiciona um sufixo incremental ao nome do arquivo de saída para evitar sobrescrita (ex: "_000.pkl", "_001.pkl", ...).

        Retorno
            Apenas salva o arquivo binário contendo os pesos do modelo.
        """

        # Se solicitado, evita sobrescrever arquivos já existentes adicionando sufixos ao arquivo de saída
        if add_sufix:
            for i in count():
                f = f"{output_file}_{i:03}.pkl"
                if not os.path.isfile(f):
                    output_file = f
                    break

        # Carrega o modelo PPO treinado a partir do arquivo de entrada
        model = PPO.load(input_file)
        # Obtém o dicionário de pesos da rede neural
        weights = model.policy.state_dict()  # dicionário com os pesos das camadas

        # Função auxiliar para extrair e converter os pesos para numpy arrays
        w = lambda name: weights[name].detach().cpu().numpy()

        var_list = []
        # Adiciona camadas ocultas: percorre os índices das camadas (step=2 conforme organização interna do SB3)
        for i in count(0, 2):
            # Verifica se a camada existe; se não existir, encerra o loop
            if f"mlp_extractor.policy_net.{i}.bias" not in weights:
                break
            # Adiciona bias, weight e função de ativação ("tanh") da camada oculta
            var_list.append([w(f"mlp_extractor.policy_net.{i}.bias"), w(f"mlp_extractor.policy_net.{i}.weight"), "tanh"])

        # Adiciona camada final da rede de ação (sem ativação "none")
        var_list.append([w("action_net.bias"), w("action_net.weight"), "none"])

        # Salva a lista de variáveis (pesos) em arquivo binário com pickle (protocol 4 para retrocompatibilidade)
        with open(output_file, "wb") as f:
            pickle.dump(var_list, f, protocol=4)


class Cyclic_Callback(BaseCallback):
    """
    Descrição:
        Callback personalizado para o Stable Baselines que executa uma função customizada a cada `freq` passos durante o treinamento.
        Pode ser usado, por exemplo, para exibir métricas, plotar gráficos ou realizar qualquer ação periódica sem interromper o aprendizado.
    """

    def __init__(self, freq, function: Callable):
        # Inicializa o callback base da biblioteca (chama a cada passo)
        super(Cyclic_Callback, self).__init__(1)
        self.freq = freq  # frequência de execução da função
        self.function = function  # função a ser chamada periodicamente

    def _on_step(self) -> bool:
        # Executa a função se a frequência for atingida
        if self.n_calls % self.freq == 0:
            self.function()
        # Retorna True para continuar o treinamento (False interromperia)
        return True


class Export_Callback(BaseCallback):
    """
    Descrição:
        Callback personalizado para o Stable Baselines que exporta o modelo treinado periodicamente durante o processo de treinamento.
        Salva o modelo compactado (.zip) e exporta seus pesos para um local específico para análise ou deploy externo.
    """

    def __init__(self, freq, load_path, export_name):
        # Inicializa o callback base da biblioteca (chama a cada passo)
        super(Export_Callback, self).__init__(1)
        self.freq = freq  # frequência de exportação
        self.load_path = load_path  # caminho para o modelo salvo
        self.export_name = export_name  # nome base do arquivo exportado

    def _on_step(self) -> bool:
        # Exporta o modelo se a frequência for atingida
        if self.n_calls % self.freq == 0:
            path = os.path.join(self.load_path, f"model_{self.num_timesteps}_steps.zip")
            # Exporta os pesos do modelo usando método estático da classe Train_Base
            Train_Base.export_model(path, f"./sobre_scripts/gyms/export/{self.export_name}")
        # Retorna True para continuar o treinamento (False interromperia)
        return True
