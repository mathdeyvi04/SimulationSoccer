import os

import os
import subprocess

class Server:
    """
    Classe responsável por interpretar, exibir e modificar as configurações do servidor SimSpark (rcssserver3d).
    Permite alternar modos de jogo, visão, ruído, frequência de monitoramento e outros parâmetros fundamentais
    para simulações de futebol robótico (RoboCup 3D).
    """

    def __init__(self, script) -> None:
        """
        Descrição:
            Inicializa a instância do servidor, definindo o caminho dos arquivos de configuração e
            as opções configuráveis disponíveis.

        Parâmetros:
            script: str
                Nome do script que está utilizando esta classe (não utilizado diretamente no código).
        """
        self.values = None  # Dicionário com os valores atuais das opções (On/Off/Error)

        # Determina o diretório onde os arquivos de configuração estão instalados (distribuições Linux padrão)
        if os.path.isdir("/usr/local/share/rcssserver3d/"):
            self.source = "/usr/local/share/rcssserver3d/"
        elif os.path.isdir("/usr/share/rcssserver3d/"):
            self.source = "/usr/share/rcssserver3d/"
        else:
            raise FileNotFoundError("The server configuration files were not found!")

        # Lista das opções configuráveis, com suas descrições e arquivos correspondentes
        self.options = [
            "Official Config", "Penalty Shootout", "Soccer Rules", "Sync Mode", "Real Time",
            "Cheats", "Full Vision", "Add Noise", "25Hz Monitor"
        ]
        self.descriptions = [
            "Configuration used in official matches",
            "Server's Penalty Shootout mode",
            "Play modes, automatic referee, etc.",
            "Synchronous communication between agents and server",
            "Real Time (or maximum server speed)",
            "Agent position & orientation, ball position",
            "See 360 deg instead of 120 deg (vertically & horizontally)",
            "Noise added to the position of visible objects",
            "25Hz Monitor (or 50Hz but RoboViz will show 2x the actual speed)"
        ]

        # Arquivos de configuração utilizados por cada opção
        spark_f = os.path.expanduser("~/.simspark/spark.rb")
        naoneckhead_f = self.source + "rsg/agent/nao/naoneckhead.rsg"

        self.files = {
            "Penalty Shootout": self.source + "naosoccersim.rb",
            "Soccer Rules": self.source + "naosoccersim.rb",
            "Sync Mode": spark_f,
            "Real Time": self.source + "rcssserver3d.rb",
            "Cheats": naoneckhead_f,
            "Full Vision": naoneckhead_f,
            "Add Noise": naoneckhead_f,
            "25Hz Monitor": spark_f
        }

    def label(self, setting_name, t_on, t_off):
        """
        Descrição:
            Lê o valor atual de uma configuração (On/Off/Error) com base na presença de strings específicas no arquivo.

        Parâmetros:
            setting_name: str
                Nome da configuração a ser lida.
            t_on: str
                Trecho de código que indica que a opção está ativada.
            t_off: str
                Trecho de código que indica que a opção está desativada.
        """
        with open(self.files[setting_name], "r") as sources:
            content = sources.read()

        if t_on in content:
            self.values[setting_name] = "On"
        elif t_off in content:
            self.values[setting_name] = "Off"
        else:
            self.values[setting_name] = "Error"

    def read_config(self):
        """
        Descrição:
            Lê e atualiza o estado atual (On/Off) de todas as configurações e define o valor de
            "Official Config" com base em uma combinação padrão de valores esperados.
        """
        v = self.values = dict()
        print("Reading server configuration files...")

        self.label("Penalty Shootout", "addSoccerVar('PenaltyShootout', true)", "addSoccerVar('PenaltyShootout', false)")
        self.label("Soccer Rules", " gameControlServer.initControlAspect('SoccerRuleAspect')", "#gameControlServer.initControlAspect('SoccerRuleAspect')")
        self.label("Real Time", "enableRealTimeMode = true", "enableRealTimeMode = false")
        self.label("Cheats", "setSenseMyPos true", "setSenseMyPos false")
        self.label("Full Vision", "setViewCones 360 360", "setViewCones 120 120")
        self.label("Add Noise", "addNoise true", "addNoise false")
        self.label("Sync Mode", "agentSyncMode = true", "agentSyncMode = false")
        self.label("25Hz Monitor", "monitorStep = 0.04", "monitorStep = 0.02")

        # Verifica se todas as configurações coincidem com a configuração oficial
        is_official_config = (
            v["Penalty Shootout"] == "Off" and
            v["Soccer Rules"] == "On" and
            v["Real Time"] == "On" and
            v["Cheats"] == "Off" and
            v["Full Vision"] == "Off" and
            v["Add Noise"] == "On" and
            v["Sync Mode"] == "Off" and
            v["25Hz Monitor"] == "On"
        )
        v["Official Config"] = "On" if is_official_config else "Off"

    def change_config(self, setting_name, t_on, t_off, current_value=None, file=None):
        """
        Descrição:
            Altera uma configuração substituindo diretamente os trechos no arquivo correspondente.

        Parâmetros:
            setting_name: str
                Nome da configuração a ser alterada.
            t_on: str
                Trecho que representa a opção ativada.
            t_off: str
                Trecho que representa a opção desativada.
            current_value: str, opcional
                Valor atual da configuração. Se None, será lido de self.values.
            file: str, opcional
                Caminho do arquivo. Se None, será buscado em self.files.
        """
        if current_value is None:
            current_value = self.values[setting_name]
        if file is None:
            file = self.files[setting_name]

        with open(file, "r") as sources:
            t = sources.read()

        if current_value == "On":
            t = t.replace(t_on, t_off, 1)
            print(f"Replacing '{t_on}' with '{t_off}' in '{file}'")
        elif current_value == "Off":
            t = t.replace(t_off, t_on, 1)
            print(f"Replacing '{t_off}' with '{t_on}' in '{file}'")
        else:
            print(setting_name, "was not changed because the value is unknown!")

        with open(file, "w") as sources:
            sources.write(t)

    def execute(self):
        """
        Descrição:
            Executa a interface interativa no terminal para exibir as configurações atuais e
            permitir a modificação de qualquer uma delas.
        """
        while True:
            self.read_config()  # Atualiza os valores atuais de todas as opções

            # Converte os valores em uma lista na mesma ordem das opções
            values_list = [self.values[o] for o in self.options]

            print()
            UserInterface.apresentar_tabela(
                [self.options, values_list, self.descriptions],
                ["Setting", "Value", "Description"],
                numbering=[True, False, False]
            )

            # Entrada do usuário
            choice = UserInterface.obter_inteiro('Choose setting (ctrl+c to return): ', 0, len(self.options))
            opt = self.options[choice]

            prefix = ['sudo', 'python3', 'scripts/utils/Server.py', opt]
            suffix = []
            if opt in self.files:
                suffix = [self.values[opt], self.files[opt]]

            # Realiza a ação correspondente à opção escolhida
            if opt == "Penalty Shootout":
                subprocess.call([*prefix, "addSoccerVar('PenaltyShootout', true)", "addSoccerVar('PenaltyShootout', false)", *suffix])
            elif opt == "Soccer Rules":
                subprocess.call([*prefix, "gameControlServer.initControlAspect('SoccerRuleAspect')", "#gameControlServer.initControlAspect('SoccerRuleAspect')", *suffix])
            elif opt == "Sync Mode":
                self.change_config(opt, "agentSyncMode = true", "agentSyncMode = false")
            elif opt == "Real Time":
                subprocess.call([*prefix, "enableRealTimeMode = true", "enableRealTimeMode = false", *suffix])
            elif opt == "Cheats":
                subprocess.call([
                    *prefix, "setSenseMyPos true", "setSenseMyPos false", *suffix,
                    opt, "setSenseMyOrien true", "setSenseMyOrien false", *suffix,
                    opt, "setSenseBallPos true", "setSenseBallPos false", *suffix
                ])
            elif opt == "Full Vision":
                subprocess.call([*prefix, "setViewCones 360 360", "setViewCones 120 120", *suffix])
            elif opt == "Add Noise":
                subprocess.call([*prefix, "addNoise true", "addNoise false", *suffix])
            elif opt == "25Hz Monitor":
                self.change_config(opt, "monitorStep = 0.04", "monitorStep = 0.02")
            elif opt == "Official Config":
                if self.values[opt] == "On":
                    print("The official configuration is already On!")
                else:
                    subprocess.call([
                        *prefix[:3],
                        "Penalty Shootout", "addSoccerVar('PenaltyShootout', false)", "addSoccerVar('PenaltyShootout', true)", "Off", self.files["Penalty Shootout"],
                        "Soccer Rules", "gameControlServer.initControlAspect('SoccerRuleAspect')", "#gameControlServer.initControlAspect('SoccerRuleAspect')", "Off", self.files["Soccer Rules"],
                        "Sync Mode", "agentSyncMode = false", "agentSyncMode = true", "Off", self.files["Sync Mode"],
                        "Real Time", "enableRealTimeMode = true", "enableRealTimeMode = false", "Off", self.files["Real Time"],
                        "Cheats", "setSenseMyPos false", "setSenseMyPos true", "Off", self.files["Cheats"],
                        "Cheats", "setSenseMyOrien false", "setSenseMyOrien true", "Off", self.files["Cheats"],
                        "Cheats", "setSenseBallPos false", "setSenseBallPos true", "Off", self.files["Cheats"],
                        "Full Vision", "setViewCones 120 120", "setViewCones 360 360", "Off", self.files["Full Vision"],
                        "Add Noise", "addNoise true", "addNoise false", "Off", self.files["Add Noise"],
                        "25Hz Monitor", "monitorStep = 0.04", "monitorStep = 0.02", "Off", self.files["25Hz Monitor"]
                    ])


# process with sudo privileges to change the configuration files
if __name__ == "__main__":
    import sys

    s = Server(None)

    for i in range(1, len(sys.argv), 5):
        s.change_config(*sys.argv[i:i + 5])
else:
    import subprocess
    from scripts.commons.UserInterface import UserInterface
