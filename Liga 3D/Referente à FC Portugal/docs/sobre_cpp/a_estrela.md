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

#  Highlights

## Problema de Performance

Ao realizar testes de valor e de tempo das respectivas funções principais, vulgo `estrela()` e `a_star()`,
obtive uma surpresa.

Apesar de estar obtendo valores iguais de resposta, os resultados temporais estavam sendo **significativamente diferentes**.

<div align="center">
<img src="https://github.com/user-attachments/assets/e178be56-2bc3-4984-8c62-94ef8c0a75d9" width="1300"/>
</div>

Observe que em meu código, referente ao terminal `a_estrela`, o tempo médio de cada execução é aproximadamente
o **dobro** do tempo para as mesmas execuções do código original, referente ao temrinal `a_star`.

A partir dessa informação de perfomance, busquei comparar linha a linha dos códigos. Até que descobri o motivo:

* Em meu código, por melhores práticas, achei necessário definir os construtores para a struct Node, 
entretanto, apesar de ser uma boa prática e não influenciar o resultado final, a perfomance foi seriamente
prejudicada.

Após comentar toda a parte referente aos construtores, pude verificar que:

<div align="center">
<img src="https://github.com/user-attachments/assets/3af0851c-c24c-47ba-903f-7eb535fd1d3b" width="1300"/>
</div>

Agora, os códigos geram a mesma resposta com a mesma perfomance. :ok:

## Consequências da solução

No arquivo `debug.cc`, há uma gama de funções que estão comentadas. Ambas somente funcionarão 
caso você, obviamente as descomente, e descomente os construtores presentes dentro da definição
da struct `Node`.

Como estarei deixando o padrão dos construtores comentados, deixarei apenas um teste referente
à função principal `a_estrela()`.
