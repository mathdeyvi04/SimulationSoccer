import subprocess

"""
Módulo psutil
-------------

Biblioteca Python que permite acessar informações e controlar processos
e recursos do sistema operacional de forma fácil e multiplataforma.
"""


class Server:
    """
    Descrição:
        A classe `Server` é responsável por iniciar, monitorar e finalizar múltiplas instâncias dos
        servidores `rcssserver3d` ou `simspark`, utilizados comumente em simulações RoboCup Soccer 3D.

        Cada servidor é iniciado com argumentos de linha de comando específicos para definir as portas
        de comunicação com agentes (players) e monitores (visão/simuladores). A classe garante o
        isolamento dos processos e realiza verificações preventivas de conflitos de porta com o auxílio
        do módulo `psutil`, se disponível.

        Ideal para cenários automatizados de simulação em larga escala, com múltiplos servidores em paralelo.

    Atributos:
        - first_server_p (int):
            Porta inicial a ser usada para comunicação com agentes (agent ports).

        - n_servers (int):
            Número de instâncias de servidor a serem criadas.

        - rcss_processes (list[subprocess.Popen]):
            Lista contendo os objetos de processo (`Popen`) referentes aos servidores criados.
            Utilizada para encerrar os processos posteriormente via `kill()`.

    Métodos:
        - __init__(first_server_p, first_monitor_p, n_servers):
            Inicializa os servidores e verifica se já há servidores rodando nas portas especificadas.

        - check_running_servers(psutil, first_server_p, first_monitor_p, n_servers):
            Verifica se há conflitos de porta com instâncias já existentes dos servidores.

        - kill():
            Finaliza todos os processos de servidor iniciados pela instância da classe.
    """
    def __init__(
            self,
            first_server_p: int,
            first_monitor_p: int,
            n_servers: int
    ) -> None:
        """
        Descrição:
            Inicializa múltiplas instâncias de servidores `rcssserver3d` ou uma instância de `simspark`,
            redirecionando suas saídas e garantindo isolamento de sessão para cada processo.

            Antes de iniciar os servidores, a função tenta importar o módulo `psutil` para verificar
            se há processos já ocupando as portas desejadas. Se `psutil` não estiver disponível,
            a verificação será ignorada, e o usuário será notificado.

            Os processos iniciados são armazenados no atributo `self.rcss_processes` para controle
            posterior, como término dos servidores com `self.kill()`.

        Parâmetros:
            - first_server_p (int):
                Porta base a ser usada para os agentes (player connections). Para cada servidor,
                o número da porta será `first_server_p + i`.

            - first_monitor_p (int):
                Porta base a ser usada para visualização (monitor). Para cada servidor, será
                `first_monitor_p + i`.

            - n_servers (int):
                Número de instâncias de servidores a serem criadas. Se `1`, será usado o comando `simspark`;
                caso contrário, `rcssserver3d` será utilizado.

        Retorno:
            - None
                O construtor apenas inicializa os processos e configura os atributos necessários.
                Se detectar conflitos de portas (caso `psutil` esteja presente), solicitará ao usuário
                que tome uma ação manual para resolvê-los.
        """
        try:
            import psutil
            self.check_running_servers(psutil, first_server_p, first_monitor_p, n_servers)
        except ModuleNotFoundError:
            print("Não é possível checar se já há servidores rodando porque o módulo psutil não foi encontrado.")

        # Usaremos esses atributos no método específico self.kill()
        self.first_server_p = first_server_p
        self.n_servers = n_servers
        self.rcss_processes = []

        # Tornará mais fácil para matar servidores de teste sem afetar servidores de teste
        cmd = "simspark" if n_servers == 1 else "rcssserver3d"
        for i in range(n_servers):
            # Observe que i garante que teremos múltiplos servidores.
            self.rcss_processes.append(
                subprocess.Popen(
                    f"{cmd} --agent-port {first_server_p + i} --server-port {first_monitor_p + i}".split(),
                    stdout=subprocess.DEVNULL,  # Descartará tudo que imprimir
                    stderr=subprocess.STDOUT,  # Como está indo para stdout, também descartará tudo.
                    start_new_session=True  # Criará um novo terminal correspondente ao processo, isolando-o.
                )
            )

    @staticmethod
    def check_running_servers(
            psutil,
            # O nome dos argumentos já é ótimo
            first_server_p: int,
            first_monitor_p: int,
            n_servers: int
    ) -> bool:
        """
        Descrição:
            Verifica se há processos ativos dos servidores `rcssserver3d` ou `simspark` utilizando
            portas que entrarão em uso por novas instâncias. Essa verificação cobre dois intervalos
            de portas: um para os servidores e outro para os monitores, com base nos parâmetros fornecidos.

            Caso sejam identificados conflitos de portas (isto é, já existirem processos ocupando as
            mesmas), os detalhes desses processos são exibidos ao usuário. A função permite, então, que
            o usuário digite 'kill' para encerrar automaticamente esses processos conflitantes.

        Parâmetros:
            - psutil:
                Módulo `psutil` (deve ser previamente importado), utilizado para inspecionar os
                processos ativos e seus argumentos de execução.

            - first_server_p (int):
                Porta base para os servidores. O intervalo checado é
                `[first_server_p, first_server_p + n_servers)`.

            - first_monitor_p (int):
                Porta base para os monitores. O intervalo checado é
                `[first_monitor_p, first_monitor_p + n_servers)`.

            - n_servers (int):
                Número total de servidores (e respectivos monitores) que serão iniciados.
                Define o tamanho dos intervalos de porta a serem analisados.

        Retorno:
            - None:
                Esta função não retorna valores. Caso encontre conflitos, imprime informações no terminal.
                Se o usuário digitar 'kill', os processos conflitantes são finalizados automaticamente.
                Caso contrário, o usuário pode abortar com `Ctrl+C`.
        """

        print(f"O tipo de psutil é {type(psutil)}")
        running_server_encontrado = False
        process_list = [
            # process_iter(): Retornará um iterador sobre todos os processos do sistema operacional.
            # p.cmdline():    Retornará a linha de comando usada para lançar o processo, caso vazia/None não ativa o if.
            # p.name():       Retornará o nome do processo.
            p for p in psutil.process_iter() if p.cmdline() and p.name() in {"rcssserver3d", "simspark"}
        ]
        range_relativo_aos_servidores = (first_server_p, first_server_p + n_servers)
        range_relativo_aos_monitores = (first_monitor_p, first_monitor_p + n_servers)
        process_conflitantes = []

        for process in process_list:
            # Obtemos todos os digitos dentro da linha de comando para o processo.
            # currently ignoring remaining default port when only one of the ports is specified (uncommon scenario)
            portas = [
                int(arg) for arg in process.cmdline()[1:] if arg.isdigit()
            ]

            # Portas Default
            if len(portas) == 0:
                portas = [3100, 3200]

            # Verificamos conflitos
            conflicts = [str(port) for port in portas if (
                    (range_relativo_aos_servidores[0] <= port < range_relativo_aos_servidores[1]) or (range_relativo_aos_monitores[0] <= port < range_relativo_aos_monitores[1]))]

            if len(conflicts) > 0:
                if not running_server_encontrado:
                    print("\n\033[7m\033[1mHá servidores rodando nas mesmas portas, cuidado!\033[0m")
                    running_server_encontrado = True
                process_conflitantes.append(process)
                print(f"Porta(s) {','.join(conflicts)} já utilizadas por \"{' '.join(process.cmdline())}\" (PID:{process.pid})")

        if running_server_encontrado:
            print("\n")
            while True:
                if input("Digite 'kill' para destruir TODOS esses processos ou ctrl+c para abortar.") == "kill":
                    for process in process_conflitantes:
                        process.kill()
                    return False
        else:

            return True

    def kill(self):
        for process in self.rcss_processes:
            process.kill()

        print(f"Killed {self.n_servers} rcssserver3d processes starting at {self.first_server_p}")
