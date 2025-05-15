# a_estrela

Módulo C++ focado no algoritmo A*, o qual foi criado em C++ para ser usado em Python.

* [a_estrela.h](../../src/sobre_cpp/a_estrela/a_estrela.h)
  * Inicialização de variáveis e structs básicas, como a árvore representante de caminhos.
  * Criação de representação do campo como uma matriz de obstáculos.
  
* [obtendo_possibilidades.py](../../src/sobre_cpp/a_estrela/obtendo_possibilidades.py)
  * Obtemos uma área circular centrada no agente que representará
  a área preferencial de cálculos para previsão de caminhos.
  * Disponibiliza o arquivo _obtendo_possibilidades.h_ que é a área anteriormente citada, literalmente
  para que o código não precise calcular todas as distâncias necessárias sempre! EXCELENTE IDEIA.
  * **Alterar parâmetros presentes no arquivo afetará a otimização.**
  
* [a_estrela.cpp](../../src/sobre_cpp/a_estrela/a_estrela.cpp)
  * Super código de implementação do A* em C++. Lê-lo é uma experiência evolutiva.
  * Criação de funções manipuladoras da árvore dentro do namespace _noding_.
  * Utilização de um algoritmo insano para calcular distâncias, criado e desenvolvido pela equipe de Portugal exclusivamente para aprimorar a velocidade da busca pelo melhor caminho possível.
  * Excelente demonstração de como codar como um Deus, agradeço ao Miguel Abreu por disponibilzar essa obra de arte.
  * Acredito que, para posteriores avanços, pode-se implementar métodos de testes automáticos a fim de apresentar e verificar o algoritmo e respectivos avanços.

* [debug_main.cc](../../src/sobre_cpp/a_estrela/debug_main.cc)
  * Código para teste de função desenvolvida dentro da pasta, vulgo a_estrela().

* [ponte_de_ligacao_entre_python_e.cpp](../../src/sobre_cpp/a_estrela/ponte_de_ligacao_entre_python_e.cpp)
  * Código responsável por prover ferramentos de conexão do ambiente Python com funções C++.

* [Makefile](../../src/sobre_cpp/a_estrela/Makefile)
  * Responsável por providenciar o Build do módulo. Não ironicamente é o segundo
  arquivo mais importante.
