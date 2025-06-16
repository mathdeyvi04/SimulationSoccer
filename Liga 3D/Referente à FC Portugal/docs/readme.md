# Dissertação Sobre Funcionalidades Individuais e Hierarquia

A seguir, uma breve explicação organizada dos arquivos e pastas do código.

Cada pasta principal será um link de referência para a mesma. 

* Caso a pasta possua outras pastas dentro dela, o link será referido a uma pasta dentro de `/docs/` que conterá outros markdowns
referentes a cada uma das subpastas.

* Caso a pasta possua apenas arquivos ou peculiariedades, não haverá pasta referente em `/docs/` e 
os arquivos serão explicados aqui mesmo.

---
## [sobre_cpp](sobre_cpp)

Pasta responsável por gerenciar as funcionalidades mais pesadas e/ou mais frequentemente
usadas, todas construídas em C++ com portabilidade para serem utilizadas no Python.

Os módulos presentes são independentes entre si e cada um é fundamental para o funcionamento da aplicação geral:

* [preditor_de_curva_da_bola](sobre_cpp/preditor_de_curva_da_bola.md)
* [a_estrela](sobre_cpp/a_estrela.md)
* [ambientacao](sobre_cpp/ambientacao.md)

Dentro de cada, há 3 arquivos essenciais para a portabilidade.

* debug.py ou debug.cc
  
Arquivo específico para realização de testes das funções implementadas no módulo.

Usando `make teste` é possível realizar as verificações. Para o caso de `preditor_de_curva_da_bola` é
necessário usar `make` antes.

* module_main.cpp

Arquivo específico **responsável pela portabilidade das aplicações para o Python**.

Para compreender como o pybind funciona de verdade, sugiro verificar o arquivo 
[module_main referente à estrela.cpp](../src/sobre_cpp/a_estrela/module_main.cpp).

Caso deseje alterar algo, saiba que o _pybind somente aceitará caracteres ASCII_, 
não tente colocar UTF-8 nas documentações ou nomes. Quando descobri isso, preferi reescrever inglês mesmo.

Utilizando `help(nome_do_modulo)` pode-se obter mais informações sobre o módulo e 
suas funcionalidades.

* Makefile 

Arquivo de build do módulo cpp, o qual gerará um arquivo _.so_.

Caso deseje compilar o módulo manualmente para realizar testes e comparações,
faça o que é descrito dentro do arquivo:

```
# Para executar a compilação manual, descomente a seguinte linha: 
# FLAGS_DE_COMPILACAO_MANUAL = -I/usr/include/python3.12 -I/usr/include/pybind11

# E substitua o termo $(PYBIND_INCLUDES) por $(FLAGS_DE_COMPILACAO_MANUAL)
```

Em minha máquina, instalei o python e pybind com `sudo apt install ...`. 
Logo, meus caminhos são esses e essas são as respectivas versões. Caso você tente
fazer o make manualmente para testes e verificações, você deve colocar o caminho 
do interpretador e o caminho da biblioteca pybind11.

* Como é feito a automação?

Estes módulos são compilados e construídos a partir de um método específico da classe 
[Script](../src/sobre_scripts/commons/Script.py).

É possível que novos módulos sejam construídos, basta que respeite dois pontos que eu acho interessantes: _independência_ e _necessidade real_.

---

## [math_ops](../src/math_ops)

Pasta responsável por gerir operações matemáticas complexas dentro do Python.

* [Neural_Network.py](../src/math_ops/Neural_Network.py)

Desenvolvido função responsável por executar o algoritmo de machine learning por perceptron.
**Nunca feito por mim, muito foda**

* [GeneralMath.py](../src/math_ops/GeneralMath.py)

Classe responsável por aglutinar funções matemáticas triviais e/ou não dispostas no numpy.
Há uma variável responsável por informar se o ambiente está sendo executado como binário.

* [Matriz3x3](../src/math_ops/Matriz3x3.py)

Classe responsável por aglutinar funções de rotação ligadas às matrizes 3x3.

* [Matriz4x4](../src/math_ops/Matriz4x4.py)

Classe responsável por aglutinar funções de rotação e de translação ligadas às matrizes 4x4.

* [InverseKinematics](../src/math_ops/InverseKinematics.py)

Classe não-trivial responsável por gerenciar e calcular movimentos relativos ao corpo do robô, possuindo
métodos relacionados à trajetória das pernas.

Apesar de ser construída a partir de uma entrada com classe específica, não há a importação explicíta
dela, logo a construção desse arquivo não precisa depender de outros anteriores.

---

## [sobre_logs](../src/sobre_logs)

Pasta responsável por armazenar todas as informações de execução como log info.

Há a classe responsável pela organização e registro das mensagens nos arquivos de log: [Logger](../src/sobre_logs/Logger.py).

---

## [world](../src/world)

Pasta responsável por agrupar as principais funcionalidades do robô em campo,
gerenciando informações do robô e do campo, e métodos intrínsecos à ação do robô.

#### [commons](world/commons.md)

Pasta responsável por agrupar todo o gerenciamento de informações básicas do robô, seja
partes corporais básicas, como ele verifica outros robôs e como calcula seu próprio caminho.

* [Robot.py](../src/world/Robot.py)

Classe focada no robô e suas capacidades, sendo responsável por agrupar ferramentas de atualização,
de transformação e de informações gerais do robô.

É nesta classe que há o método responsável pela execução dos comandos do robô.

* [World.py](../src/world/World.py)

Classe focada na interpretação do robô para com o mundo, reunindo funcionalidades ligadas aos módulos
C++ disponíveis em `/sobre_cpp/`, às atualizações de pose e às previsões de trajetória da bola.


Ambos arquivos possuem responsabilidades, caracterísitcas e padrões muito semelhantes, não devem
ser compreendidos isoladamente.

---

## [communication](../src/communication)

Pasta responsável por agrupar funcionalidades de comunicação com os softwares SimSparks e Roboviz.
É um prazer ver todos os métodos e classes se interligando, recomendo a leitura detalhada.

* [Radio.py]
* [ServerComm]
* [WorldParser](../src/communication/WorldParser.py)

Classe focada na interpretação da mensagem raiz recebida pelo SimSpark, conseguindo interpretá-la e
interligar as informações com a classe World provida em `world/World.py`

---

## [sobre_scripts](sobre_scripts)

Pasta responsável por automatizar o lançamento e execução de processos. Em outras palavras,
lida com o terminal distribuindo responsabilidades a outras pastas de código.

#### [commons](../src/sobre_scripts/commons)

Pasta responsável por lidar com os acessos mais comuns ao terminal, possuindo método de construção de 
módulos C++, classe de verificação de servidores pré-existentes, automação de ações de treinamento e 
tantas outras funcionalidades.

#### [gyms]

#### [utils]

--- 