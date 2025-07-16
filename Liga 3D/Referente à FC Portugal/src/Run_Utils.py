def main():
    """
    Descrição:
        Função principal da interface de testes e treinamento de agentes. Permite
        ao usuário interagir com diferentes scripts organizados nas pastas utils e gyms,
        oferecendo opções para executar testes, demos, ou treinar modelos de aprendizado
        por reforço (via Stable Baselines3). Também cuida do carregamento dinâmico de módulos
        e do gerenciamento de ciclo de vida dos objetos criados.
    """
    from sobre_scripts.commons.Script import Script

    # Inicializa objeto Script: carrega configs, argumentos e compila módulos C++
    script = Script()

    # Permite usar uma versão local do Stable Baselines3 (se colocado na pasta pai)
    import sys
    from os.path import dirname, abspath, join
    sys.path.insert(0, join(dirname(dirname(abspath(__file__))), "stable-baselines3"))

    from sobre_scripts.commons.UserInterface import UserInterface
    from os.path import isfile, join, realpath, dirname
    from os import listdir, getcwd
    from importlib import import_module

    # Determina diretório atual absoluto (mesmo com links simbólicos)
    _cwd = realpath(join(getcwd(), dirname(__file__)))
    gyms_path = _cwd + "/sobre_scripts/gyms/"
    utils_path = _cwd + "/sobre_scripts/utils/"
    exclusions = {"__init__.py", "Testing.py"}  # Ignora arquivos de inicialização

    # Lista scripts disponíveis (sem __init__.py), ordenando os utils com 'Server' por último
    utils = sorted(
        [f[:-3] for f in listdir(utils_path) if isfile(join(utils_path, f)) and f.endswith(".py") and f not in exclusions],
        key=lambda x: (x != "Server", x)
    )
    gyms = sorted(
        [f[:-3] for f in listdir(gyms_path) if isfile(join(gyms_path, f)) and f.endswith(".py") and f not in exclusions]
    )

    # Loop principal da interface (executado até o usuário pressionar Ctrl+C)
    while True:
        # Mostra tabela com opções de utils e gyms
        _, col_idx, col = UserInterface.print_table(
            [utils, gyms],
            ["Demos & Tests & Utils", "Gyms"],
            cols_per_title=[2, 1],
            numbering=[True] * 2,
            prompt='Insira o número do script (ctrl+c to exit): '
        )

        is_gym = False
        if col == 0:
            chosen = ("sobre_scripts.utils.", utils[col_idx])
        elif col == 1:
            chosen = ("sobre_scripts.gyms.", gyms[col_idx])
            is_gym = True
        else:
            chosen = None

        cls_name = chosen[1]  # Observe que necessariamente cada arquivo deve ter o nome de sua classe.
        '''
        Scripts não devem executar código automaticamente ao importar:
        - Multiprocessing pode duplicar execuções
        - O módulo pode ser recarregado várias vezes e gerar comportamento inesperado
        '''
        mod = import_module(chosen[0] + chosen[1])  # Importa módulo dinamicamente


        if not is_gym:
            '''
            Scripts utilitários devem implementar uma classe com método execute()
            Recebem o objeto script para acessar argumentos e utilitários
            '''
            from world.commons.Draw import Draw
            from Agent.BaseAgent import BaseAgent
            obj = getattr(mod, cls_name)(script)
            try:
                obj.execute()
            except KeyboardInterrupt:
                print("\nctrl+c pressed, returning...\n")
                
            Draw.clear_all()  # Limpa todas as marcações de desenho
            BaseAgent.terminate_all()  # Encerra sockets de agentes e monitor
            script.players = []  # Limpa lista de jogadores registrados
            del obj

        else:
            '''
            Scripts de Gym devem ter uma classe Train que implementa:
                - train(): inicia processo de treinamento e salva o modelo
                - test(): carrega modelo salvo e executa testes
            '''
            from sobre_scripts.commons.TrainBase import TrainBase

            # Avisos importantes ao usuário antes de executar treinamento/teste
            print("\nBefore using GYMS, make sure all server parameters are set correctly")
            print("(sync mode should be 'On', real time should be 'Off', cheats should be 'On', ...)")
            print("To change these parameters go to the previous menu, and select Server\n")
            print("Also, GYMS start their own servers, so don't run any server manually")

            # Loop interno para escolher entre treinar, testar ou re-treinar
            while True:
                try:
                    idx = UserInterface.apresentar_tabela(
                        [["Train", "Test", "Retrain"]],
                        numbering=[True],
                        prompt='Escolha um número (ctrl+c to return): '
                    )[0]
                except KeyboardInterrupt:
                    print()
                    break

                if idx == 0:
                    mod.Train(script).train(dict())
                else:
                    model_info = TrainBase.prompt_user_for_model()
                    if model_info is not None and idx == 1:
                        mod.Train(script).test(model_info)
                    elif model_info is not None:
                        mod.Train(script).train(model_info)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nctrl+c pressionado, saindo...")
        exit()
