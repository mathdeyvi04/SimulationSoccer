##  Hierarquia

Do arquivo / pasta mais profundo, implementado mais cedo, até o mais superficial, último implementado.

* [UserInterface](src/sobre_scripts/comuns/UserInterface.py)
  * Disponibilizar ferramentas de estilização de apresentação no terminal.
  
* [Script](src/sobre_scripts/comuns/Script.py)
  * Argumentos intrínsecos ao time estão disponíveis em [config.json](src/config.json).
  * Existe uma função responsável pela compilação de código cpp.
  * Em geral, responsável por providenciar ferramentas de execução em terminal.

* [a_estrela](src/sobre_cpp/a_estrela)
  * Módulo C++ focado no algoritmo A*, o qual foi criado em C++ para ser usado em Python.
    
    * [a_estrela.h](src/sobre_cpp/a_estrela/a_estrela.h)
      * Inicialização de variáveis e structs básicas, como a árvore representante de caminhos.
      * Criação de representação do campo como uma matriz de obstáculos.
      
    * [obtendo_possibilidades.py](src/sobre_cpp/a_estrela/obtendo_possibilidades.py)
      * Obtemos uma área circular centrada no agente que representará
      a área preferencial de cálculos para previsão de caminhos.
      * Disponibiliza para _obtendo_possibilidades.h_ a área anteriormente citada, literalmente
      para que o código não precise calcular todas as distâncias necessárias! EXCELENTE IDEIA.
      * Alterar parâmetros presentes no arquivo afetará a otimização.
      
    * [a_estrela.cpp](src/sobre_cpp/a_estrela/a_estrela.cpp)
      * Super código de implementação do A* em C++. Tente lê-lo.
      * Criação de funções manipuladoras da árvore.
      * Utilização de um algoritmo insano para calcular distâncias, criado e desenvolvido pela equipe de Portugal exclusivamente para aprimorar a velocidade da busca pelo melhor caminho possível.
      * Excelente demonstração de como codar como um Deus, agradeço ao Miguel Abreu por disponibilzar essa obra de arte.
      * Acredito que, para posteriores avanços, pode-se implementar métodos de testes automáticos a fim de apresentar e verificar o algoritmo e respectivos avanços.
    
    * [debug_main.cc](src/sobre_cpp/a_estrela/debug_main.cc)
      * Código para teste de função desenvolvida dentro da pasta, vulgo a_estrela().
  
    * [ponte_de_ligacao_entre_python_e.cpp](src/sobre_cpp/a_estrela/ponte_de_ligacao_entre_python_e.cpp)
      * Código responsável por prover ferramentos de conexão do ambiente Python com funções C++.
    
    * [Makefile](src/sobre_cpp/a_estrela/Makefile)
      * Responsável por providenciar o build do módulo. Não ironicamente é o segundo
      arquivo mais importante.
      
* [preditor_de_curva_da_bola](src/sobre_cpp/preditor_de_curva_da_bola)
  * Módulo C++ focado em previsibilidade de oportunidades com a bola.
  
    * [preditor_da_curva_da_bola.h](src/sobre_cpp/preditor_de_curva_da_bola/preditor_de_curva_da_bola.h)
      * Declaração base de ponteiros e vetores que serão amplamente utilizados.
      
    * [preditor_da_curva_da_bola.cpp](src/sobre_cpp/preditor_de_curva_da_bola/preditor_de_curva_da_bola.cpp)
      * Implementação de funções de previsão de interseção e de atributos cinemáticos.
      
    * [debug_main.cc](src/sobre_cpp/preditor_de_curva_da_bola/debug_main.cc)
      * Código para teste de funções implementadas.
  
    * [ponte_de_ligacao_entre_python_e.cpp](src/sobre_cpp/a_estrela/ponte_de_ligacao_entre_python_e.cpp)
      * Código responsável por prover ferramentos de conexão do ambiente Python com funções C++.

* [ambientacao](src/sobre_cpp/ambientacao)
  * Módulo responsável por ferramentas de localização e de visualização de mundo.
    * [Singular.h](src/sobre_cpp/ambientacao/Singular.h)
      * Criação e Implementação de Classe Abstrata que permitirá instâncias únicas de qualquer classe.
      * Um show de genialidade.
  
    * [Geometria.h](src/sobre_cpp/ambientacao/Geometria.h)
      * Criação e Implementação de Classes Vetoriais de 2 Dimensões e de 3 Dimensões.
 
    * [Matriz.h](src/sobre_cpp/ambientacao/Matriz.h)
      * Criação e Implementação de Classe Matricial 4x4 que representará diversas transformações.
      * Matemática mais profunda e divertida.
    
    * [Linha.h](src/sobre_cpp/ambientacao/Linha.h)
      * Criação e Implementação de Classe de Linha, representando retas e segmentos de reta.
      * Um show de Álgebra Linear, bem complexo matematicamente.


