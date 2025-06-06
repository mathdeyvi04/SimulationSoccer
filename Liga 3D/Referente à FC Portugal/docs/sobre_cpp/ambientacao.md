# ambientacao

Módulo C++ responsável pela interpretação visual do campo pelo agente e pela conexão com Roboviz.

Diversos arquivos complexos são implementados aqui e nada é trivial.

Houve condensação de arquivos, quando comparado com o original, e alterações consideráveis em algumas lógicas.

* [Singular.h](../../src/sobre_cpp/ambientacao/Singular.h)
  * Criação e Implementação de Classe Abstrata que permitirá instâncias únicas de qualquer classe.
  * Um show de genialidade.

* [AlgLin.h](../../src/sobre_cpp/ambientacao/AlgLin.h)
  * Criação e Implementação base de Vetores, de duas e três dimensões, Matrizes 4x4 e Linhas.
  * Diversas funcionalidades estão amplamente relacionadas com conhecimento em Álgebra Linear.
  * Alguns pequenos parâmetros podem influenciar cálculos, mas não é tão decisivo.

* [Ruido_de_Campo.h](../../src/sobre_cpp/ambientacao/Ruido_de_Campo.h)
  * Criação e Implementação de modelo de consideração de ruído.
  * Sabendo como o ruído afeta os dados, pode-se aplicar uma espécie de inversa 
  do ruído e assim conseguimos lidar com os dados reais.
  * Possuindo forte caráter matemático, **há parâmetros que afetam a aplicação geral.**
  * Há uma determinada função que na minha interpretação está errada.

* [RobovizDraw.h](../../src/sobre_cpp/ambientacao/RobovizDraw.h)
  * Criação e Implementação de funções esteticamente construídas em C que fornecem 
  buffers de informações de desenho para o software RoboViz.
  * **Parâmetros alterados aqui influenciam como as coisas serão apresentadas 
  nos desenhos do RoboViz.**

* [RobovizLogger.h](../../src/sobre_cpp/ambientacao/RobovizLogger.h)
  * Header C++ extremamente mais complexo e focado em comunicação via protocolos, não é trivial
  e parece até que estamos em um arquivo errado.
  * Possuindo forte interação com [RobovizDraw.h](../../src/sobre_cpp/ambientacao/RobovizDraw.h), 
  fornece para o software quais desenhos devem ser apresentados.
  * Há uma pequena discussão interessante sobre destrutores comuns e virtuais.

* [World.h](../../src/sobre_cpp/ambientacao/World.h)
  * Criação de classe que aglutina variáveis e structs que serão utilizadas na visualização do mundo
  externo.
  * Faz a ligação do que o agente está vendo com como será interpretado. Agindo como
  os olhos do robô.
  * **Parâmetros alterados aqui influenciam a visualização externa do agente.**

* [RobovizField.h](../../src/sobre_cpp/ambientacao/RobovizField.h)
    * Criação de classe gerenciadora da interpretação dos dados visualizados pelo agente.
    * Implementação de diversas definições de constantes e de structs essenciais para representação de elementos do campo.
    * Sugiro a leitura calma deste arquivo, pois **parâmetros alterados aqui
    influenciam o funcionamento global e na visualização externa.**
  
* [RobovizField.cpp](../../src/sobre_cpp/ambientacao/RobovizField.cpp)
     * Implementação de diversos métodos de visualização e de atualização de
     elementos no Roboviz que dependem de uma compilação profunda.
     * **Parâmetros alterados aqui influenciam o funcionamento global.**
     * Arquivo que converge todos os outros derivados de Roboviz e World.h.
     * Houve uma alteração significativa nas funções de desenho de elementos, sugiro comparação com Original.
   
* [LocalizerV2.h](../../src/sobre_cpp/ambientacao/LocalizerV2.h)
    * Criação, definição do método de localização 3D utilizado e 
    desenvolvimento pela equipe FC portugal.
    * Código extremamente mais robusto e complexo, complexo mesmo! Sua documentação completa
    está disponível em [doc_LocalizerV2.md](../../src/sobre_cpp/ambientacao/LocalizerV2.md)
    * **Parâmetos alterados aqui influenciam o funcionamento global.**
    * Métodos de ajuste de curva e de solução de sistemas lineares são utilizados aqui.

* [LocalizerV2.cpp](../../src/sobre_cpp/ambientacao/LocalizerV2.cpp)
    * Implementação de funções que precisam ser compiladas.
    * Lógica de alta complexidade e importância.
