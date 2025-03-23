# Sumário

* [Instalando Dependências](#instalando-dependências-)

* [Arquitetura Base do Código](#arquitetura-base-do-código)
    * [Explicando BashScripts](#explicando-bash-scripts)
    * [Explicando Main Python Scripts](#explicando-main-python-scripts)
    * [Explicando Classes Principais](#explicando-classes-principais)

* [Arquitetura do Base_Agent](#arquitetura-do-base_agent)

* [Ordem de Hierarquia de Criação](#ordem-de-hierarquia-de-criação)
# Instalando Dependências 

Bibliotecas de manipulação de vetores e de dados.

```
sudo apt install libgsl-dev
pip3 install numpy pybind11 psutil
```

Entre na [pasta de código-fonte]() e execute:

```
cd src
python3 Run_Utils.py
```

Na primeira execução, isso compilará o código C++ e possibilita o uso das funções construídas
nessa linguagem. Em seguida, um menu surgirá com diversas possibilidades de ações.
Explicações serão posteriormente fornecidas.

> [!IMPORTANT]
> 
> Passo Opcional para previnir que numpy utilize threads de forma ineficiente.
> ```
> export OMP_NUM_THREADS=1
> echo "export OMP_NUM_THREADS=1">>~/.bashrc
> ```

> [!IMPORTANT]
> 
> Passo Opcional para utilizar ferramentas de **Aprendizado por Reforço**.
> 
>  ```
> # Install Stable Baselines3 (SB3) (easy)
> pip3 install stable-baselines3 gym shimmy
> ```
> 
> Como a importância dessa biblioteca é insquestionável, faz-se necessário um [estudo sobre ela](https://pythonprogramming.net/introduction-reinforcement-learning-stable-baselines-3-tutorial/).
> 
> Além disso, a equipe também previu a _**possibilidade**_ de um erro e
> propôs uma [solução](https://github.com/m-abr/FCPCodebase/issues/3).

<br>

# Arquitetura Base do Código

<div align="center">
<img src="https://github.com/user-attachments/assets/eef167c1-2fd0-4279-92df-125f92f2d61d" width="900"/>
</div>

### Explicando Bash Scripts

Trata-se de arquivos _.shell_ de execução rápida, sem muitas possibilidades de 
configurações.

* [start.sh]()
  * Inicia 11 instâncias independentes do Run_Player.py, cada uma iniciando 1 Agent.

* [start_debug.sh]()
  * Mesmo que o arquivo anterior, mas iniciando no modo debugador.

* [start_penalty.sh]()
  * Inicia 11 instâncias independentes do Run_Player.py, cada uma iniciando 1 Agent_Penalty.

* [start_penalty_debug.sh]()
  * Mesmo que o arquivo anterior, mas iniciando no modo debugador.

* [kill.sh]()
  * Destroe qualquer instância de Main Python Script que foi executado.

### Explicando Main Python Scripts

Execução de Python começa aqui.

* [Run_Utils.py]()
  * Disponibilizará um menu de interações:
    * Util:
      * Conjunto de demonstrações interativas, testes e utilitários para
      executar os principais recursos da equipe/agentes, bem como ler e alterar
      parâmetros do servidor facilmente.
    
    * Gym:
      * Implementação de academia personalizadas da OpenAI para treinar novas
      habilidades por meio de Aprendizado por Reforço. Também oferece suporte
      a carregamento de modelos interativos, testes e retreinamento.

* [Run_Player.py]()
  * Possibilita a criação de um Agente ou de um Agent_Penalty, no modo normal ou
  debugador, baseado nos argumentos inseridos.

* [Run_Full_Team.py]()
  * Executa o código anterior 11 vezes para criar um time completo, usando os mesmos
  argumentos para cada tipo de Agent selecionado.
  
* [Run_One_vs_One.py]()
  * Cria 2 agentes de times diferentes.

### Explicando Classes Principais

% Colocar cada classe em um unico arquivo e marcá-lo aqui.

* [Script]()
  * Usado por Main Python Scripts, lida com argumentos de linha de comando e
  implementa operações para gerenciar múltiplos agentes no mesmo thread.
  Além disso, compila módulos C++ se os binários não forem encontrados ou
  forem mais antigos que os arquivos de origem.

* [Base_Agent]()
  * Implementa a parte básica do agente que processa informações do servidor
  para atualizar seu modelo interno de mundo. Possui ferramentas integradas 
  que automatizam muitos procedimentos, criando uma interface de alto nível
  para interagir com o robô e o ambiente. O modo de depuração exibe informações
  extras durante a inicialização, habilita logs, _desenhos_ e comandos especiais
  enviados pela porta do monitor do servidor (por exemplo, transportar jogador 
  para posição 3D, matar servidor, mover bola, definir tempo de jogo, 
  definir modo de jogo, matar qualquer agente).

* [Agent]()
  * Implementa a estratégia do agente principal, usada em partidas de futebol reais.
  Focará no modelo de decisão do jogador para comportamentos mais complexos.

* [Agent_Penalty]()
  * Implementa a estratégia do agente para o caso de penâlti.

* [Server]()
  * Cria e exclui instâncias de servidores e verifica colisões de portas TCP.

* [Train_Base]()
  * Parente da OpenAI Gym, possibilita 4 ferramentas cruciais:
    * User Interface:
      * Implementa um método auxiliar para guiar o usuário na busca por modelos
      treinados dentro do projeto. Outro recurso permite que o usuário controle
      interativamente a taxa de quadros durante o teste do modelo por meio de 
      comandos de entrada simples.

    * Train Model:
      * Treina o modelo usando Stable Baselines3 (SB3), enquanto automatiza a
      criação de callbacks para avaliar e salvar modelos (em intervalos definidos
      pelo usuário ou apenas o melhor modelo). Ele também cria backups do ambiente
      da academia para referência futura. Além disso, ele exibe um gráfico de 
      avaliação na linha de comando em intervalos definidos pelo usuário, 
      facilitando uma avaliação rápida do progresso do aprendizado.
    
    * Test Model:
      * Verifica um modelo, apresenta estatísticas úteis e salvá-as em um determinado
      arquivo.
    
    * Exports Model:
      * exporta um modelo para um arquivo binário que pode ser integrado na equipe
      como uma nova habilidade.

# Arquitetura do Base_Agent

<div align="center">
<img src="https://github.com/user-attachments/assets/7fb02478-86e3-48fe-a912-38cd4649b6c6" height="400"/>
</div>

Cada classe _Base_Agent_ possui uma unidade de cada classe:

* [Radio]()
  * Comunicação interna automatizada com companheiros de equipe para compartilhar
  a posição e o estado de cada agente visível e da bola. Tente ver o artigo desenvolvido pela equipe [
  3D Simulation League and Technical Challenge Champions 
](https://github.com/user-attachments/files/19377361/FCPortugal_3D_Simulation_League_Technical_Challenge_Champions.pdf)
  para maiores explicações sobre o assunto.

* [Logger]()
  * Um objeto logger é vinculado a um tópico específico, assumindo a
  responsabilidade de registrar data e hora e salvar mensagens em arquivos 
  de log (erros, avisos ou qualquer tipo de informação útil).

* [Server_Comm]()
  * Gerencia toda a comunicação com o servidor por meio da _porta oficial_ do agente
  (inicialização, velocidade das articulações do robô, mensagens para companheiros
  de equipe, comandos de passe e transmissões) e da _porta não oficial_ do monitor
  (transportar jogador para posição 3D, matar servidor, mover bola, definir tempo
  de jogo, definir modo de jogo, destruir qualquer agente).

* [Path_Manager]()
  * Reúne os obstáculos com base nas condições do jogo e determina o melhor caminho
  para um alvo definido pelo usuário (usando algoritmo A*). Entre vários métodos de caminho,
  o _obter_caminho_para_bola_ é o mais abrangente, evitando obstáculos e cruzando uma bola
  em movimento, enquanto posiciona e gira o robô para executar ações precisas, 
  como chutar ou começar a driblar. (**Extremamente Complexo**)
  
  * **a_star.so**: biblioteca de implementação de pathfinding A* personalizada em C++, otimizada
  para o ambiente de futebol frenético. Cada obstáculo pode ser associado a um
  raio de exclusão rígido, definindo áreas estritamente proibidas, e/ou um raio
  de exclusão suave, especificando áreas a serem evitadas. A intensidade da
  evitação na zona de exclusão suave é configurável.

* [Inverse_Kinematics]()
  * Calcula cinemática inversa para cada perna, dada a posição relativa de cada
  tornozelo e a orientação 3D de cada pé. Também fornece métodos auxiliares para
  transformações de coordenadas e computação de trajetória.

* [World_Parser]()
  * Analisador de expressão S personalizado para mensagens de servidor,
  padronizando quadros de referência ao remapeamento de certos eixos. Além disso,
  algumas articulações são invertidas, de modo que ações realizadas com os membros
  esquerdos espelham aquelas executadas pelos membros direitos (permitindo
  especificação mais fácil de comportamentos simétricos).

* [World]()
  * Modelo de mundo interno, atualizado por meio de visão e comunicação com
  companheiros de equipe. Ele monitora várias métricas de tempo, como tempo de
  jogo, tempo de servidor e tempo de cliente, além de capturar o estado do jogo 
  e objetos dinâmicos, incluindo si mesmo, companheiros de equipe, oponentes e a
  bola. 
  
  * Possui métodos para calcular a velocidade relativa e absoluta da bola,
  dada uma janela de tempo, e manipular previsões de posição.

  * Suas principais classes:
    * **ball_predict.so**: Biblioteca C++ personalizada para prever a posição
    e a velocidade da bola rolando (no chão). Ela também prevê quando um robô,
    com uma velocidade de caminhada específica, cruzará a bola rolando.
  
    * **localization.so**: Algoritmo de estimativa de posição 6D baseado em um 
    modelo probabilístico personalizado, (**algoritmo extremamente complexo sugiro buscar mais**)

    * Draw: Classe para desenhar no RoboViz, desenha setas além de todas as
    formas nativas e anotações de texto. Esta classe é acessível através do
    objeto world, mas também pode ser criada independentemente de um agente,
    como visto no utilitário Draw.
  
    * Other_Robot: informações sobre o estado dos companheiros de equipe ou
    adversários.

    * Robot: Estado interno do robô, incluindo informações sobre localização,
    orientação, juntas, alvos de juntas, partes do corpo, sensores. Fornece
    métodos para lidar com controle de juntas, computação IMU, cinemática direta,
    transformações de pose (translação e orientação 3D). Robot_Specs são arquivos
    XML com especificações de cada tipo de robô NAO.
    
* [Behavior]()
  * Head
    * Algoritmo que controla a orientação da cabeça.
  
  * Pose
    * Uma pose é definida por um único quadro-chave (1 posição angular por articulação)

  * Slot Engine
    * Gerencia a execução de comportamentos específicos.
    * Comportamento Específico - arquivos XML que definem 
    comportamentos compostos de vários quadros-chave.
    
  * Custom_Skill
    * Habilidades que necessitam de uma preparação e execuçao personalizada.


# Ordem de Hierarquia de Criação

[UserInterface](src/sobre_scripts/comuns/UserInterface.py)