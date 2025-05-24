# Dissertação Sobre Funcionalidades Individuais e Hierarquia

A seguir, uma breve explicação organizada dos arquivos e pastas do código.

Se você deseja reconstruir ou aprender do zero a aplicação, sugiro seguir a ordem
de apresentação e explicação.

## [sobre_cpp](sobre_cpp)

Pasta responsável por gerenciar as funcionalidades mais pesadas e/ou mais frequentemente
usadas, todas construídas em C++ com portabilidade para serem utilizadas no Python.

Os módulos são independentes entre si e cada um é fundamental para o funcionamento da aplicação:

* [preditor_de_curva_da_bola](sobre_cpp/preditor_de_curva_da_bola.md)
* [a_estrela](sobre_cpp/a_estrela.md)
* [ambientacao](sobre_cpp/ambientacao.md)

Dentro de cada, há 3 arquivos essenciais para a portabilidade.

* debug_main.cc
  
Arquivo específico para realização de testes das funções implementadas no módulo.

* module_main.cpp

Arquivo específico **responsável pela portabilidade das aplicações para o Python**.

* Makefile 

Arquivo de build do módulo cpp, o qual gerará um arquivo _.so_.

Estes módulos são compilados e construídos a partir de um método específico da classe 
Script.











